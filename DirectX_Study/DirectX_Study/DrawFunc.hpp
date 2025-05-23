#pragma once

#include <Windows.h>
#include <math.h>

#include "Numeric.hpp"

namespace Graphic
{
	void DrawLine(HDC _hdc, Numeric::Vector3 _p1, Numeric::Vector3 _p2);
	void DrawLine(HDC _hdc, int _x1, int _y1, int _z1, int _x2, int _y2, int _z2);


	void DrawBitmap(HDC _hdc, int _x, int _y, HBITMAP _hBit);
}