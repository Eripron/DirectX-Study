#include "GraphicUtils.h"

void GraphicUtils::DrawLine(HDC _hdc, Numeric::Vector3 _p1, Numeric::Vector3 _p2)
{
	float dx = _p2.x - _p1.x;
	float dy = _p2.y - _p1.y;

	float a = dy / dx;				// 기울기
	float b = _p1.y - a * _p1.x;	// y절편

	bool bUseAxisX = a <= 1;

	float step = bUseAxisX ? fabs(dx) : fabs(dy);
	float start = bUseAxisX ? min(_p1.x, _p2.x) : min(_p1.y, _p2.y);

	for (int i = start; i <= step; ++i)
	{
		int x = bUseAxisX ? i : MathUtils::GetLinearX(a, b, i) + 0.5f;
		int y = bUseAxisX ? MathUtils::GetLinearY(a, b, i) + 0.5f : i;

		SetPixel(_hdc, x, y, RGB(0, 0, 0));
	}
}

void GraphicUtils::DrawLine(HDC _hdc, float _x1, float _y1, float _z1, float _x2, float _y2, float _z2)
{
	DrawLine(_hdc, Numeric::Vector3(_x1, _y1, _z1), Numeric::Vector3(_x2, _y2, _z2));
}

void GraphicUtils::DrawBitmap(HDC _hdc, int _x, int _y, HBITMAP _hBit)
{
	HDC MemDC = CreateCompatibleDC(_hdc);
	HBITMAP OldBitmap = (HBITMAP)SelectObject(MemDC, _hBit);

	BITMAP bit;
	GetObject(_hBit, sizeof(BITMAP), &bit);

	BitBlt(_hdc, _x, _y, bit.bmWidth, bit.bmHeight, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}
