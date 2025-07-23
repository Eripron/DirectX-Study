#pragma once

#include <string>
#include "Transform.h"
#include "MeshRenderData.h"

namespace DK
{
	class GameObject
	{
	public:
		GameObject() = default;
		~GameObject();

		Transform& GetTransform();
		MeshRenderData* GetMeshRenderData();
		MeshSection GetMeshSection();

		void SetMeshData(MeshRenderData* pMesh, MeshSection section);

	private:
		Transform mTransform;
		MeshRenderData* mpMesh = nullptr;
		MeshSection mMeshSection;

	};

}