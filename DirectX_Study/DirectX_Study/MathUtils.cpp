#include "MathUtils.h"

float MathUtils::GetLinearX(float a, float b, float y)
{
	if (a == 0)
		return 0.0f;

	return (y - b) / a;
}

float MathUtils::GetLinearY(float a, float b, float x)
{
	return a * x + b;
}
