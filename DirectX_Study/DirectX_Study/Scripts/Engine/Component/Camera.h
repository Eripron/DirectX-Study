#pragma once

#include <DirectXMath.h>

#include "Transform.h"

namespace DK
{
	class Camera
	{
	public:
		Camera(float fNear = 1.0f, float fFar = 1000.0f, float fFovAngle = 60.0f, float fAspect = 1.0f);
		~Camera();

		Transform& GetTransform();

		XMFLOAT4X4 GetViewMatrixf4();
		XMFLOAT4X4 GetProjMatrixf4();

		XMMATRIX GetViewMatrix();
		XMMATRIX GetProjMatrix();

		XMMATRIX GetInvViewMatrix();
		XMMATRIX GetInvProjMatrix();

		float GetNear();
		float GetFar();
		float GetFovAngle();

		void SetNear(float fNear);
		void SetFar(float fFar);
		void SetFovAngle(float fFovAngle);
		void SetAspect(float fAspect);

		void Move(DirectX::XMFLOAT3 move);
		void Rotate(DirectX::XMFLOAT3 rotate);

		void SetLens(float fovY, float aspect, float zn, float zf);

		Transform m_transform;

	private:
		void UpdateViewMatrix();
		void UpdateProjMatrix();

	private:

		XMFLOAT4X4 m_viewMatrix;
		XMFLOAT4X4 m_projMatrix;

		float m_fNear = 1.0f;
		float m_fFar = 1000.0f;
		float m_fFovAngle = 60.0f;
		float m_fAspect = 1.0f;
	};
}