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

DirectX::XMFLOAT4X4 MathUtils::Identity4x4()
{
	return DirectX::XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

DirectX::XMFLOAT3 MathUtils::MultiplyValueToFloat3(DirectX::XMFLOAT3 f1, float value)
{
	return DirectX::XMFLOAT3(f1.x * value, f1.y * value, f1.z * value);
}

DirectX::XMFLOAT3 MathUtils::AddFloat3ToFloat3(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2)
{
	return DirectX::XMFLOAT3(f1.x + f2.x, f1.y + f2.y, f1.z + f2.z);
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
