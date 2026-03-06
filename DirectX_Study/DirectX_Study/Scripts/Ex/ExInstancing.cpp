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

	if(false)
	{
		// 테스트 절두체 컬링 체크
		DirectX::XMFLOAT3 rotate(0, m_gameTimer.DeltaTime() * 0.3, 0);
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
	}

	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(m_hWnd, &mouse);
	Raycast(mouse.x, mouse.y);
	//Raycast(m_nClientWidth / 2, m_nClientHeight / 2);

	//m_gizmo.SetGizmoColor(DirectX::XMFLOAT4(DirectX::Colors::Orange));

	//XMFLOAT3 o = m_cameraCullingTest.m_transform.GetPosition();
	//XMVECTOR pos = XMLoadFloat3(&o);

	//XMFLOAT3 frontf = m_cameraCullingTest.m_transform.Front();
	//XMFLOAT3 rightf = m_cameraCullingTest.m_transform.Right();
	//XMFLOAT3 upf = m_cameraCullingTest.m_transform.Up();

	//XMVECTOR front = XMLoadFloat3(&frontf);
	//XMVECTOR right = XMLoadFloat3(&rightf);
	//XMVECTOR up = XMLoadFloat3(&upf);

	//// FOV (Y)
	//float fovY = XMConvertToRadians(60.0f);
	//float halfHeight = tanf(fovY * 0.5f) * 20;   // near distance = 10
	//float halfWidth = halfHeight * AspectRatio();

	//// near plane center = pos + front * 10
	//front *= 10;
	//pos += front;

	//// 코너 4개
	//XMVECTOR upH = XMVectorScale(up, halfHeight);
	//XMVECTOR upN = XMVectorScale(up, -halfHeight);
	//XMVECTOR rightW = XMVectorScale(right, halfWidth);
	//XMVECTOR rightN = XMVectorScale(right, -halfWidth);

	//XMVECTOR ntl = XMVectorAdd(pos, XMVectorAdd(upH, rightN));
	//XMVECTOR ntr = XMVectorAdd(pos, XMVectorAdd(upH, rightW));
	//XMVECTOR nbl = XMVectorAdd(pos, XMVectorAdd(upN, rightN));
	//XMVECTOR nbr = XMVectorAdd(pos, XMVectorAdd(upN, rightW));

	//// Draw lines
	//XMFLOAT3 leftUp;
	//XMStoreFloat3(&leftUp, ntl);
	//m_gizmo.OnDrawLine(o, leftUp);

	//XMFLOAT3 rightUp;
	//XMStoreFloat3(&rightUp, ntr);
	//m_gizmo.OnDrawLine(o, rightUp);

	//XMFLOAT3 leftBottom;
	//XMStoreFloat3(&leftBottom, nbl);
	//m_gizmo.OnDrawLine(o, leftBottom);

	//XMFLOAT3 rightBottom;
	//XMStoreFloat3(&rightBottom, nbr);
	//m_gizmo.OnDrawLine(o, rightBottom);

	//m_gizmo.OnDrawLine(leftUp, rightUp);
	//m_gizmo.OnDrawLine(leftUp, leftBottom);
	//m_gizmo.OnDrawLine(leftBottom, rightBottom);
	//m_gizmo.OnDrawLine(rightBottom, rightUp);

	return true;
}

void DK::ExInstancing::Render(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(m_psos[(int)RenderLayer::Opaque].Get());

	std::vector<RenderObjectInfo*> renderItems;

	for (int i = 0; i < m_renderObjectInfos.size(); ++i)
	{
		if(m_renderObjectInfos[i]->ignoreRender == false)
			renderItems.push_back(m_renderObjectInfos[i].get());
	}

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
	mat->SrvIndex = 0;
	mat->BaseColorTextureIndex = m_textures["bricks"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.03f, 0.03f, 0.03f);
	mat->Roughness = 0.85f;
	mat->DirtyCount = FrameResourceCount;
	m_materials["bricks"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "checkboard";
	mat->SrvIndex = 1;
	mat->BaseColorTextureIndex = m_textures["checkboard"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
	mat->Roughness = 0.4f;
	mat->DirtyCount = FrameResourceCount;
	m_materials["checkboard"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "ice";
	mat->SrvIndex = 2;
	mat->BaseColorTextureIndex = m_textures["ice"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);   // 낮은 반사율
	mat->Roughness = 0.15f;                           // 매우 매끄럽고 반사 강함
	mat->DirtyCount = FrameResourceCount;
	m_materials["ice"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "white1x1";
	mat->SrvIndex = 3;
	mat->BaseColorTextureIndex = m_textures["white1x1"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	mat->Roughness = 0.5f;                            // 중간 정도
	mat->DirtyCount = FrameResourceCount;
	m_materials["white1x1"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "water1";
	mat->SrvIndex = 4;
	mat->BaseColorTextureIndex = m_textures["water1"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);   // 낮은 반사율
	mat->Roughness = 0.05f;                           // 매우 매끄럽고 반사 강함
	mat->DirtyCount = FrameResourceCount;
	m_materials["water1"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "stone";
	mat->SrvIndex = 5;
	mat->BaseColorTextureIndex = m_textures["stone"].get()->SrvHeapIndex;
	mat->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
	mat->Roughness = 0.8f;                            // 거칠고 반사 적음
	mat->DirtyCount = FrameResourceCount;
	m_materials["stone"] = std::move(mat);

	mat = std::make_unique<Material>();
	mat->Name = "grass";
	mat->SrvIndex = 6;
	mat->BaseColorTextureIndex = m_textures["grass"].get()->SrvHeapIndex;
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

		pNewGo->AddComponent(new MeshFilter<Vertex>("box"));
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
	/*for (int i = 0; i < (int)RenderLayer::Count; ++i)
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
			if (mat != nullptr) renderInfo->MaterialIndex = mat->SrvIndex;
			renderInfo->meshInfo = object->GetComponent<MeshFilter<Vertex>>();

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
	}*/

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

void DK::ExInstancing::Raycast(int screenX, int screenY)
{
	DirectX::XMFLOAT4X4 projMatrixf4 = m_camera.GetProjMatrixf4();
	
	float viewX = (2.0f * screenX / m_nClientWidth - 1.0f) / projMatrixf4(0, 0);
	float viewY = (1.0f - 2.0f * screenY / m_nClientHeight) / projMatrixf4(1, 1);

	DirectX::XMVECTOR rayOrg = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR rayDir = DirectX::XMVectorSet(viewX, viewY, 1.0f, 0.0f);
	
	DirectX::XMMATRIX invView = m_camera.GetInvViewMatrix();

	bool col = false;
	for (int i = 0; i < m_renderObjectInfos.size(); ++i)
	{
		RenderObjectInfo* objectInfo = m_renderObjectInfos[i].get();

		XMMATRIX world = XMLoadFloat4x4(&objectInfo->World);
		XMVECTOR determin = XMMatrixDeterminant(world);
		XMMATRIX invWorld = XMMatrixInverse(&determin, world);

		XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

		DirectX::XMVECTOR localWorldRay = XMVector3TransformCoord(rayOrg, invView);

		DirectX::XMVECTOR localRay = XMVector3TransformCoord(rayOrg, viewToLocal);
		DirectX::XMVECTOR localRayDir = XMVector3TransformNormal(rayDir, viewToLocal);
		localRayDir = XMVector3Normalize(localRayDir);

		float dist = 0.0f;
		if (!col && m_renderObjectInfos[i]->BoundBox.Intersects(localRay, localRayDir, dist))
		{
			col = true;
			objectInfo->MaterialIndex = 1;
		}
		else
		{
			objectInfo->MaterialIndex = 0;
		}
	}
}
