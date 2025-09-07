#pragma once
#include "Component.h"

#include <string>
#include "../Manager/MeshManager.h"

namespace DK
{
	class MeshFilter : public Component
	{
	public:
		MeshFilter(std::string meshName);
		~MeshFilter() = default;

		bool GetMeshInfo(D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, MeshSection& meshSection);

	private:
		MeshBuffer<Vertex>* m_pBindMeshBuffer;
		MeshSection m_meshSection;
	};
}
