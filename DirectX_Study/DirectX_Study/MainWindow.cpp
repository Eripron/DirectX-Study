#include <Windows.h>
#include <CommCtrl.h>

#include "resource.h"
#include "MainWindow.h"
#include "WindowsUtils.h"

#pragma comment(lib, "comctl32.lib")

namespace DK
{
	bool MainWindow::Create(HINSTANCE hInstance, LPCTSTR strClassName, LPCTSTR strWindowTitle, int nCmdShow)
	{
		_hInst = hInstance;

		WNDCLASSEX wndClassEx;
		wndClassEx.cbSize = sizeof(WNDCLASSEX);
		wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
		wndClassEx.lpfnWndProc = WndProc;
		wndClassEx.cbClsExtra = 0;
		wndClassEx.cbWndExtra = 0;
		wndClassEx.hInstance = hInstance;
		wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wndClassEx.lpszMenuName = NULL;
		wndClassEx.lpszClassName = strClassName;
		wndClassEx.hIconSm = NULL;
		
		if (!RegisterClassEx(&wndClassEx))
		{
			// fail register window
			MessageBox(0, L"Register Window Failed", L"Error", 0);
			return false;
		}

		_hWnd = CreateWindow(strClassName, strWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, this);
		if (!_hWnd)
		{
			// fail create window
			MessageBox(0, L"Create Window Failed", L"Error", 0);
			return false;
		}

		ShowWindow(_hWnd, nCmdShow);

		return true;
	}

	bool MainWindow::Run()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				return false;
		}

		return true;
	}

	void MainWindow::Render()
	{
	/*	if (_sValueBar != NULL)
			RedrawWindow(_sValueBar, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		if (_vValueBar != NULL)
			RedrawWindow(_vValueBar, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);*/
	}

	void MainWindow::Destroy()
	{
		DestroyWindow(_hWnd);
	}

	HWND MainWindow::GetHandle()
	{
		return _hWnd;
	}

	LRESULT MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		MainWindow* pThis;

		if (message == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (MainWindow*)pCreate->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
		}
		else
		{
			pThis = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis)
		{
			return pThis->HandleMessage(hWnd, message, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	LRESULT MainWindow::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CREATE:
			Create(hWnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			return 0;

		/*case WM_HSCROLL:
		{
			float s, v;

			int value = (int)SendMessage(_sValueBar, TBM_GETPOS, 0, 0);
			s = value / 1000.0f;

			value = (int)SendMessage(_vValueBar, TBM_GETPOS, 0, 0);
			v = value / 1000.0f;

			if(BarValueChanged)
				BarValueChanged(s, v);
		}*/
		/*case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			return 0;
		}*/
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void MainWindow::Create(HWND hWnd)
	{
		// load control window
		//INITCOMMONCONTROLSEX icex;
		//icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		//icex.dwICC = ICC_BAR_CLASSES;	// load control bit flag
		//if (InitCommonControlsEx(&icex))
		//{
		//	// success load control
		//	_sValueBar = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | TBS_NOTICKS, 10, 10, 200, 40, hWnd, (HMENU)0, _hInst, NULL);
		//	if (_sValueBar != NULL)
		//	{
		//		SendMessage(_sValueBar, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
		//		SendMessage(_sValueBar, TBM_SETPOS, TRUE, 1000);
		//	}

		//	_vValueBar = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | TBS_NOTICKS, 10, 60, 200, 40, hWnd, (HMENU)1, _hInst, NULL);
		//	if (_vValueBar != NULL)
		//	{
		//		SendMessage(_vValueBar, TBM_SETRANGE, FALSE, MAKELPARAM(0, 1000));
		//		SendMessage(_vValueBar, TBM_SETPOS, TRUE, 1000);
		//	}
		//}
	}

}