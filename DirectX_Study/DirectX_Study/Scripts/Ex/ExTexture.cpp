#include "ExTexture.h"

DK::ExTexture::ExTexture(HWND hWnd) : GraphicEngine(hWnd)
{
}

DK::ExTexture::~ExTexture()
{
}

bool DK::ExTexture::Init()
{
	if (GraphicEngine::Init() == false)
		return false;

	THROW_IF_FAILED(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

	m_camera.GetTransform().SetPosition(0, 0, -10);

	CreateGeometry();	// mesh, buffer 积己
	CreateMaterial();	// material 积己
	CreateGameObject();	// 免仿 go 积己
	CreateFrameResource();	// const buffer 殿 积己

	BuildInputLayoutAndShader();
	BuildRootSignature();
	BuildPSO();

	THROW_IF_FAILED(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get()};
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	FlushCommandQueue();

	return true;
}

void DK::ExTexture::CreateGeometry()
{
	GeometryGenerator geoGen;
	std::unique_ptr<MeshBuffer> meshBuffer = std::make_unique<MeshBuffer>();

	// create mesh data
	MeshData meshBox = geoGen.CreateBox(1, 1, 1);
	meshBox.Name = "box";

	// build mesh data
	meshBuffer->AddMeshData(meshBox.Name, meshBox);
	meshBuffer->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());

	// add buffer
	m_vecMeshBuffers.push_back(std::move(meshBuffer));
}

void DK::ExTexture::CreateMaterial()
{
	std::unique_ptr<Material> matWood = std::make_unique<Material>();
	matWood->Name = "wood";
	matWood->MatCBIndex = 0;
	matWood->DiffuseSrvHeapIndex = 0;
	matWood->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	matWood->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	matWood->Roughness = 0.2f;

	m_mapMaterials[matWood->Name] = std::move(matWood);
}

void DK::ExTexture::CreateGameObject()
{
	GameObject goBox;

	// set mesh
	MeshBuffer* pMeshBuffer = m_vecMeshBuffers[0].get();
	MeshSection section;

	if (pMeshBuffer->GetMeshSection("box", section))
		goBox.SetMeshData(pMeshBuffer, section);

	// set material
	goBox.SetMaterial(m_mapMaterials["wood"].get());

	goBox.GetTransform().SetScale(3, 3, 3);

	m_vecGameObjects.push_back(goBox);
}

void DK::ExTexture::CreateFrameResource()
{
	for (int i = 0; i < m_nFrameReesourceCount; ++i)
	{
		m_vecFrameResoruce.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, m_vecGameObjects.size(), m_mapMaterials.size(), 1));
	}
}

void DK::ExTexture::BuildDescriptor()
{
	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_spHeapSRV));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_spHeapSRV->GetCPUDescriptorHandleForHeapStart());

	// todo
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	////srvDesc.Format = ;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.Texture2D.MostDetailedMip = 0;
	////srvDesc.Texture2D.MipLevels = ;
	//srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//m_d3dDevice->CreateShaderResourceView()
}

void DK::ExTexture::BuildInputLayoutAndShader()
{
	m_mapShaders["VS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_mapShaders["PS"] = D3DUtils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_vecInputLayout = Vertex::GetInputLayout();
}

void DK::ExTexture::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[4];
	rootParameters[0].InitAsDescriptorTable(1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsConstantBufferView(0);
	rootParameters[2].InitAsConstantBufferView(1);
	rootParameters[3].InitAsConstantBufferView(2);

	// todo- static sampler object binding

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, 
		0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> spSerializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> spError = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1, 
		spSerializedRootSig.GetAddressOf(), 
		spError.GetAddressOf());

	if (spError != nullptr)
	{
		::OutputDebugStringA((char*)spError->GetBufferPointer());
	}
	THROW_IF_FAILED(hr);

	m_d3dDevice->CreateRootSignature(
		0, 
		spSerializedRootSig->GetBufferPointer(), 
		spSerializedRootSig->GetBufferSize(), 
		IID_PPV_ARGS(&m_spRootSignature));
}

void DK::ExTexture::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.pRootSignature = m_spRootSignature.Get();
	psoDesc.VS = 
	{
		m_mapShaders["VS"]->GetBufferPointer(),
		m_mapShaders["VS"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		m_mapShaders["PS"]->GetBufferPointer(),
		m_mapShaders["PS"]->GetBufferSize()
	};
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = { m_vecInputLayout.data(), (UINT)m_vecInputLayout.size() };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_eBackBufferFormat;
	psoDesc.DSVFormat = m_eDepthStencilFormat;
	psoDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;

	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_spPSO.GetAddressOf()));
}

bool DK::ExTexture::OnResize(int width, int height, bool force)
{
	if (GraphicEngine::OnResize(width, height, force) == false)
		return false;

	return true;
}

bool DK::ExTexture::Update()
{
	if (GraphicEngine::Update() == false)
		return false;

	m_nCurrFrameResourceIndex = (m_nCurrFrameResourceIndex + 1) % m_nFrameReesourceCount;
	m_pCurrFrameResource = m_vecFrameResoruce[m_nCurrFrameResourceIndex].get();
	if (m_pCurrFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_pCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_pCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateRenderPassCB();
	UpdateObjectCBs();
	UpdateMaterialCBs();

	return true;
}

void DK::ExTexture::UpdateRenderPassCB()
{
	DirectX::XMFLOAT4X4 viewMatrix = m_camera.GetViewMatrix();
	DirectX::XMFLOAT4X4 projMatrix = m_camera.GetProjMatrix();

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
	m_renderPassCB.NearZ = m_camera.GetNear();
	m_renderPassCB.FarZ = m_camera.GetFar();
	m_renderPassCB.TotalTime = m_gameTimer.TotalTime();
	m_renderPassCB.DeltaTime = m_gameTimer.DeltaTime();

	m_renderPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	float mSunTheta = 1.25f * DirectX::XM_PI;
	float mSunPhi = DirectX::XM_PIDIV4;
	DirectX::XMVECTOR lightDir = MathUtils::SphericalToCartesian(1.0f, mSunTheta, mSunPhi);
	lightDir = DirectX::XMVectorScale(lightDir, -1.0f);

	XMStoreFloat3(&m_renderPassCB.Lights[0].Direction, lightDir);
	m_renderPassCB.Lights[0].Strength = { 1.0f, 0.0f, 0.0f };

	auto currPassCB = m_pCurrFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::ExTexture::UpdateObjectCBs()
{
	auto objectCB = m_pCurrFrameResource->ObjectCB.get();

	int objCount = m_vecGameObjects.size();
	for (int i = 0; i < objCount; ++i)
	{
		ObjectConstants constants;
		constants.WorldMatrix = m_vecGameObjects[i].GetTransform().GetWorldMatrix();

		objectCB->CopyData(i, constants);
	}
}

void DK::ExTexture::UpdateMaterialCBs()
{
	auto meterialCB = m_pCurrFrameResource->MaterialCB.get();

	for (auto& data : m_mapMaterials)
	{
		Material* mat = data.second.get();

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
		matConstants.FresnelR0 = mat->FresnelR0;
		matConstants.Roughness = mat->Roughness;

		meterialCB->CopyData(mat->MatCBIndex, matConstants);
	}
}

bool DK::ExTexture::Render()
{
	auto cmdListAlloc = m_pCurrFrameResource->CmdListAlloc;

	cmdListAlloc->Reset();
	m_commandList->Reset(cmdListAlloc.Get(), m_spPSO.Get());

	m_commandList->RSSetViewports(1, &m_viewPortScreen);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	CD3DX12_RESOURCE_BARRIER renderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &renderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE backBuffer = CurrentBackBufferView();
	m_commandList->ClearRenderTargetView(backBuffer, DirectX::Colors::DimGray, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencil = DepthStencilView();
	m_commandList->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_commandList->OMSetRenderTargets(1, &backBuffer, true, &depthStencil);

	m_commandList->SetGraphicsRootSignature(m_spRootSignature.Get());

	auto passCB = m_pCurrFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	DrawGameObjects(m_commandList.Get());

	CD3DX12_RESOURCE_BARRIER present = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &present);

	m_commandList->Close();

	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	m_swapChain->Present(0, 0);
	m_nCurrBackBuffer = (m_nCurrBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	m_pCurrFrameResource->Fence = ++m_ullCurrentFence;
	m_commandQueue->Signal(m_fence.Get(), m_ullCurrentFence);

	return true;
}

void DK::ExTexture::DrawGameObjects(ID3D12GraphicsCommandList* cmdList)
{
	UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();

	for (size_t i = 0; i < m_vecGameObjects.size(); ++i)
	{
		GameObject go = m_vecGameObjects[i];

		auto viewVB = go.GetMeshBuffer()->VertexBufferView();
		auto viewIB = go.GetMeshBuffer()->IndexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &viewVB);
		cmdList->IASetIndexBuffer(&viewIB);
		cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * i);

		int matCBIndex = go.GetMaterial()->MatCBIndex;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBIndex * matCBByteSize);

		// todo- srv heap table 眠啊

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		MeshSection meshSection = go.GetMeshSection();
		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
	}
}
