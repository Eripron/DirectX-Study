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
	DirectX::XMFLOAT3 newPos = m_transform.GetPosition() + move;
	m_transform.SetPosition(newPos.x, newPos.y, newPos.z);
}

void DK::Camera::Rotate(DirectX::XMFLOAT3 rotRadian)
{
	m_transform.RotationQuaternion(rotRadian);

	/*XMFLOAT4 curRotate = m_transform.GetQuaternion();
	XMVECTOR camQuaternion = XMLoadFloat4(&curRotate);

	//추가 회전 quaternion
	XMVECTOR rotQuaternion = XMQuaternionRotationRollPitchYaw(rotRadian.x, rotRadian.y, rotRadian.z);

	XMVECTOR newRotQuaternion = XMQuaternionMultiply(camQuaternion, rotQuaternion);
	newRotQuaternion = XMQuaternionNormalize(newRotQuaternion);

	XMFLOAT4 resultRot;
	XMStoreFloat4(&resultRot, newRotQuaternion);
	m_transform.SetQuaternion(resultRot);*/
}

void DK::Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 cameraPosition = m_transform.GetPosition();
	DirectX::XMFLOAT4 cameraQuaternion = m_transform.GetQuaternion();

	//XMVECTOR p = XMLoadFloat3(&cameraPosition);
	//XMVECTOR q = XMLoadFloat4(&cameraQuaternion);

	//// 1. 회전과 위치 쿼터니언 누적 (로컬 회전)
	//XMMATRIX R = XMMatrixRotationQuaternion(q);

	//// 2. 카메라 이동 (월드 좌표 반대로)
	//XMMATRIX T = XMMatrixTranslation(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z);

	//// 3. View Matrix
	//XMMATRIX view = T * R;  // 또는 XMMatrixInverse(nullptr, XMMatrixAffineTransformation( ... ))
	//XMStoreFloat4x4(&m_viewMatrix, view); // 전치(R) * 이동


	DirectX::XMFLOAT3 frontDir = m_transform.Front();
	DirectX::XMFLOAT3 upDir = m_transform.Up();

	DirectX::XMVECTOR front = XMLoadFloat3(&frontDir);
	DirectX::XMVECTOR up = XMLoadFloat3(&upDir);

	XMMATRIX eyePos = XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z);
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&cameraPosition), front, up);
	XMStoreFloat4x4(&m_viewMatrix, view);



	/*Transform tranCamera = GetTransform();

	DirectX::XMFLOAT3 camPos = tranCamera.GetPosition();
	DirectX::XMFLOAT3 focusPos = camPos + (tranCamera.Front() * m_fFar);

	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(camPos.x, camPos.y, camPos.z, 1.0f);
	DirectX::XMVECTOR eyeDir = DirectX::XMLoadFloat3(&focusPos);
	DirectX::XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(eyePos, eyeDir, upDir);
	XMStoreFloat4x4(&m_viewMatrix, view);*/
}

void DK::Camera::UpdateProjMatrix()
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fFovAngle), m_fAspect, m_fNear, m_fFar);
	XMStoreFloat4x4(&m_projMatrix, proj);
}
