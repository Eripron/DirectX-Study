#include "Transform.h"

namespace DK
{
	Transform::Transform()
	{
		UpdateMatrixPosition();
		UpdateMatrixRotation();
		UpdateMatrixScale();
	}

	void Transform::SetPosition(float x, float y, float z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;

		UpdateMatrixPosition();
		UpdateMatrixWorld();
	}

	void Transform::SetPosition(const DirectX::XMFLOAT3& position)
	{
		mPosition = position;

		UpdateMatrixPosition();
		UpdateMatrixWorld();
	}

	void Transform::SetRotation(float x, float y, float z)
	{
		mRotation.x = x;
		mRotation.y = y;
		mRotation.z = z;

		UpdateMatrixRotation();
		UpdateMatrixWorld();
	}

	void Transform::SetRotation(const DirectX::XMFLOAT3& rotation)
	{
		mRotation = rotation;

		UpdateMatrixRotation();
		UpdateMatrixWorld();
	}

	void Transform::SetScale(float x, float y, float z)
	{
		mScale.x = x;
		mScale.y = y;
		mScale.z = z;

		UpdateMatrixScale();
		UpdateMatrixWorld();
	}

	void Transform::SetScale(const DirectX::XMFLOAT3& scale)
	{
		mScale = scale;

		UpdateMatrixScale();
		UpdateMatrixWorld();
	}

	DirectX::XMFLOAT3 Transform::GetPosition()
	{
		return mPosition;
	}

	DirectX::XMFLOAT3 Transform::GetRotation()
	{
		return mRotation;
	}

	DirectX::XMFLOAT3 Transform::GetScale()
	{
		return mScale;
	}

	DirectX::XMFLOAT4X4 Transform::GetMatrixWorld()
	{
		return mWorldMatrix;
	}

	DirectX::XMFLOAT3 Transform::Right()
	{
		return mRight;
	}

	DirectX::XMFLOAT3 Transform::Up()
	{
		return mUp;
	}

	DirectX::XMFLOAT3 Transform::Front()
	{
		return mFront;
	}

	void Transform::UpdateMatrixPosition()
	{
		mPositionMatrix = DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
	}

	void Transform::UpdateMatrixRotation()
	{
		mRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
			DirectX::XMConvertToRadians(mRotation.x),
			DirectX::XMConvertToRadians(mRotation.y),
			DirectX::XMConvertToRadians(mRotation.z));
	}

	void Transform::UpdateMatrixScale()
	{
		mScaleMatrix = DirectX::XMMatrixScaling(mScale.x, mScale.y, mScale.z);
	}

	void Transform::UpdateMatrixWorld()
	{
		DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		DirectX::XMFLOAT3 front = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

		DirectX::XMStoreFloat3(&mRight, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&right), mRotationMatrix));
		DirectX::XMStoreFloat3(&mUp, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&up), mRotationMatrix));
		DirectX::XMStoreFloat3(&mFront, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&front), mRotationMatrix));
		DirectX::XMStoreFloat4x4(&mWorldMatrix, mScaleMatrix * mRotationMatrix * mPositionMatrix);
	}
}