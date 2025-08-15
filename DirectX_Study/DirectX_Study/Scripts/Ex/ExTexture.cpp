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

	{
		m_gizmo.Init(m_d3dDevice.Get(), m_commandList.Get(), m_nFrameReesourceCount, m_eBackBufferFormat, m_eDepthStencilFormat);
	}

	LoadTexture();
	CreateGeometry();		// mesh, buffer 积己
	CreateMaterial();		// material 积己
	CreateGameObject();		// 免仿 go 积己
	CreateFrameResource();	// const buffer 殿 积己

	BuildDescriptor();
	BuildInputLayoutAndShader();
	BuildRootSignature();
	BuildPSO();
}

void DK::ExTexture::CreateGeometry()
{
	GeometryGenerator geoGen;

	// create mesh data
	std::unique_ptr<MeshBuffer<Vertex>> meshBuffer = std::make_unique<MeshBuffer<Vertex>>();

	// add mesh data
	MeshData<Vertex> meshBox = geoGen.CreateBox(1, 1, 1);
	meshBox.Name = "box";
	meshBuffer->AddMeshData(meshBox.Name, meshBox);

	MeshData<Vertex> meshGrid = geoGen.CreateGrid(160, 160, 50, 50);
	meshGrid.Name = "grid";

	std::vector<Vertex> vecVertex;
	for (int i = 0; i < meshGrid.Vertices.size(); ++i)
	{
		Vertex vertex = meshGrid.Vertices[i];

		DirectX::XMFLOAT3 pos = meshGrid.Vertices[i].Position;
		pos.y = 0.3f * (pos.z * sinf(0.1f * pos.x) + pos.x * cosf(0.1f * pos.z));

		vertex.Position = pos;
		vertex.Normal = GetHillNormal(vertex.Position.x, vertex.Position.z);

		vecVertex.push_back(vertex);
	}

	meshBuffer->AddMeshData(meshGrid.Name, vecVertex, meshGrid.GetIndices16());

	meshBuffer->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());

	// add buffer
	m_vecMeshBuffers.push_back(std::move(meshBuffer));


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

	std::unique_ptr<MeshBuffer<Vertex>> meshWave = std::make_unique<MeshBuffer<Vertex>>();

	meshWave->IndexBuffer = D3DUtils::CreateDefaultBuffer(m_d3dDevice.Get(), m_commandList.Get(), indices.data(), ibByteSize, meshWave->IndexUploadBuffer);

	meshWave->VertexByteStride = sizeof(Vertex);
	meshWave->VertexBufferByteSize = vbByteSize;
	meshWave->IndexBufferByteSize = ibByteSize;
	meshWave->IndexFormat = DXGI_FORMAT_R16_UINT;

	MeshSection section;
	section.IndexCount = (UINT)indices.size();
	section.StartIndexLocation = 0;
	section.BaseVertexLocation = 0;

	meshWave->MeshSections["wave"] = section;
	m_vecMeshBuffers.push_back(std::move(meshWave));
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
	matWood->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matGrass = std::make_unique<Material>();
	matGrass->Name = "grass";
	matGrass->MatCBIndex = 1;
	matGrass->DiffuseSrvHeapIndex = 1;
	matGrass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	matGrass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	matGrass->Roughness = 0.125f;
	matGrass->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matWater = std::make_unique<Material>();
	matWater->Name = "water";
	matWater->MatCBIndex = 2;
	matWater->DiffuseSrvHeapIndex = 2;
	matWater->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f);
	matWater->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	matWater->Roughness = 0.0f;
	matWater->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matWirefence = std::make_unique<Material>();
	matWirefence->Name = "wirefence";
	matWirefence->MatCBIndex = 1;
	matWirefence->DiffuseSrvHeapIndex = 0;
	matWirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	matWirefence->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	matWirefence->Roughness = 0.25f;
	matWirefence->NumFramesDirty = m_nFrameReesourceCount;

	m_mapMaterials[matWood->Name] = std::move(matWood);
	m_mapMaterials[matGrass->Name] = std::move(matGrass);
	m_mapMaterials[matWater->Name] = std::move(matWater);
	m_mapMaterials[matWirefence->Name] = std::move(matWirefence);
}

void DK::ExTexture::CreateGameObject()
{
	MeshBuffer<Vertex>* pMeshBuffer = m_vecMeshBuffers[0].get();
	MeshSection section;
	RenderItem renderItem;

	// box
	GameObject* goBox = new GameObject();

	if (pMeshBuffer->GetMeshSection("box", section))
		goBox->SetMeshData(pMeshBuffer, section);
	goBox->SetMaterial(m_mapMaterials["wirefence"].get());
	goBox->GetTransform().SetScale(3, 3, 3);

	m_vecGameObjects.push_back(goBox);

	renderItem.pGameObject = goBox;
	renderItem.NumFramesDirty = m_nFrameReesourceCount;
	renderItem.nCBIndex = 0;
	m_vecRenderItem.push_back(renderItem);

	// grid
	GameObject* goGrid = new GameObject();

	if (pMeshBuffer->GetMeshSection("grid", section))
		goGrid->SetMeshData(pMeshBuffer, section);
	goGrid->SetMaterial(m_mapMaterials["grass"].get());

	m_vecGameObjects.push_back(goGrid);

	renderItem.pGameObject = goGrid;
	XMStoreFloat4x4(&renderItem.TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	renderItem.nCBIndex = 1;

	m_vecRenderItem.push_back(renderItem);

	// water
	pMeshBuffer = m_vecMeshBuffers[1].get();
	GameObject* goWater = new GameObject();

	if (pMeshBuffer->GetMeshSection("wave", section))
		goWater->SetMeshData(pMeshBuffer, section);
	goWater->SetMaterial(m_mapMaterials["water"].get());

	m_pGoWave = goWater;
	m_vecGameObjects.push_back(goWater);

	renderItem.pGameObject = goWater;
	renderItem.nCBIndex = 1;
	XMStoreFloat4x4(&renderItem.TexTransform, XMMatrixScaling(10.0f, 10.0f, 1.0f));
	m_vecRenderItemTransparent.push_back(renderItem);
}

void DK::ExTexture::CreateFrameResource()
{
	for (int i = 0; i < m_nFrameReesourceCount; ++i)
	{
		m_vecFrameResoruce.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, m_vecGameObjects.size(), m_mapMaterials.size(), m_waves->VertexCount()));
	}
}

void DK::ExTexture::LoadTexture()
{
	LoadTexture(L"Textures/WireFence.dds");
	LoadTexture(L"Textures/grass.dds");
	LoadTexture(L"Textures/water1.dds");
}

void DK::ExTexture::LoadTexture(std::wstring path)
{
	size_t firstIdx = path.rfind(L'/') + 1;
	size_t lastIdx = path.rfind(L'.');
	std::string name = WStringToAnsi(path.substr(firstIdx, lastIdx - firstIdx));

	std::unique_ptr<Texture> spTexture = std::make_unique<Texture>();

	spTexture->FileName = path;

	DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_commandList.Get(), spTexture->FileName.c_str(),
		spTexture->Resource, spTexture->UploadHeap);

	m_mapTextures[name] = std::move(spTexture);
}

void DK::ExTexture::BuildDescriptor()
{
	int nTextureCount = m_mapTextures.size();

	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = nTextureCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_spHeapSRV)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_spHeapSRV->GetCPUDescriptorHandleForHeapStart());

	for(auto iter = m_mapTextures.begin(); iter != m_mapTextures.end(); ++iter)
	{
		Texture* texture = iter->second.get();
		D3D12_RESOURCE_DESC rscDesc = texture->Resource->GetDesc();

		// texture2D俊 措茄 view 积己
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = rscDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = rscDesc.MipLevels;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		m_d3dDevice->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, descriptorHandle);
		descriptorHandle.ptr += m_uCbvSrvUavDescriptorSize;
	}
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
	descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_mapTextures.size(), 0);

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

	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_mapPSO["std"]));

	// alpha test
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_mapPSO["alphatest"]));

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	// ** blend pso
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.LogicOpEnable = false;
	rtBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_mapPSO["transparent"]));
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

	AnimateMaterials(m_gameTimer);
	UpdateWave(m_gameTimer);
	UpdateRenderPassCB();
	UpdateObjectCBs();
	UpdateMaterialCBs();

	{
		m_gizmo.Update(&m_camera);
	}

	return true;
}

void DK::ExTexture::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat =  m_mapMaterials["water"].get();

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

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = m_nFrameReesourceCount;
}

void DK::ExTexture::UpdateWave(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((m_gameTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathUtils::Rand(4, m_waves->RowCount() - 5);
		int j = MathUtils::Rand(4, m_waves->ColumnCount() - 5);

		float r = MathUtils::RandF(0.2f, 0.5f);

		m_waves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	m_waves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = m_pCurrFrameResource->WaveVB.get();
	for (int i = 0; i < m_waves->VertexCount(); ++i)
	{
		Vertex v;
		v.Position = m_waves->Position(i);
		v.Normal = m_waves->Normal(i);

		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexC.x = 0.5f + v.Position.x / m_waves->Width();
		v.TexC.y = 0.5f - v.Position.z / m_waves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	m_pGoWave->GetMeshBuffer()->VertexBuffer = currWavesVB->GetBuffer();
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

	m_renderPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_renderPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };

	m_renderPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_renderPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

	m_renderPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_renderPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = m_pCurrFrameResource->RenderPassCB.get();
	currPassCB->CopyData(0, m_renderPassCB);
}

void DK::ExTexture::UpdateObjectCBs()
{
	auto objectCB = m_pCurrFrameResource->ObjectCB.get();

	int objCount = m_vecRenderItem.size();
	for (int i = 0; i < objCount; ++i)
	{
		DirectX::XMFLOAT4X4 worldMatrix = m_vecRenderItem[i].pGameObject->GetTransform().GetWorldMatrix();
		DirectX::XMFLOAT4X4 texTransform = m_vecRenderItem[i].TexTransform;

		XMMATRIX world = XMLoadFloat4x4(&worldMatrix);
		XMMATRIX vTexTransform = XMLoadFloat4x4(&texTransform);

		ObjectConstants constants;
		XMStoreFloat4x4(&constants.WorldMatrix, XMMatrixTranspose(world));
		XMStoreFloat4x4(&constants.TexTransform, XMMatrixTranspose(vTexTransform));

		objectCB->CopyData(i, constants);
	}
}

void DK::ExTexture::UpdateMaterialCBs()
{
	auto meterialCB = m_pCurrFrameResource->MaterialCB.get();

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
		XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

		meterialCB->CopyData(mat->MatCBIndex, matConstants);
	}
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

	{
		m_gizmo.PreRender(m_commandList.Get());
	}

	m_commandList->SetGraphicsRootSignature(m_spRootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_spHeapSRV.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	auto passCB = m_pCurrFrameResource->RenderPassCB->GetBuffer();
	m_commandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	m_commandList->SetPipelineState(m_mapPSO["std"].Get());
	DrawRenderItem(m_commandList.Get(), m_vecRenderItem[1]);

	m_commandList->SetPipelineState(m_mapPSO["alphatest"].Get());
	DrawRenderItem(m_commandList.Get(), m_vecRenderItem[0]);

	m_commandList->SetPipelineState(m_mapPSO["transparent"].Get());

	for (size_t i = 0; i < m_vecRenderItemTransparent.size(); ++i)
		DrawRenderItem(m_commandList.Get(), m_vecRenderItemTransparent[i]);

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
	/*UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();*/

	for (size_t i = 0; i < m_vecRenderItem.size(); ++i)
	{
		DrawRenderItem(cmdList, m_vecRenderItem[i]);

		/*GameObject* pGO = m_vecRenderItem[i].pGameObject;

		auto viewVB = pGO->GetMeshBuffer()->VertexBufferView();
		auto viewIB = pGO->GetMeshBuffer()->IndexBufferView();

		cmdList->IASetVertexBuffers(0, 1, &viewVB);
		cmdList->IASetIndexBuffer(&viewIB);
		cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_spHeapSRV->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(pGO->GetMaterial()->DiffuseSrvHeapIndex, m_uCbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * i);

		int matCBIndex = pGO->GetMaterial()->MatCBIndex;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBIndex * matCBByteSize);

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		MeshSection meshSection = pGO->GetMeshSection();
		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);*/
	}
}

void DK::ExTexture::DrawRenderItem(ID3D12GraphicsCommandList* cmdList, RenderItem renderItem)
{
	UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();

	GameObject* pGO = renderItem.pGameObject;

	auto viewVB = pGO->GetMeshBuffer()->VertexBufferView();
	auto viewIB = pGO->GetMeshBuffer()->IndexBufferView();

	cmdList->IASetVertexBuffers(0, 1, &viewVB);
	cmdList->IASetIndexBuffer(&viewIB);
	cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_spHeapSRV->GetGPUDescriptorHandleForHeapStart());
	tex.Offset(pGO->GetMaterial()->DiffuseSrvHeapIndex, m_uCbvSrvUavDescriptorSize);

	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * renderItem.nCBIndex);

	int matCBIndex = pGO->GetMaterial()->MatCBIndex;
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBIndex * matCBByteSize);

	cmdList->SetGraphicsRootDescriptorTable(0, tex);
	cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
	cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	MeshSection meshSection = pGO->GetMeshSection();
	cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
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
