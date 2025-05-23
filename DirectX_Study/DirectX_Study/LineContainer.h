#pragma once

#include <vector>
#include <stdexcept>
#include "Numeric.hpp"

class Line
{
private:
	Numeric::Vector3 p1;
	Numeric::Vector3 p2;

public:
	Line(Numeric::Vector3 _p1, Numeric::Vector3 _p2);

	Numeric::Vector3 GetPoint1();
	Numeric::Vector3 GetPoint2();
};

class LineContainer
{
private:
	std::vector<Line> m_vecLine;

public:
	void AddLine(Numeric::Vector3 _p1, Numeric::Vector3 _p2);
	void RemoveLine(int _index);

	Line GetLine(int _index);

	int GetCount();

};