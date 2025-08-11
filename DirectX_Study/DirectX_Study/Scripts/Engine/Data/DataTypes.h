#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

#include "../Utils/D3DUtils.h"
#include "../Utils/MathUtils.h"

namespace DK
{
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT3& t,
			const DirectX::XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {
		}

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {
		}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;	// surface normal
		DirectX::XMFLOAT2 TexC;		// Texture Coordinate
		DirectX::XMFLOAT3 TangentU;

		static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout()
		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			return inputLayout;
		}
	};

#pragma region <Mesh>

	struct MeshData
	{
		MeshData() {}

		std::string Name;
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices32;

		const std::vector<uint16_t>& GetIndices16()
		{
			// TODO: 메모리 등 최적화를 위한 작업은 알겠다.
			// 하지만 만약 static_cast<uint16_t> 부분에서 예외가 나오는 경우는 어떻게 처리할지 고민해보자.
			if (m_vecIndices16.empty())
			{
				m_vecIndices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
					m_vecIndices16[i] = static_cast<uint16_t>(Indices32[i]);
			}

			return m_vecIndices16;
		}

	private:
		std::vector<uint16_t> m_vecIndices16;
	};

	struct MeshSection
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;
	};

	struct MeshBuffer
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

		void AddMeshData(std::string name, MeshData& meshData)
		{
			AddMeshData(name, meshData.Vertices, meshData.GetIndices16());
		}

		void AddMeshData(std::string name, const std::vector<Vertex>& vertices, const std::vector<std::uint16_t>& indices)
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

		void CreateMeshBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
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

#pragma endregion

#pragma region Light

	struct Light
	{
		DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
		float FalloffStart = 1.0f;		// 빛 감쇠 시작 거리
		DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
		float FalloffEnd = 10.0f;		// 빛 감쇠 끝 거리
		DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
		float SpotPower = 30.0f;
	};

#pragma endregion

#pragma region Material

	struct Material
	{
		std::string Name;

		int MatCBIndex = -1;
		int DiffuseSrvHeapIndex = -1;	// ?
		int NormalSrvHeapIndex = -1;	// ?

		int NumFramesDirty;

		// Material constant buffer data used for shading.
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };	// md: 반사율
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };			// Rf(0): 매질
		float Roughness = 0.25f;										// m: 거칠기
		DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();	// ?
	};

#pragma endregion

#pragma region Texture

	struct Texture
	{
		std::wstring FileName;

		Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;


		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
		{
			const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
				0, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
				1, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
				2, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
				3, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

			const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
				4, // shaderRegister
				D3D12_FILTER_ANISOTROPIC, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
				0.0f,                             // mipLODBias
				8);                               // maxAnisotropy

			const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
				5, // shaderRegister
				D3D12_FILTER_ANISOTROPIC, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
				0.0f,                              // mipLODBias
				8);                                // maxAnisotropy

			return { pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp };
		}
	};

#pragma endregion


#pragma region Constants

	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 WorldMatrix = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();
	};

	struct RenderPassConstants
	{
		DirectX::XMFLOAT4X4 View = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 InvView = MathUtils::Identity4x4();

		DirectX::XMFLOAT4X4 Proj = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 InvProj = MathUtils::Identity4x4();

		DirectX::XMFLOAT4X4 ViewProj = MathUtils::Identity4x4();
		DirectX::XMFLOAT4X4 InvViewProj = MathUtils::Identity4x4();

		DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;

		DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;

		DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
		Light Lights[16];
	};

	struct MaterialConstants
	{
		DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;
		DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();
	};

#pragma endregion

}