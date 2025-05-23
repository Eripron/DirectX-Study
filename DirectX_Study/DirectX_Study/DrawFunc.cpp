#include "DrawFunc.hpp"

namespace Graphic
{
	void DrawLine(HDC _hdc, Numeric::Vector3 _p1, Numeric::Vector3 _p2)
	{
		float dx = _p2.x - _p1.x;
		float dy = _p2.y - _p1.y;

		float steps = max(fabs(dx), fabs(dy));

		float xInc = dx / steps;
		float yInc = dy / steps;

		float x = _p1.x;
		float y = _p1.y;

		for (int i = 0; i <= steps; ++i)
		{
			SetPixel(_hdc, round(x), round(y), RGB(0, 0, 0));
			x += xInc;
			y += yInc;
		}
	}

	void DrawLine(HDC _hdc, int _x1, int _y1, int _z1, int _x2, int _y2, int _z2)
	{
		DrawLine(_hdc, Numeric::Vector3(_x1, _y1, _z1), Numeric::Vector3(_x2, _y2, _z2));
	}



	void DrawBitmap(HDC _hdc, int _x, int _y, HBITMAP _hBit)
	{
		HDC MemDC = CreateCompatibleDC(_hdc);
		HBITMAP OldBitmap = (HBITMAP)SelectObject(MemDC, _hBit);

		BITMAP bit;
		GetObject(_hBit, sizeof(BITMAP), &bit);

		BitBlt(_hdc, _x, _y, bit.bmWidth, bit.bmHeight, MemDC, 0, 0, SRCCOPY);

		SelectObject(MemDC, OldBitmap);
		DeleteDC(MemDC);
	}

}