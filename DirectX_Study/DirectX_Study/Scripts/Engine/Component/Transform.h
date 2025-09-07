#pragma once

#include "Component.h"

#include <DirectXMath.h>

#include "../Utils/MathUtils.h"

namespace DK
{
	class Transform : public Component
	{
	public:
		Transform();
		~Transform() = default;
		
		DirectX::XMFLOAT3 GetPosition();
		DirectX::XMFLOAT3 GetRotation();
		DirectX::XMFLOAT3 GetScale();
		DirectX::XMFLOAT4X4 GetWorldMatrix();

		DirectX::XMFLOAT3 Right();
		DirectX::XMFLOAT3 Up();
		DirectX::XMFLOAT3 Front();

		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetScale(float x, float y, float z);

	private:
		void UpdatePosition();
		void UpdateRotation();
		void UpdateScale();
		void UpdateWorldMatrix();

	private:
		DirectX::XMFLOAT3 m_position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 m_rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 m_scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		DirectX::XMFLOAT3 m_right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 m_up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		DirectX::XMFLOAT3 m_front = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

		DirectX::XMMATRIX m_positionMatrix;
		DirectX::XMMATRIX m_rotationMatrix;
		DirectX::XMMATRIX m_scaleMatrix;

		DirectX::XMFLOAT4X4 m_worldMatrix = MathUtils::Identity4x4();

	};
}