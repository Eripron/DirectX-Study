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

	void Graphic::Fill(Renderer* rederer, COLORREF color)
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

	void Dot::Draw(Renderer* rederer)
	{
		rederer->DrawPixel(_vPos);
	}

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

	void Triangle::Draw(Renderer* rederer)
	{
		rederer->DrawLine(_dot1.GetPos(), _dot2.GetPos());
		rederer->DrawLine(_dot2.GetPos(), _dot3.GetPos());
		rederer->DrawLine(_dot3.GetPos(), _dot1.GetPos());
	}

	void Triangle::Fill(Renderer* rederer, COLORREF color)
	{
		//Vector2 minPos;
		//Vector2 maxPos;

		//minPos.x = fminf(_dot1.GetPos().x, _dot2.GetPos().x);
		//minPos.x = fminf(minPos.x, _dot3.GetPos().x);

		//maxPos.x = fmaxf(_dot1.GetPos().x, _dot2.GetPos().x);
		//maxPos.x = fmaxf(maxPos.x, _dot3.GetPos().x);

		//minPos.y = fminf(_dot1.GetPos().y, _dot2.GetPos().y);
		//minPos.y = fminf(minPos.y, _dot3.GetPos().y);

		//maxPos.y = fmaxf(_dot1.GetPos().y, _dot2.GetPos().y);
		//maxPos.y = fmaxf(maxPos.y, _dot3.GetPos().y);

		//Vector2 u = Vector2((_dot2.GetPos() - _dot1.GetPos()).x, (_dot2.GetPos() - _dot1.GetPos()).y);
		//Vector2 v = Vector2((_dot3.GetPos() - _dot1.GetPos()).x, (_dot3.GetPos() - _dot1.GetPos()).y);

		//float vv = v.Dot(v);
		//float uu = u.Dot(u);
		//float uv = u.Dot(v);

		//float denominator = uv * uv - uu * vv;

		//// 직선인 경우
		//if (denominator == 0)
		//	return;

		//Vector2 minSP = WindowsUtils::ToScreenPoint(minPos.x, minPos.y);
		//Vector2 maxSP = WindowsUtils::ToScreenPoint(maxPos.x, maxPos.y);

		//Vector2 sp1 = WindowsUtils::ToScreenPoint(_dot1.GetPos().x, _dot1.GetPos().y);
		//for (int x = minSP.x; x <= maxSP.x; ++x)
		//{
		//	for (int y = minSP.y; y <= maxSP.y; ++y)
		//	{
		//		Vector2 w = Vector2(x, y) - sp1;
		//		float wu = w.Dot(u);
		//		float wv = w.Dot(v);

		//		float s = (wv * uv - wu * vv) / denominator;
		//		float t = (wu * uv - wv * uu) / denominator;
		//		float oneMinusST = 1.f - s - t;

		//		if (((s >= 0.f) && (s <= 1.f)) && ((t >= 0.f) && (t <= 1.f)) && ((oneMinusST >= 0.f) && (oneMinusST <= 1.f)))
		//		{
		//			//rederer->DrawPixel(Vector3(x, y, 0), color);
		//			rederer->DrawPixel(Vector3(x, y, 0), RGB(255 * s, 255 * t, 255 * oneMinusST));
		//		}
		//	}
		//}
	}

	Dot Triangle::GetDot(int index)
	{
		if (index == 0) return _dot1;
		else if (index == 1) return _dot2;
		else if (index == 2) return _dot3;

		return Dot();
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

	void Square::Draw(Renderer* rederer)
	{
		_triangle1.Draw(rederer);
		_triangle2.Draw(rederer);
	}

	Dot Square::GetDot(int index)
	{
		if (index == 0) return _triangle1.GetDot(0);
		else if (index == 1) return _triangle1.GetDot(1);
		else if (index == 2) return _triangle1.GetDot(2);
		else if (index == 3) return _triangle2.GetDot(2);

		return Dot();
	}

#pragma endregion

}