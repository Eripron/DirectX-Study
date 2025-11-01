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
	CreateMesh();
	LoadTextures();
	CreateMaterial();

	CreateGameObject();

	BuildDescriptorHeap();
	BuildFrameResource();
	BuildRootSignature();
	BuildInputLayoutAndShader();
	BuildPSO();
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

	UpdateObjectCBs();
	UpdateMaterialCBs();
	UpdateMainPassCB();

	return true;
}

bool DK::EngineBase::Render()
{
	auto cmdListAlloc = m_curFrameResource->CmdListAlloc;

	cmdListAlloc->Reset();
	m_commandList->Reset(cmdListAlloc.Get(), nullptr);

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

#if GIZMO
	m_gizmo.PreRender(m_commandList.Get());
#endif

	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_heapCbvSrvUav.Get() };
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

		auto matBuffer = m_curFrameResource->MaterialBuffer->GetBuffer();
		m_commandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());

		auto passCB = m_curFrameResource->RenderPassCB->GetBuffer();
		m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

		m_commandList->SetGraphicsRootDescriptorTable(3, m_heapCbvSrvUav->GetGPUDescriptorHandleForHeapStart());

		Render(m_commandList.Get());
	}

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

void DK::EngineBase::CreateRenderItem()
{
}

void DK::EngineBase::BuildFrameResource()
{
	int totalInstCount = 0;
	for (int i = 0; i < m_renderItems.size(); ++i)
	{
		int instCount = m_renderItems[i]->InstanceDatas.size();
		totalInstCount += (instCount > 0 ? instCount : 1);
	}

	for (int i = 0; i < FrameResourceCount; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(), 1, totalInstCount, m_materials.size()));
	}
}

void DK::EngineBase::BuildDescriptorHeap()
{
	if (m_textures.size() <= 0)
		return;

	// Create Descriptor
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	ZeroMemory(&heapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	heapDesc.NumDescriptors = m_textures.size();
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapCbvSrvUav)));

	// Create View
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	for (auto iter = m_textures.begin(); iter != m_textures.end(); ++iter)
	{
		Texture* pTexture = iter->second.get();

		srvDesc.Format = pTexture->Resource->GetDesc().Format;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_heapCbvSrvUav->GetCPUDescriptorHandleForHeapStart());
		heapHandle.Offset(pTexture->SrvHeapIndex, m_uCbvSrvUavDescriptorSize);

		m_d3dDevice->CreateShaderResourceView(pTexture->Resource.Get(), &srvDesc, heapHandle);
	}
}

void DK::EngineBase::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	ZeroMemory(slotRootParameter, sizeof(CD3DX12_ROOT_PARAMETER) * 4);

	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);

	slotRootParameter[2].InitAsConstantBufferView(0);

	// PS에서 사용할 table 1개 설정
	CD3DX12_DESCRIPTOR_RANGE textureTable;
	textureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_textures.size(), 0);
	slotRootParameter[3].InitAsDescriptorTable(1, &textureTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = Texture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	THROW_IF_FAILED(hr);

	m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature));
}

void DK::EngineBase::BuildInputLayoutAndShader()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	m_shaders["standardVS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");
	m_shaders["alphaTestedPS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");

	m_inputLayouts = Vertex::GetInputLayout();
}

void DK::EngineBase::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesOpaque;

	// PSO for opaque objects.
	ZeroMemory(&psoDesOpaque, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesOpaque.InputLayout = { m_inputLayouts.data(), (UINT)m_inputLayouts.size() };
	psoDesOpaque.pRootSignature = m_rootSignature.Get();
	psoDesOpaque.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["standardVS"]->GetBufferPointer()),
		m_shaders["standardVS"]->GetBufferSize()
	};
	psoDesOpaque.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["opaquePS"]->GetBufferPointer()),
		m_shaders["opaquePS"]->GetBufferSize()
	};
	psoDesOpaque.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesOpaque.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesOpaque.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesOpaque.SampleMask = UINT_MAX;
	psoDesOpaque.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesOpaque.NumRenderTargets = 1;
	psoDesOpaque.RTVFormats[0] = m_eBackBufferFormat;
	psoDesOpaque.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDesOpaque.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	psoDesOpaque.DSVFormat = m_eDepthStencilFormat;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesOpaque, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Opaque])));

	// PSO for transparent objects
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesctransparent = psoDesOpaque;

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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = psoDesOpaque;
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
	if (m_renderItems.size() <= 0)
		return;

	DirectX::XMMATRIX invView = m_camera.GetInvViewMatrix();

	auto currInstanceBuffer = m_curFrameResource->InstanceBuffer.get();
	int renderInstanceCount = 0;

	for (auto& e : m_renderItems)
	{
		const auto& instanceData = e->InstanceDatas;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMVECTOR determin = XMMatrixDeterminant(world);
			XMMATRIX invWorld = XMMatrixInverse(&determin, world);

			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			DirectX::BoundingFrustum localSpaceFrustum;
			m_camFrustum.Transform(localSpaceFrustum, viewToLocal);

			if (localSpaceFrustum.Contains(e->BoundBox) != DirectX::DISJOINT)
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				currInstanceBuffer->CopyData(renderInstanceCount, data);
				++renderInstanceCount;
			}
		}

		e->RenderInstanceCount = renderInstanceCount;
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

		metData.DiffuseSrvHeapIndex = mat->DiffuseSrvHeapIndex;

		matBuffer->CopyData(mat->SrvHeapIndex, metData);

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

	DirectX::XMStoreFloat4x4(&m_renderPassCB.View, XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvView, XMMatrixTranspose(invView));

	DirectX::XMStoreFloat4x4(&m_renderPassCB.Proj, XMMatrixTranspose(proj));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvProj, XMMatrixTranspose(invProj));

	DirectX::XMStoreFloat4x4(&m_renderPassCB.ViewProj, XMMatrixTranspose(viewProj));
	DirectX::XMStoreFloat4x4(&m_renderPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	m_renderPassCB.EyePosW = m_camera.GetTransform().GetPosition();
	m_renderPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)m_nClientWidth, (float)m_nClientHeight);
	m_renderPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_nClientWidth, 1.0f / m_nClientHeight);
	m_renderPassCB.NearZ = m_camera.GetNear();
	m_renderPassCB.FarZ = m_camera.GetFar();
	m_renderPassCB.TotalTime = m_gameTimer.TotalTime();
	m_renderPassCB.DeltaTime = m_gameTimer.DeltaTime();

	m_renderPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	m_renderPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_renderPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };

	m_renderPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_renderPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

	m_renderPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_renderPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_curFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::EngineBase::RenderRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem*> renderItems)
{
	for (int i = 0; i < renderItems.size(); ++i)
	{
		RenderItem* curRender = renderItems[i];
		MeshFilter* meshInfo = curRender->pGameObject->GetComponent<MeshFilter>();

		D3D12_VERTEX_BUFFER_VIEW vbView;
		D3D12_INDEX_BUFFER_VIEW ibView;
		MeshSection meshSection;

		if (meshInfo->GetMeshInfo(vbView, ibView, meshSection) == false)
			continue;

		cmdList->IASetVertexBuffers(0, 1, &vbView);
		cmdList->IASetIndexBuffer(&ibView);
		cmdList->IASetPrimitiveTopology(curRender->PrimitiveType);

		int instanceCount = 0;
		if (curRender->InstanceDatas.size() > 0)
		{
			instanceCount = curRender->RenderInstanceCount;

			auto instanceBuffer = m_curFrameResource->InstanceBuffer->GetBuffer();
			cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
		}
		else
			instanceCount = 1;

		if (instanceCount <= 0)
			continue;

		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 
									  instanceCount, 
									  meshSection.StartIndexLocation, 
									  meshSection.BaseVertexLocation, 0);

		WCHAR title[60];
		GetWindowText(GetHandleWindow(), title, 49);

		std::wstring wstrCount = L"출력 instance count: " + std::to_wstring(instanceCount);
		std::wstring newTitle = std::wstring(title) + wstrCount;

		SetWindowText(GetHandleWindow(), newTitle.c_str());
	}
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
		spTexture->Resource, spTexture->UploadHeap);

	if (SUCCEEDED(hr))
		m_textures[WStringToAnsi(fileName)] = std::move(spTexture);
}
