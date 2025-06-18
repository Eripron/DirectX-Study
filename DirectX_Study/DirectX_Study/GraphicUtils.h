#pragma once

#include "Graphic.h"
#include "Numeric.h"
#include "MathUtils.h"

namespace DK
{
	class GraphicUtils
	{
	public:
		static Triangle CreateTriangle(Vector3 point, float r);

		static Square CreateSquare(Vector3 point, float r);
		static Square CreateSquare(float width, float height);
	};
}