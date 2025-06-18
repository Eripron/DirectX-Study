#pragma once

#include <Windows.h>

namespace DK
{
	class RGBColor
	{
	public:
		RGBColor() = default;
		RGBColor(BYTE r, BYTE g, BYTE b);

		HSVColor ConvertToHSV();

	public:
		BYTE R;
		BYTE G;
		BYTE B;
	};
}