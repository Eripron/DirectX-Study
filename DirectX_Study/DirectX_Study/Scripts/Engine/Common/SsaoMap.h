#pragma once

#include "../Utils/D3DUtils.h"
#include "../Resource/FrameResource.h"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace DK;
using namespace DirectX::PackedVector;

class SsaoMap
{
public:
	static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
	static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	static const int MaxBlurRadius = 5;

	SsaoMap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height);

	SsaoMap(const SsaoMap& rhs) = delete;
	SsaoMap& operator=(const SsaoMap& rhs) = delete;
	~SsaoMap() = default;

	void BuildResource();
	void BuildDescriptors(ID3D12Resource* depthStencilBuffer,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv,
		UINT cbvSrvUavDescriptorSize,
		UINT rtvDescriptorSize);
	void RebuildDescriptos(ID3D12Resource* depthStencilBuffer);

	void ComputeSsao(ID3D12GraphicsCommandList* cmdList, FrameResource* currFrame, int blurCount);

	void OnResize(UINT width, UINT height);

	ID3D12Resource* NormalMap() const;
	ID3D12Resource* AmbientMap() const;

	CD3DX12_CPU_DESCRIPTOR_HANDLE NormalMapRtv() const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NormalMapSrv() const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE AmbientMapSrv() const;

	void SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso);

	UINT GetSsaoMapWidth() const;
	UINT GetSsaoMapHeight() const;

	void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]);
	std::vector<float> CalcGaussWeights(float sigma);

private:
	void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, FrameResource* currFrame, int blurCount);
	void BlurAmbientMap(ID3D12GraphicsCommandList* cmdList, bool horzBlur);

	void BuildOffsetVectors();
	void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList);

private:
	ID3D12Device* _d3dDevice = nullptr;

	ComPtr<ID3D12RootSignature> _ssaoRootSignature;

	ID3D12PipelineState* _ssaoPso = nullptr;
	ID3D12PipelineState* _ssaoBlurPso = nullptr;

	UINT _renderTargetWidth;
	UINT _renderTargetHeight;

	D3D12_VIEWPORT _viewport;
	D3D12_RECT _scissorRect;

	XMFLOAT4 _offsets[14];

	ComPtr<ID3D12Resource> _normalMap;
	ComPtr<ID3D12Resource> _ambientMap0;
	ComPtr<ID3D12Resource> _ambientMap1;

	ComPtr<ID3D12Resource> _randomVectorMap;
	ComPtr<ID3D12Resource> _randomVectorMapUploadBuffer;

	// cpu handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hAmbientMap0CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hAmbientMap1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hNormalMapCpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hDepthMapCpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hRandomVectorMapCpuSrv;

	// gpu handle
	CD3DX12_GPU_DESCRIPTOR_HANDLE _hAmbientMap0GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _hAmbientMap1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _hNormalMapGpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _hDepthMapGpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _hRandomVectorMapGpuSrv;

	// rtv handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hNormalMapCpuRtv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hAmbientMap0CpuRtv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _hAmbientMap1CpuRtv;
};