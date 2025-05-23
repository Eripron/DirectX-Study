#include "Numeric.hpp"

namespace Numeric
{
	// vector2
	Vector2::Vector2(float _x, float _y) : x(_x), y(_y)
	{
	}

	Vector2 Vector2::operator+(const Vector2& _other) const
	{
		return Vector2(x + _other.x, y + _other.y);
	}

	Vector2 operator+(const Vector2& _a, const Vector2& _b)
	{
		return Vector2(_a.x + _b.x, _a.y + _b.y);
	}



	// vector3
	Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{
	}

	Vector3 Vector3::operator+(const Vector3& _other) const
	{
		return Vector3(x + _other.x, y + _other.y, z + _other.z);
	}

	Vector3 operator+(const Vector3& _a, const Vector3& _b)
	{
		return Vector3(_a.x + _b.x, _a.y + _b.y, _a.z + _b.z);
	}

}