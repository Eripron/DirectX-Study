#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

namespace DK
{
#pragma region Vector2

    struct Vector2
    {
        float x;
        float y;

        Vector2(float x = 0, float y = 0);

        static Vector2 Zero();
        static Vector2 One();
        static Vector2 Right();
        static Vector2 Up();

        static float Distance(const Vector2& a, const Vector2& b);
        static float DistanceSquared(const Vector2& a, const Vector2& b);
        float Magnitude();
        Vector2 Normalize();
        float Dot(const Vector2& v) const;

        // addition vector
        Vector2 operator+(const Vector2& other) const;
        Vector2& operator+=(const Vector2& other);

        // subtraction vector
        Vector2 operator-(const Vector2& other) const;
        Vector2& operator-=(const Vector2& other);

        // multiply vector
        Vector2 operator*(float value) const;
        Vector2& operator*=(float value);

        // division vector
        Vector2 operator/(float value) const;
        Vector2& operator/=(float value);

        // equality vector
        bool operator==(const Vector2& other) const;
        bool operator!=(const Vector2& other) const;

    };

    Vector2 operator*(float value, const Vector2 other);

#pragma endregion

#pragma region Vector3

    struct Vector3
    {
        float x;
        float y;
        float z;

        Vector3(float x = 0, float y = 0, float z = 0);

        static Vector3 Zero();
        static Vector3 One();
        static Vector3 Right();
        static Vector3 Up();
        static Vector3 Forward();

        static float Distance(const Vector3& a, const Vector3& b);
        static float DistanceSquared(const Vector3& a, const Vector3& b);
        float Magnitude();
        Vector3 Normalize();
        float Dot(const Vector3& v) const;

        // addition vector
        Vector3 operator+(const Vector3& other) const;
        Vector3& operator+=(const Vector3& other);

        // subtraction vector
        Vector3 operator-(const Vector3& other) const;
        Vector3& operator-=(const Vector3& other);

        // multiply vector
        Vector3 operator*(float value) const;
        Vector3& operator*=(float value);

        // division vector
        Vector3 operator/(float value) const;
        Vector3& operator/=(float value);

        // equality vector
        bool operator==(const Vector3& other) const;
        bool operator!=(const Vector3& other) const;
    };

    Vector3 operator*(float value, const Vector3 other);

#pragma endregion

#pragma region Matrix3

    struct Matrix33
    {
        union Element33
        {
            struct
            {
                float m11;
                float m12;
                float m13;

                float m21;
                float m22;
                float m23;

                float m31;
                float m32;
                float m33;
            };

            float m[3][3];
        } element33;

        Matrix33();

        static Matrix33 Identity();
        static Matrix33 MoveMatrix33(float xMove, float yMove);
        static Matrix33 RotateXMatrix33(float degree);
        static Matrix33 RotateYMatrix33(float degree);
        static Matrix33 RotateZMatrix33(float degree);
        static Matrix33 ScaleMatrix33(float xScale, float yScale);

        Matrix33 operator+(const Matrix33& other) const;
        Matrix33& operator+=(const Matrix33& other);

        Matrix33 operator-(const Matrix33& other) const;
        Matrix33& operator-=(const Matrix33& other);

        Matrix33 operator*(float value) const;
        Vector2 operator*(Vector2 v) const;
        Matrix33 operator*(const Matrix33& other) const;
        Matrix33& operator*=(const Matrix33& other);

        Matrix33 operator/(float value) const;
    };

    Matrix33 operator*(float value, const Matrix33& other);

#pragma endregion

#pragma region Matrix4

    struct Matrix44
    {
        union Element44
        {
            struct
            {
                float m11;
                float m12;
                float m13;
                float m14;

                float m21;
                float m22;
                float m23;
                float m24;

                float m31;
                float m32;
                float m33;
                float m34;

                float m41;
                float m42;
                float m43;
                float m44;
            };

            float m[4][4];
        } element44;

        Matrix44();

        static Matrix44 Identity();
        static Matrix44 MoveMatrix44(float xMove, float yMove, float zMove);
        static Matrix44 RotateXMatrix44(float degree);
        static Matrix44 RotateYMatrix44(float degree);
        static Matrix44 RotateZMatrix44(float degree);
        static Matrix44 ScaleMatrix44(float xScale, float yScale, float zScale);

        Matrix44 operator+(const Matrix44& other) const;
        Matrix44& operator+=(const Matrix44& other);

        Matrix44 operator-(const Matrix44& other) const;
        Matrix44& operator-=(const Matrix44& other);

        Matrix44 operator*(float value) const;
        Vector3 operator*(Vector3 v) const;
        Matrix44 operator*(const Matrix44& other) const;
        Matrix44& operator*=(const Matrix44& other);

        Matrix44 operator/(float value) const;
    };

    Matrix44 operator*(float value, const Matrix44& other);

#pragma endregion

#pragma region Vertex

    struct Vertex
    {
        DirectX::XMFLOAT3 Pos;      // 정점 위치
        DirectX::XMFLOAT4 Color;    // 정점 색상
    };

    // slot 0
    struct VPosData
    {
        DirectX::XMFLOAT3 Pos;      // 정점 위치
    };

    // slot 1
    struct VColorData
    {
        DirectX::PackedVector::XMCOLOR Color;
        //DirectX::XMFLOAT4 Color;    // 정점 색상
    };

#pragma endregion

}