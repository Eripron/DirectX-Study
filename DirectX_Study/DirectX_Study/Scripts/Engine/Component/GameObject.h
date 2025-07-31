#pragma once

#include <string>

#include "Transform.h"
#include "../Data/DataTypes.h"

namespace DK
{
	class GameObject
	{
	public:
		GameObject() = default;
		~GameObject();

		Transform& GetTransform();
		MeshBuffer* GetMeshBuffer();
		MeshSection GetMeshSection();
		Material* GetMaterial();

		void SetMeshData(MeshBuffer* pMeshBuffer, MeshSection meshSection);
		void SetMaterial(Material* pMat);

	private:
		Transform m_transform;

		MeshBuffer* m_pMeshBuffer = nullptr;
		MeshSection m_meshSection;

		Material* m_pMaterial = nullptr;
	};

}