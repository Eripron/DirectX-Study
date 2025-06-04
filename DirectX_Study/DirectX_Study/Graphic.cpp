#include "Graphic.h"

namespace Graphic
{

#pragma region Graphic

Graphic::Graphic()	{}
Graphic::~Graphic()	{}

#pragma endregion

#pragma region Dot

Dot::Dot(float _x, float _y, float _z) : m_vPos(Vector3(_x, _y, _z))
{
}

Dot::Dot(Vector2 _vPos) : m_vPos(Vector3(_vPos.x, _vPos.y, 0))
{
}

Dot::Dot(Vector3 _vPos) : m_vPos(_vPos)
{
}

Dot::~Dot() 
{
}

void Dot::Draw(HDC _hdc, COLORREF _color)
{
	SetPixel(_hdc, m_vPos.x, m_vPos.y, _color);
}

Vector3 Dot::GetPos()
{
	return m_vPos;
}

#pragma endregion

#pragma region Triangle

Triangle::Triangle(Dot _dot1, Dot _dot2, Dot _dot3) 
	: m_dot1(_dot1), m_dot2(_dot2), m_dot3(_dot3)
{
}

Triangle::~Triangle()
{
}

void Triangle::Draw(HDC _hdc, COLORREF _color)
{
	GraphicUtils::DrawLine(_hdc, m_dot1.GetPos(), m_dot2.GetPos(), _color);
	GraphicUtils::DrawLine(_hdc, m_dot2.GetPos(), m_dot3.GetPos(), _color);
	GraphicUtils::DrawLine(_hdc, m_dot3.GetPos(), m_dot1.GetPos(), _color);
}

#pragma endregion

#pragma region Circle

#pragma endregion

}