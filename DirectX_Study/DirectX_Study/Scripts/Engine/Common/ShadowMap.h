#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "..\..\Header\d3dx12.h"

namespace DK
{
	class ShadowMap
	{
	public:
		ShadowMap(ID3D12Device* device, UINT width, UINT height);

		ShadowMap(const ShadowMap& rhs) = delete;
		ShadowMap& operator=(const ShadowMap& rhs) = delete;
		~ShadowMap() = default;

		UINT Width() const;
		UINT Height() const;
		ID3D12Resource* Resource();
		CD3DX12_GPU_DESCRIPTOR_HANDLE Srv() const;
		CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv() const;

		D3D12_VIEWPORT Viewport()const;
		D3D12_RECT ScissorRect()const;

		void BuildDescriptor(
			CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
			CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
			CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

		void OnResize(UINT newWidth, UINT newHeight);

	private:
		void BuildResource();
		void BuildDescriptors();

	private:
		ID3D12Device* _d3d12Device;

		UINT _width;
		UINT _height;

		D3D12_VIEWPORT _viewPort;
		D3D12_RECT _scissorRect;

		DXGI_FORMAT _format = DXGI_FORMAT_R24G8_TYPELESS;
		Microsoft::WRL::ComPtr<ID3D12Resource> _shadowTexture = nullptr;

		CD3DX12_CPU_DESCRIPTOR_HANDLE _hCpuSrv;		// shader¿¡ ¹­ÀÌ´Â Resource(cpu)
		CD3DX12_GPU_DESCRIPTOR_HANDLE _hGpuSrv;		// shader¿¡ ¹­ÀÌ´Â Resource(gpu)
		CD3DX12_CPU_DESCRIPTOR_HANDLE _hCpuDsv;		// depth-stencil·Î ¹­ÀÌ´Â Resource
	};
}