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
	return sinf(Deg2Rad(degree));
}

float MathUtils::CosF(float degree)
{
	return cosf(Deg2Rad(degree));
}

float MathUtils::TanF(float degree)
{
	return tanf(Deg2Rad(degree));
}
