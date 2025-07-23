#include "GraphicEngine.h"

namespace DK
{
	GraphicEngine::GraphicEngine(HWND hWnd)
	{
		mhWnd = hWnd;

		size_t lenText = GetWindowTextLength(MainWnd()) + 1;
		mWndTitle = std::wstring(L"\0", lenText);
		GetWindowText(MainWnd(), &mWndTitle[0], lenText);
		mWndTitle.pop_back();
	}

	GraphicEngine::~GraphicEngine()
	{
		// ** GPU가 아직 명령을 처리하는 중 일수 있으니 한번 비운다.
		if (md3dDevice != nullptr)
			FlushCommandQueue();
	}

	HWND GraphicEngine::MainWnd() const
	{
		return mhWnd;
	}

	float GraphicEngine::AspectRatio() const
	{
		return static_cast<float>(mClientWidth) / mClientHeight;
	}

	bool GraphicEngine::Get4xMassState() const
	{
		return m4xMsaaState;
	}

	void GraphicEngine::Set4xMassState(bool state)
	{
		if (m4xMsaaState != state)
		{
			m4xMsaaState = state;

			// todo: 이유
			// Recreate the swapchain and buffers with new multisample settings.
			CreateSwapChain();
			OnResize(mClientWidth, mClientHeight);
		}
	}

	bool GraphicEngine::Init()
	{
		if (InitDirect3D() == false)
			return false;

		RECT rt;
		GetClientRect(MainWnd(), &rt);
		OnResize(rt.right - rt.left, rt.bottom - rt.top);

		mTimer.Reset();

		return true;
	}

	void GraphicEngine::Run()
	{
		mTimer.Tick();

		if (!mAppPaused)
		{
			CalculateFrameStats();
			Update();
			Render();
		}
		else
		{
			Sleep(100);
		}
	}

	void GraphicEngine::OnResize(int width, int height)
	{
		if (mClientWidth == width && mClientHeight == height)
			return;

		mClientWidth = width;
		mClientHeight = height;

		assert(md3dDevice);
		assert(mSwapChain);
		assert(mDirectCmdListAlloc);

		// 리소스 사용중일수 있으니 GPU작업 끝나기를 기다린다.
		FlushCommandQueue();

		THROW_IF_FAILED(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

		// 버퍼 참조 해제
		for (int i = 0; i < SwapChainBufferCount; ++i)
			mSwapChainBuffer[i].Reset();
		mDepthStencilBuffer.Reset();

		// Swap Chain 사이즈 수정
		THROW_IF_FAILED(mSwapChain->ResizeBuffers(
			SwapChainBufferCount,
			mClientWidth, 
			mClientHeight,
			mBackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		mCurrBackBuffer = 0;

		// RTV Buffer 리소스 생성
		D3D12_CPU_DESCRIPTOR_HANDLE handleRTV = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < SwapChainBufferCount; ++i)
		{
			THROW_IF_FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
			md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, handleRTV);
			handleRTV.ptr += mRtvDescriptorSize;	// 한칸씩 이동
		}

		// 스탠실.뎁스 버퍼 리소스 및 뷰 생성
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;	// 정렬 기준(0 기본값)
		depthStencilDesc.Width = mClientWidth;
		depthStencilDesc.Height = mClientHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.  
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = mDepthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapPropertis;
		heapPropertis.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapPropertis.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropertis.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropertis.CreationNodeMask = 1;
		heapPropertis.VisibleNodeMask = 1;

		THROW_IF_FAILED(md3dDevice->CreateCommittedResource(
			&heapPropertis,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(&mDepthStencilBuffer)));

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

		// Transition the resource from its initial state to be used as a depth buffer.
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = mDepthStencilBuffer.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		mCommandList->ResourceBarrier(1, &barrier);

		// Execute the resize commands.
		THROW_IF_FAILED(mCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Wait until resize is complete.
		FlushCommandQueue();

		// 뷰포트와 Rect 수정
		mScreenViewport.TopLeftX = 0;
		mScreenViewport.TopLeftY = 0;
		mScreenViewport.Width = static_cast<float>(mClientWidth);
		mScreenViewport.Height = static_cast<float>(mClientHeight);
		mScreenViewport.MinDepth = 0.0f;
		mScreenViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, mClientWidth, mClientHeight };
	}

	void GraphicEngine::Update()
	{
	}

	bool GraphicEngine::InitDirect3D()
	{
		// todo: debuf 매크로 설정에 대해서, 해당 기능에 대한 이유와 무엇인지
#if defined(DEBUG) || defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
#endif

		// dxgi factory 생성
		THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

		// adapter 생성
		HRESULT hResultDevice = D3D12CreateDevice(
			nullptr,	// nullptr = use default adapter
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice));
		
		// adapter 생성 실패 시 WARP으로 adapter 생성
		if (FAILED(hResultDevice))
		{
			// Window Advanced Rasterization Platform
			// : GPU가 없거나 호환되지 않는 경우 directx 기능을 실행할 수 있도록 하는 소프트웨어 랜더러
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

			D3D12CreateDevice(
				pWarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&md3dDevice));
		}

		// Fence와 서술사(뷰) 크기 가져오기
		THROW_IF_FAILED(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

		mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// MASS 수준 체크
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = mBackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		THROW_IF_FAILED(md3dDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels)));

		m4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
		LogAdapters();
#endif 

		// GPU Command 관련 생성
		CreateCommandObjects();
		// 출력 버퍼 생성
		CreateSwapChain();
		// 서술자(뷰) 힙 생성
		CreateRtvAndDsvDescriptorHeaps();

		return true;
	}

	void GraphicEngine::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		THROW_IF_FAILED(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

		THROW_IF_FAILED(md3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&mDirectCmdListAlloc)));

		THROW_IF_FAILED(md3dDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			mDirectCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(&mCommandList)));

		// CommandList는 생성하면 기본적으로 열린 상태가 된다.
		// 하지만 명령을 기록하기 위해서는 닫혀이어야 하므로 닫는다.
		mCommandList->Close();
	}

	void GraphicEngine::CreateSwapChain()
	{
		// Release the previous swapchain we will be recreating.
		mSwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = mClientWidth;
		sd.BufferDesc.Height = mClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = mBackBufferFormat;
		// ScanlineOrdering: 화면 출력순서 지정 타입
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// default value(디스플레이에게 맡김)
		// Scaling: 해상도와 화면크기가 맞지 않을때 어떻게 처리할지
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// default(하드웨어 자동 판단)
		sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = SwapChainBufferCount;
		sd.OutputWindow = mhWnd;
		sd.Windowed = true;
		// SwapEffect: Buffer를 어떻게 변경할 것인지
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 권장: buffer포인터 변경 + back Buffer 버림
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// 전체화면에서 여러 해상도 모드 변경 가능

		// Note: Swap chain uses queue to perform flush.
		THROW_IF_FAILED(mdxgiFactory->CreateSwapChain(
			mCommandQueue.Get(),
			&sd,
			&mSwapChain));
	}

	void GraphicEngine::CreateRtvAndDsvDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// Render Target View로 사용한다.
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;	// 뷰를 2개 생성
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		THROW_IF_FAILED(md3dDevice->CreateDescriptorHeap(
			&rtvHeapDesc,
			IID_PPV_ARGS(&mRtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;	// Depth-Stencil View로 사용한다.
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		THROW_IF_FAILED(md3dDevice->CreateDescriptorHeap(
			&dsvHeapDesc,
			IID_PPV_ARGS(&mDsvHeap)));
	}

	void GraphicEngine::FlushCommandQueue()
	{
		// Advance the fence value to mark commands up to this fence point.
		mCurrentFence += 1;

		// gpu가 쌓인 CommandList를 실행하고 모두 완료 후 Fence에 해당 값을 설정한다.
		THROW_IF_FAILED(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

		if (mFence->GetCompletedValue() < mCurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

			THROW_IF_FAILED(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	ID3D12Resource* GraphicEngine::CurrentBackBuffer() const
	{
		return mSwapChainBuffer[mCurrBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicEngine::CurrentBackBufferView() const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (mCurrBackBuffer * mRtvDescriptorSize);
		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicEngine::DepthStencilView() const
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void GraphicEngine::CalculateFrameStats()
	{
#if defined(DEBUG) || defined(_DEBUG)
		static int frameCount = 0;
		static float elafasedTime = 0.0f;

		frameCount += 1;

		if (mTimer.TotalTime() - elafasedTime >= 1.0f)
		{
			float fps = (float)frameCount;
			float mspf = 1000.0f / fps;

			std::wstring strFps = std::to_wstring((int)fps);
			std::wstring strMspf = std::to_wstring(mspf);
			std::wstring text = mWndTitle + L"  (fps: " + strFps + L", mspf: " + strMspf + L")";

			SetWindowText(MainWnd(), text.c_str());

			frameCount = 0;
			elafasedTime += 1.0f;
		}
#endif
	}

	void GraphicEngine::LogAdapters()
	{
		int i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;

		while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			adapterList.push_back(adapter);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			OutputDebugString(text.c_str());

			++i;
		}

		for (i = 0; i < adapterList.size(); ++i)
		{
			if (adapterList[i]) adapterList[i]->Release();
		}
	}

	void GraphicEngine::LogAdapterOutputs(IDXGIAdapter* adapter)
	{
	}

	void GraphicEngine::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{
	}

	void GraphicEngine::Render()
	{
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		// GPU가 모든 명령 실행하면 모든 명령을 초기화하고 다시 기록하기 위해서 reset
		THROW_IF_FAILED(mDirectCmdListAlloc->Reset());

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		// Command List는 초기화하면 Alloc의 연결도 해제되므로 다시 연결
		THROW_IF_FAILED(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

		// 백버퍼 상태 전환 (Present -> Render Target)
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = CurrentBackBuffer();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCommandList->ResourceBarrier(1, &barrier);

		// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);

		// 백버퍼, 스탠실.뎁스 버퍼 초기화
		const float clearColor[] = { 255.0f, 0.0f, 255.0f, 1.0f };
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Output Merger에서 렌더 버퍼와 스탠실.뎁스 버퍼 등록
		D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = CurrentBackBufferView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
		mCommandList->OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

		// 백버퍼 상태 전환 (Render Target -> Present)
		D3D12_RESOURCE_BARRIER barrier1 = {};
		barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier1.Transition.pResource = CurrentBackBuffer();
		barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier1.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		mCommandList->ResourceBarrier(1, &barrier1);

		// Command 기록이 끝나면
		THROW_IF_FAILED(mCommandList->Close());

		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		THROW_IF_FAILED(mSwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

		// Wait until frame commands are complete.  This waiting is inefficient and is
		// done for simplicity.  Later we will show how to organize our rendering code
		// so we do not have to wait per frame.
		FlushCommandQueue();
	}

	float GraphicEngine::GetXAxisInput()
	{
		bool bLeft = GetAsyncKeyState(VK_LEFT);
		bool bRight = GetAsyncKeyState(VK_RIGHT);
		if (bLeft ^ bRight)
		{
			return bLeft ? -1.0f : 1.0f;
		}
		return 0.f;
	}

	float GraphicEngine::GetYAxisInput()
	{
		bool bDown = GetAsyncKeyState(VK_DOWN);
		bool bUp = GetAsyncKeyState(VK_UP);
		if (bDown ^ bUp)
		{
			return bDown ? -1.0f : 1.0f;
		}
		return 0.0f;
	}

	float GraphicEngine::GetRotateInput()
	{
		bool bDown = GetAsyncKeyState(0x41);	// a
		bool bUp = GetAsyncKeyState(0x51);		// q
		if (bDown ^ bUp)
		{
			return bDown ? -1.0f : 1.0f;
		}
		return 0.0f;
	}

	float GraphicEngine::GetScaleInput()
	{
		bool bDown = GetAsyncKeyState(0x53);	// s 
		bool bUp = GetAsyncKeyState(0x57);		// w
		if (bDown ^ bUp)
		{
			return bDown ? -1.0f : 1.0f;
		}
		return 0.0f;
	}

	bool GraphicEngine::LoadBitmapData(LPCTSTR path, BITMAP& bitmap)
	{
		// open file
		HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		// read bmp file header
		DWORD byteRead;
		BITMAPFILEHEADER bmpFileHeader;
		if (ReadFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &byteRead, NULL) == 0 ||
			bmpFileHeader.bfType != 0x4d42)
		{
			CloseHandle(hFile);
			return false;
		}

		// read bmp info header
		BITMAPINFOHEADER bmpInfoHeader;
		if (ReadFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &byteRead, NULL) == 0)
		{
			CloseHandle(hFile);
			return false;
		}

		SetFilePointer(hFile, bmpFileHeader.bfOffBits, NULL, FILE_BEGIN);

		DWORD pixelDataSize = bmpFileHeader.bfSize - bmpFileHeader.bfOffBits;
		BYTE* data = (BYTE*)malloc(pixelDataSize);
		if (ReadFile(hFile, data, pixelDataSize, &byteRead, NULL) == 0)
		{
			free(data);
			CloseHandle(hFile);
			return false;
		}

		bitmap.bmType = 0;
		bitmap.bmWidth = bmpInfoHeader.biWidth;
		bitmap.bmHeight = bmpInfoHeader.biHeight;
		bitmap.bmWidthBytes = (((bmpInfoHeader.biWidth * bmpInfoHeader.biBitCount) + 31) / 32) * 4;
		bitmap.bmPlanes = bmpInfoHeader.biPlanes;
		bitmap.bmBitsPixel = bmpInfoHeader.biBitCount;
		bitmap.bmBits = data;

		CloseHandle(hFile);

		return true;
	}

}