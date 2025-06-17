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

		void DrawPixel(Vector3 point, COLORREF color = RGB(0, 0, 0));
		void DrawLine(Vector3 p1, Vector3 p2, COLORREF color = RGB(0, 0, 0));

		void DrawAxisX(int y);
		void DrawAxisY(int x);

	private:
		void Clear();

		Vector2 ToScreenPoint(float x, float y);
		Vector2 ToScreenCoordinate(const Vector2& vPos);
		Vector3 ToScreenCoordinate(const Vector3& vPos);

		void DrawLine(HDC hdc, Vector2 p1, Vector2 p2, COLORREF color);
		void DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color);

	private:
		HWND _hWnd;
		HDC _hdc;
		HDC _memDC;
		HBITMAP _hBitmap;
		HBITMAP _hOldBitmap;

		Vector2 _vScreenSize;
	};
}