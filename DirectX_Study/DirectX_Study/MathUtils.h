#pragma once

#include <cmath>

class MathUtils
{
public:

	// constexpr: constant expression, which can be evaluated at compile time
	static constexpr float PI = { 3.14159265358979323846f };

	// 직선에서 특정 값에 대응되는 값 찾기
	static float GetLinearX(float a, float b, float y);
	static float GetLinearY(float a, float b, float x);

	static float Deg2Rad(float degree);
	static float Rad2Deg(float radian);

	static float SinF(float degree);
	static float CosF(float degree);
	static float TanF(float degree);

};