#include "Transform.h"

namespace DK
{
	void Transform::SetPosition(float x, float y, float z)
	{
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;

		UpdateMatrixWorld();
	}

	void Transform::SetPosition(const DirectX::XMFLOAT3& position)
	{
		mPosition = position;

		UpdateMatrixWorld();
	}

	void Transform::SetRotation(float x, float y, float z)
	{
		mRotation.x = x;
		mRotation.y = y;
		mRotation.z = z;

		UpdateMatrixWorld();
	}

	void Transform::SetRotation(const DirectX::XMFLOAT3& rotation)
	{
		mRotation = rotation;

		UpdateMatrixWorld();
	}

	void Transform::SetScale(float x, float y, float z)
	{
		mScale.x = x;
		mScale.y = y;
		mScale.z = z;

		UpdateMatrixWorld();
	}

	void Transform::SetScale(const DirectX::XMFLOAT3& scale)
	{
		mScale = scale;

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

	void Transform::UpdateMatrixWorld()
	{
		DirectX::XMStoreFloat4x4(&mWorldMatrix, DirectX::XMMatrixScaling(mScale.x, mScale.y, mScale.z) *
												DirectX::XMMatrixRotationRollPitchYaw(
												DirectX::XMConvertToRadians(mRotation.x), 
												DirectX::XMConvertToRadians(mRotation.y), 
												DirectX::XMConvertToRadians(mRotation.z)) *
												DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z));
	}
}