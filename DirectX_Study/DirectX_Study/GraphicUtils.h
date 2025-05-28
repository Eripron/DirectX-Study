#pragma once

#include <Windows.h>
#include <math.h>

#include "Numeric.h"
#include "MathUtils.h"

class GraphicUtils
{
public:
	static void DrawLine(HDC _hdc, Numeric::Vector3 _p1, Numeric::Vector3 _p2);
	static void DrawLine(HDC _hdc, float _x1, float _y1, float _z1, float _x2, float _y2, float _z2);

	static void DrawBitmap(HDC _hdc, int _x, int _y, HBITMAP _hBit);
};
