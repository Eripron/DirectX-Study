#pragma once

#include "../Engine/Utils/D3DUtils.h"

namespace DK
{
	class BlurFilter
	{
	public:
		BlurFilter(const BlurFilter& rhs) = delete;
		BlurFilter& operator=(const BlurFilter& rhs) = delete;

	public:
		///<summary>
		/// The width and height should match the dimensions of the input texture to blur.
		/// Recreate when the screen is resized. 
		///</summary>
		BlurFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format);
		~BlurFilter() = default;

		ID3D12Resource* Output();

		void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor, UINT descriptorSize);

		void OnResize(UINT newWidth, UINT newHeight);

		///<summary>
		/// Blurs the input texture blurCount times.
		///</summary>
		void Execute(
			ID3D12GraphicsCommandList* cmdList,
			ID3D12RootSignature* rootSig,
			ID3D12PipelineState* horzBlurPSO,
			ID3D12PipelineState* vertBlurPSO,
			ID3D12Resource* input,
			int blurCount);

	private:
		std::vector<float> CalcGaussWeights(float sigma);

		void BuildDescriptors();
		void BuildResources();

	private:
		const int MAX_BLUR_RADIUS = 5;

		ID3D12Device* m_d3dDevice = nullptr;

		UINT m_width = 0;
		UINT m_height = 0;
		DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blur0CpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blur0CpuUav;

		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blur1CpuSrv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_blur1CpuUav;

		CD3DX12_GPU_DESCRIPTOR_HANDLE m_blur0GpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_blur0GpuUav;

		CD3DX12_GPU_DESCRIPTOR_HANDLE m_blur1GpuSrv;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_blur1GpuUav;

		// Two for ping-ponging the textures.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_blurMap0 = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_blurMap1 = nullptr;

	};
}