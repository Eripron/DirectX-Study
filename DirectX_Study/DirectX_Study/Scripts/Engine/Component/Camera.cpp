#include "Camera.h"

using namespace DK;

DK::Camera::Camera(float fNear, float fFar, float fFovAngle, float fAspect)
	: m_fNear(fNear), m_fFar(fFar), m_fFovAngle(fFovAngle), m_fAspect(fAspect)
{
	UpdateViewMatrix();
	UpdateProjMatrix();
}

DK::Camera::~Camera()
{
}

Transform& DK::Camera::GetTransform()
{
	return m_transform;
}

XMFLOAT4X4 DK::Camera::GetViewMatrix()
{
	UpdateViewMatrix();

	return m_f4x4ViewMatrix;
}

XMFLOAT4X4 DK::Camera::GetProjMatrix()
{
	UpdateProjMatrix();

	return m_f4x4ProjMatrix;
}

float DK::Camera::GetNear()
{
	return m_fNear;
}

float DK::Camera::GetFar()
{
	return m_fFar;
}

float DK::Camera::GetFovAngle()
{
	return m_fFovAngle;
}

void DK::Camera::SetNear(float fNear)
{
	m_fNear = fNear;
}

void DK::Camera::SetFar(float fFar)
{
	m_fFar = fFar;
}

void DK::Camera::SetFovAngle(float fFovAngle)
{
	m_fFovAngle = fFovAngle;
}

void DK::Camera::SetAspect(float fAspect)
{
	m_fAspect = fAspect;
}

void DK::Camera::UpdateViewMatrix()
{
	Transform tranCamera = GetTransform();

	DirectX::XMFLOAT3 camPos = tranCamera.GetPosition();
	DirectX::XMFLOAT3 focusPos = camPos + (tranCamera.Front() * m_fFar);

	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(camPos.x, camPos.y, camPos.z, 1.0f);
	DirectX::XMVECTOR eyeDir = DirectX::XMLoadFloat3(&focusPos);
	DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(eyePos, eyeDir, upDir);
	XMStoreFloat4x4(&m_f4x4ViewMatrix, view);
}

void DK::Camera::UpdateProjMatrix()
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fFovAngle), m_fAspect, m_fNear, m_fFar);
	XMStoreFloat4x4(&m_f4x4ProjMatrix, proj);
}
