#include "Windows.h"

#include "Renderer.h"
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
	_MemDC = CreateCompatibleDC(_hdc);

	float fWidth = 0.0f;
	float fHeight = 0.0f;
	WindowsUtils::GetScreenSize(_hWnd, &fWidth, &fHeight);

	CreateCompatibleBitmap(_hdc, fWidth, fHeight);

}


