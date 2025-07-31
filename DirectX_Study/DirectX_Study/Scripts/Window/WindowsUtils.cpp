#include "WindowsUtils.h"

using namespace DK;

WindowsUtils::~WindowsUtils()
{
}

std::wstring DK::WindowsUtils::GetWindowTextTitle(HWND hWnd)
{
	int len = GetWindowTextLength(hWnd) + 1;
	std::wstring wstr = std::wstring(L"\0", len);
	GetWindowText(hWnd, &wstr[0], len);

	return wstr;
}

void WindowsUtils::GetWindowSize(HWND hWnd, int* pWidth, int* pHeight)
{
	RECT rt;
	GetWindowRect(hWnd, &rt);

	*pWidth = static_cast<int>(rt.right - rt.left);
	*pHeight = static_cast<int>(rt.bottom - rt.top);
}

void WindowsUtils::GetScreenSize(HWND hWnd, int* pWidth, int* pHeight)
{
	RECT rt;
	GetClientRect(hWnd, &rt);

	*pWidth = static_cast<int>(rt.right - rt.left);
	*pHeight = static_cast<int>(rt.bottom - rt.top);
}

