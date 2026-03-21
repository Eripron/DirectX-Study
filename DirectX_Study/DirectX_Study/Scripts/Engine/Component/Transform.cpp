#include "Transform.h"

using namespace DK;

Transform::Transform() : Component(CT_Transform)
{
	UpdatePosition();
	UpdateRotation();
	UpdateScale();
	UpdateWorldMatrix();
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return m_position;
}

//DirectX::XMFLOAT3 Transform::GetRotation()
//{
//	return m_rotation;
//}

DirectX::XMFLOAT4 DK::Transform::GetQuaternion()
{
	return m_quaternion;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return m_scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	return m_worldMatrix;
}

DirectX::XMFLOAT3 Transform::Right()
{
	return m_right;
}

DirectX::XMFLOAT3 Transform::Up()
{
	return m_up;
}

DirectX::XMFLOAT3 Transform::Front()
{
	return m_front;
}

void Transform::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;

	UpdatePosition();
	UpdateWorldMatrix();
}

void DK::Transform::SetQuaternion(DirectX::XMFLOAT4 quaternion)
{
	m_quaternion = quaternion;

	UpdateRotation();
	UpdateWorldMatrix();
}

void Transform::SetScale(float x, float y, float z)
{
	m_scale.x = x;
	m_scale.y = y;
	m_scale.z = z;

	UpdateScale();
	UpdateWorldMatrix();
}

void Transform::UpdatePosition()
{
	m_positionMatrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
}

void Transform::UpdateRotation()
{
	XMVECTOR q = XMLoadFloat4(&m_quaternion);
	m_rotationMatrix = XMMatrixRotationQuaternion(q);

	// update local dir
	DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMFLOAT3 front = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

	DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&right), m_rotationMatrix));
	DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&up), m_rotationMatrix));
	DirectX::XMStoreFloat3(&m_front, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&front), m_rotationMatrix));
}

void Transform::UpdateScale()
{
	m_scaleMatrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
}

void Transform::UpdateWorldMatrix()
{
	DirectX::XMStoreFloat4x4(&m_worldMatrix, m_scaleMatrix * m_rotationMatrix * m_positionMatrix);
}

void DK::Transform::RotateQuaternionAxis(XMFLOAT3 axis, float radian)
{
	if (radian == 0.0f) return;

	// 계산에 필요한 인자
	float halfAngle = radian / 2.0f;
	float w = cosf(halfAngle);
	float sinValue = sinf(halfAngle);

	XMFLOAT4 newQuaternion(axis.x * sinValue, axis.y * sinValue, axis.z * sinValue, w);

	// 쿼터니언 곱 계산
	DirectX::XMFLOAT4 result;
	// newQuaternion * m_quaternion(먼저 회전 적용됨)
	result.x = newQuaternion.w * m_quaternion.x + newQuaternion.x * m_quaternion.w + newQuaternion.y * m_quaternion.z - newQuaternion.z * m_quaternion.y;
	result.y = newQuaternion.w * m_quaternion.y - newQuaternion.x * m_quaternion.z + newQuaternion.y * m_quaternion.w + newQuaternion.z * m_quaternion.x;
	result.z = newQuaternion.w * m_quaternion.z + newQuaternion.x * m_quaternion.y - newQuaternion.y * m_quaternion.x + newQuaternion.z * m_quaternion.w;
	result.w = newQuaternion.w * m_quaternion.w - newQuaternion.x * m_quaternion.x - newQuaternion.y * m_quaternion.y - newQuaternion.z * m_quaternion.z;

	m_quaternion = result;

	// 로컬 축 갱신
	XMVECTOR front = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);

	// 누적 오차 제거
	XMVECTOR q = XMLoadFloat4(&m_quaternion);
	q = XMQuaternionNormalize(q);
	DirectX::XMStoreFloat4(&m_quaternion, q);

	DirectX::XMStoreFloat3(&m_front, XMVector3Rotate(front, q));
	DirectX::XMStoreFloat3(&m_up, XMVector3Rotate(up, q));
	DirectX::XMStoreFloat3(&m_right, XMVector3Rotate(right, q));

	UpdateRotation();
	UpdateWorldMatrix();
}

void DK::Transform::RotationQuaternion(DirectX::XMFLOAT3 radian)
{
	DirectX::XMFLOAT3 up(0, 1, 0);
	RotateQuaternionAxis(up, radian.y);
	RotateQuaternionAxis(m_right, radian.x);
}
