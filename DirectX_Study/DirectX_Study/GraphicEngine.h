#pragma once

#include <Windows.h>
#include "Renderer.h"

// directX
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <intsafe.h>
#include <string>
#include <comdef.h>
#include <synchapi.h>

#include "GameTimer.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}

namespace DK
{
	class GraphicEngine
	{
	public:
		GraphicEngine(HWND hWnd);
		~GraphicEngine();

		HWND      MainWnd() const;
		float     AspectRatio() const;

		bool Get4xMassState() const;
		void Set4xMassState(bool state);

		bool Init();
		void Run();
		void BarValueChanged(float s, float v);

	private:

		void OnResize(int width, int height);
		void Update();
		void Render();

		bool InitDirect3D();
		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();

		void FlushCommandQueue();

		ID3D12Resource* CurrentBackBuffer()const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

		void CalculateFrameStats();

		void LogAdapters();
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

		float GetXAxisInput();
		float GetYAxisInput();
		float GetRotateInput();
		float GetScaleInput();

		bool LoadBitmapData(LPCTSTR path, BITMAP& bitmap);

	private:
		Renderer _render;

		float S = 1.f;
		float V = 1.f;

	private:
		HWND mhWnd;
		int mClientWidth;
		int mClientHeight;
		bool mAppPaused = false;		// is the application paused?
		bool mMinimized = false;		// is the application minimized?
		bool mMaximized = false;		// is the application maximized?
		bool mResizing = false;			// are the resize bars being dragged?
		bool mFullscreenState = false;	// fullscreen enabled

		bool m4xMsaaState = false;
		UINT m4xMsaaQuality = 0;

		GameTimer mTimer;

		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;

		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		UINT64 mCurrentFence = 0;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		static const int SwapChainBufferCount = 2;
		int mCurrBackBuffer = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

		D3D12_VIEWPORT mScreenViewport;;
		D3D12_RECT mScissorRect;

		UINT mRtvDescriptorSize = 0;		// Render Target View
		UINT mDsvDescriptorSize = 0;		// Depth Stencil View
		UINT mCbvSrvUavDescriptorSize = 0;	//Constant Buffer Viewe, Shader Resource View, Unordered Access View

		D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	};

	class DxException
	{
	public:
		DxException() = default;
		DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber);

		std::wstring ToString() const;

		HRESULT ErrorCode = S_OK;
		std::wstring FunctionName;
		std::wstring FileName;
		int LineNumber = -1;
	};

	inline std::wstring AnsiToWString(const std::string& str)
	{
		WCHAR buffer[512];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
		return std::wstring(buffer);
	}

}