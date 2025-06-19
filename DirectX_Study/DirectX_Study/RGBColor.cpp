#include <math.h>
#include "RGBColor.h"
#include "HSVColor.h"

namespace DK
{
	RGBColor::RGBColor(BYTE r, BYTE g, BYTE b) : R(r), G(g), B(b)
	{
	}

	HSVColor RGBColor::ConvertToHSV()
	{
		float r = R / 255.0f;
		float g = G / 255.0f;
		float b = B / 255.0f;

		float max = max(max(r, g), b);
		float min = min(min(r, g), b);
		float delta = max - min;

		float h = 0.f, s = 0.f, v = 0.f;

		if (delta == 0)
		{
			h = 0.0;
		}
		else if (max == r)
		{
			h = fmodf((g - b) / delta, 6.0f);
			if (h < 0) 
				h += 6.0f;
			h *= 60.0f;
		}
		else if (max == g)
		{
			h = (((b - r) / delta) + 2) * 60;
		}
		else if (max == b)
		{
			h = (((r - g) / delta) + 4) * 60;
		}

		s = (max == 0) ? 0 : (delta / max);
		v = max;

		return HSVColor(h, s, v);
	}
}