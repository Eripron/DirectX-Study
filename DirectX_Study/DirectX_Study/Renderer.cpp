#include "Renderer.h"

#include "GraphicUtils.h"
#include "WindowsUtils.h"

using namespace DK;

Renderer::~Renderer()
{
}

void Renderer::Init(HWND hWnd)
{
	if (hWnd == INVALID_HANDLE_VALUE)
		return;

	_hWnd = hWnd;
	GetClientRect(_hWnd, &_rtScreen);
	WindowsUtils::GetScreenSize(_hWnd, &_vScreenSize.x, &_vScreenSize.y);

	_hdc = GetDC(_hWnd);
	_memDC = CreateCompatibleDC(_hdc);
	_hBitmap = CreateCompatibleBitmap(_hdc, (int)_vScreenSize.x, (int)_vScreenSize.y);
	_hOldBitmap = (HBITMAP)SelectObject(_memDC, _hBitmap);
}

void DK::Renderer::PreUpdate()
{
	FillRect(_memDC, &_rtScreen, GetSysColorBrush(COLOR_WINDOW));
}

void DK::Renderer::LastUpdate()
{
	BitBlt(_hdc, 0, 0, _vScreenSize.x, _vScreenSize.y, _memDC, 0, 0, SRCCOPY);
}

void DK::Renderer::DrawPixel(Vector3 point)
{
	SetPixel(_memDC, point.x, point.y, RGB(0, 0, 0));
}

void DK::Renderer::DrawLine(Vector3 p1, Vector3 p2)
{
	GraphicUtils::DrawLine(_memDC, p1, p2, RGB(0, 0, 0));
}
