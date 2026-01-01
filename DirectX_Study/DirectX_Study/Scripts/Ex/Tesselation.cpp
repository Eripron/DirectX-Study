#include "Tesselation.h"

using namespace DK;

DK::ExTesselation::ExTesselation(HWND hWnd) : EngineBase(hWnd)
{
}

DK::ExTesselation::~ExTesselation()
{
}

void DK::ExTesselation::Render(ID3D12GraphicsCommandList* cmdList)
{
	/*cmdList->SetPipelineState(m_psos[(int)RenderLayer::Patch].Get());

	RenderRenderItems(cmdList, m_gameObjects[(int)RenderLayer::Patch]);*/
}

void DK::ExTesselation::LoadTextures()
{
	LoadTexture(L"Textures/bricks.dds");
	LoadTexture(L"Textures/checkboard.dds");
	LoadTexture(L"Textures/ice.dds");
	LoadTexture(L"Textures/white1x1.dds");
}

void DK::ExTesselation::CreateMesh()
{
	GeometryGenerator geoGen;
	MeshData<Vertex> grid = geoGen.CreateGrid(30, 30, 2, 2);
	grid.Indices32[0] = 0;
	grid.Indices32[1] = 1;
	grid.Indices32[2] = 1;
	grid.Indices32[3] = 2;
	grid.Indices32[4] = 2;
	grid.Indices32[5] = 3;

	MeshManager::GetInstance()->AddMeshData("grid", grid);

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
}

void DK::ExTesselation::CreateMaterial()
{
	std::unique_ptr<Material> mat = std::make_unique<Material>();

	mat->Name = "grid";
	mat->SrvIndex = 0;
	mat->BaseColorTextureIndex = 0;
	mat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mat->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	mat->Roughness = 0.5f;
	mat->DirtyCount = FrameResourceCount;

	m_materials[mat->Name] = std::move(mat);
}

void DK::ExTesselation::CreateGameObject()
{
	/*GameObject* go = new GameObject();
	go->m_nCBIndex = 0;
	go->m_nFrameDirty = FrameResourceCount;
	go->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;

	go->SetMaterial(m_materials["grid"].get());
	go->AddComponent(new MeshFilter("grid"));

	m_gameObjects[(int)RenderLayer::Patch].push_back(go);*/
}

void DK::ExTesselation::BuildInputLayoutAndShader()
{
	EngineBase::BuildInputLayoutAndShader();

	m_shaders["tessVS"] = D3DUtils::CompileShader(L"Shaders\\Tessellation.hlsl", nullptr, "VS", "vs_5_0");
	m_shaders["tessHS"] = D3DUtils::CompileShader(L"Shaders\\Tessellation.hlsl", nullptr, "HS", "hs_5_0");
	m_shaders["tessDS"] = D3DUtils::CompileShader(L"Shaders\\Tessellation.hlsl", nullptr, "DS", "ds_5_0");
	m_shaders["tessPS"] = D3DUtils::CompileShader(L"Shaders\\Tessellation.hlsl", nullptr, "PS", "ps_5_0");
}

void DK::ExTesselation::BuildPSO()
{
	EngineBase::BuildPSO();


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescPatch;

	// PSO for opaque objects.
	ZeroMemory(&psoDescPatch, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDescPatch.InputLayout = { m_inputLayouts.data(), (UINT)m_inputLayouts.size() };
	psoDescPatch.pRootSignature = m_rootSignature.Get();
	psoDescPatch.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tessVS"]->GetBufferPointer()),
		m_shaders["tessVS"]->GetBufferSize()
	};
	psoDescPatch.HS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tessHS"]->GetBufferPointer()),
		m_shaders["tessHS"]->GetBufferSize()
	};
	psoDescPatch.DS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tessDS"]->GetBufferPointer()),
		m_shaders["tessDS"]->GetBufferSize()
	};
	psoDescPatch.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders["tessPS"]->GetBufferPointer()),
		m_shaders["tessPS"]->GetBufferSize()
	};
	psoDescPatch.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDescPatch.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	psoDescPatch.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDescPatch.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDescPatch.SampleMask = UINT_MAX;
	psoDescPatch.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoDescPatch.NumRenderTargets = 1;
	psoDescPatch.RTVFormats[0] = m_eBackBufferFormat;
	psoDescPatch.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDescPatch.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	psoDescPatch.DSVFormat = m_eDepthStencilFormat;
	THROW_IF_FAILED(m_d3dDevice->CreateGraphicsPipelineState(&psoDescPatch, IID_PPV_ARGS(&m_psos[(int)RenderLayer::Patch])));
}
