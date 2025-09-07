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

DirectX::XMFLOAT3 Transform::GetRotation()
{
	return m_rotation;
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

void Transform::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;

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
	m_rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		DirectX::XMConvertToRadians(m_rotation.x),
		DirectX::XMConvertToRadians(m_rotation.y),
		DirectX::XMConvertToRadians(m_rotation.z));

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
