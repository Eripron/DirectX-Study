#include "MeshManager.h"

using namespace DK;

void DK::MeshManager::Init()
{
	m_pMeshBuffer = nullptr;
	m_pMeshBuffer = std::make_unique<MeshBuffer<Vertex>>();
}

void DK::MeshManager::AddMeshData(std::string name, MeshData<Vertex>& meshData)
{
	if (m_bInit == false)
	{
		m_bInit = true;
		Init();
	}

	m_pMeshBuffer->AddMeshData(name, meshData);

	if (m_mapVertexInfo.find(name) == m_mapVertexInfo.end())
		m_mapVertexInfo[name] = meshData.Vertices;
}

MeshBuffer<Vertex>* DK::MeshManager::GetMeshData(std::string name, MeshSection& meshSection)
{
	if (m_bInit == false)
	{
		m_bInit = true;
		Init();
	}

	if (m_pMeshBuffer->GetMeshSection(name, meshSection))
		return m_pMeshBuffer.get();

	return nullptr;
}

std::vector<Vertex> DK::MeshManager::GetVertexInfo(std::string name)
{
	if (m_mapVertexInfo.find(name) == m_mapVertexInfo.end())
		return std::vector<Vertex>();

	return m_mapVertexInfo[name];
}

void DK::MeshManager::CreateMeshBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	m_pMeshBuffer->CreateMeshBuffer(device, cmdList);
}

