#pragma once

#include <cmath>
#include <DirectXMath.h>

class MathUtils
{
public:

	// constexpr: constant expression, which can be evaluated at compile time
	//static constexpr float PI = { 3.14159265358979323846f };

	// 직선에서 특정 값에 대응되는 값 찾기
	static float GetLinearX(float a, float b, float y);
	static float GetLinearY(float a, float b, float x);

	static DirectX::XMFLOAT4X4 Identity4x4();

	static DirectX::XMFLOAT3 MultiplyValueToFloat3(DirectX::XMFLOAT3 f1, float value);
	static DirectX::XMFLOAT3 AddFloat3ToFloat3(DirectX::XMFLOAT3 f1, DirectX::XMFLOAT3 f2);

	static int Rand(int a, int b);
	static float RandF();
	static float RandF(float a, float b);
};