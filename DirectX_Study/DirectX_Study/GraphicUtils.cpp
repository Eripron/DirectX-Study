#include "GraphicUtils.h"

namespace DK
{

	void GraphicUtils::DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color)
	{
		float dx = p2.x - p1.x;
		float dy = p2.y - p1.y;

		if (dx == 0.0f)
		{
			int startY = (int)min(p1.y, p2.y);
			int endY = (int)max(p1.y, p2.y);
			int x = (int)(p1.x + 0.5f);

			for (int y = startY; y <= endY; ++y)
				SetPixel(hdc, x, y, color);
		}
		else
		{
			float a = dy / dx;				// 기울기
			float b = p1.y - a * p1.x;	// y절편

			bool bUseAxisX = fabs(a) <= 1;

			float step = bUseAxisX ? fabs(dx) : fabs(dy);
			float start = bUseAxisX ? min(p1.x, p2.x) : min(p1.y, p2.y);

			for (int i = 0; i <= step; ++i)
			{
				int x = bUseAxisX ? start + i : MathUtils::GetLinearX(a, b, start + i) + 0.5f;
				int y = bUseAxisX ? MathUtils::GetLinearY(a, b, start + i) + 0.5f : start + i;

				SetPixel(hdc, x, y, color);
			}
		}
	}

	void GraphicUtils::DrawLine(HDC hdc, float x1, float y1, float z1, float x2, float y2, float z2, COLORREF color)
	{
		DrawLine(hdc, Vector3(x1, y1, z1), Vector3(x2, y2, z2), color);
	}

	void GraphicUtils::DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
	{
		HDC MemDC = CreateCompatibleDC(hdc);
		HBITMAP oldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

		BITMAP bit;
		GetObject(hBit, sizeof(BITMAP), &bit);
		BitBlt(hdc, x, y, bit.bmWidth, bit.bmHeight, MemDC, 0, 0, SRCCOPY);

		SelectObject(MemDC, oldBitmap);
		DeleteDC(MemDC);
	}

	Triangle GraphicUtils::CreateTriangle(Vector3 point, float r)
	{
		Vector3 p1 = point + (Vector3::Up() * -r);
		Vector3 p2 = point + (Matrix44::RotateZMatrix44(30) * Vector3::Right() * r);
		Vector3 p3 = point + (Matrix44::RotateZMatrix44(150) * Vector3::Right() * r);

		return Triangle(p1, p2, p3);
	}
	
}