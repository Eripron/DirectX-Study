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

float MathUtils::Deg2Rad(float degree)
{
	return degree * (PI / 180.0f);
}

float MathUtils::Rad2Deg(float radian)
{
	return radian * (180.0f / PI);
}

float MathUtils::SinF(float degree)
{
	float normalized = fmodf(degree, 360.0f);
	if (normalized < 0.0f)
		normalized += 360.0f;

	const float tolerance = 1e-4f;
	if (fabsf(normalized - 0.0f) < tolerance || fabsf(normalized - 180.0f) < tolerance || fabsf(normalized - 360.0f) < tolerance) 
		return 0.0f;

	return sinf(Deg2Rad(degree));
}

float MathUtils::CosF(float degree)
{
	float normalized = fmodf(degree, 360.0f);
	if (normalized < 0.0f)
		normalized += 360.0f;

	const float tolerance = 1e-4f;
	if (fabsf(normalized - 90.0f) < tolerance || fabsf(normalized - 270.0f) < tolerance) 
		return 0.0f;

	return cosf(Deg2Rad(degree));
}

float MathUtils::TanF(float degree)
{
	return tanf(Deg2Rad(degree));
}
