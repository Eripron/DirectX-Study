#include "DrawOffset.hpp"

DrawOffset::DrawOffset(Numeric::Vector2 vCenter)
	: m_vCenter(vCenter), m_vOffset(Numeric::Vector2(0, 0)), m_bOffsetMoving(false)
{
}

DrawOffset::~DrawOffset()
{
}

void DrawOffset::StartMove(Numeric::Vector2 vMoveStartPos)
{
	m_bOffsetMoving = true;

	m_vMoveStartPos = vMoveStartPos;
}

void DrawOffset::Moving(Numeric::Vector2 vMovePos)
{
	if (m_bOffsetMoving == false)
		return;

	int dx = vMovePos.x - m_vMoveStartPos.x;
	int dy = vMovePos.y - m_vMoveStartPos.y;

	m_vOffset.x = dx;
	m_vOffset.y = dy;
}

void DrawOffset::EndMove()
{
	m_bOffsetMoving = false;

	m_vCenter.x += m_vOffset.x;
	m_vCenter.y += m_vOffset.y;

	m_vOffset.x = 0;
	m_vOffset.y = 0;
}

Numeric::Vector2 DrawOffset::GetOffset()
{
	int x = m_vCenter.x + m_vOffset.x;
	int y = m_vCenter.y + m_vOffset.y;

	return Numeric::Vector2(x, y);
}
