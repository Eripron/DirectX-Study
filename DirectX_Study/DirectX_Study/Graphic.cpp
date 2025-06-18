#include "Graphic.h"
#include "WindowsUtils.h"

namespace DK
{
	Graphic::~Graphic()
	{
	}

	void Graphic::ApplyTransform(Matrix44 matrix, Vector3 center)
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

	Vector2 Dot::GetUV()
	{
		return _vUV;
	}

	void Dot::SetPos(Vector3 vPos)
	{
		_vPos = vPos;
	}

	void Dot::SetUV(Vector2 vUV)
	{
		_vUV = vUV;
	}
	
	void Dot::ApplyTransform(Matrix44 matrix)
	{
		SetPos(matrix * _vPos);
	}

	void Dot::ApplyTransform(Matrix44 matrix, Vector3 center)
	{
		Vector3 p1 = GetPos();
		p1 = matrix * (p1 - center) + center;
		SetPos(p1);
	}

	/*void Dot::Draw(Renderer* rederer)
	{
		rederer->DrawPixel(_vPos);
	}*/

#pragma endregion


#pragma region Triangle

	Triangle::Triangle()
	{
	}

	Triangle::Triangle(Vector2 p1, Vector2 p2, Vector2 p3)
		: _dot1(Dot(p1)), _dot2(Dot(p2)), _dot3(Dot(p3))
	{
	}

	Triangle::Triangle(Vector3 p1, Vector3 p2, Vector3 p3)
		: _dot1(Dot(p1)), _dot2(Dot(p2)), _dot3(Dot(p3))
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

		ApplyTransform(matrix, center);
	}

	void Triangle::ApplyTransform(Matrix44 matrix, Vector3 center)
	{
		_dot1.ApplyTransform(matrix, center);
		_dot2.ApplyTransform(matrix, center);
		_dot3.ApplyTransform(matrix, center);
	}

	/*void Triangle::Draw(Renderer* rederer)
	{
		rederer->DrawLine(_dot1.GetPos(), _dot2.GetPos());
		rederer->DrawLine(_dot2.GetPos(), _dot3.GetPos());
		rederer->DrawLine(_dot3.GetPos(), _dot1.GetPos());
	}*/

	Dot Triangle::GetDot(int index)
	{
		if (index == 0) return _dot1;
		else if (index == 1) return _dot2;
		else if (index == 2) return _dot3;

		return Dot();
	}

	void Triangle::SetUV(Vector2 uv1, Vector2 uv2, Vector2 uv3)
	{
		_dot1.SetUV(uv1);
		_dot2.SetUV(uv2);
		_dot3.SetUV(uv3);
	}

#pragma endregion

#pragma region Square

	Square::Square(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
		: _triangle1(p1, p2, p3), _triangle2(p1, p3, p4)
	{
	}

	Square::~Square()
	{
	}

	void Square::ApplyTransform(Matrix44 matrix)
	{
		Vector3 p1 = GetDot(0).GetPos();
		Vector3 p2 = GetDot(1).GetPos();
		Vector3 p3 = GetDot(2).GetPos();
		Vector3 p4 = GetDot(3).GetPos();
		Vector3 center = (p1 + p2 + p3 + p4) / 4.0f;

		ApplyTransform(matrix, center);
	}

	void Square::ApplyTransform(Matrix44 matrix, Vector3 center)
	{
		_triangle1.ApplyTransform(matrix, center);
		_triangle2.ApplyTransform(matrix, center);
	}

	/*void Square::Draw(Renderer* rederer)
	{
		_triangle1.Draw(rederer);
		_triangle2.Draw(rederer);
	}*/

	Dot Square::GetDot(int index)
	{
		if (index == 0) return _triangle1.GetDot(0);
		else if (index == 1) return _triangle1.GetDot(1);
		else if (index == 2) return _triangle1.GetDot(2);
		else if (index == 3) return _triangle2.GetDot(2);

		return Dot();
	}

	std::vector<Triangle> Square::GetTriangles()
	{
		std::vector<Triangle> vec;
		vec.push_back(_triangle1);
		vec.push_back(_triangle2);

		return vec;
	}

	void Square::SetUV(Vector2 uv1, Vector2 uv2, Vector2 uv3, Vector2 uv4)
	{
		_triangle1.SetUV(uv1, uv2, uv3);
		_triangle2.SetUV(uv1, uv3, uv4);
	}

#pragma endregion

}