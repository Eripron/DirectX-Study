#include "ExTexture.h"

DK::ExTexture::ExTexture(HWND hWnd) : GraphicEngine(hWnd)
{
}

DK::ExTexture::~ExTexture()
{
}

void DK::ExTexture::Init()
{
	m_camera.GetTransform().SetPosition(0, 0, -10);

	LoadTexture();
	CreateGeometry();
	CreateMaterial();
	CreateGameObject();
	CreateFrameResource();
	BuildRootSignature();
	BuildDescriptor();
	BuildInputLayoutAndShader();
	BuildPSO();

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
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

	AnimateMaterials(m_gameTimer);
	UpdateObjectCBs();
	UpdateMaterialCBs();
	UpdateRenderPassCB();

	return true;
}

bool DK::ExTexture::Render()
{
	auto cmdListAlloc = m_pCurrFrameResource->CmdListAlloc;

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

	m_commandList->SetGraphicsRootSignature(m_spRootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_spHeapSRV.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto passCB = m_pCurrFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	// render logic
	{
		m_commandList->SetPipelineState(m_mapPSO["opaque"].Get());
		DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::Opaque]);

		m_commandList->SetPipelineState(m_mapPSO["treeSprites"].Get());
		DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::AlphaTestedTreeSprites]);
	}

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

void DK::ExTexture::LoadTexture()
{
	LoadTexture(L"Textures/WoodCrate02.dds", 0);
	LoadTexture(L"Textures/treeArray2.dds", 1);
}

void DK::ExTexture::LoadTexture(std::wstring path, int texCBIndex)
{
	size_t firstIdx = path.rfind(L'/') + 1;
	size_t lastIdx = path.rfind(L'.');
	std::string name = WStringToAnsi(path.substr(firstIdx, lastIdx - firstIdx));

	std::unique_ptr<Texture> spTexture = std::make_unique<Texture>();

	spTexture->FileName = path;
	spTexture->SrvHeapIndex = texCBIndex;

	DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_commandList.Get(), spTexture->FileName.c_str(),
		spTexture->Resource, spTexture->UploadHeap);

	m_mapTextures[name] = std::move(spTexture);
}

void DK::ExTexture::CreateMaterial()
{
	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "tree";
	treeSprites->SrvHeapIndex = 0;
	treeSprites->DiffuseSrvHeapIndex = m_mapTextures["treeArray2"]->SrvHeapIndex;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;
	treeSprites->DirtyCount = m_nFrameReesourceCount;

	auto matBox = std::make_unique<Material>();
	matBox->Name = "box";
	matBox->SrvHeapIndex = 1;
	matBox->DiffuseSrvHeapIndex = m_mapTextures["WoodCrate02"]->SrvHeapIndex;
	matBox->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	matBox->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	matBox->Roughness = 0.125f;
	matBox->DirtyCount = m_nFrameReesourceCount;

	m_mapMaterials[treeSprites->Name] = std::move(treeSprites);
	m_mapMaterials[matBox->Name] = std::move(matBox);
}

void DK::ExTexture::CreateGeometry()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> meshBox = geoGen.CreateBox(4, 4, 4);

	MeshData<Vertex> treePosition;
	int treeCount = 20;
	for (int i = 0; i < treeCount; ++i)
	{
		float x = MathUtils::RandF(-100.0f, 100.0f);
		float y = MathUtils::RandF(0.0f, 10.0f);
		float z = MathUtils::RandF(-100.0f, 100.0f);

		Vertex vertex;
		vertex.Position = { x, y, z };

		treePosition.Vertices.push_back(vertex);
		treePosition.Indices32.push_back(i);
	}
	
	MeshManager::GetInstance()->AddMeshData("box", meshBox);
	MeshManager::GetInstance()->AddMeshData("tree", treePosition);
}

void DK::ExTexture::CreateGameObject()
{
	/*int nCBIndex = 0;

	auto objTree = new GameObject();
	objTree->m_nCBIndex = nCBIndex++;
	objTree->m_nFrameDirty = m_nFrameReesourceCount;
	objTree->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	objTree->SetMaterial(m_mapMaterials["tree"].get());
	objTree->AddComponent(new MeshFilter("tree"));

	m_vecGameObjects[(int)RenderLayer::AlphaTestedTreeSprites].push_back(objTree);


	auto objBox = new GameObject();
	objBox->m_nCBIndex = nCBIndex++;
	objBox->m_nFrameDirty = m_nFrameReesourceCount;
	objBox->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	objBox->SetMaterial(m_mapMaterials["box"].get());
	objBox->AddComponent(new MeshFilter("box"));

	m_vecGameObjects[(int)RenderLayer::Opaque].push_back(objBox);*/
}

void DK::ExTexture::CreateFrameResource()
{
	int objectCount = 0;
	for (int i = 0; i < (int)RenderLayer::Count; ++i)
		objectCount += m_vecGameObjects[i].size();

	/*for (int i = 0; i < m_nFrameReesourceCount; ++i)
	{
		m_vecFrameResoruce.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, objectCount, m_mapMaterials.size(), 1));
	}*/
}

void DK::ExTexture::BuildDescriptor()
{
	int nTextureCount = m_mapTextures.size();

	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = nTextureCount;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_spHeapSRV)));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_spHeapSRV->GetCPUDescriptorHandleForHeapStart());

	// woodCreate2에 대한 view 생성
	auto tex = m_mapTextures["WoodCrate02"]->Resource;
	srvDesc.Format = tex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, descriptorHandle);

	auto texArray = m_mapTextures["treeArray2"]->Resource;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = texArray->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = texArray->GetDesc().DepthOrArraySize;
	descriptorHandle.Offset(1, m_uCbvSrvUavDescriptorSize);

	m_d3dDevice->CreateShaderResourceView(texArray.Get(), &srvDesc, descriptorHandle);

	//for(auto iter = m_mapTextures.begin(); iter != m_mapTextures.end(); ++iter)
	//{
	//	Texture* texture = iter->second.get();
	//	D3D12_RESOURCE_DESC rscDesc = texture->Resource->GetDesc();

	//	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_spHeapSRV->GetCPUDescriptorHandleForHeapStart());
	//	descriptorHandle.Offset(texture->TexCBIndex, m_uCbvSrvUavDescriptorSize);

	//	// texture2D에 대한 view 생성
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	//	srvDesc.Format = rscDesc.Format;
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.Texture2D.MostDetailedMip = 0;
	//	srvDesc.Texture2D.MipLevels = rscDesc.MipLevels;
	//	srvDesc.Texture2D.PlaneSlice = 0;
	//	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//	m_d3dDevice->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, descriptorHandle);
	//}
}

void DK::ExTexture::BuildInputLayoutAndShader()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	m_mapShaders["VS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
	m_mapShaders["PS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_0");
	m_mapShaders["alphaTestedPS"] = D3DUtils::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_0");

	m_mapShaders["treeSpriteVS"] = D3DUtils::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_0");

	m_mapShaders["treeSpriteGS"] = D3DUtils::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_0");

	m_mapShaders["treeSpritePS"] = D3DUtils::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	m_vecInputLayout = Vertex::GetInputLayout();
	m_vecInputLayoutTree =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
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

	auto staticSamplers = Texture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(4, rootParameters, 
		staticSamplers.size(), staticSamplers.data(), 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> spSerializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> spError = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, 
		spSerializedRootSig.GetAddressOf(), spError.GetAddressOf());

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

	// opaque pso
	psoDesc.InputLayout = { m_vecInputLayout.data(), (UINT)m_vecInputLayout.size() };
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

	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_mapPSO["opaque"]));
	
	// transparent pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = psoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
	m_d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_mapPSO["transparent"]));

	// alpha tested pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = psoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_mapShaders["alphaTestedPS"]->GetBufferPointer()),
		m_mapShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_d3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_mapPSO["alphaTested"]));
	
	// stencil
	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xff;
	dsDesc.StencilWriteMask = 0xff;
	dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC stencilPso = psoDesc;
	stencilPso.BlendState = blendDesc;
	stencilPso.DepthStencilState = dsDesc;
	m_d3dDevice->CreateGraphicsPipelineState(&stencilPso, IID_PPV_ARGS(&m_mapPSO["stencil"]));

	// tree sprite pso
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = psoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_mapShaders["treeSpriteVS"]->GetBufferPointer()),
		m_mapShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_mapShaders["treeSpriteGS"]->GetBufferPointer()),
		m_mapShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_mapShaders["treeSpritePS"]->GetBufferPointer()),
		m_mapShaders["treeSpritePS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { m_vecInputLayoutTree.data(), (UINT)m_vecInputLayoutTree.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	m_d3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&m_mapPSO["treeSprites"]));
}

void DK::ExTexture::AnimateMaterials(const GameTimer& gt)
{
	/*auto waterMat =  m_mapMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.05f * gt.DeltaTime();
	tv += 0.01f * gt.DeltaTime();

	if (tu >= 1.0f)
		tu -= 1.0f;

	if (tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	waterMat->NumFramesDirty = m_nFrameReesourceCount;*/
}

void DK::ExTexture::UpdateWave(const GameTimer& gt)
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

	//	// Derive tex-coords from position by 
	//	// mapping [-w/2,w/2] --> [0,1]
	//	v.TexC.x = 0.5f + v.Position.x / m_waves->Width();
	//	v.TexC.y = 0.5f - v.Position.z / m_waves->Depth();

	//	currWavesVB->CopyData(i, v);
	//}

	//// Set the dynamic VB of the wave renderitem to the current frame VB.
	//m_pGoWave->GetMeshBuffer()->VertexBuffer = currWavesVB->GetBuffer();
}

void DK::ExTexture::UpdateObjectCBs()
{
	/*auto objectCB = m_pCurrFrameResource->ObjectCB.get();

	for (int i = 0; i < (int)RenderLayer::Count; ++i)
	{
		int objCount = m_vecGameObjects[i].size();

		for (int j = 0; j < objCount; ++j)
		{
			if (m_vecGameObjects[i][j]->m_nFrameDirty <= 0)
				continue;

			DirectX::XMFLOAT4X4 worldMatrix = MathUtils::Identity4x4();

			Transform* transform = m_vecGameObjects[i][j]->GetComponent<Transform>();
			if (transform != nullptr)
				worldMatrix = transform->GetWorldMatrix();

			DirectX::XMFLOAT4X4 texTransform = m_vecGameObjects[i][j]->TexTransform;

			XMMATRIX world = XMLoadFloat4x4(&worldMatrix);
			XMMATRIX vTexTransform = XMLoadFloat4x4(&texTransform);

			ObjectConstants constants;
			DirectX::XMStoreFloat4x4(&constants.WorldMatrix, XMMatrixTranspose(world));
			DirectX::XMStoreFloat4x4(&constants.TexTransform, XMMatrixTranspose(vTexTransform));

			objectCB->CopyData(m_vecGameObjects[i][j]->m_nCBIndex, constants);

			m_vecGameObjects[i][j]->m_nFrameDirty -= 1;
		}
	}*/
}

void DK::ExTexture::UpdateMaterialCBs()
{
	/*auto meterialCB = m_pCurrFrameResource->MaterialCB.get();

	for (auto& data : m_mapMaterials)
	{
		Material* mat = data.second.get();

		if (mat->NumFramesDirty <= 0)
			continue;

		XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
		matConstants.FresnelR0 = mat->FresnelR0;
		matConstants.Roughness = mat->Roughness;
		DirectX::XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

		meterialCB->CopyData(mat->MatCBIndex, matConstants);

		mat->NumFramesDirty -= 1;
	}*/
}

void DK::ExTexture::UpdateRenderPassCB()
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

	auto currPassCB = m_pCurrFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::ExTexture::DrawGameObjects(ID3D12GraphicsCommandList* cmdList, std::vector<GameObject*> vecGameObject)
{
	/*UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();

	for (size_t i = 0; i < vecGameObject.size(); ++i)
	{
		GameObject* pGameObject = vecGameObject[i];

		MeshFilter* meshFilter = pGameObject->GetComponent<MeshFilter>();
		if (meshFilter == nullptr)
			continue;

		D3D12_VERTEX_BUFFER_VIEW vbView;
		D3D12_INDEX_BUFFER_VIEW ibView;
		MeshSection meshSection;

		if (meshFilter->GetMeshInfo(vbView, ibView, meshSection) == false)
			continue;

		cmdList->IASetVertexBuffers(0, 1, &vbView);
		cmdList->IASetIndexBuffer(&ibView);
		cmdList->IASetPrimitiveTopology(pGameObject->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_spHeapSRV->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(pGameObject->GetMaterial()->DiffuseSrvHeapIndex, m_uCbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * pGameObject->m_nCBIndex);

		int matCBIndex = pGameObject->GetMaterial()->MatCBIndex;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBByteSize * matCBIndex);

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
	}*/
}

DirectX::XMFLOAT3 DK::ExTexture::GetHillNormal(float x, float z)
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
