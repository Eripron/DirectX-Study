#include "CubeRenderTarget.h"

DK::CubeRenderTarget::CubeRenderTarget(ID3D12Device* pDevice, UINT unWidth, UINT unHeight, DXGI_FORMAT format)
	: _pDevice(pDevice), _unWidth(unWidth), _unHeight(unHeight), _format(format)
{
	_viewPort = { 0.0f, 0.0f, (float)unWidth, (float)unHeight, 0.0f, 1.0f };
	_rect = { 0, 0, (int)unWidth, (int)unHeight };

	BuildResource();
}

void DK::CubeRenderTarget::BindDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv[6], CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv)
{
	_hCpuSrv = hCpuSrv;
	_hGpuSrv = hGpuSrv;

	for (int i = 0; i < 6; ++i)
		_hCpuRtv[i] = hCpuRtv[i];

	_hCpuDsv = hCpuDsv;

	BuildDescriptors();
}

void DK::CubeRenderTarget::DrawSceneToCubeMap(ID3D12GraphicsCommandList* cmdList, std::function<void(int)> drawFunc)
{
	cmdList->RSSetViewports(1, &_viewPort);
	cmdList->RSSetScissorRects(1, &_rect);

	// cube map을 render target으로 상태 변경

	CD3DX12_RESOURCE_BARRIER transition = CD3DX12_RESOURCE_BARRIER::Transition(_cubeMap.Get(), 
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	cmdList->ResourceBarrier(1, &transition);

	// 6개 면 돌면서 바인딩하고 출력을 해야겠지?
	for (int i = 0; i < 6; ++i)
	{
		// render target & depth stencil 클리어
		cmdList->ClearRenderTargetView(_hCpuRtv[i], DirectX::Colors::DimGray, 0, nullptr);
		cmdList->ClearDepthStencilView(_hCpuDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// render target 바인딩
		cmdList->OMSetRenderTargets(1, &_hCpuRtv[i], true, &_hCpuDsv);
		
		drawFunc(i);
	}

	transition = CD3DX12_RESOURCE_BARRIER::Transition(_cubeMap.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	cmdList->ResourceBarrier(1, &transition);
}

void DK::CubeRenderTarget::BuildResource()
{
	// device가 없거나 
	if (_pDevice == nullptr || _cubeMap != nullptr) return;

	D3D12_RESOURCE_DESC rscDesc;
	ZeroMemory(&rscDesc, sizeof(D3D12_RESOURCE_DESC));
	rscDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
#pragma region Region
	rscDesc.Alignment = 0;
	rscDesc.Width = _unWidth;
	rscDesc.Height = _unHeight;
#pragma endregion
	rscDesc.DepthOrArraySize = 6;		// 6개의 면을 가지므로 배열 사이즈 6으로 설정
#pragma region Region
	rscDesc.MipLevels = 1;
	rscDesc.Format = _format;
	rscDesc.SampleDesc.Count = 1;
	rscDesc.SampleDesc.Quality = 0;
	rscDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
#pragma endregion

	rscDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;	// 각 면에 그려야 하므로 Render Target으로 설정

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	_pDevice->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &rscDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_cubeMap));
}

void DK::CubeRenderTarget::BuildDescriptors()
{
	// CubeMap 리소스에 대한 Descriptor View 생성
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = _format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	_pDevice->CreateShaderResourceView(_cubeMap.Get(), &srvDesc, _hCpuSrv);

	// CubeMap 리소스를 사용해서 Render Target View를 생성
	for(int i = 0; i < 6; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = _format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;

		_pDevice->CreateRenderTargetView(_cubeMap.Get(), &rtvDesc, _hCpuRtv[i]);
	}
}

