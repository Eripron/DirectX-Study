#pragma once

#include <string>
#include <wrl.h>

#include "../Data/DataTypes.h"
#include "../Common/Singleton.h"

namespace DK
{
	class MeshManager : public Singleton<MeshManager>
	{
	public:
		void Init();

		void AddMeshData(std::string name, MeshData<Vertex>& meshData);
		void CreateMeshBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

		MeshBuffer<Vertex>* GetMeshData(std::string name, MeshSection& meshSection);

	private:
		std::unique_ptr<MeshBuffer<Vertex>> m_pMeshBuffer;
	};
	
}