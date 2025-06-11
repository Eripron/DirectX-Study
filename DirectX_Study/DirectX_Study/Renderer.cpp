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
	_hdc = GetDC(_hWnd);
	_memDC = CreateCompatibleDC(_hdc);

	float fWidth = 0.0f;
	float fHeight = 0.0f;
	WindowsUtils::GetScreenSize(_hWnd, &fWidth, &fHeight);

	_hBitmap = CreateCompatibleBitmap(_hdc, (int)fWidth, (int)fHeight);
	_hOldBitmap = (HBITMAP)SelectObject(_memDC, _hBitmap);
}

void DK::Renderer::PreUpdate()
{
	RECT rt;
	GetClientRect(_hWnd, &rt);
	FillRect(_memDC, &rt, GetSysColorBrush(COLOR_WINDOW));
}

void DK::Renderer::LastUpdate()
{
	RECT rt;
	GetClientRect(_hWnd, &rt);
	BitBlt(_hdc, 0, 0, rt.right - rt.left, rt.bottom - rt.top, _memDC, 0, 0, SRCCOPY);
}

void DK::Renderer::DrawLine(Vector3 a, Vector3 b)
{
	GraphicUtils::DrawLine(_memDC, a, b, RGB(0, 0, 0));
}

void DK::Renderer::Render()
{
	Vector3 point(500, 500, 0);
	int r = 200;

	Vector3 p1 = point + (Vector3::Up() * -r);
	Vector3 p2 = point + (Matrix44::RotateZMatrix44(30) * Vector3::Right() * r);
	Vector3 p3 = point + (Matrix44::RotateZMatrix44(150) * Vector3::Right() * r);

	GraphicUtils::DrawLine(_memDC, p1, p2, RGB(0, 0, 0));
	GraphicUtils::DrawLine(_memDC, p2, p3, RGB(0, 0, 0));
	GraphicUtils::DrawLine(_memDC, p3, p1, RGB(0, 0, 0));
}
