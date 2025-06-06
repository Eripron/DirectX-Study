#include "Numeric.h"

namespace Numeric
{
#pragma region Vector2

	// vector2
	Vector2::Vector2(float _x, float _y) : x(_x), y(_y)
	{
	}

	Vector2 Vector2::Zero()
	{
		return Vector2(0, 0);
	}

	Vector2 Vector2::One()
	{
		return Vector2(1, 1);
	}

	float Vector2::Distance(const Vector2& _a, const Vector2& _b)
	{ 
		return sqrtf(DistanceSquared(_a, _b));
	}

	float Vector2::DistanceSquared(const Vector2& _a, const Vector2& _b)
	{
		float dx = _b.x - _a.x;
		float dy = _b.y - _a.y;

		return (dx * dx) + (dy * dy);
	}

	float Vector2::Magnitude()
	{
		return Distance(Vector2(0, 0), *this);
	}

	Vector2 Vector2::Normalize()
	{
		return Vector2(x, y) / Magnitude();
	}

#pragma region operator

	Vector2 Vector2::operator+(const Vector2& _other) const
	{
		return Vector2(x + _other.x, y + _other.y);
	}

	Vector2& Vector2::operator+=(const Vector2& _other)
	{
		x += _other.x;
		y += _other.y;

		return *this;
	}

	Vector2 Vector2::operator-(const Vector2& _other) const
	{
		return Vector2(x - _other.x, y - _other.y);
	}

	Vector2& Vector2::operator-=(const Vector2& _other)
	{
		x -= _other.x;
		y -= _other.y;

		return *this;
	}

	Vector2 Vector2::operator*(float _value) const
	{
		return Vector2(x * _value, y * _value);
	}

	Vector2& Vector2::operator*=(float _value)
	{
		x *= _value;
		y *= _value;

		return *this;
	}

	Vector2 Vector2::operator/(float _value) const
	{
		return Vector2(x / _value, y / _value);
	}

	Vector2& Vector2::operator/=(float _value)
	{
		x /= _value;
		y /= _value;

		return *this;
	}

	bool Vector2::operator==(const Vector2& _other) const
	{
		return x == _other.x && y == _other.y;
	}

	bool Vector2::operator!=(const Vector2& _other) const
	{
		return x != _other.x || y != _other.y;
	}

	Vector2 operator*(float _value, const Vector2 _other)
	{
		return _other * _value;
	}

#pragma endregion

#pragma endregion


#pragma region Vector3

	// vector3
	Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{
	}
	
	Vector3 Vector3::Zero()
	{
		return Vector3(0, 0, 0);
	}

	Vector3 Vector3::One()
	{
		return Vector3(1, 1, 1);
	}

	float Vector3::Distance(const Vector3& _a, const Vector3& _b)
	{
		return sqrtf(DistanceSquared(_a, _b));
	}

	float Vector3::DistanceSquared(const Vector3& _a, const Vector3& _b)
	{
		float dx = _b.x - _a.x;
		float dy = _b.y - _a.y;
		float dz = _b.z - _a.z;

		return pow(dx, 2) + pow(dy, 2) + pow(dz, 2);
	}

	float Vector3::Magnitude()
	{
		return Distance(Vector3(0, 0, 0), *this);
	}

	Vector3 Vector3::Normalize()
	{
		return Vector3(x, y, z) / Magnitude();
	}

#pragma region operator

	Vector3 Vector3::operator+(const Vector3& _other) const
	{
		return Vector3(x + _other.x, y + _other.y, z + _other.z);
	}

	Vector3& Vector3::operator+=(const Vector3& _other)
	{
		x += _other.x;
		y += _other.y;
		z += _other.z;

		return *this;
	}

	Vector3 Vector3::operator-(const Vector3& _other) const
	{
		return Vector3(x - _other.x, y - _other.y, z - _other.z);
	}

	Vector3& Vector3::operator-=(const Vector3& _other)
	{
		x -= _other.x;
		y -= _other.y;
		z -= _other.z;

		return *this;
	}

	Vector3 Vector3::operator*(float _value) const
	{
		return Vector3(x * _value, y * _value, z * _value);
	}

	Vector3& Vector3::operator*=(float _value)
	{
		x *= _value;
		y *= _value;
		z *= _value;

		return *this;
	}

	Vector3 Vector3::operator/(float _value) const
	{
		return Vector3(x / _value, y / _value, z / _value);
	}

	Vector3& Vector3::operator/=(float _value)
	{
		x /= _value;
		y /= _value;
		z /= _value;

		return *this;
	}

	bool Vector3::operator==(const Vector3& _other) const
	{
		return (x == _other.x && y == _other.y && z == _other.z);
	}

	bool Vector3::operator!=(const Vector3& _other) const
	{
		return (x != _other.x || y != _other.y || z != _other.z);
	}

	Vector3 operator*(float _value, const Vector3 _other)
	{
		return _other * _value;
	}

	

#pragma endregion

#pragma endregion


#pragma region Matrix33

	Matrix33::Matrix33()
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
				element33.m[row][col] = 0;
		}
	}

	Matrix33 Matrix33::operator+(const Matrix33& _other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] + _other.element33.m[row][col];
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator+=(const Matrix33& _other)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				element33.m[row][col] +=_other.element33.m[row][col];
			}
		}

		return *this;
	}

	Matrix33 Matrix33::operator-(const Matrix33& _other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] - _other.element33.m[row][col];
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator-=(const Matrix33& _other)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				element33.m[row][col] -= _other.element33.m[row][col];
			}
		}

		return *this;
	}

	Matrix33 Matrix33::operator*(float _value) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] * _value;
			}
		}

		return ret;
	}

	Matrix33 Matrix33::operator*(const Matrix33& _other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				for (int k = 0; k < 3; ++k)
				{
					ret.element33.m[row][col] += element33.m[row][k] * _other.element33.m[k][col];
				}
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator*=(const Matrix33& _other)
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				for (int k = 0; k < 3; ++k)
				{
					ret.element33.m[row][col] += element33.m[row][k] * _other.element33.m[k][col];
				}
			}
		}

		*this = ret;

		return *this;
	}

	Matrix33 operator*(float _value, const Matrix33& _other)
	{
		return _other * _value;
	}

#pragma endregion

	Matrix44::Matrix44()
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				element44.m[row][col] = 0;
			}
		}
	}

	Matrix44 Matrix44::operator+(const Matrix44& _other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] + _other.element44.m[row][col];
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator+=(const Matrix44& _other)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				element44.m[row][col] += _other.element44.m[row][col];
			}
		}

		return *this;
	}

	Matrix44 Matrix44::operator-(const Matrix44& _other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] - _other.element44.m[row][col];
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator-=(const Matrix44& _other)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				element44.m[row][col] -= _other.element44.m[row][col];
			}
		}

		return *this;
	}

	Matrix44 Matrix44::operator*(float _value) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] * _value;
			}
		}

		return ret;
	}

	Matrix44 Matrix44::operator*(const Matrix44& _other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					ret.element44.m[row][col] += element44.m[row][k] * _other.element44.m[k][col];
				}
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator*=(const Matrix44& _other)
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					ret.element44.m[row][col] += element44.m[row][k] * _other.element44.m[k][col];
				}
			}
		}

		*this = ret;

		return *this;
	}

	Matrix44 operator*(float _value, const Matrix44& _other)
	{
		return _other * _value;
	}

}