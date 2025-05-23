#pragma once

namespace Numeric
{
    struct Vector2
    {
        float x;
        float y;

        Vector2(float _x = 0, float _y = 0);

        Vector2 operator+(const Vector2& _other) const;
    };

    Vector2 operator+(const Vector2& _a, const Vector2& _b);



    struct Vector3
	{
		float x;
		float y;
		float z;

		Vector3(float _x = 0, float _y = 0, float _z = 0);

        Vector3 operator+(const Vector3& _other) const;
	};

    Vector3 operator+(const Vector3& _a, const Vector3& _b);

}
