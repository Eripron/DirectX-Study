#pragma once

#include "Numeric.h"

#include <Windows.h>
#include <string>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <intsafe.h>
#include <comdef.h>
#include <synchapi.h>
#include <unordered_map>
#include <DirectXCollision.h>
#include <d3dcompiler.h>
#include <DirectXColors.h>
#include <vector>
#include "GeometryGenerator.h"

#include "d3dx12.h"


inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring FileName;
	int LineNumber = -1;
};

#ifndef THROW_IF_FAILED
#define THROW_IF_FAILED(x)                                            \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

namespace DK
{
	class D3DUtils
	{
	public:

		// const buffer는 256 바이트로 정렬해야한다.
		static UINT CalcConstantBufferByteSize(UINT byteSize)
		{
			return (byteSize + 255) & ~255;
		}

		static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
			ID3D12Device* device,
			ID3D12GraphicsCommandList* cmdList,
			const void* initData,
			UINT64 byteSize,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

			CD3DX12_HEAP_PROPERTIES propertyDefault(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC rscDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

			THROW_IF_FAILED(device->CreateCommittedResource(
				&propertyDefault,
				D3D12_HEAP_FLAG_NONE,
				&rscDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

			CD3DX12_HEAP_PROPERTIES propertyUpload(D3D12_HEAP_TYPE_UPLOAD);

			// 업로드 버퍼 생성하기
			device->CreateCommittedResource(
				&propertyUpload,
				D3D12_HEAP_FLAG_NONE,
				&rscDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(uploadBuffer.GetAddressOf()));

			D3D12_SUBRESOURCE_DATA subResourceData;
			subResourceData.pData = initData;
			subResourceData.RowPitch = byteSize;
			subResourceData.SlicePitch = subResourceData.RowPitch;

			CD3DX12_RESOURCE_BARRIER tranToCopy = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			cmdList->ResourceBarrier(1, &tranToCopy);
			
			// TODO - 내부 분석해보기
			UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

			CD3DX12_RESOURCE_BARRIER tranToRead = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
			cmdList->ResourceBarrier(1, &tranToRead);

			return defaultBuffer;
		}

		static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
			const std::wstring& fileName,
			const D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			const std::string& target)
		{
			UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
			compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			HRESULT hr = S_OK;

			Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errors;

			// D3DCompileFromFile
			// : Microsoft HLSL(High Level Shader Language) 코드를 지정된 대상에 대한 바이트코드로 컴파일합니다.
			hr = D3DCompileFromFile(fileName.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

			THROW_IF_FAILED(hr);

			return byteCode;
		}

	};

	struct SubmeshGeometry
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;

		// Bounding box of the geometry defined by this submesh. 
		// This is used in later chapters of the book.
		DirectX::BoundingBox Bounds;
	};

	struct MeshGeometry
	{
		std::string Name;

		/*
		  ID3DBlob은 Direct3D에서 데이터를 담는 데 사용되는 바이너리 버퍼 인터페이스입니다. 
		  주로 셰이더 코드, 컴파일된 데이터, 에러 메시지 등을 담을 때 사용됩니다.
		*/
		Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> ColorBufferCPU = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> ColorBufferGPU = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> ColorBufferUploader = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

		// Data about the buffers.
		UINT VertexByteStride = 0;
		UINT ColorByteStride = 0;
		UINT VertexBufferByteSize = 0;
		UINT ColorBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
		UINT IndexBufferByteSize = 0;

		// A MeshGeometry may store multiple geometries in one vertex/index buffer.
		// Use this container to define the Submesh geometries so we can draw
		// the Submeshes individually.
		std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
			vbv.StrideInBytes = VertexByteStride;	// 각 항목의 사이즈
			vbv.SizeInBytes = VertexBufferByteSize;	// buffer 사이즈

			return vbv;
		}

		D3D12_VERTEX_BUFFER_VIEW ColorBufferView() const
		{
			D3D12_VERTEX_BUFFER_VIEW rbv;
			rbv.BufferLocation = ColorBufferGPU->GetGPUVirtualAddress();
			rbv.StrideInBytes = ColorByteStride;
			rbv.SizeInBytes = ColorBufferByteSize;

			return rbv;
		}

		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
			ibv.Format = IndexFormat;
			ibv.SizeInBytes = IndexBufferByteSize;

			return ibv;
		}

		// We can free this memory after we finish upload to the GPU.
		void DisposeUploaders()
		{
			VertexBufferUploader = nullptr;
			ColorBufferUploader = nullptr;
			IndexBufferUploader = nullptr;
		}
	};

	struct MeshBuffer;
	struct MeshDataDesc
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;
		MeshBuffer* Buffer;
	};

	struct MeshBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadVertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadIndexBuffer = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer = nullptr;

		UINT VertexByteStride = 0;
		UINT VertexBufferByteSize = 0;
		UINT IndexBufferByteSize = 0;
		DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;

		std::vector<Vertex> Vertices;
		std::vector<std::uint16_t> Indices;

		std::unordered_map<std::string, MeshDataDesc> MestDataDescs;

		void AddMeshData(std::string name, GeometryGenerator::MeshData& meshData, DirectX::XMFLOAT4 color)
		{
			std::vector<std::uint16_t> indices = meshData.GetIndices16();

			MeshDataDesc meshDataDesc;
			meshDataDesc.IndexCount = indices.size();
			meshDataDesc.StartIndexLocation = Indices.size();
			meshDataDesc.BaseVertexLocation = Vertices.size();
			meshDataDesc.Buffer = this;

			MestDataDescs[name] = meshDataDesc;

			for (size_t i = 0; i < meshData.Vertices.size(); ++i)
			{
				Vertex vertex;
				vertex.Pos = meshData.Vertices[i].Position;
				vertex.Color = color;

				Vertices.push_back(vertex);
			}

			Indices.insert(Indices.end(), indices.begin(), indices.end());
		}

		void BuildBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
		{
			VertexByteStride = sizeof(Vertex);
			VertexBufferByteSize = VertexByteStride * Vertices.size();
			IndexBufferByteSize = sizeof(std::uint16_t) * Indices.size();

			VertexBuffer = D3DUtils::CreateDefaultBuffer(device, cmdList, Vertices.data(), VertexBufferByteSize, UploadVertexBuffer);
			IndexBuffer = D3DUtils::CreateDefaultBuffer(device, cmdList, Indices.data(), IndexBufferByteSize, UploadIndexBuffer);
		}

		MeshDataDesc* GetMeshDataDesc(std::string name)
		{
			auto got = MestDataDescs.find(name);

			if (got == MestDataDescs.end())
				return nullptr;

			return &(got->second);
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