#include "Testing.h"

DK::Testing::Testing(HWND hWnd) : EngineBase(hWnd)
{
}

DK::Testing::~Testing()
{
}

void DK::Testing::Init()
{
	EngineBase::Init();
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

	if (m_renderList[(int)RenderLayer::Opaque].size() > 0)
		RenderRenderItems(cmdList, m_renderList[(int)RenderLayer::Opaque]);

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
	LoadTexture(L"Textures/white1x1.dds");
	LoadTexture(L"Textures/default_nmap.dds");

	LoadTexture(L"Textures/bricks.dds");
	LoadTexture(L"Textures/bricks_nmap.dds");

	LoadTexture(L"Textures/bricks2.dds");
	LoadTexture(L"Textures/bricks2_nmap.dds");

	LoadTexture(L"Textures/tile.dds");
	LoadTexture(L"Textures/tile_nmap.dds");

	LoadTexture(L"Textures/grasscube1024.dds");
}

void DK::Testing::CreateMesh()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> box = geoGen.CreateBox(1, 1, 1);
	MeshData<Vertex> grid = geoGen.CreateGrid(40, 40, 20, 20);

	MeshData<Vertex> skull = LoadMeshData(L"Models/skull.txt");
	MeshData<Vertex> car = LoadMeshData(L"Models/car.txt");

	MeshManager::GetInstance()->AddMeshData("box", box);
	MeshManager::GetInstance()->AddMeshData("grid", grid);
	MeshManager::GetInstance()->AddMeshData("skull", skull);
	MeshManager::GetInstance()->AddMeshData("car", car);

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
}

DK::MeshData<DK::Vertex> DK::Testing::LoadMeshData(const std::wstring& fileName)
{
	std::ifstream fin(fileName);

	if (!fin)
	{
		wstring errorMsg = fileName + L" 파일을 찾을 수 없습니다.";
		MessageBox(0, errorMsg.c_str(), 0, 0);
		return MeshData<Vertex>();
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 vMaxf3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	MeshData<Vertex> meshData;
	meshData.Vertices.resize(vcount);

	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> meshData.Vertices[i].Position.x >> meshData.Vertices[i].Position.y >> meshData.Vertices[i].Position.z;
		fin >> meshData.Vertices[i].Normal.x >> meshData.Vertices[i].Normal.y >> meshData.Vertices[i].Normal.z;

		meshData.Vertices[i].TexC = { 0.0f, 0.0f };

		XMVECTOR P = XMLoadFloat3(&meshData.Vertices[i].Position);
		XMVECTOR N = XMLoadFloat3(&meshData.Vertices[i].Normal);

		// Generate a tangent vector so normal mapping works.  We aren't applying
		// a texture map to the skull, so we just need any tangent vector so that
		// the math works out to give us the original interpolated vertex normal.
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
		{
			XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
			XMStoreFloat3(&meshData.Vertices[i].TangentU, T);
		}
		else
		{
			up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
			XMStoreFloat3(&meshData.Vertices[i].TangentU, T);
		}

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, XMVectorScale(XMVectorAdd(vMin, vMax), 0.5f));
	XMStoreFloat3(&bounds.Extents, XMVectorScale(XMVectorSubtract(vMax, vMin), 0.5f));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	meshData.Indices32.resize(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> meshData.Indices32[i * 3 + 0] >> meshData.Indices32[i * 3 + 1] >> meshData.Indices32[i * 3 + 2];
	}

	fin.close();

	return meshData;
}

void DK::Testing::CreateMaterial()
{
	Texture* texture = nullptr;
	Texture* normalTex = nullptr;

	// brick material
	texture = GetTexture("tile");
	normalTex = GetTexture("tile_nmap");

	auto tile = std::make_unique<Material>();
	tile->Name = "tile";
	tile->SrvIndex = 0;
	tile->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	tile->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	tile->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	tile->Roughness = 0.3f;
	DirectX::XMStoreFloat4x4(&tile->MatTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));

	// bricks material
	texture = GetTexture("bricks");
	normalTex = GetTexture("bricks_nmap");

	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->SrvIndex = 1;
	bricks->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	bricks->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks->Roughness = 0.3f;

	// tile material
	texture = GetTexture("bricks2");
	normalTex = GetTexture("bricks2_nmap");

	auto bricks2 = std::make_unique<Material>();
	bricks2->Name = "bricks2";
	bricks2->SrvIndex = 2;
	bricks2->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	bricks2->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	bricks2->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks2->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks2->Roughness = 0.3f;

	// sky material
	texture = GetTexture("grasscube1024");
	normalTex = nullptr;

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->SrvIndex = 3;
	sky->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	sky->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	m_materials[tile->Name] = std::move(tile);
	m_materials[bricks->Name] = std::move(bricks);
	m_materials[bricks2->Name] = std::move(bricks2);
	m_materials[sky->Name] = std::move(sky);
}

void DK::Testing::CreateGameObject()
{
	GameObject* box = new GameObject();
	box->AddComponent(new MeshFilter("box"));
	box->SetMaterial(m_materials["bricks2"].get());
	box->GetComponent<Transform>()->SetPosition(0, 5, 0);
	box->GetComponent<Transform>()->SetScale(5, 5, 5);

	m_gameObjects[(int)RenderLayer::Opaque].push_back(box);

	GameObject* grid = new GameObject();
	grid->AddComponent(new MeshFilter("grid"));
	grid->SetMaterial(m_materials["tile"].get());
	grid->GetComponent<Transform>()->SetScale(5, 1, 5);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(grid);

	GameObject* sky = new GameObject();
	sky->AddComponent(new MeshFilter("box"));
	sky->SetMaterial(m_materials["sky"].get());
	Transform* trans = sky->GetComponent<Transform>();
	trans->SetScale(1000, 1000, 1000);

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
			if (mat != nullptr) renderInfo->MaterialIndex = mat->SrvIndex;

			renderInfo->meshInfo = mesh;

			m_renderList[i].push_back(renderInfo.get());
			m_renderObjectInfos.push_back(std::move(renderInfo));
		}
	}
}

void DK::Testing::BuildRootSignature()
{
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
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0);

	int textureCount = m_textures.size() - 1;	// 큐브맵 제외
	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, textureCount, 2, 0);

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

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> sampler = Texture::GetStaticSamplers();

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

	// pso for shadow
	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = psoDesc;
	shadowPsoDesc.pRootSignature = m_rootSignature.Get();
	shadowPsoDesc.RasterizerState.DepthBias = 100000;
	shadowPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	shadowPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	shadowPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["shadowVS"]->GetBufferPointer()),
		m_shaders["shadowVS"]->GetBufferSize()
	};
	shadowPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["shadowOpaquePS"]->GetBufferPointer()),
		m_shaders["shadowOpaquePS"]->GetBufferSize()
	};
	shadowPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	shadowPsoDesc.NumRenderTargets = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Shadow])));

	// pso for sky.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = psoDesc;

	// The camera is inside the sky sphere, so just turn  culling.
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = psoDesc;
	drawNormalsPsoDesc.VS = 
	{
		reinterpret_cast<BYTE*>(m_shaders["drawNormalsVS"]->GetBufferPointer()),
		m_shaders["drawNormalsVS"]->GetBufferSize()
	};
	drawNormalsPsoDesc.PS = 
	{
		reinterpret_cast<BYTE*>(m_shaders["drawNormalsPS"]->GetBufferPointer()),
		m_shaders["drawNormalsPS"]->GetBufferSize()
	};
	drawNormalsPsoDesc.RTVFormats[0] = SsaoMap::NormalMapFormat;
	drawNormalsPsoDesc.SampleDesc.Count = 1;
	drawNormalsPsoDesc.SampleDesc.Quality = 0;
	drawNormalsPsoDesc.DSVFormat = m_eDepthStencilFormat;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&drawNormalsPsoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::DrawNormals])));

	// TODO: 아래의 PSO 분석 필요
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = psoDesc;
	ssaoPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["ssaoVS"]->GetBufferPointer()),
		m_shaders["ssaoVS"]->GetBufferSize()
	};
	ssaoPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["ssaoPS"]->GetBufferPointer()),
		m_shaders["ssaoPS"]->GetBufferSize()
	};
	ssaoPsoDesc.DepthStencilState.DepthEnable = false;
	ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	ssaoPsoDesc.RTVFormats[0] = SsaoMap::AmbientMapFormat;
	ssaoPsoDesc.SampleDesc.Count = 1;
	ssaoPsoDesc.SampleDesc.Quality = 0;
	ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Ssao])));
}

void DK::Testing::RenderCubeMap(ID3D12GraphicsCommandList* cmdList, int i)
{
	UINT passCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(RenderPassConstants));

	auto passCB = m_curFrameResource->RenderPassCB->GetBuffer();
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + (1 + i) * passCBByteSize;
	cmdList->SetGraphicsRootConstantBufferView(1, passCBAddress);

	RenderRenderItems(cmdList, m_renderList[(int)RenderLayer::Opaque]);

	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Sky].Get());
	RenderRenderItems(cmdList, m_renderList[(int)RenderLayer::Sky]);

	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Opaque].Get());
}
