#include "ExShape.h"

using namespace DK;

ExShape::ExShape(HWND hWnd) : GraphicEngine(hWnd)
{
}

ExShape::~ExShape()
{
}

void ExShape::Init()
{
	m_camera.GetTransform().SetPosition(0, 0, 10.0f);

	CreateGeometry();
	CreateMaterials();
	BuildRenderObject();
	BuildShadersAndInputLayout();
	BuildFrameResources();
	BuildRootSignature();
	BuildPSOs();
}

bool ExShape::OnResize(int width, int height, bool force)
{
	if (GraphicEngine::OnResize(width, height, force) == false)
		return false;

	return true;
}

bool DK::ExShape::Update()
{
	if (GraphicEngine::Update() == false)
		return false;

	// Cycle through the circular frame resource array.
	m_nCurrFrameResourceIndex = (m_nCurrFrameResourceIndex + 1) % g_NumFrameResources;
	m_pCurrFrameResource = m_frameResources[m_nCurrFrameResourceIndex].get();
	
	if (m_pCurrFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_pCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_pCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateWave(m_gameTimer);
	UpdateRenderPassCB(m_gameTimer);
	UpdateObjectCBs(m_gameTimer);
	UpdateMaterialCB(m_gameTimer);

	return true;
}

bool DK::ExShape::Render()
{
	auto cmdListAlloc = m_pCurrFrameResource->CmdListAlloc;
	THROW_IF_FAILED(cmdListAlloc->Reset());
	THROW_IF_FAILED(m_commandList->Reset(cmdListAlloc.Get(), m_pso.Get()));

	m_commandList->RSSetViewports(1, &m_viewPortScreen);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	CD3DX12_RESOURCE_BARRIER renderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &renderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE backBuffer = CurrentBackBufferView();
	m_commandList->ClearRenderTargetView(backBuffer, DirectX::Colors::LightSteelBlue, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencil = DepthStencilView();
	m_commandList->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_commandList->OMSetRenderTargets(1, &backBuffer, true, &depthStencil);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	auto passCB = m_pCurrFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawRenderItems(m_commandList.Get(), m_vecRenderObject);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER present = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &present);

	// Done recording commands.
	THROW_IF_FAILED(m_commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	THROW_IF_FAILED(m_swapChain->Present(0, 0));
	m_nCurrBackBuffer = (m_nCurrBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	// Advance the fence value to mark commands up to this fence point.
	m_pCurrFrameResource->Fence = ++m_ullCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_commandQueue->Signal(m_fence.Get(), m_ullCurrentFence);

	return true;
}

void DK::ExShape::UpdateWave(const GameTimer& gt)
{
	//// Every quarter second, generate a random wave.
	//static float t_base = 0.0f;
	//if ((m_gameTimer.TotalTime() - t_base) >= 0.25f)
	//{
	//	t_base += 0.25f;

	//	int i = MathUtils::Rand(4, m_waves->RowCount() - 5);
	//	int j = MathUtils::Rand(4, m_waves->ColumnCount() - 5);

	//	float r = MathUtils::RandF(0.2f, 0.5f);

	//	m_waves->Disturb(i, j, r);
	//}

	//// Update the wave simulation.
	//m_waves->Update(gt.DeltaTime());

	//// Update the wave vertex buffer with the new solution.
	//auto currWavesVB = m_pCurrFrameResource->WaveVB.get();
	//for (int i = 0; i < m_waves->VertexCount(); ++i)
	//{
	//	Vertex v;
	//	v.Position = m_waves->Position(i);
	//	v.Normal = m_waves->Normal(i);

	//	currWavesVB->CopyData(i, v);
	//}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
//	m_pGOWave->GetMeshBuffer()->VertexBuffer = currWavesVB->GetBuffer();
}

void DK::ExShape::UpdateObjectCBs(const GameTimer& gt)
{
	//auto currObjectCB = m_pCurrFrameResource->ObjectCB.get();
	//int i = 0;
	//for (auto& ro : m_vecRenderObject)
	//{
	//	if (ro.NumFramesDirty > 0)
	//	{
	//		/*DirectX::XMFLOAT4X4 world = ro.pGameObject->GetTransform().GetWorldMatrix();
	//		DirectX::XMMATRIX worldMatrix = XMLoadFloat4x4(&world);

	//		ObjectConstants objConstants;
	//		XMStoreFloat4x4(&objConstants.WorldMatrix, XMMatrixTranspose(worldMatrix));

	//		currObjectCB->CopyData(i, objConstants);

	//		ro.NumFramesDirty -= 1;
	//		++i;*/
	//	}
	//}
}

void DK::ExShape::UpdateRenderPassCB(const GameTimer& gt)
{
	DirectX::XMFLOAT4X4 viewMatrix = m_camera.GetViewMatrixf4();
	DirectX::XMFLOAT4X4 projMatrix = m_camera.GetProjMatrixf4();

	DirectX::XMMATRIX view = XMLoadFloat4x4(&viewMatrix);
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&projMatrix);
	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	DirectX::XMVECTOR viewDetermin = XMMatrixDeterminant(view);
	DirectX::XMMATRIX invView = XMMatrixInverse(&viewDetermin, view);

	DirectX::XMVECTOR projDetermin = XMMatrixDeterminant(proj);
	DirectX::XMMATRIX invProj = XMMatrixInverse(&projDetermin, proj);

	DirectX::XMVECTOR viewprojDetermin = XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&viewprojDetermin, viewProj);

	XMStoreFloat4x4(&m_renderPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_renderPassCB.InvView, XMMatrixTranspose(invView));

	XMStoreFloat4x4(&m_renderPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_renderPassCB.InvProj, XMMatrixTranspose(invProj));

	XMStoreFloat4x4(&m_renderPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_renderPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	m_renderPassCB.EyePosW = m_camera.GetTransform().GetPosition();
	m_renderPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)m_nClientWidth, (float)m_nClientHeight);
	m_renderPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_nClientWidth, 1.0f / m_nClientHeight);
	m_renderPassCB.NearZ = 1.0f;
	m_renderPassCB.FarZ = 1000.0f;
	m_renderPassCB.TotalTime = gt.TotalTime();
	m_renderPassCB.DeltaTime = gt.DeltaTime();

	m_renderPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	float mSunTheta = 1.25f * DirectX::XM_PI;
	float mSunPhi = DirectX::XM_PIDIV4;
	DirectX::XMVECTOR lightDir = MathUtils::SphericalToCartesian(1.0f, mSunTheta, mSunPhi);
	lightDir = DirectX::XMVectorScale(lightDir, -1.0f);

	//XMStoreFloat3(&m_renderPassCB.Lights[0].Direction, lightDir);

	//m_renderPassCB.Lights[0].Strength = { 1.0f, 0.0f, 0.0f };

	Light spotLight;
	spotLight.Strength = { 1.0f, 1.0f, 1.0f };
	spotLight.FalloffStart = 10;
	spotLight.FalloffEnd = 20;
	spotLight.Position = { 10.0f, 15.0f, 0.0f };

	Light pointLight;
	pointLight.Strength = { 1.0f, 1.0f, 0.9f };
	pointLight.FalloffStart = 15;
	pointLight.FalloffEnd = 20;

	/*for (int i = 0; i < 5; ++i)
		m_renderPassCB.Lights[i] = pointLight;

	m_renderPassCB.Lights[0].Position = { 0.0f, 10.0f, 0.0f };
	m_renderPassCB.Lights[1].Position = { 30.0f, 10.0f, 30.0f };
	m_renderPassCB.Lights[2].Position = { -30.0f, 10.0f, -30.0f };
	m_renderPassCB.Lights[3].Position = { -30.0f, 10.0f, 30.0f };
	m_renderPassCB.Lights[4].Position = { 30.0f, 10.0f, -30.0f };*/

	m_renderPassCB.Lights[0] = spotLight;

	auto currPassCB = m_pCurrFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::ExShape::UpdateMaterialCB(const GameTimer& gt)
{
	//auto currMaterialCB = m_pCurrFrameResource->MaterialCB.get();
	//for (auto& data : m_materials)
	//{
	//	// Only update the cbuffer data if the constants have changed.  If the cbuffer
	//	// data changes, it needs to be updated for each FrameResource.
	//	Material* mat = data.second.get();
	//	if (mat->NumFramesDirty > 0)
	//	{
	//		DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

	//		MaterialConstants matConstants;
	//		matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
	//		matConstants.FresnelR0 = mat->FresnelR0;
	//		matConstants.Roughness = mat->Roughness;

	//		currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

	//		// Next FrameResource need to be updated too.
	//		mat->NumFramesDirty--;
	//	}
	//}
}

void DK::ExShape::CreateGeometry()
{
	// ** land geometry
	GeometryGenerator geoGen;
	MeshData<Vertex> gridMeshData = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	// grid vertex 추가
	std::vector<Vertex> vecVertex;
	for (int i = 0; i < gridMeshData.Vertices.size(); ++i)
	{
		Vertex vertex = gridMeshData.Vertices[i];

		DirectX::XMFLOAT3 pos = gridMeshData.Vertices[i].Position;
		pos.y = 0.3f * (pos.z * sinf(0.1f * pos.x) + pos.x * cosf(0.1f * pos.z));

		vertex.Position = pos;
		vertex.Normal = GetHillNormal(vertex.Position.x, vertex.Position.z);

		vecVertex.push_back(vertex);
	}

	std::string meshName = "land";
	auto meshLand = std::make_unique<MeshBuffer<Vertex>>();
	meshLand->AddMeshData(meshName, vecVertex, gridMeshData.GetIndices16());
	meshLand->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());

	m_meshBuffers[meshName] = std::move(meshLand);


	// ** wave geometry
	m_waves = std::make_unique<Wave>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	std::vector<std::uint16_t> indices(3 * m_waves->TriangleCount());
	assert(m_waves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = m_waves->RowCount();
	int n = m_waves->ColumnCount();
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

	UINT vbByteSize = m_waves->VertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto meshWave = std::make_unique<MeshBuffer<Vertex>>();

	meshWave->IndexBuffer = D3DUtils::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, meshWave->IndexUploadBuffer);

	meshWave->VertexByteStride = sizeof(Vertex);
	meshWave->VertexBufferByteSize = vbByteSize;
	meshWave->IndexBufferByteSize = ibByteSize;
	meshWave->IndexFormat = DXGI_FORMAT_R16_UINT;

	MeshSection section;
	section.IndexCount = (UINT)indices.size();
	section.StartIndexLocation = 0;
	section.BaseVertexLocation = 0;

	meshName = "wave";
	meshWave->MeshSections[meshName] = section;
	m_meshBuffers[meshName] = std::move(meshWave);
}

void DK::ExShape::CreateMaterials()
{
	std::unique_ptr<Material> matGrass = std::make_unique<Material>();
	matGrass->Name = "grass";
	matGrass->SrvHeapIndex = 0;
	matGrass->DiffuseAlbedo = DirectX::XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	matGrass->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	matGrass->Roughness = 0.125f;
	matGrass->DirtyCount = g_NumFrameResources;

	std::unique_ptr<Material> matWater = std::make_unique<Material>();
	matWater->Name = "water";
	matWater->SrvHeapIndex = 1;
	matWater->DiffuseAlbedo = DirectX::XMFLOAT4(0.0f, 0.2f, 0.6f, 1.0f);
	matWater->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
	matWater->Roughness = 0.0f;
	matWater->DirtyCount = g_NumFrameResources;

	m_materials[matGrass->Name] = std::move(matGrass);
	m_materials[matWater->Name] = std::move(matWater);
}

void DK::ExShape::BuildRenderObject()
{
	std::unique_ptr<GameObject> objGrid = std::make_unique<GameObject>();

	MeshBuffer<Vertex>* meshLand = m_meshBuffers["land"].get();
	MeshSection section;
	if (meshLand->GetMeshSection("land", section))
	{
		/*objGrid->SetMeshData(meshLand, section);
		objGrid->SetMaterial(m_materials["grass"].get());

		m_gameObjects.push_back(std::move(objGrid));*/
	}

	std::unique_ptr<GameObject> objWave = std::make_unique<GameObject>();

	MeshBuffer<Vertex>* meshWave = m_meshBuffers["wave"].get();
	if (meshWave->GetMeshSection("wave", section))
	{
		/*objWave->SetMeshData(meshWave, section);
		objWave->SetMaterial(m_materials["water"].get());

		m_pGOWave = objWave.get();
		m_gameObjects.push_back(std::move(objWave));*/
	}

	for (auto& go : m_gameObjects)
	{
		RenderObject renderObject;
		renderObject.pGameObject = go.get();

		m_vecRenderObject.push_back(renderObject);
	}
}

void DK::ExShape::BuildShadersAndInputLayout()
{
	m_shaderByteCode["standardVS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_shaderByteCode["opaquePS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout = Vertex::GetInputLayout();
}

void DK::ExShape::BuildFrameResources()
{
	/*for (int i = 0; i < g_NumFrameResources; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(), 
			1, (UINT)m_gameObjects.size(), (UINT)m_materials.size(), m_waves->VertexCount()));
	}*/
}

void DK::ExShape::BuildRootSignature()
{
	// const buffer view 3개를 바로 사용할 것이다.	
	CD3DX12_ROOT_PARAMETER rootParameter[3];

	// passVB, objectCB, materialCB
	rootParameter[0].InitAsConstantBufferView(0);
	rootParameter[1].InitAsConstantBufferView(1);
	rootParameter[2].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignature(3, rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignature, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSignature.GetAddressOf(), error.GetAddressOf());

	if (error != nullptr)
	{
		::OutputDebugStringA((char*)error->GetBufferPointer());
	}
	THROW_IF_FAILED(hr);

	THROW_IF_FAILED(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
}

void DK::ExShape::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaderByteCode["standardVS"]->GetBufferPointer()),
		m_shaderByteCode["standardVS"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaderByteCode["opaquePS"]->GetBufferPointer()),
		m_shaderByteCode["opaquePS"]->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_eBackBufferFormat;
	psoDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_eDepthStencilFormat;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}

void DK::ExShape::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderObject>& renderObjects)
{
	/*UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();*/

	// For each render item...
	for (size_t i = 0; i < renderObjects.size(); ++i)
	{
		/*RenderObject renderObject = renderObjects[i];

		D3D12_VERTEX_BUFFER_VIEW vbView = renderObject.pGameObject->GetMeshBuffer()->VertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibView = renderObject.pGameObject->GetMeshBuffer()->IndexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &vbView);
		cmdList->IASetIndexBuffer(&ibView);
		cmdList->IASetPrimitiveTopology(renderObject.PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * i);

		int matCBIndex = renderObject.pGameObject->GetMaterial()->MatCBIndex;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBIndex * matCBByteSize);

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		MeshSection meshSection = renderObject.pGameObject->GetMeshSection();
		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);*/
	}
}

DirectX::XMFLOAT3 DK::ExShape::GetHillNormal(float x, float z)
{
	// n = (-df/dx, 1, -df/dz)
	DirectX::XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

	DirectX::XMVECTOR unitNormal = DirectX::XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}
