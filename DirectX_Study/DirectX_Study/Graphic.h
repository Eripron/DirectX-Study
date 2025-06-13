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
		virtual void ApplyTransform(Matrix44 matrix, Vector3 center);
		virtual void Draw(Renderer* rederer) = 0;
		virtual void Fill(Renderer* rederer, COLORREF color);
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
		void Draw(Renderer* rederer) override;

		Vector3 GetPos();
		void SetPos(Vector3 vPos);

	protected:
		Vector3 _vPos;
	};

	class Triangle : public Graphic
	{
	public:
		Triangle();
		Triangle(Vector2 p1, Vector2 p2, Vector2 p3);
		Triangle(Vector3 p1, Vector3 p2, Vector3 p3);
		Triangle(Dot dot1, Dot dot2, Dot dot3);
		~Triangle();

		void ApplyTransform(Matrix44 matrix) override;
		virtual void ApplyTransform(Matrix44 matrix, Vector3 center);
		void Draw(Renderer* rederer) override;
		void Fill(Renderer* rederer, COLORREF color) override;

		Dot GetDot(int index);

	protected:
		Dot _dot1;
		Dot _dot2;
		Dot _dot3;
	};

	class Square : Graphic
	{
	public:
		Square(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4);
		~Square();

		// Graphic을(를) 통해 상속됨
		void ApplyTransform(Matrix44 matrix) override;
		void Draw(Renderer* rederer) override;
		void Fill(Renderer* rederer, COLORREF color) override;

	private:
		Triangle _triangle1;
		Triangle _triangle2;
	};

}