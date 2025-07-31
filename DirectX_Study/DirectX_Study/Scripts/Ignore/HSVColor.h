#pragma once

namespace DK
{
	class RGBColor;

	class HSVColor
	{
	public:
		HSVColor() = default;
		HSVColor(float h, float s, float v);

		RGBColor ConvertToRGB();

	public:
		float H;	// Hue(0.0 ~ 360)
		float S;	// Stauration(0.0 ~ 1.0)
		float V;	// Value(0.0:black ~ 1.0:white)
	};
}