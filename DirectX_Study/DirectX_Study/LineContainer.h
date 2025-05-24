#pragma once

#include <Windows.h>
#include <vector>
#include <stdexcept>

#include "Numeric.hpp"
#include "DrawFunc.hpp"

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
	LineContainer();
	~LineContainer();

	void AddLine(Numeric::Vector3 _p1, Numeric::Vector3 _p2);
	void RemoveLine(int _index);

	void DrawLine(HDC _hdc, HBITMAP _hBit, Numeric::Vector2 _offset);

	Line GetLine(int _index);

	int GetCount();

};