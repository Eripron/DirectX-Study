#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "D3DUtils.h"

namespace DK
{
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;      // 정점 위치
		DirectX::XMFLOAT4 Color;    // 정점 색상
	};

	struct MeshSection
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;
	};

	struct MeshRenderData
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> VertexUploadBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexUploadBuffer = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer = nullptr;

		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		UINT IndexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;

		std::vector<Vertex> Vertices;
		std::vector<std::uint16_t> Indices;

		std::unordered_map<std::string, MeshSection> MeshSections;


		void AddVertexData(std::string name, const std::vector<Vertex>& vertices, const std::vector<std::uint16_t>& indices)
		{
			if (MeshSections.find(name) != MeshSections.end())
			{
				return;
			}

			MeshSection meshSection;
			meshSection.IndexCount = indices.size();
			meshSection.StartIndexLocation = Indices.size();
			meshSection.BaseVertexLocation = Vertices.size();

			MeshSections[name] = meshSection;

			Vertices.insert(Vertices.end(), vertices.begin(), vertices.end());
			Indices.insert(Indices.end(), indices.begin(), indices.end());
		}

		void BuildMeshRenderData(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		{
			VertexByteStride = sizeof(Vertex);
			VertexBufferByteSize = VertexByteStride * Vertices.size();
			IndexBufferByteSize = sizeof(std::uint16_t) * Indices.size();

			VertexBuffer = D3DUtils::CreateDefaultBuffer(device, cmdList, Vertices.data(), VertexBufferByteSize, VertexUploadBuffer);
			IndexBuffer = D3DUtils::CreateDefaultBuffer(device, cmdList, Indices.data(), IndexBufferByteSize, IndexUploadBuffer);
		}

		bool GetMeshSection(std::string name, MeshSection& section)
		{
			if (MeshSections.find(name) != MeshSections.end())
			{
				section = MeshSections[name];
				return true;
			}

			return false;
		}

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = VertexBufferByteSize;
			vbv.StrideInBytes = VertexByteStride;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
			ibv.SizeInBytes = IndexBufferByteSize;
			ibv.Format = IndexFormat;

			return ibv;
		}

	};


}