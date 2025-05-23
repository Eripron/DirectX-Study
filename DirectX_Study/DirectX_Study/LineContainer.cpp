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
