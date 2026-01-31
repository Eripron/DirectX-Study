#include "GraphicEngine.h"
#include "../Window/WindowsUtils.h"

using namespace DK;

GraphicEngine::GraphicEngine(HWND hWnd)
{
	m_hWnd = hWnd;
	
	m_wstrWndTitle = WindowsUtils::GetWindowTextTitle(m_hWnd);
	m_wstrWndTitle.pop_back();

	WindowsUtils::GetScreenSize(m_hWnd, &m_nClientWidth, &m_nClientHeight);
}

GraphicEngine::~GraphicEngine()
{
	// GPU가 아직 실행 중인데 프로그램을 종료하면 문제가 될 수 있으므로 gpu를 강제로 한번 비운다.
	if (m_d3dDevice != nullptr)
		FlushCommandQueue();
}

bool DK::GraphicEngine::Initialize()
{
	if (InitDirect3D() == false)
		return false;

	m_gameTimer.Init();
	OnResize(m_nClientWidth, m_nClientHeight, true);

	THROW_IF_FAILED(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

#if GIZMO
	m_gizmo.Init(m_d3dDevice.Get(), m_commandList.Get(), 3, m_eBackBufferFormat, m_eDepthStencilFormat);
#endif

	_shadowMap = std::make_unique<ShadowMap>(m_d3dDevice.Get(), 2048, 2048);
	_ssaoMap = std::make_unique<SsaoMap>(m_d3dDevice.Get(), m_commandList.Get(), m_nClientWidth, m_nClientHeight);

	Init();

	THROW_IF_FAILED(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, cmdLists);

	FlushCommandQueue();

	return true;
}

void GraphicEngine::Run()
{
	m_gameTimer.Tick();

	if (m_bAppPaused == false)
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

bool GraphicEngine::InitDirect3D()
{
	// TODO: D3D12GetDebugInterface함수의 목적 & 디버깅 관리 방법
#if defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
	THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif

	// create dxgi factory
	THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	// create adapter
	HRESULT hResult = D3D12CreateDevice(
		nullptr,					// nullptr = use default adapter
		D3D_FEATURE_LEVEL_11_0,		// version 11
		IID_PPV_ARGS(&m_d3dDevice));

	// if failed create adapter
	// create warp adapter
	if (FAILED(hResult))
	{
		// Window Advanced Rasterization Platform
		// : GPU가 없거나 호환되지 않는 경우 directx 기능을 실행할 수 있도록 하는 소프트웨어 랜더러
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));

		D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));
	}

	// create fence
	m_ullCurrentFence = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateFence(m_ullCurrentFence, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	// get Descriptor size
	m_uRtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_uDsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_uCbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// multi sample anti aliasing check
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_eBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	THROW_IF_FAILED(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_b4xMsaaState = false;
	m_u4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_u4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

#ifdef _DEBUG
	LogAdapters();
#endif 

	return true;
}

void GraphicEngine::CreateCommandObjects()
{
	D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = commandListType;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	THROW_IF_FAILED(m_d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue)));
	THROW_IF_FAILED(m_d3dDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&m_commandAlloc)));
	THROW_IF_FAILED(m_d3dDevice->CreateCommandList(0, commandListType, m_commandAlloc.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

	// CommandList는 생성하면 기본적으로 열린 상태가 된다.
	// 하지만 명령을 기록하기 위해서는 닫혀이어야 하므로 닫는다.
	m_commandList->Close();
}

void GraphicEngine::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	m_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = m_nClientWidth;
	swapChainDesc.BufferDesc.Height = m_nClientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = m_eBackBufferFormat;
	// ScanlineOrdering: 화면 출력순서 지정 타입
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// default value(디스플레이에게 맡김)
	// Scaling: 해상도와 화면크기가 맞지 않을때 어떻게 처리할지
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// default(하드웨어 자동 판단)
	swapChainDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = true;
	// SwapEffect: Buffer를 어떻게 변경할 것인지
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// 권장: buffer포인터 변경 + back Buffer 버림
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// 전체화면에서 여러 해상도 모드 변경 가능

	// Note: Swap chain uses queue to perform flush.
	THROW_IF_FAILED(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &m_swapChain));
}

void GraphicEngine::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// Render Target View로 사용한다.
	rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT + 3;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	int shadowDsvCount = 1;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;		// Depth-Stencil View로 사용한다.
	dsvHeapDesc.NumDescriptors = 1 + shadowDsvCount;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void GraphicEngine::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_ullCurrentFence += 1;

	// gpu가 쌓인 CommandList를 실행하고 모두 완료 후 Fence에 해당 값을 설정한다.
	THROW_IF_FAILED(m_commandQueue->Signal(m_fence.Get(), m_ullCurrentFence));

	if (m_fence->GetCompletedValue() < m_ullCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		THROW_IF_FAILED(m_fence->SetEventOnCompletion(m_ullCurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void GraphicEngine::Init()
{
}

bool GraphicEngine::OnResize(int width, int height, bool force)
{
	if (!force && m_nClientWidth == width && m_nClientHeight == height)
		return false;

	m_nClientWidth = width;
	m_nClientHeight = height;

	assert(m_d3dDevice);
	assert(m_swapChain);
	assert(m_commandAlloc);

	// 리소스 사용중일수 있으니 GPU작업 끝나기를 기다린다.
	FlushCommandQueue();

	THROW_IF_FAILED(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

	// 버퍼 참조 해제
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		m_swapChainBuffer[i].Reset();
	}
	m_depthStencilBuffer.Reset();

	// Swap Chain 사이즈 수정
	THROW_IF_FAILED(m_swapChain->ResizeBuffers(
		SWAP_CHAIN_BUFFER_COUNT,
		m_nClientWidth,
		m_nClientHeight,
		m_eBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_nCurrBackBuffer = 0;

	// RTV Buffer 리소스 생성
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandleRTV = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		THROW_IF_FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, heapHandleRTV);
		heapHandleRTV.ptr += m_uRtvDescriptorSize;	// 한칸씩 이동
	}

	// 스탠실.뎁스 버퍼 리소스 및 뷰 생성
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;		// 정렬 기준(0 기본값)
	depthStencilDesc.Width = m_nClientWidth;
	depthStencilDesc.Height = m_nClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_u4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_eDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapPropertis;
	heapPropertis.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertis.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertis.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertis.CreationNodeMask = 1;
	heapPropertis.VisibleNodeMask = 1;

	THROW_IF_FAILED(m_d3dDevice->CreateCommittedResource(
		&heapPropertis,			// default heap
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(&m_depthStencilBuffer)));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_eDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_depthStencilBuffer.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);

	// Execute the resize commands.
	THROW_IF_FAILED(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// 뷰포트와 Rect 수정
	m_viewPortScreen.TopLeftX = 0;
	m_viewPortScreen.TopLeftY = 0;
	m_viewPortScreen.Width = static_cast<float>(m_nClientWidth);
	m_viewPortScreen.Height = static_cast<float>(m_nClientHeight);
	m_viewPortScreen.MinDepth = 0.0f;
	m_viewPortScreen.MaxDepth = 1.0f;

	m_rectScissor = { 0, 0, m_nClientWidth, m_nClientHeight };

	m_camera.SetAspect(AspectRatio());

	return true;
}

bool GraphicEngine::Update()
{
	UpdateCamera();

#if GIZMO
	m_gizmo.Update(&m_camera);
#endif

	return true;
}

bool GraphicEngine::Render()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	// GPU가 모든 명령 실행하면 모든 명령을 초기화하고 다시 기록하기 위해서 reset
	THROW_IF_FAILED(m_commandAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	// Command List는 초기화하면 Alloc의 연결도 해제되므로 다시 연결
	THROW_IF_FAILED(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

	// 백버퍼 상태 전환 (Present -> Render Target)
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = CurrentBackBuffer();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrier);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	m_commandList->RSSetViewports(1, &m_viewPortScreen);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	// 백버퍼, 스탠실.뎁스 버퍼 초기화
	const float clearColor[] = { 255.0f, 0.0f, 255.0f, 1.0f };
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Output Merger에서 렌더 버퍼와 스탠실.뎁스 버퍼 등록
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = CurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
	m_commandList->OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	// 백버퍼 상태 전환 (Render Target -> Present)
	D3D12_RESOURCE_BARRIER barrier1 = {};
	barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier1.Transition.pResource = CurrentBackBuffer();
	barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier1.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrier1);

	// Command 기록이 끝나면
	THROW_IF_FAILED(m_commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	THROW_IF_FAILED(m_swapChain->Present(0, 0));
	m_nCurrBackBuffer = (m_nCurrBackBuffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();

	return true;
}

void DK::GraphicEngine::UpdateCamera()
{
	bool bRButtonState = GetRMouseDown();
	if (m_bRButtonClicked == false && bRButtonState)
	{
		m_bRButtonClicked = true;
		GetCursorPos(&m_preCursorPos);
	}
	else if (m_bRButtonClicked && bRButtonState == false)
	{
		m_bRButtonClicked = false;
	}

	if (bRButtonState)
	{
		// rotation
		POINT curCursorPos;
		GetCursorPos(&curCursorPos);

		//DirectX::XMFLOAT3 camRot = m_camera.GetTransform().GetRotation();
		// 마우스 이동을 회전 값으로 세팅
		float sensitivity = 0.05f;
		float dx = XMConvertToRadians(static_cast<float>(curCursorPos.y - m_preCursorPos.y) * sensitivity);
		float dy = XMConvertToRadians(static_cast<float>(curCursorPos.x - m_preCursorPos.x) * sensitivity);

		DirectX::XMFLOAT3 rotation(dx, dy, 0.0f);
		m_camera.Rotate(rotation);
		//m_camera.GetTransform().SetRotation(camRot.x, camRot.y, camRot.z);

		m_preCursorPos = curCursorPos;

		// move
		DirectX::XMFLOAT3 dir =
		{
			GetKeyDownValue('a', -1.0f, 'd', 1.0f),
			GetKeyDownValue('q', -1.0f, 'e', 1.0f),
			GetKeyDownValue('s', -1.0f, 'w', 1.0f)
		};

		float speed = 0.005f;
		if (GetKeyDown(VK_SHIFT)) speed *= 2.5f;

		m_camera.Move(dir * speed);
	}
}

HWND GraphicEngine::GetHandleWindow() const
{
	return m_hWnd;
}

float GraphicEngine::AspectRatio() const
{
	return static_cast<float>(m_nClientWidth) / m_nClientHeight;
}

ID3D12Resource* GraphicEngine::CurrentBackBuffer() const
{
	return m_swapChainBuffer[m_nCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicEngine::CurrentBackBufferView() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (m_nCurrBackBuffer * m_uRtvDescriptorSize);
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicEngine::DepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

bool GraphicEngine::Get4xMassState() const
{
	return m_b4xMsaaState;
}

void GraphicEngine::Set4xMassState(bool state)
{
	if (m_b4xMsaaState != state)
	{
		m_b4xMsaaState = state;

		// Recreate the swapchain and buffers with new multisample settings.
		CreateSwapChain();
		OnResize(m_nClientWidth, m_nClientHeight, true);
	}
}

bool DK::GraphicEngine::GetKeyDown(int vKey)
{
	if (vKey >= 'a' && vKey <= 'z')
		vKey -= ('a' - 'A');

	return GetAsyncKeyState(vKey);
}

float DK::GraphicEngine::GetKeyDownValue(char c1, float f1, char c2, float f2)
{
	bool bKeyC1 = GetKeyDown(c1);
	bool bKeyC2 = GetKeyDown(c2);
	if (bKeyC1 ^ bKeyC2)
	{
		return bKeyC1 ? f1 : f2;
	}
	return 0.f;
}

bool DK::GraphicEngine::GetRMouseDown()
{
	return GetKeyDown(VK_RBUTTON);
}

void GraphicEngine::CalculateFrameStats()
{
#if defined(_DEBUG)
	static int nFrameCount = 0;
	static float fElafasedTime = 0.0f;

	nFrameCount += 1;

	if (m_gameTimer.TotalTime() - fElafasedTime >= 1.0f)
	{
		float fps = (float)nFrameCount;
		float mspf = 1000.0f / fps;

		std::wstring strFps = std::to_wstring((int)fps);
		std::wstring strMspf = std::to_wstring(mspf);
		std::wstring text = m_wstrWndTitle + L"  (fps: " + strFps + L", mspf: " + strMspf + L")";

		SetWindowText(GetHandleWindow(), text.c_str());

		nFrameCount = 0;
		fElafasedTime += 1.0f;
	}
#endif
}

void GraphicEngine::LogAdapters()
{
	int i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;

	while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);

		adapterList.push_back(adapter);

		std::wstring text = L"***Adapter: ";
		text += adapterDesc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		++i;
	}

	for (i = 0; i < adapterList.size(); ++i)
	{
		if (adapterList[i])
			adapterList[i]->Release();
	}
}
