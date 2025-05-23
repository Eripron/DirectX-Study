#pragma once

#include "Numeric.hpp"

class DrawOffset
{
private:
	Numeric::Vector2 m_vCenter;
	Numeric::Vector2 m_vOffset;

	bool m_bOffsetMoving;

	Numeric::Vector2 m_vMoveStartPos; // °è»ê¿ë

public:
	DrawOffset(Numeric::Vector2);
	~DrawOffset();

	void StartMove(Numeric::Vector2);
	void Moving(Numeric::Vector2);
	void EndMove();

	Numeric::Vector2 GetOffset();
};
