#include "ExInstancing.h"

using namespace DK;

ExInstancing::ExInstancing(HWND hWnd) : EngineBase(hWnd)
{
}

ExInstancing::~ExInstancing()
{
}

bool DK::ExInstancing::Update()
{
	if (!EngineBase::Update())
		return false;

	DirectX::XMFLOAT3 rotate(0, m_gameTimer.DeltaTime() * 0.1, 0);
	m_cameraCullingTest.Rotate(rotate);

	auto objectConstBuffer = m_curFrameResource->ObjectCB.get();

	DirectX::XMMATRIX invView = m_cameraCullingTest.GetInvViewMatrix();

	for (int i = 0; i < m_renderObjectInfos.size(); ++i)
	{
		RenderObjectInfo* objectInfo = m_renderObjectInfos[i].get();

		XMMATRIX world = XMLoadFloat4x4(&objectInfo->World);
		XMMATRIX texTransform = XMLoadFloat4x4(&objectInfo->TexTransform);

		XMVECTOR determin = XMMatrixDeterminant(world);
		XMMATRIX invWorld = XMMatrixInverse(&determin, world);

		XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

		DirectX::BoundingFrustum localSpaceFrustum;
		m_frustomCulTest.Transform(localSpaceFrustum, viewToLocal);

		objectInfo->ObjBufferIndex = i;
		objectInfo->ignoreRender = true;

		if (localSpaceFrustum.Contains(objectInfo->BoundBox) != DirectX::DISJOINT)
			objectInfo->ignoreRender = false;
	}

	return true;
}

void DK::ExInstancing::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Opaque].Get());

	std::vector<RenderObjectInfo*> renderItems;

	for (int i = 0; i < m_renderObjectInfos.size(); ++i)
		renderItems.push_back(m_renderObjectInfos[i].get());

	RenderRenderItems(cmdList, renderItems);
}

bool DK::ExInstancing::OnResize(int width, int height, bool force)
{
	if (EngineBase::OnResize(width, height, force) == false)
		return false;

	m_cameraCullingTest.SetAspect(AspectRatio());

	DirectX::XMMATRIX proj = m_cameraCullingTest.GetProjMatrix();
	DirectX::BoundingFrustum::CreateFromMatrix(m_frustomCulTest, proj);

	return true;
}

void DK::ExInstancing::CreateMesh()
{
	GeometryGenerator geoGen;

	MeshData<Vertex> box = geoGen.CreateBox(1, 1, 1);
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
	int x = 5;
	int y = 5;
	int z = 5;
	for (int i = 0; i < x * y * z; ++i)
	{
		GameObject* pNewGo = new GameObject();

		pNewGo->AddComponent(new MeshFilter("box"));
		pNewGo->SetMaterial(m_materials["bricks"].get());

		Transform* transform = pNewGo->GetComponent<Transform>();
		if (transform != nullptr)
		{
			DirectX::XMFLOAT3 pos = GetPositionByIndex(x, y, z, 20, 20, 20, i);
			transform->SetPosition(pos.x, pos.y, pos.z);
		}

		m_gameObjects[(int)RenderLayer::Opaque].push_back(pNewGo);
	}
}

void DK::ExInstancing::CreateRenderObjectInfo()
{
	for (int i = 0; i < (int)RenderLayer::Count; ++i)
	{
		for (int j = 0; j < m_gameObjects[i].size(); ++j)
		{
			if (m_gameObjects[i][j] == nullptr) continue;

			GameObject* object = m_gameObjects[i][j];
			Transform* transform = object->GetComponent<Transform>();
			Material* mat = object->GetMaterial();

			std::unique_ptr<RenderObjectInfo> renderInfo = std::make_unique<RenderObjectInfo>();

			renderInfo->ObjBufferIndex = i + j;
			if (transform != nullptr) renderInfo->World = transform->GetWorldMatrix();
			if (mat != nullptr) renderInfo->MaterialIndex = mat->SrvHeapIndex;
			renderInfo->meshInfo = object->GetComponent<MeshFilter>();

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
			XMStoreFloat3(&renderInfo->BoundBox.Center, vCenter);

			XMVECTOR vExtents = DirectX::XMVectorMultiply(DirectX::XMVectorSubtract(vMax, vMin), DirectX::XMVectorReplicate(0.5f));
			XMStoreFloat3(&renderInfo->BoundBox.Extents, vExtents);

			m_renderObjectInfos.push_back(std::move(renderInfo));
		}
	}

}

DirectX::XMFLOAT3 DK::ExInstancing::GetPositionByIndex(int x, int y, int z, float width, float depth, float height, int index)
{
	int col = index % x;
	int row = (index / x) % y;
	int h = index / (x * y);

	float xGap = width / (x - 1);
	float zGap = depth / (z - 1);
	float yGap = height / (y - 1);

	return DirectX::XMFLOAT3(-width / 2 + xGap * col, -height / 2 + yGap * h, depth / 2 - zGap * row);
}
