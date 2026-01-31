#include "SsaoMap.h"

SsaoMap::SsaoMap(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, UINT width, UINT height)
	: _d3dDevice(device), _renderTargetWidth(width), _renderTargetHeight(height)
{
	OnResize(width, height);

	BuildOffsetVectors();
	BuildRandomVectorTexture(cmdList);
}

void SsaoMap::BuildResource()
{
	_normalMap.Reset();
	_ambientMap0.Reset();
	_ambientMap1.Reset();

	// 화면의 normal을 출력할 texture
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = _renderTargetWidth;
	texDesc.Height = _renderTargetHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = NormalMapFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

	float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	CD3DX12_CLEAR_VALUE clearValue(NormalMapFormat, normalClearColor);

	_d3dDevice->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(&_normalMap));

	texDesc.Width = _renderTargetWidth / 2;
	texDesc.Height = _renderTargetHeight / 2;
	texDesc.Format = AmbientMapFormat;

	float ambientClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CD3DX12_CLEAR_VALUE ambientClearValue(AmbientMapFormat, ambientClearColor);

	_d3dDevice->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&ambientClearValue,
		IID_PPV_ARGS(&_ambientMap0));

	_d3dDevice->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&ambientClearValue,
		IID_PPV_ARGS(&_ambientMap1));
}

void SsaoMap::BuildDescriptors(
	ID3D12Resource* depthStencilBuffer, 
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, 
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, 
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv, 
	UINT cbvSrvUavDescriptorSize, UINT rtvDescriptorSize)
{
	_hAmbientMap0CpuRtv = hCpuSrv;
	_hAmbientMap1CpuRtv = hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hNormalMapCpuSrv = hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hDepthMapCpuSrv = hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hRandomVectorMapCpuSrv = hCpuSrv.Offset(1, cbvSrvUavDescriptorSize);

	_hAmbientMap0GpuSrv = hGpuSrv;
	_hAmbientMap1GpuSrv = hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hNormalMapGpuSrv = hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hDepthMapGpuSrv = hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);
	_hRandomVectorMapGpuSrv = hGpuSrv.Offset(1, cbvSrvUavDescriptorSize);

	_hNormalMapCpuRtv = hCpuRtv;
	_hAmbientMap0CpuRtv = hCpuRtv.Offset(1, rtvDescriptorSize);
	_hAmbientMap1CpuRtv = hCpuRtv.Offset(2, rtvDescriptorSize);

	RebuildDescriptos(depthStencilBuffer);
}

void SsaoMap::RebuildDescriptos(ID3D12Resource* depthStencilBuffer)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = NormalMapFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	_d3dDevice->CreateShaderResourceView(_normalMap.Get(), &srvDesc, _hNormalMapCpuSrv);

	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	_d3dDevice->CreateShaderResourceView(depthStencilBuffer, &srvDesc, _hDepthMapCpuSrv);

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	_d3dDevice->CreateShaderResourceView(_randomVectorMap.Get(), &srvDesc, _hRandomVectorMapCpuSrv);

	srvDesc.Format = AmbientMapFormat;
	_d3dDevice->CreateShaderResourceView(_ambientMap0.Get(), &srvDesc, _hAmbientMap0CpuSrv);
	_d3dDevice->CreateShaderResourceView(_ambientMap1.Get(), &srvDesc, _hAmbientMap1CpuSrv);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = NormalMapFormat;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	_d3dDevice->CreateRenderTargetView(_normalMap.Get(), &rtvDesc, _hNormalMapCpuRtv);

	rtvDesc.Format = AmbientMapFormat;
	_d3dDevice->CreateRenderTargetView(_ambientMap0.Get(), &rtvDesc, _hAmbientMap0CpuRtv);
	_d3dDevice->CreateRenderTargetView(_ambientMap1.Get(), &rtvDesc, _hAmbientMap1CpuRtv);
}

void SsaoMap::ComputeSsao(ID3D12GraphicsCommandList* cmdList, FrameResource* currFrame, int blurCount)
{
	cmdList->RSSetViewports(1, &_viewport);
	cmdList->RSSetScissorRects(1, &_scissorRect);

	// We compute the initial SSAO to AmbientMap0.

	// ambientMap0을 Read -> RenderTarget으로 변경
	auto readToRt = CD3DX12_RESOURCE_BARRIER::Transition(_ambientMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList->ResourceBarrier(1, &readToRt);

	float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	cmdList->ClearRenderTargetView(_hAmbientMap0CpuSrv, clearValue, 0, nullptr);

	// Specify the buffers we are going to render to.
	cmdList->OMSetRenderTargets(1, &_hAmbientMap0CpuSrv, true, nullptr);

	// Bind the constant buffer for this pass.
	auto ssaoCBAddress = currFrame->SsaoCB->GetBuffer()->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootConstantBufferView(0, ssaoCBAddress);
	cmdList->SetGraphicsRoot32BitConstant(1, 0, 0);

	// normal map & depth map 바인드
	cmdList->SetGraphicsRootDescriptorTable(2, _hNormalMapGpuSrv);

	// random vector map 바인드
	cmdList->SetGraphicsRootDescriptorTable(3, _hRandomVectorMapGpuSrv);

	cmdList->SetPipelineState(_ssaoPso);

	// Draw fullscreen quad.
	cmdList->IASetVertexBuffers(0, 0, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	auto rtToRead = CD3DX12_RESOURCE_BARRIER::Transition(_ambientMap0.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &rtToRead);

	//BlurAmbientMap(cmdList, currFrame, blurCount);
}

void SsaoMap::OnResize(UINT width, UINT height)
{
	if (_renderTargetWidth != width || _renderTargetHeight != height)
	{
		_renderTargetWidth = width;
		_renderTargetHeight = height;

		_viewport.TopLeftX = 0.0f;
		_viewport.TopLeftY = 0.0f;
		_viewport.Width = _renderTargetWidth / 2;
		_viewport.Height = _renderTargetHeight / 2;
		_viewport.MinDepth = 0.0f;
		_viewport.MaxDepth = 1.0f;

		_scissorRect.left = 0;
		_scissorRect.top = 0;
		_scissorRect.right = static_cast<LONG>(_renderTargetWidth / 2);
		_scissorRect.bottom = static_cast<LONG>(_renderTargetHeight / 2);

		BuildResource();
	}
}

UINT SsaoMap::GetSsaoMapWidth() const
{
	return _renderTargetWidth / 2;
}

UINT SsaoMap::GetSsaoMapHeight() const
{
	return _renderTargetHeight / 2;
}

void SsaoMap::GetOffsetVectors(DirectX::XMFLOAT4 offsets[14])
{
	std::copy(&_offsets[0], &_offsets[14], &_offsets[0]);
}

std::vector<float> SsaoMap::CalcGaussWeights(float sigma)
{
	return std::vector<float>();
}

ID3D12Resource* SsaoMap::NormalMap() const
{
	return _normalMap.Get();
}

ID3D12Resource* SsaoMap::AmbientMap() const
{
	return _ambientMap0.Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE SsaoMap::NormalMapRtv() const
{
	return _hNormalMapCpuRtv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE SsaoMap::NormalMapSrv() const
{
	return _hNormalMapGpuSrv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE SsaoMap::AmbientMapSrv() const
{
	return _hAmbientMap0GpuSrv;
}

void SsaoMap::SetPSOs(ID3D12PipelineState* ssaoPso, ID3D12PipelineState* ssaoBlurPso)
{
	_ssaoPso = ssaoPso;
	_ssaoBlurPso = ssaoBlurPso;
}

/// <summary>
/// ssao의 한 지점에서 반원의 샘플링을 하기위한 균등 분포 벡터 생성
/// </summary>
void SsaoMap::BuildOffsetVectors()
{
	// 8 cube corners
	_offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	_offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	_offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	_offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	_offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	_offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	_offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	_offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	_offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	_offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	_offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	_offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	_offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	_offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	// TODO: 무엇을 위한 코드인가?
	for (int i = 0; i < 14; ++i)
	{
		float s = MathUtils::RandF(0.25f, 1.0f);
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&_offsets[i]));
		XMStoreFloat4(&_offsets[i], v);
	}
}

/// <summary>
/// 
/// </summary>
void SsaoMap::BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList)
{
	// TODO: 왜 256x256 텍스쳐인가?
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	_d3dDevice->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&_randomVectorMap));


	// 임시 upload buffer 생성
	// TODO: 아래 코드 설명 필요
	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(_randomVectorMap.Get(), 0, num2DSubresources);

	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	_d3dDevice->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_randomVectorMapUploadBuffer.GetAddressOf()));

	// TODO: 아래 코드의 이유에 대해서 조사 필요
	XMCOLOR initData[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			XMFLOAT3 v(MathUtils::RandF(), MathUtils::RandF(), MathUtils::RandF());

			initData[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * 256;

	//
	// Schedule to copy the data to the default resource, and change states.
	// Note that mCurrSol is put in the GENERIC_READ state so it can be 
	// read by a shader.
	//

	auto readToCopy = CD3DX12_RESOURCE_BARRIER::Transition(_randomVectorMap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &readToCopy);

	UpdateSubresources(cmdList, _randomVectorMap.Get(), _randomVectorMapUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);

	auto copyToRead = CD3DX12_RESOURCE_BARRIER::Transition(_randomVectorMap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &copyToRead);
}
