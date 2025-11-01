#pragma once

#include <string>
#include <wrl.h>

#include "../Data/DataTypes.h"
#include "../Common/Singleton.h"

namespace DK
{
	class MeshManager : public Singleton<MeshManager>
	{
	private:
		void Init();

	public:
		void AddMeshData(std::string name, MeshData<Vertex>& meshData);
		void CreateMeshBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

		MeshBuffer<Vertex>* GetMeshData(std::string name, MeshSection& meshSection);
		std::vector<Vertex> GetVertexInfo(std::string name);

	private:
		bool m_bInit = false;

		std::unique_ptr<MeshBuffer<Vertex>> m_pMeshBuffer = nullptr;
		std::unordered_map<std::string, std::vector<Vertex>> m_mapVertexInfo;
	};
	
}