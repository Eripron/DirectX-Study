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

		void DrawLine(Vector3 a, Vector3 b);


	private:
		void Render();	// del

	private:
		HWND _hWnd;
		HDC _hdc;
		HDC _memDC;
		HBITMAP _hBitmap;
		HBITMAP _hOldBitmap;
	};
}