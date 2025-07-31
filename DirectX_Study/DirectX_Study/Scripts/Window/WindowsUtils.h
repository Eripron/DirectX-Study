#pragma once

#include <Windows.h>
#include <string>

namespace DK
{
	class WindowsUtils
	{
	public:
		WindowsUtils() = default;
		~WindowsUtils();

		static std::wstring GetWindowTextTitle(HWND hWnd);

		static void GetWindowSize(HWND hWnd, int* width, int* height);
		static void GetScreenSize(HWND hWnd, int* width, int* height);

	};
}