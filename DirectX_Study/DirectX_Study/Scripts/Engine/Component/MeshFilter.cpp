#include "MeshFilter.h"

DK::MeshFilter::MeshFilter(std::string meshName) : Component(ComponentType::CT_MeshFilter), m_pBindMeshBuffer(nullptr)
{
	if (MeshManager::GetInstance() != nullptr)
	{
		m_pBindMeshBuffer = MeshManager::GetInstance()->GetMeshData(meshName, m_meshSection);
	}
}

bool DK::MeshFilter::GetMeshInfo(D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, MeshSection& meshSection)
{
	if (m_pBindMeshBuffer == nullptr)
		return false;

	vbView = m_pBindMeshBuffer->VertexBufferView();
	ibView = m_pBindMeshBuffer->IndexBufferView();
	meshSection = m_meshSection;

	return true;
}
