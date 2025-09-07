#include "MeshManager.h"

using namespace DK;

void DK::MeshManager::Init()
{
	m_pMeshBuffer = nullptr;
	m_pMeshBuffer = std::make_unique<MeshBuffer<Vertex>>();
}

void DK::MeshManager::AddMeshData(std::string name, MeshData<Vertex>& meshData)
{
	m_pMeshBuffer->AddMeshData(name, meshData);
}

void DK::MeshManager::CreateMeshBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	m_pMeshBuffer->CreateMeshBuffer(device, cmdList);
}

MeshBuffer<Vertex>* DK::MeshManager::GetMeshData(std::string name, MeshSection& meshSection)
{
	if (m_pMeshBuffer->GetMeshSection(name, meshSection))
		return m_pMeshBuffer.get();

	return nullptr;
}
