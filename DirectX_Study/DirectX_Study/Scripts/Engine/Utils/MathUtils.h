#pragma once

#include <cmath>
#include <DirectXMath.h>

using namespace DirectX;

namespace DK
{
	struct MathUtils
	{
		static float GetLinearX(float a, float b, float y);
		static float GetLinearY(float a, float b, float x);

		static XMFLOAT4X4 Identity4x4();
		static XMVECTOR SphericalToCartesian(float radius, float theta, float phi);

		static int Rand(int a, int b);
		static float RandF();
		static float RandF(float a, float b);

	};

	/// operator ///

		// XMFLOAT3
	static XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b)
	{
		return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
	}

	static XMFLOAT3 operator*(const XMFLOAT3& float3, const float& value)
	{
		return XMFLOAT3(float3.x * value, float3.y * value, float3.z * value);
	}

	static XMFLOAT3 operator*(const float& value, const XMFLOAT3& float3)
	{
		return XMFLOAT3(float3.x * value, float3.y * value, float3.z * value);
	}
}