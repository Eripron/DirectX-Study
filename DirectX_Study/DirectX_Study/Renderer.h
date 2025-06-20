#pragma once

#include "Windows.h"
#include "Numeric.h"
#include "Graphic.h"

namespace DK
{
	class RGBColor;

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

		void DrawSquare(Square square, BITMAP* bmp, float S, float V);
		void DrawTriangle(Triangle triangle, BITMAP* bmp, float S, float V);

		void DrawTextIn(LPCTSTR str, int x, int y);

	private:
		void Clear();

		Vector2 ToScreenPoint(float x, float y);
		Vector2 ToScreenCoordinate(const Vector2& vPos);
		Vector3 ToScreenCoordinate(const Vector3& vPos);

		void DrawLine(HDC hdc, Vector2 p1, Vector2 p2, COLORREF color);
		void DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color);

		RGBColor GetColor(BITMAP& bitmap, Vector2 uv);
		RGBColor GetColorLerp(BITMAP& bitmap, Vector2 uv);
		RGBColor GetColor(BITMAP& bitmap, int x, int y);

	private:
		HWND _hWnd;
		HDC _hdc;
		HDC _memDC;
		HBITMAP _hBitmap;
		HBITMAP _hOldBitmap;

		Vector2 _vScreenSize;
	};
}