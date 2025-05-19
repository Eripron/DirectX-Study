#include <Windows.h>
#include "WndProcFunc.hpp"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Graphic");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpszCmdParam, int nCmdShow)
{
	g_hInst = hInstance;

	MSG Message;
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(RGB(0, 0, 0));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	int x = GetSystemMetrics(SM_CXFULLSCREEN) / 4;
	int y = GetSystemMetrics(SM_CYFULLSCREEN) / 4;
	HWND hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, x, y, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static WndProcFunc wndProcFunc(g_hInst);

	switch (iMessage)
	{
	case WM_CREATE:
		return wndProcFunc.OnCreate(hWnd);

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		wndProcFunc.OnPaint(hWnd, hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		return wndProcFunc.OnDestroy(hWnd);
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));;
}
