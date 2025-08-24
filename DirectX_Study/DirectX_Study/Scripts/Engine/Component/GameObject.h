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

	public:
		int m_nCBIndex = -1;
		int m_nFrameDirty = -1;
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();

	private:
		Transform m_transform;

		MeshBuffer<Vertex>* m_pMeshBuffer = nullptr;
		MeshSection m_meshSection;

		Material* m_pMaterial = nullptr;
	};

}