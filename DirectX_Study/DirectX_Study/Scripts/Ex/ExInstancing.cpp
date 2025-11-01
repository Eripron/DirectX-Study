#include "ExInstancing.h"

using namespace DK;

ExInstancing::ExInstancing(HWND hWnd) : EngineBase(hWnd)
{
}

ExInstancing::~ExInstancing()
{
}

void DK::ExInstancing::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Opaque].Get());

	std::vector<RenderItem*> renderItems;

	for (int i = 0; i < m_renderItems.size(); ++i)
		renderItems.push_back(m_renderItems[i].get());
	RenderRenderItems(cmdList, renderItems);
}

void DK::ExInstancing::CreateMesh()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> box = geoGen.CreateBox(4, 4, 4);
	MeshManager::GetInstance()->AddMeshData("box", box);

	MeshManager::GetInstance()->CreateMeshBuffer(m_d3dDevice.Get(), m_commandList.Get());
}

void DK::ExInstancing::LoadTextures()
{
	LoadTexture(L"Textures/bricks.dds");
	LoadTexture(L"Textures/checkboard.dds");
	LoadTexture(L"Textures/ice.dds");
	LoadTexture(L"Textures/white1x1.dds");
	LoadTexture(L"Textures/water1.dds");
	LoadTexture(L"Textures/stone.dds");
	LoadTexture(L"Textures/grass.dds");
}

void DK::ExInstancing::CreateMaterial()
{
	std::unique_ptr<Material> mat = std::make_unique<Material>();

	mat->Name = "bricks";
	mat->SrvHeapIndex = 0;
	mat->DiffuseSrvHeapIndex = m_textures["bricks"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.03f, 0.03f, 0.03f);
	mat->Roughness = 0.85f;
	mat->DirtyCount = FrameResourceCount;
	m_materials["bricks"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "checkboard";
	mat->SrvHeapIndex = 1;
	mat->DiffuseSrvHeapIndex = m_textures["checkboard"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
	mat->Roughness = 0.4f;
	mat->DirtyCount = FrameResourceCount;
	m_materials["checkboard"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "ice";
	mat->SrvHeapIndex = 2;
	mat->DiffuseSrvHeapIndex = m_textures["ice"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);   // 낮은 반사율
	mat->Roughness = 0.15f;                           // 매우 매끄럽고 반사 강함
	mat->DirtyCount = FrameResourceCount;
	m_materials["ice"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "white1x1";
	mat->SrvHeapIndex = 3;
	mat->DiffuseSrvHeapIndex = m_textures["white1x1"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	mat->Roughness = 0.5f;                            // 중간 정도
	mat->DirtyCount = FrameResourceCount;
	m_materials["white1x1"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "water1";
	mat->SrvHeapIndex = 4;
	mat->DiffuseSrvHeapIndex = m_textures["water1"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);   // 낮은 반사율
	mat->Roughness = 0.05f;                           // 매우 매끄럽고 반사 강함
	mat->DirtyCount = FrameResourceCount;
	m_materials["water1"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "stone";
	mat->SrvHeapIndex = 5;
	mat->DiffuseSrvHeapIndex = m_textures["stone"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
	mat->Roughness = 0.8f;                            // 거칠고 반사 적음
	mat->DirtyCount = FrameResourceCount;
	m_materials["stone"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "grass";
	mat->SrvHeapIndex = 6;
	mat->DiffuseSrvHeapIndex = m_textures["grass"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.03f, 0.03f, 0.03f);
	mat->Roughness = 0.7f;                            // 거칠고 반사 적음
	mat->DirtyCount = FrameResourceCount;
	m_materials["grass"] = std::move(mat);
}

void DK::ExInstancing::CreateGameObject()
{
	GameObject* pGo = new GameObject();

	pGo->AddComponent(new MeshFilter("box"));
	pGo->SetMaterial(m_materials["ice"].get());

	m_gameObjects[(int)RenderLayer::Opaque].push_back(pGo);

	// render info
	std::unique_ptr<RenderItem> spRenderItem = std::make_unique<RenderItem>();
	spRenderItem->ObjBufferIndex = 0;
	spRenderItem->pGameObject = pGo;
	spRenderItem->DirtyCount = FrameResourceCount;

	// instance data setting
	const int n = 5;
	int instanceCount = n * n * n;
	spRenderItem->InstanceDatas.resize(instanceCount);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				int index = k * n * n + i * n + j;
				// Position instanced along a 3D grid.
				spRenderItem->InstanceDatas[index].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j * dx, y + i * dy, z + k * dz, 1.0f);

				XMStoreFloat4x4(&spRenderItem->InstanceDatas[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
				spRenderItem->InstanceDatas[index].MaterialIndex = index % m_materials.size();
			}
		}
	}

	// bound box setting
	std::vector<Vertex> boxVertex(MeshManager::GetInstance()->GetVertexInfo("box"));

	XMFLOAT3 vMinf3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 vMaxf3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	for (int i = 0; i < boxVertex.size(); ++i)
	{
		DirectX::XMFLOAT3 vPosf3 = boxVertex[i].Position;
		DirectX::XMVECTOR vPos = DirectX::XMLoadFloat3(&vPosf3);

		vMin = DirectX::XMVectorMin(vMin, vPos);
		vMax = DirectX::XMVectorMax(vMax, vPos);
	}

	XMVECTOR vCenter = DirectX::XMVectorMultiply(DirectX::XMVectorAdd(vMax, vMin), DirectX::XMVectorReplicate(0.5f));
	XMStoreFloat3(&spRenderItem->BoundBox.Center, vCenter);

	XMVECTOR vExtents = DirectX::XMVectorMultiply(DirectX::XMVectorSubtract(vMax, vMin), DirectX::XMVectorReplicate(0.5f));
	XMStoreFloat3(&spRenderItem->BoundBox.Extents, vExtents);

	m_renderItems.push_back(std::move(spRenderItem));
}
