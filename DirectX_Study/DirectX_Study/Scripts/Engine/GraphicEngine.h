#pragma once

#include "Utils/D3DUtils.h"
#include "Utils/Gizmo.h"

#include "Data/DataTypes.h"
#include "Common/GameTimer.h"
#include "Resource/FrameResource.h"
#include <dwrite.h>
#include <d2d1.h>
#include "Resource/CubeRenderTarget.h"
#include "..\Engine\Common\ShadowMap.h"

#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#define GIZMO true;

namespace DK
{
	enum class RenderLayer : int
	{
		Opaque = 0,
		Transparent,
		AlphaTested,
		AlphaTestedTreeSprites,
		Patch,
		Sky,
		Reflection,
		Shadow,

		Count
	};

	enum class PsoType : int
	{
		Solid = 0,
		WireFrame,
	};

	class GraphicEngine
	{
	public:
		GraphicEngine(HWND hWnd);
		virtual ~GraphicEngine();

		bool Initialize();
		void Run();

	private:
		bool InitDirect3D();

		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();

		void FlushCommandQueue();

	protected:

		virtual void Init();
		virtual bool OnResize(int width, int height, bool force);

		virtual bool Update();
		virtual bool Render();

		virtual void UpdateCamera();

		// get set
		HWND GetHandleWindow() const;
		float AspectRatio() const;

		ID3D12Resource* CurrentBackBuffer()const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

		bool Get4xMassState() const;
		void Set4xMassState(bool state);

		bool GetKeyDown(int vKey);
		float GetKeyDownValue(char c1, float f1, char c2, float f2);

		bool GetRMouseDown();

		// debug
		void CalculateFrameStats();
		void LogAdapters();

	protected:
		HWND			m_hWnd;
		std::wstring	m_wstrWndTitle;
		int				m_nClientWidth;
		int				m_nClientHeight;

		bool			m_bAppPaused = false;			// is the application paused?
		bool			m_bMinimized = false;			// is the application minimized?
		bool			m_bMaximized = false;			// is the application maximized?
		bool			m_bResizing = false;			// are the resize bars being dragged?
		bool			m_bFullscreenState = false;		// fullscreen enabled

		GameTimer		m_gameTimer;

		DXGI_FORMAT		m_eBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT		m_eDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		Microsoft::WRL::ComPtr<IDXGIFactory4>	m_dxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_swapChain;

		UINT64									m_ullCurrentFence = 0;
		Microsoft::WRL::ComPtr<ID3D12Fence>		m_fence;

		UINT m_uRtvDescriptorSize = 0;			// Render Target View
		UINT m_uDsvDescriptorSize = 0;			// Depth Stencil View
		UINT m_uCbvSrvUavDescriptorSize = 0;	//Constant Buffer Viewe, Shader Resource View, Unordered Access View

		UINT m_u4xMsaaQuality = 0;
		bool m_b4xMsaaState = false;

		static const int	SWAP_CHAIN_BUFFER_COUNT = 2;
		int					m_nCurrBackBuffer = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffer[SWAP_CHAIN_BUFFER_COUNT];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_commandAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

		D3D12_VIEWPORT	m_viewPortScreen;
		D3D12_RECT		m_rectScissor;

		// camera
		Camera	m_camera;
		bool	m_bRButtonClicked = false;
		POINT	m_preCursorPos;

		// shadow map
		std::unique_ptr<ShadowMap> _shadowMap;
		int dsvHeapIndexShadowMap = 1;

		// gizmo
		Gizmo	m_gizmo;
	};

}