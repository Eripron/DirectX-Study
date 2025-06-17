#pragma once

#include "Numeric.h"

namespace DK
{
	class Transform
	{
	public:
		Transform() = default;
		
		void SetPosition(const Vector3& vPosision);
		void SetRotation(const Vector3& vRotation);
		void SetScale(const Vector3& vScale);

		Vector3 GetPosition();
		Vector3 GetRotation();
		Vector3 GetScale();

	private:
		Vector3 _vPosition = Vector3::Zero();
		Vector3 _vRotation = Vector3::Zero();
		Vector3 _vScale = Vector3::One();

		// local axis
		/*Vector3 _vRight = Vector3::Right();
		Vector3 _vUp = Vector3::Up();
		Vector3 _vForward = Vector3::Forward();*/
	};
}