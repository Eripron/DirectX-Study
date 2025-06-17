#include "Transform.h"

namespace DK
{
	void Transform::SetPosition(const Vector3& vPosision) { _vPosition = vPosision; }
	void Transform::SetRotation(const Vector3& vRotation) { _vRotation = vRotation; }
	void Transform::SetScale(const Vector3& vScale) { _vScale = vScale; }

	Vector3 Transform::GetPosition() { return _vPosition; }
	Vector3 Transform::GetRotation() { return _vRotation; }
	Vector3 Transform::GetScale() { return _vScale; }

}