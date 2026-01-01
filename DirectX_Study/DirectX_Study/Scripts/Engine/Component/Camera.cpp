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

XMFLOAT4X4 DK::Camera::GetViewMatrixf4()
{
	UpdateViewMatrix();

	return m_viewMatrix;
}

XMFLOAT4X4 DK::Camera::GetProjMatrixf4()
{
	UpdateProjMatrix();

	return m_projMatrix;
}

XMMATRIX DK::Camera::GetViewMatrix()
{
	XMFLOAT4X4 viewf4 = GetViewMatrixf4();
	return XMLoadFloat4x4(&viewf4);
}

XMMATRIX DK::Camera::GetProjMatrix()
{
	XMFLOAT4X4 projf4 = GetProjMatrixf4();
	return XMLoadFloat4x4(&projf4);
}

XMMATRIX DK::Camera::GetInvViewMatrix()
{
	XMMATRIX view = GetViewMatrix();
	DirectX::XMVECTOR viewDetermin = XMMatrixDeterminant(view);

	return XMMatrixInverse(&viewDetermin, view);
}

XMMATRIX DK::Camera::GetInvProjMatrix()
{
	XMMATRIX proj = GetProjMatrix();
	DirectX::XMVECTOR projDetermin = XMMatrixDeterminant(proj);

	return XMMatrixInverse(&projDetermin, proj);
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

void DK::Camera::Move(DirectX::XMFLOAT3 move)
{
	DirectX::XMFLOAT3 newPos = m_transform.GetPosition();

	newPos = newPos + m_transform.Front() * move.z;
	newPos = newPos + m_transform.Right() * move.x;
	newPos.y += move.y;

	m_transform.SetPosition(newPos.x, newPos.y, newPos.z);
}

void DK::Camera::Rotate(DirectX::XMFLOAT3 rotRadian)
{
	m_transform.RotationQuaternion(rotRadian);
}

void DK::Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	m_fFovAngle = fovY;
	m_fAspect = aspect;
	m_fNear = zn;
	m_fFar = zf;

	float mNearWindowHeight = 2.0f * m_fNear * tanf(0.5f * m_fFovAngle);
	float mFarWindowHeight = 2.0f * m_fFar * tanf(0.5f * m_fFovAngle);

	XMMATRIX P = XMMatrixPerspectiveFovLH(m_fFovAngle, m_fAspect, m_fNear, m_fFar);
	XMStoreFloat4x4(&m_projMatrix, P);
}

void DK::Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 cameraPosition = m_transform.GetPosition();
	DirectX::XMFLOAT4 cameraQuaternion = m_transform.GetQuaternion();

	DirectX::XMFLOAT3 frontDir = m_transform.Front();
	DirectX::XMFLOAT3 upDir = m_transform.Up();

	DirectX::XMVECTOR front = XMLoadFloat3(&frontDir);
	DirectX::XMVECTOR up = XMLoadFloat3(&upDir);

	XMMATRIX eyePos = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&cameraPosition), front, up);
	XMStoreFloat4x4(&m_viewMatrix, view);
}

void DK::Camera::UpdateProjMatrix()
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fFovAngle), m_fAspect, m_fNear, m_fFar);
	XMStoreFloat4x4(&m_projMatrix, proj);
}
