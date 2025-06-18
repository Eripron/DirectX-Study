#pragma once

#include <Windows.h>
#include <math.h>
#include <vector>
#include "Numeric.h"

namespace DK
{
	class Graphic
	{
	public:
		Graphic() = default;
		virtual ~Graphic();

		virtual void ApplyTransform(Matrix44 matrix) = 0;
		virtual void ApplyTransform(Matrix44 matrix, Vector3 center);

	};

	class Dot : public Graphic
	{
	public:
		Dot(float x = 0, float y = 0, float z = 0);
		Dot(Vector2 vPos);
		Dot(Vector3 vPos);
		~Dot();

		void ApplyTransform(Matrix44 matrix) override;
		void ApplyTransform(Matrix44 matrix, Vector3 center) override;

		Vector3 GetPos();
		Vector2 GetUV();

		void SetPos(Vector3 vPos);
		void SetUV(Vector2 vUV);

	protected:
		Vector3 _vPos;
		Vector2 _vUV;
	};

	class Triangle : public Graphic
	{
	public:
		Triangle();
		Triangle(Vector2 p1, Vector2 p2, Vector2 p3);
		Triangle(Vector3 p1, Vector3 p2, Vector3 p3);
		~Triangle();

		void ApplyTransform(Matrix44 matrix) override;
		virtual void ApplyTransform(Matrix44 matrix, Vector3 center);

		Dot GetDot(int index);
		void SetUV(Vector2 uv1, Vector2 uv2, Vector2 uv3);

	protected:
		Dot _dot1;
		Dot _dot2;
		Dot _dot3;
	};

	class Square : Graphic
	{
	public:
		Square() = default;
		Square(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4);
		~Square();

		void ApplyTransform(Matrix44 matrix) override;
		virtual void ApplyTransform(Matrix44 matrix, Vector3 center);

		Dot GetDot(int index);
		std::vector<Triangle> GetTriangles();
		void SetUV(Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector2 uv4);

	private:
		Triangle _triangle1;
		Triangle _triangle2;

	};

}