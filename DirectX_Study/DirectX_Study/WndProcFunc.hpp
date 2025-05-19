#ifndef WND_PROC_FUNC_HPP
#define WND_PROC_FUNC_HPP

#include <Windows.h>

#include "DrawFunc.hpp"

class WndProcFunc
{
private:
	HINSTANCE m_hInst;

public:

	WndProcFunc(HINSTANCE hInst) : m_hInst(hInst)
	{
	}

	~WndProcFunc()
	{
	}

public:

	UINT OnCreate(HWND hWnd)
	{
		return 0;
	}

	UINT OnDestroy(HWND hWnd)
	{
		PostQuitMessage(0);
		return 0;
	}

	VOID OnPaint(HWND hWnd, HDC hdc)
	{
		DrawLine(hdc, 100, 50, 0, 600, 300, 0);
	}

};

#endif