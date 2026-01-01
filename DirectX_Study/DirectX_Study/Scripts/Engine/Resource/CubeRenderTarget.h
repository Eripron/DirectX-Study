#pragma once

#include "../Utils/D3DUtils.h"
#include <functional>

namespace DK
{
	/// <summary>
	/// 동적으로 cubemap 리소스를 만들기 위해서
	/// </summary>
	class CubeRenderTarget
	{
	public:
		CubeRenderTarget(ID3D12Device* pDevice, UINT unWidth, UINT unHeight, DXGI_FORMAT format);

		CubeRenderTarget(const CubeRenderTarget& rhs) = delete;
		CubeRenderTarget& operator=(const CubeRenderTarget& rhs) = delete;
		~CubeRenderTarget() = default;

		ID3D12Resource* Resource();
		CD3DX12_GPU_DESCRIPTOR_HANDLE Srv();
		CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(int faceIndex);

		D3D12_VIEWPORT Viewport()const;
		D3D12_RECT ScissorRect()const;

		void BindDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
							 CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
							 CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv[6],
							 CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

		void OnResize(UINT newWidth, UINT newHeight);

		void DrawSceneToCubeMap(ID3D12GraphicsCommandList* cmdList, std::function<void(int)> drawFunc);

	private:
		void BuildResource();
		void BuildDescriptors();

	private:
		ID3D12Device* _pDevice = nullptr;

		UINT _unWidth;
		UINT _unHeight;
		DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12_VIEWPORT _viewPort;
		D3D12_RECT _rect;

		CD3DX12_CPU_DESCRIPTOR_HANDLE _hCpuSrv;		// Shader에 묶기 위한 Descriptor handle(cpu)
		CD3DX12_GPU_DESCRIPTOR_HANDLE _hGpuSrv;		// Shader에 묶기 위한 Descriptor handle(gpu)
		CD3DX12_CPU_DESCRIPTOR_HANDLE _hCpuRtv[6];	// Render Target View를 위한 Descriptor handle(cpu)
		CD3DX12_CPU_DESCRIPTOR_HANDLE _hCpuDsv;

		Microsoft::WRL::ComPtr<ID3D12Resource> _cubeMap = nullptr;
	};
}