#ifndef COMMON_HPP
#define COMMON_HPP

struct Vector2
{
	float x;
	float y;

	Vector2(float _x = 0, float _y = 0) : x(_x), y(_y)
	{
	}

	Vector2 operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}
};

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z)
	{
	}

	Vector3 operator+(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

};

#endif