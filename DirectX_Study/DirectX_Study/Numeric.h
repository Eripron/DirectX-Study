#pragma once

#include <math.h>

namespace Numeric
{

#pragma region Vector2

    struct Vector2
    {
        float x;
        float y;

        Vector2(float _x = 0, float _y = 0);

        static Vector2 Zero();
        static Vector2 One();

        static float Distance(const Vector2& _a, const Vector2& _b);
        static float DistanceSquared(const Vector2& _a, const Vector2& _b);
        float Magnitude();
        Vector2 Normalize();

        // addition vector
        Vector2 operator+(const Vector2& _other) const;
        Vector2& operator+=(const Vector2& _other);

        // subtraction vector
        Vector2 operator-(const Vector2& _other) const;
        Vector2& operator-=(const Vector2& _other);

        // multiply vector
        Vector2 operator*(float _value) const;
        Vector2& operator*=(float _value);

        // division vector
        Vector2 operator/(float _value) const;
        Vector2& operator/=(float _value);

        // equality vector
        bool operator==(const Vector2& _other) const;
        bool operator!=(const Vector2& _other) const;

    };

    Vector2 operator*(float _value, const Vector2 _other);

#pragma endregion

#pragma region Vector3

    struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3(float _x = 0, float _y = 0, float _z = 0);

        static Vector3 Zero();
        static Vector3 One();

        static float Distance(const Vector3& _a, const Vector3& _b);
        static float DistanceSquared(const Vector3& _a, const Vector3& _b);
        float Magnitude();
        Vector3 Normalize();

        // addition vector
        Vector3 operator+(const Vector3& _other) const;
        Vector3& operator+=(const Vector3& _other);

        // subtraction vector
        Vector3 operator-(const Vector3& _other) const;
        Vector3& operator-=(const Vector3& _other);

        // multiply vector
        Vector3 operator*(float _value) const;
        Vector3& operator*=(float _value);

        // division vector
        Vector3 operator/(float _value) const;
        Vector3& operator/=(float _value);

        // equality vector
        bool operator==(const Vector3& _other) const;
        bool operator!=(const Vector3& _other) const;
    };

    Vector3 operator*(float _value, const Vector3 _other);

#pragma endregion

#pragma region Matrix3


#pragma endregion

#pragma region Matrix4
#pragma endregion

}
