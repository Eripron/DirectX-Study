#pragma once

#include <DirectXMath.h>
#include "MathUtils.h"

namespace DK
{
	class Transform
	{
	public:
		Transform();
		
		void SetPosition(float x, float y, float z);
		void SetPosition(const DirectX::XMFLOAT3& position);

		void SetRotation(float x, float y, float z);
		void SetRotation(const DirectX::XMFLOAT3& rotation);

		void SetScale(float x, float y, float z);
		void SetScale(const DirectX::XMFLOAT3& scale);

		DirectX::XMFLOAT3 GetPosition();
		DirectX::XMFLOAT3 GetRotation();
		DirectX::XMFLOAT3 GetScale();

		DirectX::XMFLOAT4X4 GetMatrixWorld();

		DirectX::XMFLOAT3 Right();
		DirectX::XMFLOAT3 Up();
		DirectX::XMFLOAT3 Front();

	private:
		void UpdateMatrixPosition();
		void UpdateMatrixRotation();
		void UpdateMatrixScale();
		void UpdateMatrixWorld();

	private:
		DirectX::XMFLOAT3 mPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 mRotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 mScale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		DirectX::XMFLOAT3 mRight = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 mUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		DirectX::XMFLOAT3 mFront = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

		DirectX::XMFLOAT4X4 mWorldMatrix = MathUtils::Identity4x4();

		DirectX::XMMATRIX mPositionMatrix;
		DirectX::XMMATRIX mRotationMatrix;
		DirectX::XMMATRIX mScaleMatrix;

	};
}