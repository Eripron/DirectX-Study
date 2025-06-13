#include "GraphicUtils.h"
#include "WindowsUtils.h"

namespace DK
{
	void GraphicUtils::DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color)
	{
		Vector2 sp1 = WindowsUtils::ToScreenPoint(p1.x, p1.y);
		Vector2 sp2 = WindowsUtils::ToScreenPoint(p2.x, p2.y);

		float dx = sp2.x - sp1.x;
		float dy = sp2.y - sp1.y;

		if (dx == 0.0f)
		{
			int sy = dy > 0 ? 1 : -1;
			int step = fabs(dy);
			for (int i = 0; i <= step; ++i)
				SetPixel(hdc, sp1.x, sp1.y + i * sy, color);
		}
		else
		{
			float a = dy / dx;				// 기울기
			float b = sp1.y - a * sp1.x;	// y절편

			bool bUseAxisX = fabs(a) <= 1;

			float step = bUseAxisX ? fabs(dx) : fabs(dy);
			float start = bUseAxisX ? min(sp1.x, sp2.x) : min(sp1.y, sp2.y);

			for (int i = 0; i <= step; ++i)
			{
				float x = bUseAxisX ? start + i : floorf(MathUtils::GetLinearX(a, b, start + i));
				float y = bUseAxisX ? floorf(MathUtils::GetLinearY(a, b, start + i)) : start + i;

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
		Vector3 p2 = point + (Matrix44::RotateZMatrix44(-30) * Vector3::Right() * -r);
		Vector3 p3 = point + (Matrix44::RotateZMatrix44(-150) * Vector3::Right() * -r);

		return Triangle(p1, p2, p3);
	}

	Square GraphicUtils::CreateSquare(Vector3 point, float r)
	{
		Vector3 p1 = point + (Matrix44::RotateZMatrix44(45) * Vector3::Up() * -r);
		Vector3 p2 = point + (Matrix44::RotateZMatrix44(-45) * Vector3::Up() * -r);
		Vector3 p3 = point + (Matrix44::RotateZMatrix44(45) * Vector3::Up() * r);
		Vector3 p4 = point + (Matrix44::RotateZMatrix44(-45) * Vector3::Up() * r);

		return Square(p1, p2, p3, p4);
	}
	
}