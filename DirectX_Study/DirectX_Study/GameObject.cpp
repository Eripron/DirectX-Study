#include "GameObject.h"

namespace DK
{
	GameObject::~GameObject()
	{
	}

	Transform& GameObject::GetTransform()
	{
		return mTransform;
	}

	MeshRenderData* GameObject::GetMeshRenderData()
	{
		return mpMesh;
	}

	MeshSection GameObject::GetMeshSection()
	{
		return mMeshSection;
	}

	void GameObject::SetMeshData(MeshRenderData* pMesh, MeshSection section)
	{
		mpMesh = pMesh;
		mMeshSection = section;
	}

}