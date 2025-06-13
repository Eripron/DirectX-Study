#include "Numeric.h"
#include "MathUtils.h"

namespace DK
{
#pragma region Vector2

	// vector2
	Vector2::Vector2(float x, float y) : x(x), y(y)
	{
	}

	Vector2 Vector2::Zero()
	{
		return Vector2(0.0f, 0.0f);
	}

	Vector2 Vector2::One()
	{
		return Vector2(1.0f, 1.0f);
	}

	Vector2 Vector2::Right()
	{
		return Vector2(1.0f, 0.0f);
	}

	Vector2 Vector2::Up()
	{
		return Vector2(0.0f, 1.0f);
	}

	float Vector2::Distance(const Vector2& a, const Vector2& b)
	{
		return sqrtf(DistanceSquared(a, b));
	}

	float Vector2::DistanceSquared(const Vector2& a, const Vector2& b)
	{
		float dx = b.x - a.x;
		float dy = b.y - a.y;

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

	float Vector2::Dot(const Vector2& v) const
	{
		return x * v.x + y * v.y;
	}

	Vector2 Vector2::operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2& Vector2::operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	Vector2 Vector2::operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2& Vector2::operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	Vector2 Vector2::operator*(float value) const
	{
		return Vector2(x * value, y * value);
	}

	Vector2& Vector2::operator*=(float value)
	{
		x *= value;
		y *= value;

		return *this;
	}

	Vector2 Vector2::operator/(float value) const
	{
		return Vector2(x / value, y / value);
	}

	Vector2& Vector2::operator/=(float value)
	{
		x /= value;
		y /= value;

		return *this;
	}

	bool Vector2::operator==(const Vector2& other) const
	{
		return x == other.x && y == other.y;
	}

	bool Vector2::operator!=(const Vector2& other) const
	{
		return x != other.x || y != other.y;
	}

	Vector2 operator*(float value, const Vector2 other)
	{
		return other * value;
	}

#pragma endregion


#pragma region Vector3

	// vector3
	Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	Vector3 Vector3::Zero()
	{
		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 Vector3::One()
	{
		return Vector3(1.0f, 1.0f, 1.0f);
	}

	Vector3 Vector3::Right()
	{
		return Vector3(1.0f, 0.0f, 0.0f);
	}

	Vector3 Vector3::Up()
	{
		return Vector3(0.0f, 1.0f, 0.0f);
	}

	Vector3 Vector3::Forward()
	{
		return Vector3(0.0f, 0.0f, 1.0f);
	}

	float Vector3::Distance(const Vector3& a, const Vector3& b)
	{
		return sqrtf(DistanceSquared(a, b));
	}

	float Vector3::DistanceSquared(const Vector3& a, const Vector3& b)
	{
		float dx = b.x - a.x;
		float dy = b.y - a.y;
		float dz = b.z - a.z;

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

	float Vector3::Dot(const Vector3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vector3 Vector3::operator+(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3& Vector3::operator+=(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	Vector3 Vector3::operator-(const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	Vector3& Vector3::operator-=(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	Vector3 Vector3::operator*(float value) const
	{
		return Vector3(x * value, y * value, z * value);
	}

	Vector3& Vector3::operator*=(float value)
	{
		x *= value;
		y *= value;
		z *= value;

		return *this;
	}

	Vector3 Vector3::operator/(float value) const
	{
		return Vector3(x / value, y / value, z / value);
	}

	Vector3& Vector3::operator/=(float value)
	{
		x /= value;
		y /= value;
		z /= value;

		return *this;
	}

	bool Vector3::operator==(const Vector3& other) const
	{
		return (x == other.x && y == other.y && z == other.z);
	}

	bool Vector3::operator!=(const Vector3& other) const
	{
		return (x != other.x || y != other.y || z != other.z);
	}

	Vector3 operator*(float value, const Vector3 other)
	{
		return other * value;
	}

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

	Matrix33 Matrix33::Identity()
	{
		Matrix33 matrix;

		matrix.element33.m11 = 1.0f;
		matrix.element33.m22 = 1.0f;
		matrix.element33.m33 = 1.0f;

		return matrix;
	}

	Matrix33 Matrix33::MoveMatrix33(float xMove, float yMove)
	{
		Matrix33 matrix = Identity();

		matrix.element33.m13 = xMove;
		matrix.element33.m23 = yMove;

		return matrix;
	}

	Matrix33 Matrix33::RotateXMatrix33(float degree)
	{
		Matrix33 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element33.m11 = 1.0f;

		matrix.element33.m22 = cos;
		matrix.element33.m23 = -sin;
		matrix.element33.m32 = sin;
		matrix.element33.m33 = cos;

		return matrix;
	}

	Matrix33 Matrix33::RotateYMatrix33(float degree)
	{
		Matrix33 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element33.m22 = 1.0f;

		matrix.element33.m11 = cos;
		matrix.element33.m13 = sin;
		matrix.element33.m31 = -sin;
		matrix.element33.m33 = cos;

		return matrix;
	}

	Matrix33 Matrix33::RotateZMatrix33(float degree)
	{
		Matrix33 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element33.m33 = 1.0f;

		matrix.element33.m11 = cos;
		matrix.element33.m12 = -sin;
		matrix.element33.m21 = sin;
		matrix.element33.m22 = cos;

		return matrix;
	}

	Matrix33 Matrix33::ScaleMatrix33(float xScale, float yScale)
	{
		Matrix33 matrix;

		matrix.element33.m11 = xScale;
		matrix.element33.m22 = yScale;
		matrix.element33.m33 = 1;

		return matrix;
	}

	Matrix33 Matrix33::operator+(const Matrix33& other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] + other.element33.m[row][col];
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator+=(const Matrix33& other)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				element33.m[row][col] += other.element33.m[row][col];
			}
		}

		return *this;
	}

	Matrix33 Matrix33::operator-(const Matrix33& other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] - other.element33.m[row][col];
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator-=(const Matrix33& other)
	{
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				element33.m[row][col] -= other.element33.m[row][col];
			}
		}

		return *this;
	}

	Matrix33 Matrix33::operator*(float value) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] * value;
			}
		}

		return ret;
	}

	Vector2 Matrix33::operator*(Vector2 v) const
	{
		float value[2];
		for (int row = 0; row < 2; ++row)
		{
			value[row] = element33.m[row][0] * v.x + element33.m[row][1] * v.y + element33.m[row][2];
		}

		Vector2 ret(value[0], value[1]);

		return ret;
	}

	Matrix33 Matrix33::operator*(const Matrix33& other) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				for (int k = 0; k < 3; ++k)
				{
					ret.element33.m[row][col] += element33.m[row][k] * other.element33.m[k][col];
				}
			}
		}

		return ret;
	}

	Matrix33& Matrix33::operator*=(const Matrix33& other)
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				for (int k = 0; k < 3; ++k)
				{
					ret.element33.m[row][col] += element33.m[row][k] * other.element33.m[k][col];
				}
			}
		}

		*this = ret;

		return *this;
	}

	Matrix33 Matrix33::operator/(float value) const
	{
		Matrix33 ret;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				ret.element33.m[row][col] = element33.m[row][col] / value;
			}
		}

		return ret;
	}

	Matrix33 operator*(float value, const Matrix33& other)
	{
		return other * value;
	}

#pragma endregion


#pragma region Matrix44

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

	Matrix44 Matrix44::Identity()
	{
		Matrix44 matrix;

		matrix.element44.m11 = 1.0f;
		matrix.element44.m22 = 1.0f;
		matrix.element44.m33 = 1.0f;
		matrix.element44.m44 = 1.0f;

		return matrix;
	}

	Matrix44 Matrix44::MoveMatrix44(float xMove, float yMove, float zMove)
	{
		Matrix44 matrix = Identity();

		matrix.element44.m14 = xMove;
		matrix.element44.m24 = yMove;
		matrix.element44.m34 = zMove;

		return matrix;
	}

	Matrix44 Matrix44::RotateXMatrix44(float degree)
	{
		Matrix44 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element44.m11 = 1.0f;
		matrix.element44.m44 = 1.0f;

		matrix.element44.m22 = cos;
		matrix.element44.m23 = -sin;
		matrix.element44.m32 = sin;
		matrix.element44.m33 = cos;

		return matrix;
	}

	Matrix44 Matrix44::RotateYMatrix44(float degree)
	{
		Matrix44 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element44.m22 = 1.0f;
		matrix.element44.m44 = 1.0f;

		matrix.element44.m11 = cos;
		matrix.element44.m13 = sin;
		matrix.element44.m31 = -sin;
		matrix.element44.m33 = cos;

		return matrix;
	}

	Matrix44 Matrix44::RotateZMatrix44(float degree)
	{
		Matrix44 matrix;

		float sin = MathUtils::SinF(degree);
		float cos = MathUtils::CosF(degree);

		matrix.element44.m33 = 1.0f;
		matrix.element44.m44 = 1.0f;

		matrix.element44.m11 = cos;
		matrix.element44.m12 = -sin;
		matrix.element44.m21 = sin;
		matrix.element44.m22 = cos;

		return matrix;
	}

	Matrix44 Matrix44::ScaleMatrix44(float xScale, float yScale, float zScale)
	{
		Matrix44 matrix;

		matrix.element44.m11 = xScale;
		matrix.element44.m22 = yScale;
		matrix.element44.m33 = zScale;
		matrix.element44.m44 = 1;

		return matrix;
	}

	Matrix44 Matrix44::operator+(const Matrix44& other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] + other.element44.m[row][col];
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator+=(const Matrix44& other)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				element44.m[row][col] += other.element44.m[row][col];
			}
		}

		return *this;
	}

	Matrix44 Matrix44::operator-(const Matrix44& other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] - other.element44.m[row][col];
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator-=(const Matrix44& other)
	{
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				element44.m[row][col] -= other.element44.m[row][col];
			}
		}

		return *this;
	}

	Matrix44 Matrix44::operator*(float value) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] * value;
			}
		}

		return ret;
	}

	Vector3 Matrix44::operator*(Vector3 v) const
	{
		float value[3];
		for (int row = 0; row < 3; ++row)
		{
			value[row] = element44.m[row][0] * v.x + element44.m[row][1] * v.y + element44.m[row][2] * v.z + element44.m[row][3];
		}

		Vector3 ret(value[0], value[1], value[2]);

		return ret;
	}

	Matrix44 Matrix44::operator*(const Matrix44& other) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					ret.element44.m[row][col] += element44.m[row][k] * other.element44.m[k][col];
				}
			}
		}

		return ret;
	}

	Matrix44& Matrix44::operator*=(const Matrix44& other)
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				for (int k = 0; k < 4; ++k)
				{
					ret.element44.m[row][col] += element44.m[row][k] * other.element44.m[k][col];
				}
			}
		}

		*this = ret;

		return *this;
	}

	Matrix44 Matrix44::operator/(float value) const
	{
		Matrix44 ret;

		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				ret.element44.m[row][col] = element44.m[row][col] / value;
			}
		}

		return ret;
	}

	Matrix44 operator*(float value, const Matrix44& other)
	{
		return other * value;
	}

#pragma endregion
}