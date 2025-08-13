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
		MeshBuffer<Vertex>* GetMeshBuffer();
		MeshSection GetMeshSection();
		Material* GetMaterial();

		void SetMeshData(MeshBuffer<Vertex>* pMeshBuffer, MeshSection meshSection);
		void SetMaterial(Material* pMat);

	private:
		Transform m_transform;

		MeshBuffer<Vertex>* m_pMeshBuffer = nullptr;
		MeshSection m_meshSection;

		Material* m_pMaterial = nullptr;
	};

}