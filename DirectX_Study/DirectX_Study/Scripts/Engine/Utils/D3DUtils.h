#pragma once

/// header

///

#include <Windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <intsafe.h>
#include <comdef.h>
#include <synchapi.h>
#include <d3dcompiler.h>

#include "../../Header/d3dx12.h"

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
	struct D3DUtils
	{
		static UINT CalcConstBufferByteSize(UINT byteSize)
		{
			// const buffer는 256 바이트로 정렬해야한다.
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

}