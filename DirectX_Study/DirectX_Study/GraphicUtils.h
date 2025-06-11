#pragma once

#include <Windows.h>
#include <math.h>

#include "Graphic.h"
#include "Numeric.h"
#include "MathUtils.h"

namespace DK
{
	class GraphicUtils
	{
	public:
		static void DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color);
		static void DrawLine(HDC hdc, float x1, float y1, float z1, float x2, float y2, float z2, COLORREF color);

		static void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);

		static Triangle CreateTriangle(Vector3 point, float r);
	};
}