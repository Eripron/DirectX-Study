#pragma once

#include <Windows.h>
#include <math.h>

#include "Numeric.h"
#include "Renderer.h"

namespace DK
{
	class Graphic
	{
	public:
		Graphic() = default;
		virtual ~Graphic();

		virtual void ApplyTransform(Matrix44 matrix) = 0;
		virtual void Draw(Renderer* rederer) = 0;
	};

	class Dot : public Graphic
	{
	public:
		Dot(float x = 0, float y = 0, float z = 0);
		Dot(Vector2 vPos);
		Dot(Vector3 vPos);
		~Dot();

		void ApplyTransform(Matrix44 matrix) override;
		void Draw(Renderer* rederer) override;

		Vector3 GetPos();

	protected:
		Vector3 _vPos;

	};

	class Triangle : public Graphic
	{
	public:
		Triangle(Vector2 p1, Vector2 p2, Vector2 p3);
		Triangle(Vector3 p1, Vector3 p2, Vector3 p3);
		Triangle(Dot dot1, Dot dot2, Dot dot3);
		~Triangle();

		void ApplyTransform(Matrix44 matrix) override;
		void Draw(Renderer* rederer) override;

	protected:
		Dot _dot1;
		Dot _dot2;
		Dot _dot3;
	};

}