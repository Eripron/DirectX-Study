#include "Testing.h"

DK::Testing::Testing(HWND hWnd) : EngineBase(hWnd)
{
	DefineAnimationKeyFrames();

	m_camera.GetTransform().SetPosition(0, 10, -8);
}

DK::Testing::~Testing()
{
}

bool DK::Testing::Update()
{
	_accumTime += m_gameTimer.DeltaTime();

	int count = _boneObjects.size();
	for (int i = 0; i < count; ++i)
	{
		XMMATRIX finalAnimation = XMMatrixIdentity();

		// 애니메이션이 정의되지 않은 본은 건너뛰기
		if (_boneAnimations.find(i) != _boneAnimations.end())
		{
			for (int j = 0; j < _boneAnimations[i].size(); ++j)
			{
				BoneAnimation& animation = _boneAnimations[i][j];

				XMFLOAT4X4 M;
				UpdateAnimation(animation, m_gameTimer.DeltaTime(), M);

				XMMATRIX matrix = XMLoadFloat4x4(&M);
				finalAnimation = XMMatrixMultiply(finalAnimation, matrix);
			}
		}

		RenderObjectInfo* objectInfo = _boneObjects[i];
		XMMATRIX baseWorld = XMLoadFloat4x4(&objectInfo->BaseWorld);

		XMMATRIX localWorld = XMMatrixMultiply(baseWorld, finalAnimation);

		// 공전 위치 계산
		if (_planetRadius[i] > 0)
		{
			XMMATRIX T = XMMatrixTranslation(_planetRadius[i], 0, 0);

			XMVECTOR R = XMQuaternionRotationAxis(XMVectorSet(0, -1, 0, 0), _accumTime * _orbitSpeed[i]);
			R = XMQuaternionNormalize(R);

			XMMATRIX Rm = XMMatrixRotationQuaternion(R);

			XMMATRIX Orbit =  T * Rm;

			int parentIndex = _boneIndexing[i];
			if (parentIndex >= 0)
			{
				XMMATRIX parent = _parentWorlds[parentIndex];

				parent.r[0] = XMVector3Normalize(parent.r[0]);
				parent.r[1] = XMVector3Normalize(parent.r[1]);
				parent.r[2] = XMVector3Normalize(parent.r[2]);

				Orbit = Orbit * parent;

				_parentWorlds[i] = Orbit;	// 공전 위치 저장
			}

			localWorld = XMMatrixMultiply(localWorld, Orbit);	// 공전 위치 적용
		}

		XMVECTOR pos = localWorld.r[3];
		_parentWorlds[i] = XMMatrixTranslationFromVector(pos);

		XMStoreFloat4x4(&objectInfo->World, localWorld);
	}

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
	LoadTexture(L"Textures/bricks2.dds");
	LoadTexture(L"Textures/bricks2_nmap.dds");

	LoadTexture(L"Textures/tile.dds");
	LoadTexture(L"Textures/tile_nmap.dds");

	LoadTexture(L"Textures/universe/sun.dds");
	LoadTexture(L"Textures/universe/mercury.dds");
	LoadTexture(L"Textures/universe/venus.dds");
	LoadTexture(L"Textures/universe/earth.dds");
	LoadTexture(L"Textures/universe/mars.dds");
	LoadTexture(L"Textures/universe/jupiter.dds");
	LoadTexture(L"Textures/universe/saturn.dds");
	LoadTexture(L"Textures/universe/uranus.dds");
	LoadTexture(L"Textures/universe/moon.dds");

	LoadTexture(L"Textures/default_nmap.dds");

	LoadTexture(L"Textures/grasscube1024.dds");
}

void DK::Testing::CreateMesh()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> box = geoGen.CreateBox(1, 1, 1);
	MeshManager::GetInstance()->AddMeshData("box", box);

	MeshData<Vertex> sphere = geoGen.CreateSphere(1, 30, 30);
	MeshManager::GetInstance()->AddMeshData("sphere", sphere);

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
}

void DK::Testing::CreateMaterial()
{
	int matIndex = 0;
	Texture* texture = nullptr;
	Texture* normalTex = nullptr;

	texture = GetTexture("bricks2");
	normalTex = GetTexture("bricks2_nmap");

	auto brick = std::make_unique<Material>();
	brick->Name = "bricks2";
	brick->SrvIndex = matIndex++;
	brick->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	brick->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	brick->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	brick->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	brick->Roughness = 0.3f;

	// 태양
	texture = GetTexture("sun");
	normalTex = GetTexture("default_nmap");

	auto sun = std::make_unique<Material>();
	sun->Name = "sun";
	sun->SrvIndex = matIndex++;
	sun->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	sun->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;

	// 수성
	texture = GetTexture("mercury");

	auto mercury = std::make_unique<Material>();
	mercury->Name = "mercury";
	mercury->SrvIndex = matIndex++;
	mercury->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	mercury->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	mercury->DiffuseAlbedo = XMFLOAT4(0.75f, 0.75f, 0.72f, 1.0f);
	mercury->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	mercury->Roughness = 0.9f;

	// 금성
	texture = GetTexture("venus");

	auto venus = std::make_unique<Material>();
	venus->Name = "venus";
	venus->SrvIndex = matIndex++;
	venus->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;	
	venus->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	venus->DiffuseAlbedo = XMFLOAT4(0.95f, 0.85f, 0.55f, 1.0f);
	venus->FresnelR0 = XMFLOAT3(0.15f, 0.15f, 0.15f);
	venus->Roughness = 0.6f;

	// 지구
	texture = GetTexture("earth");

	auto earth = std::make_unique<Material>();
	earth->Name = "earth";
	earth->SrvIndex = matIndex++;
	earth->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	earth->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	earth->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	earth->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	earth->Roughness = 0.25f;

	// 화성
	texture = GetTexture("mars");

	auto mars = std::make_unique<Material>();
	mars->Name = "mars";
	mars->SrvIndex = matIndex++;
	mars->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	mars->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	mars->DiffuseAlbedo = XMFLOAT4(0.8f, 0.45f, 0.3f, 1.0f);
	mars->FresnelR0 = XMFLOAT3(0.03f, 0.03f, 0.03f);
	mars->Roughness = 0.85f;

	// 목성
	texture = GetTexture("jupiter");

	auto jupiter = std::make_unique<Material>();
	jupiter->Name = "jupiter";
	jupiter->SrvIndex = matIndex++;
	jupiter->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	jupiter->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	jupiter->DiffuseAlbedo = XMFLOAT4(1.0f, 0.9f, 0.8f, 1.0f);
	jupiter->FresnelR0 = XMFLOAT3(0.12f, 0.12f, 0.12f);
	jupiter->Roughness = 0.5f;
	
	// 토성
	texture = GetTexture("saturn");

	auto saturn = std::make_unique<Material>();
	saturn->Name = "saturn";
	saturn->SrvIndex = matIndex++;
	saturn->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	saturn->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	saturn->DiffuseAlbedo = XMFLOAT4(0.95f, 0.9f, 0.7f, 1.0f);
	saturn->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	saturn->Roughness = 0.55f;

	// 천왕성
	texture = GetTexture("uranus");

	auto uranus = std::make_unique<Material>();
	uranus->Name = "uranus";
	uranus->SrvIndex = matIndex++;
	uranus->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	uranus->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	uranus->DiffuseAlbedo = XMFLOAT4(0.7f, 0.9f, 0.95f, 1.0f);
	uranus->FresnelR0 = XMFLOAT3(0.08f, 0.08f, 0.08f);
	uranus->Roughness = 0.4f;

	// 달
	texture = GetTexture("moon");

	auto moon = std::make_unique<Material>();
	moon->Name = "moon";
	moon->SrvIndex = matIndex++;
	moon->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	moon->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	moon->DiffuseAlbedo = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	moon->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
	moon->Roughness = 0.9f;


	texture = GetTexture("grasscube1024");
	normalTex = nullptr;

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->SrvIndex = matIndex++;
	sky->BaseColorTextureIndex = texture == nullptr ? -1 : texture->SrvHeapIndex;
	sky->NormalTextureIndex = normalTex == nullptr ? -1 : normalTex->SrvHeapIndex;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	m_materials[brick->Name] = std::move(brick);
	m_materials[sun->Name] = std::move(sun);
	m_materials[mercury->Name] = std::move(mercury);
	m_materials[venus->Name] = std::move(venus);
	m_materials[earth->Name] = std::move(earth);
	m_materials[mars->Name] = std::move(mars);
	m_materials[jupiter->Name] = std::move(jupiter);
	m_materials[saturn->Name] = std::move(saturn);
	m_materials[uranus->Name] = std::move(uranus);
	m_materials[moon->Name] = std::move(moon);
	m_materials[sky->Name] = std::move(sky);
}

void DK::Testing::CreateGameObject()
{
	// 태양
	GameObject* sun = new GameObject();
	sun->AddComponent(new MeshFilter("sphere"));
	sun->GetComponent<Transform>()->SetPosition(0, 3, 0);
	sun->SetMaterial(m_materials["sun"].get());
	sun->GetComponent<Transform>()->SetScale(1.0f, 1.0f, 1.0f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(sun);

	// 수성
	GameObject* mercury = new GameObject();
	mercury->AddComponent(new MeshFilter("sphere"));
	mercury->GetComponent<Transform>()->SetPosition(0, 0, 0);
	mercury->SetMaterial(m_materials["mercury"].get());
	mercury->GetComponent<Transform>()->SetScale(0.15f, 0.15f, 0.15f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(mercury);

	// 금성
	GameObject* venus = new GameObject();
	venus->AddComponent(new MeshFilter("sphere"));
	venus->GetComponent<Transform>()->SetPosition(0, 0, 0);
	venus->SetMaterial(m_materials["venus"].get());
	venus->GetComponent<Transform>()->SetScale(0.25f, 0.25f, 0.25f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(venus);

	// 지구
	GameObject* earth = new GameObject();
	earth->AddComponent(new MeshFilter("sphere"));
	earth->GetComponent<Transform>()->SetPosition(0, 0, 0);
	earth->SetMaterial(m_materials["earth"].get());
	earth->GetComponent<Transform>()->SetScale(0.4f, 0.4f, 0.4f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(earth);

	// 화성
	GameObject* mars = new GameObject();
	mars->AddComponent(new MeshFilter("sphere"));
	mars->GetComponent<Transform>()->SetPosition(0, 0, 0);
	mars->SetMaterial(m_materials["mars"].get());
	mars->GetComponent<Transform>()->SetScale(0.36f, 0.36f, 0.36f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(mars);

	// 목성
	GameObject* jupiter = new GameObject();
	jupiter->AddComponent(new MeshFilter("sphere"));
	jupiter->GetComponent<Transform>()->SetPosition(0, 0, 0);
	jupiter->SetMaterial(m_materials["jupiter"].get());
	jupiter->GetComponent<Transform>()->SetScale(0.8f, 0.8f, 0.8f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(jupiter);

	// 토성
	GameObject* saturn = new GameObject();
	saturn->AddComponent(new MeshFilter("sphere"));
	saturn->GetComponent<Transform>()->SetPosition(0, 0, 0);
	saturn->SetMaterial(m_materials["saturn"].get());
	saturn->GetComponent<Transform>()->SetScale(0.7f, 0.7f, 0.7f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(saturn);

	// 천왕성
	GameObject* uranus = new GameObject();
	uranus->AddComponent(new MeshFilter("sphere"));
	uranus->GetComponent<Transform>()->SetPosition(0, 0, 0);
	uranus->SetMaterial(m_materials["uranus"].get());
	uranus->GetComponent<Transform>()->SetScale(0.6f, 0.6f, 0.6f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(uranus);

	// 달
	GameObject* moon = new GameObject();
	moon->AddComponent(new MeshFilter("sphere"));
	moon->GetComponent<Transform>()->SetPosition(0, 0, 0);
	moon->SetMaterial(m_materials["moon"].get());
	moon->GetComponent<Transform>()->SetScale(0.1f, 0.1f, 0.1f);
	m_gameObjects[(int)RenderLayer::Opaque].push_back(moon);

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
			renderInfo->BaseWorld = transform->GetWorldMatrix();
			renderInfo->World = transform->GetWorldMatrix();

			Material* mat = object->GetMaterial();
			if (mat != nullptr) renderInfo->MaterialIndex = mat->SrvIndex;

			renderInfo->meshInfo = mesh;

			m_renderList[i].push_back(renderInfo.get());
			m_renderObjectInfos.push_back(std::move(renderInfo));
		}
	}

	auto list = m_renderList[(int)RenderLayer::Opaque];

	_boneObjects.push_back(list[0]);	// 태양
	_boneObjects.push_back(list[1]);	// 수성
	_boneObjects.push_back(list[2]);	// 금성
	_boneObjects.push_back(list[3]);	// 지구
	_boneObjects.push_back(list[4]);	// 화성
	_boneObjects.push_back(list[5]);	// 목성
	_boneObjects.push_back(list[6]);	// 토성
	_boneObjects.push_back(list[7]);	// 천왕성
	_boneObjects.push_back(list[8]);	// 달

	_boneIndexing.push_back(-1);	// 태양은 부모가 없으므로 -1
	_boneIndexing.push_back(0);	// 수성
	_boneIndexing.push_back(0);	// 금성
	_boneIndexing.push_back(0);	// 지구
	_boneIndexing.push_back(0);	// 화성
	_boneIndexing.push_back(0);	// 목성
	_boneIndexing.push_back(0);	// 토성
	_boneIndexing.push_back(0);	// 천왕성
	_boneIndexing.push_back(3);	// 달은 지구의 자식

	_planetRadius.push_back(0);		// 태양
	_planetRadius.push_back(2.0f);	// 수성
	_planetRadius.push_back(3.0f);	// 금성
	_planetRadius.push_back(5.0f);	// 지구
	_planetRadius.push_back(7.0f);	// 화성
	_planetRadius.push_back(9.0f);	// 목성
	_planetRadius.push_back(11.5f);	// 토성
	_planetRadius.push_back(14.0f);	// 천왕성
	_planetRadius.push_back(1.0f);	// 달

	float m = 4.0f;
	_orbitSpeed.push_back(0.0f);   // 태양
	_orbitSpeed.push_back(4.15f / m);  // 수성
	_orbitSpeed.push_back(1.62f / m);  // 금성
	_orbitSpeed.push_back(1.0f / m);   // 지구
	_orbitSpeed.push_back(0.53f / m);  // 화성
	_orbitSpeed.push_back(0.3f / m); // 목성
	_orbitSpeed.push_back(0.2f / m); // 토성
	_orbitSpeed.push_back(0.15f / m); // 천왕성
	_orbitSpeed.push_back(4.0f / m);  // 달

	_parentWorlds.resize(_boneObjects.size());
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
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 20, 1, 0);

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

void DK::Testing::DefineAnimationKeyFrames()
{
	// 태양
	_boneAnimations[0].push_back(CreateSpinAnimation(40.0f));

	// 수성
	_boneAnimations[1].push_back(CreateSpinAnimation(20.0f));
	_boneAnimations[1].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 0.03f));
	//_boneAnimations[1].push_back(CreateOrbitAnimation(40.0f, 2.0f));

	// 금성
	_boneAnimations[2].push_back(CreateSpinAnimation(50.0f));
	_boneAnimations[2].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 3.4f));
	//_boneAnimations[2].push_back(CreateOrbitAnimation(70.0f, 4.0f));

	// 지구
	_boneAnimations[3].push_back(CreateSpinAnimation(30.0f));
	_boneAnimations[3].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 23.4f));
	//_boneAnimations[3].push_back(CreateOrbitAnimation(120.0f, 6.0f));

	// 화성
	_boneAnimations[4].push_back(CreateSpinAnimation(32.0f));
	_boneAnimations[4].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), 25.2f));	
	//_boneAnimations[4].push_back(CreateOrbitAnimation(180.0f, 10.0f));

	// 목성
	_boneAnimations[5].push_back(CreateSpinAnimation(12.0f));
	_boneAnimations[5].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 3.1f));
	//_boneAnimations[5].push_back(CreateOrbitAnimation(300.0f, 13.0f));

	// 토성
	_boneAnimations[6].push_back(CreateSpinAnimation(14.0f));
	_boneAnimations[6].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 26.7f));
	//_boneAnimations[6].push_back(CreateOrbitAnimation(420.0f, 17.0f));

	// 천왕성
	_boneAnimations[7].push_back(CreateSpinAnimation(22.0f));
	_boneAnimations[7].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), 97.8f));	
	//_boneAnimations[7].push_back(CreateOrbitAnimation(600.0f, 20.0f));

	// 달
	_boneAnimations[8].push_back(CreateSpinAnimation(30.0f));
	_boneAnimations[8].push_back(CreateTiltAnimation(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), 6.7f));
	//_boneAnimations[8].push_back(CreateOrbitAnimation(30.0f, 1.0f));

}

void DK::Testing::UpdateAnimation(BoneAnimation animation, float deltaTime, XMFLOAT4X4& M)
{
	int aniEndTime = (int)animation.GetEndTime();
	int count = _accumTime / aniEndTime;

	float aniTime = (_accumTime - (count * aniEndTime));
	animation.Interpolate(aniTime, M);
}

BoneAnimation DK::Testing::CreateOrbitAnimation(float time, float radius)
{
	BoneAnimation animation;

	int keyframeCount = 100;
	animation.Keyframes.resize(keyframeCount + 1);

	XMMATRIX pos = XMMatrixTranslation(radius, 0, 0);

	for (int i = 0; i <= keyframeCount; ++i)
	{
		float t = (float)i / keyframeCount;
		float timePos = t * time;

		float theta = XM_2PI * t;

		float x = radius * cosf(theta);
		float z = radius * sinf(theta);

		auto& key = animation.Keyframes[i];

		key.TimePos = timePos;

		XMVECTOR q = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), theta);
		XMStoreFloat4(&key.RotationQuat, q);

		key.Translation = XMFLOAT3(x, 0.0f, z);
	}

	return animation;
}

BoneAnimation DK::Testing::CreateTiltAnimation(XMVECTOR axis, float angle)
{
	BoneAnimation animation;

	animation.Keyframes.resize(1);

	auto& keyTilt = animation.Keyframes[0];
	XMVECTOR qTilt = XMQuaternionRotationAxis(axis, XMConvertToRadians(angle));
	XMStoreFloat4(&keyTilt.RotationQuat, qTilt);

	return animation;
}

BoneAnimation DK::Testing::CreateSpinAnimation(float time, int spinDir)
{
	BoneAnimation animation;

	int keyframeCount = 4;

	animation.Keyframes.resize(keyframeCount + 1);

	for (int i = 0; i <= keyframeCount; ++i)
	{
		float t = (float)i / keyframeCount;

		auto& keySpin = animation.Keyframes[i];

		keySpin.TimePos = t * time;

		XMVECTOR q = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(-360.0f * t * spinDir));
		XMStoreFloat4(&keySpin.RotationQuat, q);
	}

	return animation;
}
