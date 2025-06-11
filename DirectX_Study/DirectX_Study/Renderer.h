#pragma once

#include "Windows.h"
#include "Numeric.h"

namespace DK
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer();

		void Init(HWND hWnd);

		void PreUpdate();
		void LastUpdate();

		void DrawPixel(Vector3 point);
		void DrawLine(Vector3 p1, Vector3 p2);

	private:
		HWND _hWnd;
		HDC _hdc;
		HDC _memDC;
		HBITMAP _hBitmap;
		HBITMAP _hOldBitmap;

		RECT _rtScreen;
		Vector2 _vScreenSize;
	};
}