#include <Windows.h>

#include "Numeric.h" 
#include "Graphic.h"
#include "MathUtils.h"

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

#pragma endregion




#pragma region Triangle

	Triangle::Triangle(Dot _dot1, Dot _dot2, Dot _dot3)
		: m_dot1(_dot1), m_dot2(_dot2), m_dot3(_dot3)
	{
	}

	Triangle::~Triangle()
	{
	}

#pragma endregion

#pragma region Circle

#pragma endregion

}