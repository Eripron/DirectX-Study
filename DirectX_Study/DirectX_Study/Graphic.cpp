#include "Graphic.h"

namespace DK
{
	Graphic::~Graphic()
	{
	}

#pragma region Dot

	Dot::Dot(float x, float y, float z) : _vPos(Vector3(x, y, z))
	{
	}

	Dot::Dot(Vector2 vPos) : _vPos(Vector3(vPos.x, vPos.y, 0))
	{
	}

	Dot::Dot(Vector3 vPos) : _vPos(vPos)
	{
	}

	Dot::~Dot()
	{
	}

	Vector3 Dot::GetPos()
	{
		return _vPos;
	}

	void Dot::SetPos(Vector3 vPos)
	{
		_vPos = vPos;
	}

	void Dot::ApplyTransform(Matrix44 matrix)
	{
		SetPos(matrix * _vPos);
	}

	void Dot::Draw(Renderer* rederer)
	{
		rederer->DrawPixel(_vPos);
	}

#pragma endregion


#pragma region Triangle

	Triangle::Triangle(Vector2 p1, Vector2 p2, Vector2 p3)
		: _dot1(Dot(p1)), _dot2(Dot(p2)), _dot3(Dot(p3))
	{
	}

	Triangle::Triangle(Vector3 p1, Vector3 p2, Vector3 p3)
		: _dot1(Dot(p1)), _dot2(Dot(p2)), _dot3(Dot(p3))
	{
	}

	Triangle::Triangle(Dot dot1, Dot dot2, Dot dot3)
		: _dot1(dot1), _dot2(dot2), _dot3(dot3)
	{
	}

	Triangle::~Triangle()
	{
	}

	void Triangle::ApplyTransform(Matrix44 matrix)
	{
		Vector3 p1 = _dot1.GetPos();
		Vector3 p2 = _dot2.GetPos();
		Vector3 p3 = _dot3.GetPos();

		Vector3 center = (p1 + p2 + p3) / 3.0f;
		p1 = matrix * (p1 - center) + center;
		p2 = matrix * (p2 - center) + center;
		p3 = matrix * (p3 - center) + center;

		_dot1.SetPos(p1);
		_dot2.SetPos(p2);
		_dot3.SetPos(p3);
	}

	void Triangle::Draw(Renderer* rederer)
	{
		rederer->DrawLine(_dot1.GetPos(), _dot2.GetPos());
		rederer->DrawLine(_dot2.GetPos(), _dot3.GetPos());
		rederer->DrawLine(_dot3.GetPos(), _dot1.GetPos());
	}

#pragma endregion

#pragma region Square

#pragma endregion

}