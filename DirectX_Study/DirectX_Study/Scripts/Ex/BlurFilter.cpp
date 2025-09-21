#include "BlurFilter.h"

DK::BlurFilter::BlurFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	m_d3dDevice = device;
	m_width = width;
	m_height = height;
	m_format = format;
}

ID3D12Resource* DK::BlurFilter::Output()
{
	return nullptr;
}

void DK::BlurFilter::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor, 
									  CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor, 
									  UINT descriptorSize)
{
}

void DK::BlurFilter::OnResize(UINT newWidth, UINT newHeight)
{
}

void DK::BlurFilter::Execute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* horzBlurPSO, ID3D12PipelineState* vertBlurPSO, ID3D12Resource* input, int blurCount)
{
}

std::vector<float> DK::BlurFilter::CalcGaussWeights(float sigma)
{
	return std::vector<float>();
}

void DK::BlurFilter::BuildDescriptors()
{
	// Q? 
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	// Q? 
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;


}

void DK::BlurFilter::BuildResources()
{
}
