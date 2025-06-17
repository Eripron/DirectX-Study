#include "GraphicUtils.h"

namespace DK
{
	Triangle GraphicUtils::CreateTriangle(Vector3 point, float r)
	{
		Vector3 p1 = point + (Vector3::Up() * r);
		Vector3 p2 = point + (Matrix44::RotateZMatrix44(-30) * Vector3::Right() * r);
		Vector3 p3 = point + (Matrix44::RotateZMatrix44(-150) * Vector3::Right() * r);

		return Triangle(p1, p2, p3);
	}

	Square GraphicUtils::CreateSquare(Vector3 point, float r)
	{
		Vector3 p1 = point + (Matrix44::RotateZMatrix44(45) * Vector3::Up() * r);
		Vector3 p2 = point + (Matrix44::RotateZMatrix44(-45) * Vector3::Up() * r);
		Vector3 p3 = point + (Matrix44::RotateZMatrix44(45) * Vector3::Up() * -r);
		Vector3 p4 = point + (Matrix44::RotateZMatrix44(-45) * Vector3::Up() * -r);

		return Square(p1, p2, p3, p4);
	}
	
}