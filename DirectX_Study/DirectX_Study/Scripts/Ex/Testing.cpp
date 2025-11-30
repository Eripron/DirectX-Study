#include "Testing.h"

DK::Testing::Testing(HWND hWnd) : EngineBase(hWnd)
{
}

DK::Testing::~Testing()
{
}

bool DK::Testing::Update()
{
	if (!EngineBase::Update())
		return false;

	return true;
}

void DK::Testing::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Opaque].Get());

	/*if (m_renderList[(int)RenderLayer::Opaque].size() > 0)
		RenderRenderItems(cmdList, m_renderList[(int)RenderLayer::Opaque]);*/

	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Sky].Get());
	if (m_renderList[(int)RenderLayer::Sky].size() > 0)
		RenderRenderItems(cmdList, m_renderList[(int)RenderLayer::Sky]);
}

bool DK::Testing::OnResize(int width, int height, bool force)
{
	if (EngineBase::OnResize(width, height, force) == false)
		return false;

	return true;
}

void DK::Testing::LoadTextures()
{
	LoadTexture(L"Textures/bricks2.dds");
	LoadTexture(L"Textures/tile.dds");
	LoadTexture(L"Textures/white1x1.dds");
	LoadTexture(L"Textures/stone.dds");
	LoadTexture(L"Textures/grasscube1024.dds");

	/*std::wstring pathFolder = L"Textures/*.dds";
	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(pathFolder.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		std::wstring fileName = findData.cFileName;
		std::wstring path = L"Textures/" + fileName;

		if (fileName.length() > 0)
			LoadTexture(path);

	} while (FindNextFileW(hFind, &findData) != 0);*/
}

void DK::Testing::CreateMesh()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> ground = geoGen.CreateGrid(100, 100, 10, 10);
	MeshManager::GetInstance()->AddMeshData("ground", ground);

	MeshData<Vertex> box = geoGen.CreateBox(1, 1, 1);
	MeshManager::GetInstance()->AddMeshData("box", box);

	MeshData<Vertex> sphere = geoGen.CreateSphere(100, 100, 100);
	MeshManager::GetInstance()->AddMeshData("sphere", box);

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
}

void DK::Testing::CreateMaterial()
{
	Texture* texture = GetTexture("stone");

	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "stone";
	bricks0->SrvHeapIndex = 0;
	bricks0->DiffuseSrvHeapIndex = texture == nullptr ? -1 :  texture->SrvHeapIndex;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks0->Roughness = 0.3f;

	texture = GetTexture("tile");
	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile";
	tile0->SrvHeapIndex = 1;
	tile0->DiffuseSrvHeapIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	tile0->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile0->Roughness = 0.1f;
	
	texture = GetTexture("white1x1");
	auto mirror0 = std::make_unique<Material>();
	mirror0->Name = "mirror";
	mirror0->SrvHeapIndex = 2;
	mirror0->DiffuseSrvHeapIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	mirror0->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.1f, 1.0f);
	mirror0->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
	mirror0->Roughness = 0.1f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->SrvHeapIndex = 3;
	skullMat->DiffuseSrvHeapIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	skullMat->DiffuseAlbedo = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	skullMat->Roughness = 0.2f;

	texture = GetTexture("grasscube1024");
	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->SrvHeapIndex = 4;
	sky->DiffuseSrvHeapIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	m_materials[bricks0->Name] = std::move(bricks0);
	m_materials[tile0->Name] = std::move(tile0);
	m_materials[mirror0->Name] = std::move(mirror0);
	m_materials[skullMat->Name] = std::move(skullMat);
	m_materials[sky->Name] = std::move(sky);
}

void DK::Testing::CreateGameObject()
{
	GameObject* ground = new GameObject();

	ground->AddComponent(new MeshFilter("ground"));
	ground->SetMaterial(m_materials["stone"].get());

	m_gameObjects[(int)RenderLayer::Opaque].push_back(ground);

	GameObject* sky = new GameObject();
	sky->AddComponent(new MeshFilter("sphere"));
	Transform* trans = sky->GetComponent<Transform>();
	trans->SetScale(30, 30, 30);

	m_gameObjects[(int)RenderLayer::Sky].push_back(sky);
}

void DK::Testing::CreateRenderObjectInfo()
{
	int objectIndex = 0;
	for(int i = 0; i < (int)RenderLayer::Count; ++i)
	{
		for (int j = 0; j < m_gameObjects[i].size(); ++j)
		{
			GameObject* object = m_gameObjects[i][j];

			MeshFilter* mesh = object->GetComponent<MeshFilter>();
			if (mesh == nullptr)continue;

			auto renderInfo = std::make_unique<RenderObjectInfo>();

			renderInfo->ObjBufferIndex = objectIndex++;

			Transform* transform = object->GetComponent<Transform>();
			renderInfo->World = transform->GetWorldMatrix();

			Material* mat = object->GetMaterial();
			if (mat != nullptr) renderInfo->MaterialIndex = mat->SrvHeapIndex;

			renderInfo->meshInfo = mesh;

			m_renderList[i].push_back(renderInfo.get());
			m_renderObjectInfos.push_back(std::move(renderInfo));
		}
	}
}

void DK::Testing::BuildRootSignature()
{
	//EngineBase::BuildRootSignature();

	/*
	root parameter에 연결할 수 있는 타입
	1. Constants(셰이더 상수)
	2. Descriptor
		- Constant Buffer View
		- Shader Resource View
		- Unordered Access View
	3. Descriptor Table
		- CBV, SRV, UAV table
	*/

	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	int textureCount = m_textures.size();
	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, textureCount, 1, 0);

	// 5개의 리소스 사용
	CD3DX12_ROOT_PARAMETER rootParameter[5];
	ZeroMemory(rootParameter, sizeof(CD3DX12_ROOT_PARAMETER) * 5);

	// const buffer view 2개
	rootParameter[0].InitAsConstantBufferView(0);
	rootParameter[1].InitAsConstantBufferView(1);
	// shader resource view 1개
	rootParameter[2].InitAsShaderResourceView(0, 1);
	// descriptor table 2개
	rootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> sampler = Texture::GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, rootParameter, 
		(UINT)sampler.size(), sampler.data(), 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());

	THROW_IF_FAILED(hr);

	m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature));
}

void DK::Testing::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// opaque pso
	psoDesc.InputLayout = { m_inputLayouts.data(), (UINT)m_inputLayouts.size() };
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

	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Opaque])));

	// PSO for sky.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = psoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = m_rootSignature.Get();
	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["skyVS"]->GetBufferPointer()),
		m_shaders["skyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["skyPS"]->GetBufferPointer()),
		m_shaders["skyPS"]->GetBufferSize()
	};
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Sky])));
}
