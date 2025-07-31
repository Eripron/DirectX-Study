#include <Windows.h>
#include <CommCtrl.h>

#include "MainWindow.h"
#include "resource.h"
#include "WindowsUtils.h"

#pragma comment(lib, "comctl32.lib")

namespace DK
{
	bool MainWindow::Create(HINSTANCE hInstance, LPCTSTR strClassName, LPCTSTR strWindowTitle, int nCmdShow)
	{
		m_hInst = hInstance;

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

		m_hWnd = CreateWindow(strClassName, strWindowTitle, WS_OVERLAPPEDWINDOW, 
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, (HMENU)NULL, hInstance, this);

		if (!m_hWnd)
		{
			// fail create window
			MessageBox(0, L"Create Window Failed", L"Error", 0);
			return false;
		}

		ShowWindow(m_hWnd, nCmdShow);

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

	void MainWindow::Destroy()
	{
		DestroyWindow(m_hWnd);
	}

	HWND MainWindow::GetHandle()
	{
		return m_hWnd;
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
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			return 0;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

}