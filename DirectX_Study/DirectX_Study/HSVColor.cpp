#include <math.h>
#include "HSVColor.h"
#include "RGBColor.h"

namespace DK
{
	HSVColor::HSVColor(float h, float s, float v) : H(h), S(s), V(v)
	{
	}

	RGBColor HSVColor::ConvertToRGB()
	{
		int i = floorf(H / 60.0f);
		float f = (H / 60.0f) - i;

		float p = V * (1 - S);
		float q = V * (1 - f * S);
		float t = V * (1 - (1 - f) * S);

		float r = 0.f, g = 0.f, b = 0.f;
		switch (i % 6)
		{
		case 0: r = V, g = t, b = p; break;
		case 1: r = q, g = V, b = p; break;
		case 2: r = p, g = V, b = t; break;
		case 3: r = p, g = q, b = V; break;
		case 4: r = t, g = p, b = V; break;
		case 5: r = V, g = p, b = q; break;
		}

		return RGBColor(r * 255, g * 255, b * 255);
	}
}