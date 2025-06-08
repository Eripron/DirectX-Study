#pragma once

#include <Windows.h>
#include <math.h>

#include "GraphicUtils.h"
#include "MathUtils.h"
#include "Numeric.h"

namespace DK
{
	//struct Vector2;
	//struct Vector3;

	class Graphic
	{
	public:
		Graphic() = default;
		virtual ~Graphic();

		/*virtual void Move(float x, float y, float z) = 0;
		virtual void Move(Vector3 move) = 0;

		virtual void Rotate(float degree) = 0;
		virtual void Scale(float scale) = 0;*/
	};

	class Dot : public Graphic
	{
	public:
		Dot(float x = 0, float y = 0, float z = 0);
		Dot(Vector2 vPos);
		Dot(Vector3 vPos);
		~Dot();

		Vector3 GetPos();

	protected:
		Vector3 _vPos;

	};

	class Triangle : public Graphic
	{
	protected:
		Dot m_dot1;
		Dot m_dot2;
		Dot m_dot3;

	public:
		Triangle(Dot _dot1, Dot _dot2, Dot _dot3);
		~Triangle();

	};

	class Circle : public Graphic
	{
	protected:
		Dot m_dotLB;
		Dot m_dotLU;
		Dot m_dotRB;
		Dot m_dotRU;

	public:
		Circle(float _lbx = 0, float _lby = 0, float _rux = 0, float _ruy = 0);
		~Circle();

	};

}