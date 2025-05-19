#ifndef DRAW_FUNC_HPP
#define DRAW_FUNC_HPP

#include <Windows.h>
#include <math.h>

#include "Common.hpp"

void DrawLine(HDC hdc, Vector3 p1, Vector3 p2)
{
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;

	float steps = max(fabs(dx), fabs(dy));

	float xInc = dx / steps;
	float yInc = dy / steps;

	float x = p1.x;
	float y = p1.y;

	for (int i = 0; i <= steps; ++i)
	{
		SetPixel(hdc, round(x), round(y), RGB(0, 0, 0));
		x += xInc;
		y += yInc;
	}
}

void DrawLine(HDC hdc, int x1, int y1, int z1, int x2, int y2, int z2)
{
	DrawLine(hdc, Vector3(x1, y1, z1), Vector3(x2, y2, z2));
}


#endif