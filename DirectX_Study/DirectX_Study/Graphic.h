#pragma once

#include <Windows.h>

#include "Numeric.h"
#include "MathUtils.h"
#include "GraphicUtils.h"

using namespace Numeric;

namespace Graphic
{
	class Graphic
	{
	public:
		Graphic();
		virtual ~Graphic();

		virtual void Draw(HDC _hdc, COLORREF _color) = 0;
		// TODO: move, rotate, scale
	};

	class Dot : public Graphic
	{
	protected:
		Vector3 m_vPos;

	public:
		Dot(float _x = 0, float _y = 0, float _z = 0);
		Dot(Vector2 _vPos);
		Dot(Vector3 _vPos);
		~Dot();

		void Draw(HDC _hdc, COLORREF _color) override;

		Vector3 GetPos();
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

		void Draw(HDC _hdc, COLORREF _color) override;
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

		void Draw(HDC _hdc, COLORREF _color) override;
	};

}
