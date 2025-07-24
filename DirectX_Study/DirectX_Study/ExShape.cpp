#include "ExShape.h"

DK::ExShape::ExShape(HWND hWnd) : GraphicEngine(hWnd)
{
}

DK::ExShape::~ExShape()
{
}

bool DK::ExShape::Init()
{
	if (GraphicEngine::Init() == false)
		return false;

	THROW_IF_FAILED(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mWaves = std::make_unique<Wave>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	camera.SetPosition(0, 0, -10);

	BuildShapeGeometry();	// vertex, index의 default buffer 생성
	BuildWavesGeometryBuffers();
	BuildRenderItems();
	BuildShadersAndInputLayout();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildConstantBufferViews();
	BuildRootSignature();
	BuildPSOs();

	THROW_IF_FAILED(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return false;
}

void DK::ExShape::OnResize(int width, int height)
{
	GraphicEngine::OnResize(width, height);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void DK::ExShape::Update()
{
	UpdateCamera(mTimer);

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs(mTimer);
	UpdateMainPassCB(mTimer);
	UpdateWave(mTimer);
}

void DK::ExShape::Render()
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
	THROW_IF_FAILED(cmdListAlloc->Reset());
	THROW_IF_FAILED(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.

	CD3DX12_RESOURCE_BARRIER renderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &renderTarget);

	// Clear the back buffer and depth buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE backBuffer = CurrentBackBufferView();
	mCommandList->ClearRenderTargetView(backBuffer, DirectX::Colors::LightSteelBlue, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencil = DepthStencilView();
	mCommandList->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &backBuffer, true, &depthStencil);

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	int passCbvIndex = mPassCbvOffset + mCurrFrameResourceIndex;
	auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

	DrawRenderItems(mCommandList.Get(), mRenderObjects);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER present = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &present);

	// Done recording commands.
	THROW_IF_FAILED(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	THROW_IF_FAILED(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void DK::ExShape::UpdateCamera(const GameTimer& gt)
{
	bool bRButtonState = GetRButtonDown();
	if (bRButtonClicked == false && bRButtonState)
	{
		bRButtonClicked = true;
		GetCursorPos(&mPreMousePoint);
	}
	else if (bRButtonClicked && bRButtonState == false)
	{
		bRButtonClicked = false;
	}

	if (bRButtonState)
	{
		// 회전
		POINT curMousePoint;
		GetCursorPos(&curMousePoint);

		DirectX::XMFLOAT3 camRot = camera.GetRotation();
		camRot.x += static_cast<float>(curMousePoint.y - mPreMousePoint.y) * 0.3f;
		camRot.y += static_cast<float>(curMousePoint.x - mPreMousePoint.x) * 0.3f;

		camera.SetRotation(camRot);

		mPreMousePoint = curMousePoint;
	}

	// camera 이동
	DirectX::XMFLOAT3 dirRight = camera.Right();
	DirectX::XMFLOAT3 dirFront = camera.Front();
	DirectX::XMFLOAT3 dirUp = camera.Up();

	DirectX::XMFLOAT3 dirMove = MathUtils::AddFloat3ToFloat3(
		MathUtils::MultiplyValueToFloat3(dirRight, GetXMoveInput()), 
		MathUtils::MultiplyValueToFloat3(dirFront, GetZMoveInput()));
	dirMove = MathUtils::AddFloat3ToFloat3(dirMove, MathUtils::MultiplyValueToFloat3(dirUp, GetYMoveInput()));

	DirectX::XMFLOAT3 cameraPos = camera.GetPosition();

	cameraPos = MathUtils::AddFloat3ToFloat3(cameraPos, MathUtils::MultiplyValueToFloat3(dirMove, 0.1f));
	camera.SetPosition(cameraPos);

	DirectX::XMFLOAT3 targetPos = MathUtils::AddFloat3ToFloat3(cameraPos, MathUtils::MultiplyValueToFloat3(dirFront, 100.0f));

	// Build the view matrix.
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMLoadFloat3(&targetPos);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void DK::ExShape::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	int i = 0;
	for (auto& ro : mRenderObjects)
	{
		if (ro.NumFramesDirty > 0)
		{
			DirectX::XMFLOAT4X4 world = ro.pRenderObject->GetTransform().GetMatrixWorld();
			DirectX::XMMATRIX worldMatrix = XMLoadFloat4x4(&world);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(worldMatrix));

			currObjectCB->CopyData(i, objConstants);

			// Next FrameResource need to be updated too.
			ro.NumFramesDirty -= 1;
			++i;
		}
	}
}

void DK::ExShape::UpdateMainPassCB(const GameTimer& gt)
{
	DirectX::XMMATRIX view = XMLoadFloat4x4(&mView);
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&mProj);

	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	DirectX::XMVECTOR viewDetermin = XMMatrixDeterminant(view);
	DirectX::XMMATRIX invView = XMMatrixInverse(&viewDetermin, view);

	DirectX::XMVECTOR projDetermin = XMMatrixDeterminant(proj);
	DirectX::XMMATRIX invProj = XMMatrixInverse(&projDetermin, proj);

	DirectX::XMVECTOR viewprojDetermin = XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&viewprojDetermin, viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void DK::ExShape::UpdateWave(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathUtils::Rand(4, mWaves->RowCount() - 5);
		int j = MathUtils::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathUtils::RandF(0.2f, 0.5f);

		mWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = mCurrFrameResource->WavesVB.get();
	for (int i = 0; i < mWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = mWaves->Position(i);
		v.Color = DirectX::XMFLOAT4(DirectX::Colors::Blue);

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	mpWaveGameObject->GetMeshRenderData()->VertexBuffer = currWavesVB->GetResource();
}

void DK::ExShape::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	 
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	THROW_IF_FAILED(hr);

	md3dDevice->CreateRootSignature(
		0, 
		serializedRootSig->GetBufferPointer(), 
		serializedRootSig->GetBufferSize(), 
		IID_PPV_ARGS(&mRootSignature));
}

void DK::ExShape::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void DK::ExShape::BuildShapeGeometry()
{
	// 각 구조의 정점과 인덱스 데이터를 생성하였다.
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//mMeshBuffer.AddMeshData("box", box, DirectX::XMFLOAT4(DirectX::Colors::DarkGreen));

	// grid vertex 추가
	std::vector<Vertex> gridVertex;
	for (int i = 0; i < grid.Vertices.size(); ++i)
	{
		DirectX::XMFLOAT3 pos = grid.Vertices[i].Position;
		pos.y = 0.3f * (pos.z * sinf(0.1f * pos.x) + pos.x * cosf(0.1f * pos.z));

		Vertex vertex;
		vertex.Pos = pos;

		DirectX::XMFLOAT4 color;
		if (pos.y < -10.0f)
		{
			// Sandy beach color.
			color = DirectX::XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (pos.y < 5.0f)
		{
			// Light yellow-green.
			color = DirectX::XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (pos.y < 12.0f)
		{
			// Dark yellow-green.
			color = DirectX::XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (pos.y < 20.0f)
		{
			// Dark brown.
			color = DirectX::XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		vertex.Color = color;

		gridVertex.push_back(vertex);
	}
	mMeshRenderData.AddVertexData("grid", gridVertex, grid.GetIndices16());
	
	//mMeshBuffer.AddMeshData("sphere", sphere, DirectX::XMFLOAT4(DirectX::Colors::Crimson));
	//mMeshBuffer.AddMeshData("cylinder", cylinder, DirectX::XMFLOAT4(DirectX::Colors::SteelBlue));

	// ** build
	mMeshRenderData.BuildMeshRenderData(md3dDevice.Get(), mCommandList.Get());
}

void DK::ExShape::BuildWavesGeometryBuffers()
{
	std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = mWaves->RowCount();
	int n = mWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = mWaves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto meshRenderData = std::make_unique<MeshRenderData>();

	meshRenderData->IndexBuffer = D3DUtils::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, meshRenderData->IndexUploadBuffer);

	meshRenderData->VertexByteStride = sizeof(Vertex);
	meshRenderData->VertexBufferByteSize = vbByteSize;
	meshRenderData->IndexBufferByteSize = ibByteSize;
	meshRenderData->IndexFormat = DXGI_FORMAT_R16_UINT;

	MeshSection section;
	section.IndexCount = (UINT)indices.size();
	section.StartIndexLocation = 0;
	section.BaseVertexLocation = 0;

	meshRenderData->MeshSections["wave"] = section;

	mWaveMeshData = std::move(meshRenderData);
}

void DK::ExShape::BuildRenderItems()
{
	/*std::unique_ptr<GameObject> objBox = std::make_unique<GameObject>();
	objBox->mTransform.SetPosition(DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f));
	objBox->mTransform.SetScale(DirectX::XMFLOAT3(2.0f, 2.0f, 2.0f));
	objBox->mMeshData = mMeshBuffer.GetMeshDataDesc("box");
	mGameObjects.push_back(std::move(objBox));*/

	std::unique_ptr<GameObject> objGrid = std::make_unique<GameObject>();
	MeshSection section;
	if (mMeshRenderData.GetMeshSection("grid", section))
	{
		objGrid->SetMeshData(&mMeshRenderData, section);
		mGameObjects.push_back(std::move(objGrid));
	}

	/*for (int i = 0; i < 5; ++i)
	{
		std::unique_ptr<GameObject> objCylinder1 = std::make_unique<GameObject>();
		objCylinder1->mTransform.SetPosition(-5.0f, 1.5f, -10.0f + i * 5.0f);
		objCylinder1->mMeshData = mMeshBuffer.GetMeshDataDesc("cylinder");
		mGameObjects.push_back(std::move(objCylinder1));

		std::unique_ptr<GameObject> objCylinder2 = std::make_unique<GameObject>();
		objCylinder2->mTransform.SetPosition(5.0f, 1.5f, -10.0f + i * 5.0f);
		objCylinder2->mMeshData = mMeshBuffer.GetMeshDataDesc("cylinder");
		mGameObjects.push_back(std::move(objCylinder2));

		std::unique_ptr<GameObject> objSphere1 = std::make_unique<GameObject>();
		objSphere1->mTransform.SetPosition(-5.0f, 3.5f, -10.0f + i * 5.0f);
		objSphere1->mMeshData = mMeshBuffer.GetMeshDataDesc("sphere");
		mGameObjects.push_back(std::move(objSphere1));

		std::unique_ptr<GameObject> objSphere2 = std::make_unique<GameObject>();
		objSphere2->mTransform.SetPosition(5.0f, 3.5f, -10.0f + i * 5.0f);
		objSphere2->mMeshData = mMeshBuffer.GetMeshDataDesc("sphere");
		mGameObjects.push_back(std::move(objSphere2));
	}*/

	std::unique_ptr<GameObject> objWave = std::make_unique<GameObject>();
	if (mWaveMeshData->GetMeshSection("wave", section))
	{
		objWave->SetMeshData(mWaveMeshData.get(), section);
		mpWaveGameObject = objWave.get();
		mGameObjects.push_back(std::move(objWave));
	}

	for (auto& object : mGameObjects)
	{
		RenderObject ro;
		ro.pRenderObject = object.get();
		mRenderObjects.push_back(ro);
	}
}

void DK::ExShape::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1, (UINT)mGameObjects.size(), mWaves->VertexCount()));
	}
}

void DK::ExShape::BuildDescriptorHeaps()
{
	UINT objCount = (UINT)mGameObjects.size();

	mPassCbvOffset = objCount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = (objCount + 1) * gNumFrameResources;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

void DK::ExShape::BuildConstantBufferViews()
{
	UINT objCount = (UINT)mGameObjects.size();
	UINT objCBByteSize = D3DUtils::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto objectCB = mFrameResources[frameIndex]->ObjectCB->GetResource();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();
			cbAddress += (i * objCBByteSize);

			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
			int offsetDescriptor = (frameIndex * objCount + i);
			handle.Offset(offsetDescriptor, mCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	UINT passCBByteSize = D3DUtils::CalcConstantBufferByteSize(sizeof(PassConstants));

	// Last three descriptors are the pass CBVs for each frame resource.
	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto passCB = mFrameResources[frameIndex]->PassCB->GetResource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		int offsetDescriptor = mPassCbvOffset + frameIndex;
		handle.Offset(offsetDescriptor, mCbvSrvUavDescriptorSize);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void DK::ExShape::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	THROW_IF_FAILED(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	// PSO for opaque wireframe objects.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	THROW_IF_FAILED(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
}

void DK::ExShape::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderObject>& renderObjects)
{
	UINT objCBByteSize = D3DUtils::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->GetResource();

	// For each render item...
	for (size_t i = 0; i < renderObjects.size(); ++i)
	{
		RenderObject renderObject = renderObjects[i];

		D3D12_VERTEX_BUFFER_VIEW vbView = renderObject.pRenderObject->GetMeshRenderData()->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibView = renderObject.pRenderObject->GetMeshRenderData()->IndexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &vbView);
		cmdList->IASetIndexBuffer(&ibView);

		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
		UINT offsetDescriptor = mCurrFrameResourceIndex * (UINT)mGameObjects.size() + i;
		cbvHandle.Offset(offsetDescriptor, mCbvSrvUavDescriptorSize);

		cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

		MeshSection meshSection = renderObject.pRenderObject->GetMeshSection();
		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
	}
}

bool DK::ExShape::GetRButtonDown()
{
	return GetAsyncKeyState(VK_RBUTTON) & 0x8000;
}

float DK::ExShape::GetXMoveInput()
{
	bool bLeft = GetKeyDown('a');
	bool bRight = GetKeyDown('d');
	if (bLeft ^ bRight)
	{
		return bLeft ? -1.0f : 1.0f;
	}
	return 0.f;
}

float DK::ExShape::GetYMoveInput()
{
	bool bDown = GetKeyDown('q');
	bool bUp = GetKeyDown('e');
	if (bDown ^ bUp)
	{
		return bDown ? -1.0f : 1.0f;
	}
	return 0.0f;
}

float DK::ExShape::GetZMoveInput()
{
	bool bFront = GetKeyDown('w');
	bool bBack = GetKeyDown('s');
	if (bFront ^ bBack)
	{
		return bFront ? 1.0f : -1.0f;
	}
	return 0.0f;
}

bool DK::ExShape::GetKeyDown(char c)
{
	if (c >= 'a' && c <= 'z')
		c -= ('a' - 'A');

	if (c >= 'A' && c <= 'Z')
		return GetAsyncKeyState(c);
	else
		return false;
}
