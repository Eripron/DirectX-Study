#pragma once

namespace DK
{
	class WindowsUtils
	{
	public:
		WindowsUtils() = default;
		~WindowsUtils();

		static void GetWindowSize(HWND hWnd, float* width, float* height);
		static void GetScreenSize(HWND hWnd, float* width, float* height);

	};
}