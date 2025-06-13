#include <Windows.h>
#include <math.h>
#include "WindowsUtils.h"

using namespace DK;

WindowsUtils::~WindowsUtils()
{
}

void DK::WindowsUtils::GetWindowSize(HWND hWnd, float* width, float* height)
{
	RECT rt;
	GetWindowRect(hWnd, &rt);

	*width = static_cast<float>(rt.right - rt.left);
	*height = static_cast<float>(rt.bottom - rt.top);
}

void WindowsUtils::GetScreenSize(HWND hWnd, float* width, float* height)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	*width = static_cast<float>(rt.right - rt.left);
	*height = static_cast<float>(rt.bottom - rt.top);
}

Vector2 DK::WindowsUtils::ToScreenPoint(float x, float y)
{
	return Vector2(floorf(x), floorf(y));
}


