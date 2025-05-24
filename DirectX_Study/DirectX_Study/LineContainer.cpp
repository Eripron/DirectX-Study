#include "LineContainer.h"

// line
Line::Line(Numeric::Vector3 _p1, Numeric::Vector3 _p2) : p1(_p1), p2(_p2)
{
}

Numeric::Vector3 Line::GetPoint1()
{
	return p1;
}

Numeric::Vector3 Line::GetPoint2()
{
	return p2;
}


// line container
LineContainer::LineContainer()
{
}

LineContainer::~LineContainer()
{
}

void LineContainer::AddLine(Numeric::Vector3 _p1, Numeric::Vector3 _p2)
{
	m_vecLine.push_back(Line(_p1, _p2));
}

void LineContainer::RemoveLine(int _index)
{
	if (_index < 0 || _index >= GetCount())
		return;

	std::vector<Line>::iterator iter = m_vecLine.begin();
	iter += _index;

	m_vecLine.erase(iter);
}

void LineContainer::DrawLine(HDC _hdc, HBITMAP _hBit, Numeric::Vector2 _offset)
{
	HBITMAP hOldBit = (HBITMAP)SelectObject(_hdc, _hBit);

	for (int i = 0; i < GetCount(); ++i)
	{
		Line line = m_vecLine[i];

		Numeric::Vector3 p1 = line.GetPoint1();
		p1.x += _offset.x;
		p1.y = _offset.y;

		Numeric::Vector3 p2 = line.GetPoint2();
		p2.x += _offset.x;
		p2.y = _offset.y;

		Graphic::DrawLine(_hdc, p1, p2);
	}

	SelectObject(_hdc, hOldBit);
}

Line LineContainer::GetLine(int _index)
{
	if (_index < 0 || _index >= GetCount())
	{
		throw std::out_of_range("LineContainer::GetLine - index out of range");
	}

	return m_vecLine[_index];
}

int LineContainer::GetCount()
{
	return m_vecLine.size();
}
