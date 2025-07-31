#pragma once

#include <Windows.h>

namespace DK
{
	class HSVColor;

	class RGBColor
	{
	public:
		RGBColor() = default;
		RGBColor(BYTE r, BYTE g, BYTE b, BYTE a = 255);

		HSVColor ConvertToHSV();

		static RGBColor LerpColor(RGBColor color1, RGBColor color2, float t);

	public:
		BYTE R;
		BYTE G;
		BYTE B;
		BYTE A;
	};
}