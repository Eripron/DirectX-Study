#include "EngineBase.h"

using namespace DK;

DK::EngineBase::EngineBase(HWND hWnd) : GraphicEngine(hWnd)
{
}

DK::EngineBase::~EngineBase()
{
}

void DK::EngineBase::Init()
{
	m_camera.GetTransform().SetPosition(0, 5, -20);

	_sceneBounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	_sceneBounds.Radius = 100.0f;

	CreateMesh();
	LoadTextures();
	CreateMaterial();

	BuildDescriptorHeap();

	CreateGameObject();
	CreateRenderObjectInfo();

	BuildFrameResource();

	BuildRootSignature();
	BuildRootSignatureSSAO();
	BuildInputLayoutAndShader();

	BuildPSO();

	_ssaoMap->SetPSOs(m_psos[(int)RenderLayer::Ssao].Get(), m_psos[0].Get());
}

bool DK::EngineBase::Update()
{
	if (GraphicEngine::Update() == false)
		return false;

	m_curFrameResourceIndex = (m_curFrameResourceIndex + 1) % FrameResourceCount;

	m_curFrameResource = m_frameResources[m_curFrameResourceIndex].get();
	if (m_curFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_curFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_curFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	_lightRotationAngle += 0.1f * m_gameTimer.DeltaTime();

	XMMATRIX R = XMMatrixRotationY(_lightRotationAngle);
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&_baseLightDirections[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&_lightDir[i], lightDir);
	}

	UpdateShadowTransform();
	UpdateObjectCBs();
	UpdateMaterialCBs();
	UpdateMainPassCB();
	UpdateShadowPassCB();
	UpdateSsaoCB();

	return true;
}

bool DK::EngineBase::Render()
{
	auto cmdListAlloc = m_curFrameResource->CmdListAlloc;

	cmdListAlloc->Reset();
	m_commandList->Reset(cmdListAlloc.Get(), nullptr);

	// texture descriptor heap을 설정하자.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_heapCbvSrvUav.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// root signature를 설정하자.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// 2번 루트 파라미터에 연결하는 SRV (쉐이더 리소스 뷰) : Material
	auto matBuffer = m_curFrameResource->MaterialBuffer->GetBuffer();
	m_commandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	m_commandList->SetGraphicsRootDescriptorTable(3, srvGpuNull);

	m_commandList->SetGraphicsRootDescriptorTable(4, m_heapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());

	RenderShadowMap();

	RenderNormalAndDepth();

	m_commandList->SetGraphicsRootSignature(_ssaoRootSignature.Get());
	_ssaoMap->ComputeSsao(m_commandList.Get(), m_curFrameResource, 3);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_commandList->RSSetViewports(1, &m_viewPortScreen);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_heapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
	Texture* texture = GetTexture("grasscube1024");
	if (texture != nullptr)
		skyTexDescriptor.Offset(texture->SrvHeapIndex, m_uCbvSrvUavDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

	CD3DX12_RESOURCE_BARRIER renderTarget = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &renderTarget);

	D3D12_CPU_DESCRIPTOR_HANDLE backBuffer = CurrentBackBufferView();
	m_commandList->ClearRenderTargetView(backBuffer, DirectX::Colors::DimGray, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencil = DepthStencilView();
	m_commandList->ClearDepthStencilView(depthStencil, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_commandList->OMSetRenderTargets(1, &backBuffer, true, &depthStencil);

#if GIZMO
	//m_gizmo.PreRender(m_commandList.Get());
#endif

	auto passCB = m_curFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());

	Render(m_commandList.Get());

	CD3DX12_RESOURCE_BARRIER present = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &present);

	m_commandList->Close();

	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	m_swapChain->Present(0, 0);
	m_nCurrBackBuffer = (m_nCurrBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	m_curFrameResource->Fence = ++m_ullCurrentFence;
	m_commandQueue->Signal(m_fence.Get(), m_ullCurrentFence);

	return true;
}

void DK::EngineBase::Render(ID3D12GraphicsCommandList* cmdList)
{
}

bool DK::EngineBase::OnResize(int width, int height, bool force)
{
	GraphicEngine::OnResize(width, height, force);

	DirectX::XMMATRIX proj = m_camera.GetProjMatrix();
	DirectX::BoundingFrustum::CreateFromMatrix(m_camFrustum, proj);

	return true;
}

// init
void DK::EngineBase::LoadTextures()
{
}

void DK::EngineBase::CreateMesh()
{
}

void DK::EngineBase::CreateMaterial()
{
}

void DK::EngineBase::CreateGameObject()
{
}

void DK::EngineBase::CreateRenderObjectInfo()
{
}

void DK::EngineBase::BuildFrameResource()
{
	int objectCount = 0;
	for (int i = 0; i < (int)RenderLayer::Count; ++i)
		objectCount += m_gameObjects[i].size();

	int renderPassCount = 1;
	int shadowPassCount = 1;
	int totalPassCount = renderPassCount + shadowPassCount;

	for (int i = 0; i < FrameResourceCount; ++i)
		m_frameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(), totalPassCount, objectCount, m_materials.size()));
}

void DK::EngineBase::BuildDescriptorHeap()
{
	if (m_textures.size() <= 0)
		return;

	// Create Descriptor
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	heapDesc.NumDescriptors = m_textures.size() + 20;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapCbvSrvUav)));

	// Create View
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	for (auto iter = m_textures.begin(); iter != m_textures.end(); ++iter)
	{
		Texture* pTexture = iter->second.get();

		srvDesc.ViewDimension = pTexture->Dimension;
		srvDesc.Format = pTexture->Resource->GetDesc().Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = pTexture->Resource->GetDesc().MipLevels;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_heapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
		heapHandle.Offset(pTexture->SrvHeapIndex, m_uCbvSrvUavDescriptorSize);

		m_d3dDevice->CreateShaderResourceView(pTexture->Resource.Get(), &srvDesc, heapHandle);
	}

	auto srvCpuStart = m_heapCbvSrvUav->GetCPUDescriptorHandleForHeapStart();
	auto srvGpuStart = m_heapCbvSrvUav->GetGPUDescriptorHandleForHeapStart();
	auto dsvCpuStart = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_shadowSrvIndex = m_textures.size();
	_ssaoSrvIndex = _shadowSrvIndex + 1;
	_ssaoAmbientSrvIndex = _ssaoSrvIndex + 3;
	_nullCubeSrvIndex = _ssaoSrvIndex + 5;
	_nullTexSrvIndex = _nullCubeSrvIndex + 1;
	_nullTexSrvIndex1 = _nullTexSrvIndex + 1;

	auto nullSrv = GetSrvCpuHandle(_nullCubeSrvIndex);
	m_d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	nullSrv = GetSrvCpuHandle(_nullTexSrvIndex);
	m_d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	nullSrv = GetSrvCpuHandle(_nullTexSrvIndex1);
	m_d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);

	_shadowMap->BuildDescriptor(GetSrvCpuHandle(_shadowSrvIndex), GetSrvGpuHandle(_shadowSrvIndex), GetDsvCpuHandle(1));

	_ssaoMap->BuildDescriptors(
		m_depthStencilBuffer.Get(),
		GetSrvCpuHandle(_ssaoSrvIndex),
		GetSrvGpuHandle(_ssaoSrvIndex),
		GetRtvCpuHandle(SWAP_CHAIN_BUFFER_COUNT),
		m_uCbvSrvUavDescriptorSize,
		m_uRtvDescriptorSize);
}

void DK::EngineBase::BuildRootSignature()
{
}

void DK::EngineBase::BuildRootSignatureSSAO()
{
	// t0 ~ t1: normal map, depth map
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

	// t2: random vector map
	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	ZeroMemory(slotRootParameter, sizeof(CD3DX12_ROOT_PARAMETER) * 4);

	slotRootParameter[0].InitAsConstantBufferView(0);	// ssao constants
	slotRootParameter[1].InitAsConstants(1, 1);			// blur value
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
	{
		pointClamp, linearClamp, depthMapSam, linearWrap
	};

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	m_d3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(_ssaoRootSignature.GetAddressOf()));
}

void DK::EngineBase::BuildInputLayoutAndShader()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	m_shaders["standardVS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["shadowVS"] = D3DUtils::CompileShader(L"Shaders\\Shadow.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["shadowOpaquePS"] = D3DUtils::CompileShader(L"Shaders\\Shadow.hlsl", nullptr, "PS", "ps_5_1");
	m_shaders["shadowAlphaTestedPS"] = D3DUtils::CompileShader(L"Shaders\\Shadow.hlsl", alphaTestDefines, "PS", "ps_5_1");

	m_shaders["drawNormalsVS"] = D3DUtils::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["drawNormalsPS"] = D3DUtils::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "PS", "ps_5_1");

	m_shaders["ssaoVS"] = D3DUtils::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["ssaoPS"] = D3DUtils::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "PS", "ps_5_1");

	/*m_shaders["ssaoBlurVS"] = D3DUtils::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["ssaoBlurPS"] = D3DUtils::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "PS", "ps_5_1");*/

	m_shaders["skyVS"] = D3DUtils::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "SkyVS", "vs_5_1");
	m_shaders["skyPS"] = D3DUtils::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "SkyPS", "ps_5_1");

	m_inputLayouts = Vertex::GetInputLayout();
}

void DK::EngineBase::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// opaque pso
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = { m_inputLayouts.data(), (UINT)m_inputLayouts.size() };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_eBackBufferFormat;
	psoDesc.DSVFormat = m_eDepthStencilFormat;
	psoDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Opaque])));

	// PSO for transparent objects
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesctransparent = psoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesctransparent.BlendState.RenderTarget[0] = transparencyBlendDesc;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesctransparent, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Transparent])));

	// PSO for alpha tested objects
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = psoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["alphaTestedPS"]->GetBufferPointer()),
		m_shaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::AlphaTested])));
}

void DK::EngineBase::UpdateObjectCBs()
{
	if (m_renderObjectInfos.size() <= 0)
		return;

	auto objectConstBuffer = m_curFrameResource->ObjectCB.get();

	for (int i = 0; i < m_renderObjectInfos.size(); ++i)
	{
		RenderObjectInfo* objectInfo = m_renderObjectInfos[i].get();

		XMMATRIX world = XMLoadFloat4x4(&objectInfo->World);
		XMMATRIX texTransform = XMLoadFloat4x4(&objectInfo->TexTransform);

		objectInfo->ObjBufferIndex = i;

		ObjectConstants data;
		XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
		data.MaterialIndex = objectInfo->MaterialIndex;
		objectInfo->ignoreRender = false;

		objectConstBuffer->CopyData(i, data);
	}
}

void DK::EngineBase::UpdateMaterialCBs()
{
	if (m_curFrameResource->MaterialBuffer == nullptr)
		return;

	auto matBuffer = m_curFrameResource->MaterialBuffer.get();

	for (auto& data : m_materials)
	{
		Material* mat = data.second.get();

		if (mat->DirtyCount <= 0) 
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

		// gpu에게 넘겨줄 material data 복사
		MaterialData metData;
		metData.DiffuseAlbedo = mat->DiffuseAlbedo;
		metData.FresnelR0 = mat->FresnelR0;
		metData.Roughness = mat->Roughness;
		DirectX::XMStoreFloat4x4(&metData.MatTransform, XMMatrixTranspose(matTransform));

		metData.BaseColorTextureIndex = mat->BaseColorTextureIndex;
		metData.NormalTextureIndex = mat->NormalTextureIndex;

		matBuffer->CopyData(mat->SrvIndex, metData);

		mat->DirtyCount -= 1;
	}
}

void DK::EngineBase::UpdateMainPassCB()
{
	DirectX::XMMATRIX view = m_camera.GetViewMatrix();
	DirectX::XMMATRIX invView = m_camera.GetInvViewMatrix();

	DirectX::XMMATRIX proj = m_camera.GetProjMatrix();
	DirectX::XMMATRIX invProj = m_camera.GetInvProjMatrix();

	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	DirectX::XMVECTOR viewprojDetermin = XMMatrixDeterminant(viewProj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&viewprojDetermin, viewProj);

	CXMMATRIX shadowTransform = XMLoadFloat4x4(&_shadowTransform);

	DirectX::XMStoreFloat4x4(&m_renderPassCB.View, XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvView, XMMatrixTranspose(invView));

	DirectX::XMStoreFloat4x4(&m_renderPassCB.Proj, XMMatrixTranspose(proj));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvProj, XMMatrixTranspose(invProj));

	DirectX::XMStoreFloat4x4(&m_renderPassCB.ViewProj, XMMatrixTranspose(viewProj));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	DirectX::XMStoreFloat4x4(&m_renderPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));

	m_renderPassCB.EyePosW = m_camera.GetTransform().GetPosition();
	m_renderPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)m_nClientWidth, (float)m_nClientHeight);
	m_renderPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_nClientWidth, 1.0f / m_nClientHeight);
	m_renderPassCB.NearZ = m_camera.GetNear();
	m_renderPassCB.FarZ = m_camera.GetFar();
	m_renderPassCB.TotalTime = m_gameTimer.TotalTime();
	m_renderPassCB.DeltaTime = m_gameTimer.DeltaTime();

	m_renderPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	m_renderPassCB.Lights[0].Direction = _lightDir[0];
	m_renderPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };

	m_renderPassCB.Lights[1].Direction = _lightDir[1];
	m_renderPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

	m_renderPassCB.Lights[2].Direction = _lightDir[2];
	m_renderPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_curFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::EngineBase::UpdateShadowTransform()
{
	// 빛 시점의 뷰-투영 행렬 정보가 필요하다.
	XMVECTOR lightDir = XMLoadFloat3(&_lightDir[0]);
	XMVECTOR lightPos = DirectX::XMVectorScale(lightDir, -2.0f * _sceneBounds.Radius);
	XMVECTOR targetPos = XMLoadFloat3(&_sceneBounds.Center);
	XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&_lightPos, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - _sceneBounds.Radius;
	float b = sphereCenterLS.y - _sceneBounds.Radius;
	float n = sphereCenterLS.z - _sceneBounds.Radius;
	float r = sphereCenterLS.x + _sceneBounds.Radius;
	float t = sphereCenterLS.y + _sceneBounds.Radius;
	float f = sphereCenterLS.z + _sceneBounds.Radius;

	_lightNearZ = n;
	_lightFarZ = f;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;
	XMStoreFloat4x4(&_lightView, lightView);
	XMStoreFloat4x4(&_lightProj, lightProj);
	XMStoreFloat4x4(&_shadowTransform, S);
}

void DK::EngineBase::UpdateShadowPassCB()
{
	XMMATRIX view = XMLoadFloat4x4(&_lightView);
	XMVECTOR viewDerminant = XMMatrixDeterminant(view);
	XMMATRIX invView = XMMatrixInverse(&viewDerminant, view);

	XMMATRIX proj = XMLoadFloat4x4(&_lightProj);
	XMVECTOR projDerminant = XMMatrixDeterminant(proj);
	XMMATRIX invProj = XMMatrixInverse(&projDerminant, proj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMVECTOR viewprojDeterminant = XMMatrixDeterminant(viewProj);

	XMMATRIX invViewProj = XMMatrixInverse(&viewprojDeterminant, viewProj);

	UINT w = _shadowMap->Width();
	UINT h = _shadowMap->Height();

	XMStoreFloat4x4(&_shadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&_shadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&_shadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&_shadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&_shadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&_shadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	_shadowPassCB.EyePosW = _lightPos;
	_shadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	_shadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	_shadowPassCB.NearZ = _lightNearZ;
	_shadowPassCB.FarZ = _lightFarZ;

	auto currPassCB = m_curFrameResource->RenderPassCB.get();
	currPassCB->CopyData(1, _shadowPassCB);
}

void DK::EngineBase::UpdateSsaoCB()
{
	SsaoConstants ssaoCB;

	XMMATRIX P = m_camera.GetProjMatrix();

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	ssaoCB.Proj = m_renderPassCB.Proj;
	ssaoCB.InvProj = m_renderPassCB.InvProj;
	XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

	_ssaoMap->GetOffsetVectors(ssaoCB.OffsetVectors);

	auto blurWeights = _ssaoMap->CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

	ssaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / _ssaoMap->GetSsaoMapWidth(), 1.0f / _ssaoMap->GetSsaoMapHeight());

	// Coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 1.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;

	auto currSsaoCB = m_curFrameResource->SsaoCB.get();
	currSsaoCB->CopyData(0, ssaoCB);
}

void DK::EngineBase::RenderRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderObjectInfo*> renderInfos)
{
	UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));

	for (int i = 0; i < renderInfos.size(); ++i)
	{
		RenderObjectInfo* renderInfo = renderInfos[i];

		if (renderInfo->ignoreRender || renderInfo->meshInfo == nullptr) 
			continue;

		D3D12_VERTEX_BUFFER_VIEW vbView;
		D3D12_INDEX_BUFFER_VIEW ibView;
		MeshSection meshSection;

		if (renderInfo->meshInfo->GetMeshInfo(vbView, ibView, meshSection) == false)
			continue;

		cmdList->IASetVertexBuffers(0, 1, &vbView);		// vertex buffer 바인딩
		cmdList->IASetIndexBuffer(&ibView);				// index buffer 바인딩
		cmdList->IASetPrimitiveTopology(renderInfo->PrimitiveType);

		auto objCBAddr = m_curFrameResource->ObjectCB->GetBuffer()->GetGPUVirtualAddress();

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCBAddr + (objCBByteSize * renderInfo->ObjBufferIndex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
	}
}

void DK::EngineBase::RenderShadowMap()
{
	// viewport, scissor rect 설정
	auto viewport = _shadowMap->Viewport();
	auto rect = _shadowMap->ScissorRect();
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &rect);

	// shadow map을 그리기 위해 depth write 상태로 변경
	auto changeWrite = CD3DX12_RESOURCE_BARRIER::Transition(_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_commandList->ResourceBarrier(1, &changeWrite);

	// shadow map 지우기
	m_commandList->ClearDepthStencilView(_shadowMap->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// depth 버퍼만 출력하고 render target 출력 과정을 생략하기 위해서 nullptr로 지정
	auto dsv = _shadowMap->Dsv();
	m_commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

	// shadow const pass를 설정
	UINT passCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(RenderPassConstants));
	auto passCB = m_curFrameResource->RenderPassCB->GetBuffer();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	m_commandList->SetGraphicsRootConstantBufferView(0, passCBAddress);

	m_commandList->SetPipelineState(m_psos[(int)RenderLayer::Shadow].Get());

	RenderRenderItems(m_commandList.Get(), m_renderList[(int)RenderLayer::Opaque]);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	auto changeRead = CD3DX12_RESOURCE_BARRIER::Transition(_shadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	m_commandList->ResourceBarrier(1, &changeRead);
}

void DK::EngineBase::RenderNormalAndDepth()
{
	m_commandList->RSSetViewports(1, &m_viewPortScreen);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	ID3D12Resource* normalMap = _ssaoMap->NormalMap();
	CD3DX12_CPU_DESCRIPTOR_HANDLE normalMapRtv = _ssaoMap->NormalMapRtv();

	// normal map을 render target으로 전환
	auto readToRt = CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &readToRt);

	// depth, stencil 초기화
	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_commandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DepthStencilView();
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->OMSetRenderTargets(1, &normalMapRtv, true, &dsvHandle);

	// Bind the constant buffer for this pass.
	auto passCB = m_curFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	m_commandList->SetPipelineState(m_psos[(int)RenderLayer::DrawNormals].Get());

	RenderRenderItems(m_commandList.Get(), m_renderList[(int)RenderLayer::Opaque]);

	// normal map을 read로 전환
	auto rtToRead = CD3DX12_RESOURCE_BARRIER::Transition(normalMap, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	m_commandList->ResourceBarrier(1, &rtToRead);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DK::EngineBase::GetRtvCpuHandle(int index)
{
	auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtv.Offset(index, m_uRtvDescriptorSize);
	return rtv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DK::EngineBase::GetDsvCpuHandle(int index)
{
	auto dsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	dsv.Offset(index, m_uDsvDescriptorSize);
	return dsv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DK::EngineBase::GetSrvCpuHandle(int index)
{
	auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
	srv.Offset(index, m_uCbvSrvUavDescriptorSize);
	return srv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DK::EngineBase::GetSrvGpuHandle(int index)
{
	auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());
	srv.Offset(index, m_uCbvSrvUavDescriptorSize);
	return srv;
}

void DK::EngineBase::LoadTexture(std::wstring path)
{
	size_t firstIdx = path.rfind(L'/') + 1;
	size_t lastIdx = path.rfind(L'.');
	std::wstring fileName = path.substr(firstIdx, lastIdx - firstIdx);

	std::unique_ptr<Texture> spTexture = std::make_unique<Texture>();

	int srvHeapIndex = m_textures.size();

	spTexture->FileName = fileName;
	spTexture->SrvHeapIndex = srvHeapIndex;

	HRESULT hr = DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_commandList.Get(), path.c_str(),
		spTexture->Resource, spTexture->UploadHeap, spTexture->Dimension);

	if (SUCCEEDED(hr))
	{
		m_textures[WStringToAnsi(fileName)] = std::move(spTexture);
	}
	else
	{
		OutputDebugString(fileName.c_str());
		OutputDebugString(L"Texture 파일을 Load할 수 없습니다.");
	}
}

Texture* DK::EngineBase::GetTexture(string textureName)
{
	if (m_textures.find(textureName) == m_textures.end())
		return nullptr;

	return m_textures[textureName].get();
}
