#include "MeshFilter.h"

//template<class T>
//DK::MeshFilter<T>::MeshFilter(std::string meshName) : Component(ComponentType::CT_MeshFilter), m_pBindMeshBuffer(nullptr)
//{
//	if (MeshManager::GetInstance() != nullptr)
//		m_pBindMeshBuffer = MeshManager::GetInstance()->GetMeshData(meshName, m_meshSection);
//}
//
//template<class T>
//DK::MeshFilter<T>::MeshFilter(std::string meshName, MeshBuffer<T>* pMeshBuffer) 
//	: Component(ComponentType::CT_MeshFilter), m_pBindMeshBuffer(pMeshBuffer)
//{
//	if (m_pBindMeshBuffer != nullptr)
//		m_pBindMeshBuffer->GetMeshSection(meshName, m_meshSection);
//}

//template<class T>
//bool DK::MeshFilter<T>::GetMeshInfo(D3D12_VERTEX_BUFFER_VIEW& vbView, D3D12_INDEX_BUFFER_VIEW& ibView, MeshSection& meshSection)
//{
//	if (m_pBindMeshBuffer == nullptr)
//		return false;
//
//	vbView = m_pBindMeshBuffer->VertexBufferView();
//	ibView = m_pBindMeshBuffer->IndexBufferView();
//	meshSection = m_meshSection;
//
//	return true;
//}
