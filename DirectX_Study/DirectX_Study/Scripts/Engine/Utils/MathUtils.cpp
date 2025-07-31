#include "MathUtils.h"

using namespace DK;

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

DirectX::XMFLOAT4X4 MathUtils::Identity4x4()
{
	return DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

DirectX::XMVECTOR MathUtils::SphericalToCartesian(float radius, float theta, float phi)
{
	return DirectX::XMVectorSet(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta),
		1.0f);
}

int MathUtils::Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

float MathUtils::RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

float MathUtils::RandF(float a, float b)
{
	return a + RandF() * (b - a);

}
