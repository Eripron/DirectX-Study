#include "BlurFilter.h"

// 호출 필요
DK::BlurFilter::BlurFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	m_d3dDevice = device;
	m_width = width;
	m_height = height;
	m_format = format;

	BuildResources();
}

ID3D12Resource* DK::BlurFilter::Output()
{
	return m_blurMap0.Get();
}

// 호출 필요
void DK::BlurFilter::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
										CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
										UINT descriptorSize)
{
	m_srvHandleBlur0 = hCpuDescriptor;
	m_uavHandleBlur0 = hCpuDescriptor.Offset(1, descriptorSize);
	m_srvHandleBlur1 = hCpuDescriptor.Offset(1, descriptorSize);
	m_uavHandleBlur1 = hCpuDescriptor.Offset(1, descriptorSize);

	m_srvGPUHandleBlur0 = hGpuDescriptor;
	m_uavGPUHandleBlur0 = hGpuDescriptor.Offset(1, descriptorSize);
	m_srvGPUHandleBlur1 = hGpuDescriptor.Offset(1, descriptorSize);
	m_uavGPUHandleBlur1 = hGpuDescriptor.Offset(1, descriptorSize);

	BuildDescriptors();
}

void DK::BlurFilter::OnResize(UINT width, UINT height)
{
	if ((m_width != width) || (m_height != height))
	{
		m_width = width;
		m_height = height;

		BuildResources();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}

void DK::BlurFilter::Execute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* horzBlurPSO, ID3D12PipelineState* vertBlurPSO, ID3D12Resource* input, int blurCount)
{
	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;

	// root signature 바인딩
	cmdList->SetComputeRootSignature(rootSig);

	// 상수 바인딩
	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	// blur0 리소스에 Render Target Buffer 복사
	auto RenderTarget2CopySource = CD3DX12_RESOURCE_BARRIER::Transition(input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto Common2CopyDest = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	auto CopyDest2Read = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto Common2UnorderedAccess = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	cmdList->ResourceBarrier(1, &RenderTarget2CopySource);
	cmdList->ResourceBarrier(1, &Common2CopyDest);
	cmdList->CopyResource(m_blurMap0.Get(), input);
	cmdList->ResourceBarrier(1, &CopyDest2Read);
	// blur1 리소스에 write 할 수 있도록 수정
	cmdList->ResourceBarrier(1, &Common2UnorderedAccess);

	for (int i = 0; i < blurCount; ++i)
	{
		// 수평 흐리기 pso 설정
		cmdList->SetPipelineState(horzBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, m_srvGPUHandleBlur0);
		cmdList->SetComputeRootDescriptorTable(2, m_uavGPUHandleBlur1);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(m_width / 256.0f);
		cmdList->Dispatch(numGroupsX, m_height, 1);
		// cs를 통해서 blur1에 수평 흐리기가 출력되었음

		// blur0에 결과를 출력하기 위해서 uav로 설정
		auto Read2UA = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap0.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->ResourceBarrier(1, &Read2UA);
		// blur1(수평 흐리기 출력 결과)를 입력으로 사용

		auto UA2Read = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap1.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &UA2Read);

		// 수직 흐리기 pso 설정
		cmdList->SetPipelineState(vertBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, m_srvGPUHandleBlur1);
		cmdList->SetComputeRootDescriptorTable(2, m_uavGPUHandleBlur0);

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		UINT numGroupsY = (UINT)ceilf(m_height / 256.0f);
		cmdList->Dispatch(m_width, numGroupsY, 1);

		auto UA2ReadBlur0 = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap0.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->ResourceBarrier(1, &UA2ReadBlur0);

		auto Read2UABlur1 = CD3DX12_RESOURCE_BARRIER::Transition(m_blurMap1.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->ResourceBarrier(1, &Read2UABlur1);
	}
}

std::vector<float> DK::BlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	//assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void DK::BlurFilter::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_d3dDevice->CreateShaderResourceView(m_blurMap0.Get(), &srvDesc, m_srvHandleBlur0);
	m_d3dDevice->CreateUnorderedAccessView(m_blurMap0.Get(), nullptr, &uavDesc, m_uavHandleBlur0);

	m_d3dDevice->CreateShaderResourceView(m_blurMap1.Get(), &srvDesc, m_srvHandleBlur1);
	m_d3dDevice->CreateUnorderedAccessView(m_blurMap1.Get(), nullptr, &uavDesc, m_uavHandleBlur1);
}

void DK::BlurFilter::BuildResources()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;	// uav resource

	CD3DX12_HEAP_PROPERTIES property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	THROW_IF_FAILED(m_d3dDevice->CreateCommittedResource(
		&property,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_blurMap0)));

	THROW_IF_FAILED(m_d3dDevice->CreateCommittedResource(
		&property,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_blurMap1)));
}
