#pragma once

namespace DK
{
	class Renderer
	{
	private:
		HWND _hWnd;
		HDC _hdc;
		HDC _MemDC;
		HBITMAP _bitmap;
		HBITMAP _oldBitmap;

	public:
		Renderer() = default;
		~Renderer();

		void Init(HWND hWnd);
	};
}