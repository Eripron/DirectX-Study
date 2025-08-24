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
	CreateMaterial();
	CreateGeometry();
	CreateGameObject();
	CreateFrameResource();

	BuildDescriptor();
	BuildInputLayoutAndShader();
	BuildRootSignature();
	BuildPSO();
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
	UpdateReflectRenderPassCB();

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

		// Mark the visible mirror pixels in the stencil buffer with the value 1
		m_commandList->OMSetStencilRef(1);
		m_commandList->SetPipelineState(m_mapPSO["stencil"].Get());
		DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::Mirror]);

		// Draw the reflection into the mirror only (only for pixels where the stencil buffer is 1).
		// Note that we must supply a different per-pass constant buffer--one with the lights reflected.
		UINT passCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(RenderPassConstants));
		m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + passCBByteSize);
		m_commandList->SetPipelineState(m_mapPSO["stencilReflections"].Get());
		DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::Reflected]);

		//// Restore main pass constants and stencil ref.
		m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
		m_commandList->OMSetStencilRef(0);

		//// Draw mirror with transparency so reflection blends through.
		m_commandList->SetPipelineState(m_mapPSO["transparent"].Get());
		DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::Transparent]);

		//// Draw shadows
		//m_commandList->SetPipelineState(m_mapPSO["shadow"].Get());
		//DrawGameObjects(m_commandList.Get(), m_vecGameObjects[(int)RenderLayer::Shadow]);
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
	LoadTexture(L"Textures/bricks3.dds", 0);
	LoadTexture(L"Textures/checkboard.dds", 1);
	LoadTexture(L"Textures/ice.dds", 2);
	LoadTexture(L"Textures/white1x1.dds", 3);
}

void DK::ExTexture::LoadTexture(std::wstring path, int texCBIndex)
{
	size_t firstIdx = path.rfind(L'/') + 1;
	size_t lastIdx = path.rfind(L'.');
	std::string name = WStringToAnsi(path.substr(firstIdx, lastIdx - firstIdx));

	std::unique_ptr<Texture> spTexture = std::make_unique<Texture>();

	spTexture->FileName = path;
	spTexture->TexCBIndex = texCBIndex;

	DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_commandList.Get(), spTexture->FileName.c_str(),
		spTexture->Resource, spTexture->UploadHeap);

	m_mapTextures[name] = std::move(spTexture);
}

void DK::ExTexture::CreateMaterial()
{
	std::unique_ptr<Material> matBrick = std::make_unique<Material>();
	matBrick->Name = "bricks";
	matBrick->MatCBIndex = 0;
	matBrick->DiffuseSrvHeapIndex = m_mapTextures["bricks3"]->TexCBIndex;
	matBrick->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	matBrick->Roughness = 0.25f;
	matBrick->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matTile = std::make_unique<Material>();
	matTile->Name = "checkertile";
	matTile->MatCBIndex = 1;
	matTile->DiffuseSrvHeapIndex = m_mapTextures["checkboard"]->TexCBIndex;
	matTile->FresnelR0 = XMFLOAT3(0.07f, 0.07f, 0.07f);
	matTile->Roughness = 0.3f;
	matTile->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matIce = std::make_unique<Material>();
	matIce->Name = "ice";
	matIce->MatCBIndex = 2;
	matIce->DiffuseSrvHeapIndex = m_mapTextures["ice"]->TexCBIndex;
	matIce->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.25f);
	matIce->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	matIce->Roughness = 0.5f;
	matIce->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matSkull = std::make_unique<Material>();
	matSkull->Name = "skull";
	matSkull->MatCBIndex = 3;
	matSkull->DiffuseSrvHeapIndex = m_mapTextures["white1x1"]->TexCBIndex;
	matSkull->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	matSkull->Roughness = 0.3f;
	matSkull->NumFramesDirty = m_nFrameReesourceCount;

	std::unique_ptr<Material> matShadow = std::make_unique<Material>();
	matShadow->Name = "shadow";
	matShadow->MatCBIndex = 4;
	matShadow->DiffuseSrvHeapIndex = m_mapTextures["white1x1"]->TexCBIndex;;
	matShadow->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	matShadow->FresnelR0 = XMFLOAT3(0.001f, 0.001f, 0.001f);
	matShadow->Roughness = 0.0f;
	matShadow->NumFramesDirty = m_nFrameReesourceCount;

	m_mapMaterials[matBrick->Name] = std::move(matBrick);
	m_mapMaterials[matTile->Name] = std::move(matTile);
	m_mapMaterials[matIce->Name] = std::move(matIce);
	m_mapMaterials[matSkull->Name] = std::move(matSkull);
	m_mapMaterials[matShadow->Name] = std::move(matShadow);
}

void DK::ExTexture::CreateGeometry()
{
	std::array<Vertex, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		Vertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f), // 0
		Vertex(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f),
		Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		Vertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f), // 4
		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f),
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 2.0f),

		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f), // 8 
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f),
		Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f),

		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), // 12
		Vertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 6.0f, 1.0f),

		// Mirror
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f), // 16
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f)
	};

	std::array<std::int16_t, 30> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	MeshSection floorSection;
	floorSection.IndexCount = 6;
	floorSection.StartIndexLocation = 0;
	floorSection.BaseVertexLocation = 0;

	MeshSection wallSection;
	wallSection.IndexCount = 18;
	wallSection.StartIndexLocation = 6;
	wallSection.BaseVertexLocation = 0;

	MeshSection mirrorSectioin;
	mirrorSectioin.IndexCount = 6;
	mirrorSectioin.StartIndexLocation = 24;
	mirrorSectioin.BaseVertexLocation = 0;

	auto meshBuffer = std::make_unique<MeshBuffer<Vertex>>();

	meshBuffer->Vertices.resize(vertices.size());
	std::copy(vertices.begin(), vertices.end(), meshBuffer->Vertices.begin());

	meshBuffer->Indices.resize(indices.size());
	std::copy(indices.begin(), indices.end(), meshBuffer->Indices.begin());

	meshBuffer->MeshSections["floor"] = floorSection;
	meshBuffer->MeshSections["wall"] = wallSection;
	meshBuffer->MeshSections["mirror"] = mirrorSectioin;

	meshBuffer->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());

	m_mapMeshBuffer["room"] = std::move(meshBuffer);

	{
		std::ifstream fin("Models/skull.txt");

		if (!fin)
		{
			MessageBox(0, L"Models/skull.txt not found.", 0, 0);
			return;
		}

		UINT vcount = 0;
		UINT tcount = 0;
		std::string ignore;

		fin >> ignore >> vcount;
		fin >> ignore >> tcount;
		fin >> ignore >> ignore >> ignore >> ignore;

		std::vector<Vertex> vertices2(vcount);
		for (UINT i = 0; i < vcount; ++i)
		{
			fin >> vertices2[i].Position.x >> vertices2[i].Position.y >> vertices2[i].Position.z;
			fin >> vertices2[i].Normal.x >> vertices2[i].Normal.y >> vertices2[i].Normal.z;

			// Model does not have texture coordinates, so just zero them out.
			vertices2[i].TexC = { 0.0f, 0.0f };
		}

		fin >> ignore;
		fin >> ignore;
		fin >> ignore;

		std::vector<std::int32_t> indices2(3 * tcount);
		for (UINT i = 0; i < tcount; ++i)
		{
			fin >> indices2[i * 3 + 0] >> indices2[i * 3 + 1] >> indices2[i * 3 + 2];
		}

		fin.close();

		//
		// Pack the indices of all the meshes into one index buffer.
		//

		auto geo = std::make_unique<MeshBuffer<Vertex>>();
		
		geo->Vertices.resize(vertices2.size());
		std::copy(vertices2.begin(), vertices2.end(), geo->Vertices.begin());

		geo->Indices.resize(indices2.size());
		std::copy(indices2.begin(), indices2.end(), geo->Indices.begin());

		MeshSection submesh;
		submesh.IndexCount = (UINT)indices2.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->MeshSections["skull"] = submesh;
		geo->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());

		m_mapMeshBuffer["skull"] = std::move(geo);
	}
}

void DK::ExTexture::CreateGameObject()
{
	int nCBIndex = 0;

	// floor
	auto objFloor = new GameObject();
	objFloor->m_nCBIndex = nCBIndex++;
	objFloor->m_nFrameDirty = m_nFrameReesourceCount;
	objFloor->SetMaterial(m_mapMaterials["checkertile"].get());
	
	MeshBuffer<Vertex>* pMeshBuffer = m_mapMeshBuffer["room"].get();
	MeshSection meshSection;

	if (pMeshBuffer->GetMeshSection("floor", meshSection))
		objFloor->SetMeshData(pMeshBuffer, meshSection);

	m_vecGameObjects[(int)RenderLayer::Opaque].push_back(objFloor);

	// wall
	auto objWall = new GameObject();
	objWall->m_nCBIndex = nCBIndex++;
	objWall->m_nFrameDirty = m_nFrameReesourceCount;
	objWall->SetMaterial(m_mapMaterials["bricks"].get());

	if (pMeshBuffer->GetMeshSection("wall", meshSection))
		objWall->SetMeshData(pMeshBuffer, meshSection);

	m_vecGameObjects[(int)RenderLayer::Opaque].push_back(objWall);

	// skull
	auto objSkull = new GameObject();
	objSkull->GetTransform().SetPosition(0, 0, -5);
	objSkull->GetTransform().SetScale(0.45f, 0.45f, 0.45f);
	objSkull->GetTransform().SetRotation(0, 90, 0);
	objSkull->m_nCBIndex = nCBIndex++;
	objSkull->m_nFrameDirty = m_nFrameReesourceCount;
	objSkull->SetMaterial(m_mapMaterials["skull"].get());

	pMeshBuffer = m_mapMeshBuffer["skull"].get();
	if (pMeshBuffer->GetMeshSection("skull", meshSection))
		objSkull->SetMeshData(pMeshBuffer, meshSection);

	m_vecGameObjects[(int)RenderLayer::Opaque].push_back(objSkull);

	auto objSkullReflection = new GameObject();
	//XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	//XMMATRIX R = XMMatrixReflect(mirrorPlane);
	objSkullReflection->GetTransform().SetPosition(0, 0, 5);
	objSkullReflection->GetTransform().SetScale(0.45f, 0.45f, 0.45f);
	objSkullReflection->GetTransform().SetRotation(0, 90, 0);
	objSkullReflection->m_nCBIndex = nCBIndex++;
	objSkullReflection->m_nFrameDirty = m_nFrameReesourceCount;
	objSkullReflection->SetMaterial(m_mapMaterials["skull"].get());

	pMeshBuffer = m_mapMeshBuffer["skull"].get();
	if (pMeshBuffer->GetMeshSection("skull", meshSection))
		objSkullReflection->SetMeshData(pMeshBuffer, meshSection);

	m_vecGameObjects[(int)RenderLayer::Reflected].push_back(objSkullReflection);

	// mirror
	pMeshBuffer = m_mapMeshBuffer["room"].get();
	auto objMirror = new GameObject();
	objMirror->m_nCBIndex = nCBIndex++;
	objMirror->m_nFrameDirty = m_nFrameReesourceCount;
	objMirror->SetMaterial(m_mapMaterials["ice"].get());

	if (pMeshBuffer->GetMeshSection("mirror", meshSection))
		objMirror->SetMeshData(pMeshBuffer, meshSection);

	m_vecGameObjects[(int)RenderLayer::Mirror].push_back(objMirror);
	m_vecGameObjects[(int)RenderLayer::Transparent].push_back(objMirror);
}

void DK::ExTexture::CreateFrameResource()
{
	int objectCount = 0;
	for (int i = 0; i < (int)RenderLayer::Count; ++i)
		objectCount += m_vecGameObjects[i].size();

	for (int i = 0; i < m_nFrameReesourceCount; ++i)
	{
		m_vecFrameResoruce.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 2, objectCount, m_mapMaterials.size(), 1));
	}
}

void DK::ExTexture::BuildDescriptor()
{
	int nTextureCount = m_mapTextures.size();

	// create descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = nTextureCount;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_spHeapSRV)));

	for(auto iter = m_mapTextures.begin(); iter != m_mapTextures.end(); ++iter)
	{
		Texture* texture = iter->second.get();
		D3D12_RESOURCE_DESC rscDesc = texture->Resource->GetDesc();

		CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(m_spHeapSRV->GetCPUDescriptorHandleForHeapStart());
		descriptorHandle.Offset(texture->TexCBIndex, m_uCbvSrvUavDescriptorSize);

		// texture2D에 대한 view 생성
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = rscDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = rscDesc.MipLevels;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		m_d3dDevice->CreateShaderResourceView(texture->Resource.Get(), &srvDesc, descriptorHandle);
	}
}

void DK::ExTexture::BuildInputLayoutAndShader()
{
	/*const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};*/

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

	// opaque
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
	
	// transparent
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = psoDesc;

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

	transparentPsoDesc.BlendState.RenderTarget[0] = rtBlendDesc;
	m_d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_mapPSO["transparent"]));

	// alpha test
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTest = psoDesc;
	alphaTest.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_d3dDevice->CreateGraphicsPipelineState(&alphaTest, IID_PPV_ARGS(&m_mapPSO["alphatest"]));
	

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

	// reflection stencil
	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	reflectionsDSS.DepthEnable = true;
	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSS.StencilEnable = true;
	reflectionsDSS.StencilReadMask = 0xff;
	reflectionsDSS.StencilWriteMask = 0xff;

	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawReflectionsPsoDesc = psoDesc;
	drawReflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	drawReflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	drawReflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;
	m_d3dDevice->CreateGraphicsPipelineState(&drawReflectionsPsoDesc, IID_PPV_ARGS(&m_mapPSO["stencilReflections"]));

	// shadow
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparentPsoDesc;
	shadowPsoDesc.DepthStencilState = shadowDSS;
	m_d3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_mapPSO["shadow"]));
}

void DK::ExTexture::AnimateMaterials(const GameTimer& gt)
{
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
	auto objectCB = m_pCurrFrameResource->ObjectCB.get();

	for (int i = 0; i < (int)RenderLayer::Count; ++i)
	{
		int objCount = m_vecGameObjects[i].size();

		for (int j = 0; j < objCount; ++j)
		{
			if (m_vecGameObjects[i][j]->m_nFrameDirty <= 0)
				continue;

			DirectX::XMFLOAT4X4 worldMatrix = m_vecGameObjects[i][j]->GetTransform().GetWorldMatrix();
			DirectX::XMFLOAT4X4 texTransform = m_vecGameObjects[i][j]->TexTransform;

			XMMATRIX world = XMLoadFloat4x4(&worldMatrix);
			XMMATRIX vTexTransform = XMLoadFloat4x4(&texTransform);

			ObjectConstants constants;
			XMStoreFloat4x4(&constants.WorldMatrix, XMMatrixTranspose(world));
			XMStoreFloat4x4(&constants.TexTransform, XMMatrixTranspose(vTexTransform));

			objectCB->CopyData(m_vecGameObjects[i][j]->m_nCBIndex, constants);

			m_vecGameObjects[i][j]->m_nFrameDirty -= 1;
		}
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

		mat->NumFramesDirty -= 1;
	}
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

void DK::ExTexture::UpdateReflectRenderPassCB()
{
	m_reflectRenderPassCB = m_renderPassCB;

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&m_reflectRenderPassCB.Lights[i].Direction);
		XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&m_reflectRenderPassCB.Lights[i].Direction, reflectedLightDir);
	}

	// Reflected pass stored in index 1
	auto currPassCB = m_pCurrFrameResource->RenderPassCB.get();
	currPassCB->CopyData(1, m_reflectRenderPassCB);
}

void DK::ExTexture::DrawGameObjects(ID3D12GraphicsCommandList* cmdList, std::vector<GameObject*> vecGameObject)
{
	UINT objCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtils::CalcConstBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_pCurrFrameResource->ObjectCB->GetBuffer();
	auto materialCB = m_pCurrFrameResource->MaterialCB->GetBuffer();

	for (size_t i = 0; i < vecGameObject.size(); ++i)
	{
		GameObject* pGameObject = vecGameObject[i];

		auto viewVB = pGameObject->GetMeshBuffer()->VertexBufferView();
		auto viewIB = pGameObject->GetMeshBuffer()->IndexBufferView();

		cmdList->IASetVertexBuffers(0, 1, &viewVB);
		cmdList->IASetIndexBuffer(&viewIB);
		cmdList->IASetPrimitiveTopology(pGameObject->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_spHeapSRV->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(pGameObject->GetMaterial()->DiffuseSrvHeapIndex, m_uCbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (objCBByteSize * pGameObject->m_nCBIndex);

		int matCBIndex = pGameObject->GetMaterial()->MatCBIndex;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = materialCB->GetGPUVirtualAddress() + (matCBByteSize * matCBIndex);

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		MeshSection meshSection = pGameObject->GetMeshSection();
		cmdList->DrawIndexedInstanced(meshSection.IndexCount, 1, meshSection.StartIndexLocation, meshSection.BaseVertexLocation, 0);
	}
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
