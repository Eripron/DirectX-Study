//***************************************************************************************
// AnimationHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "AnimationHelper.h"

using namespace DirectX;

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}
 
float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	if( t <= Keyframes.front().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if( t >= Keyframes.back().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for(UINT i = 0; i < Keyframes.size()-1; ++i)
		{
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				if(XMVectorGetX(XMVector4Dot(q0, q1)) < 0.0f)
					q1 = XMVectorNegate(q1);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime() const
{
	float startTime = FLT_MAX;
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		float boneStartTime = BoneAnimations[i].GetStartTime();
		if (boneStartTime < startTime)
			startTime = boneStartTime;
	}

	return startTime;
}

float AnimationClip::GetClipEndTime() const
{
	float endTime = 0.0f;
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		float boneEndTime = BoneAnimations[i].GetEndTime();
		if (boneEndTime > endTime)
			endTime = boneEndTime;
	}

	return endTime;
}

void AnimationClip::Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const
{
	for (int i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

UINT SkinnedData::BoneCount() const
{
	return _boneHierarchy.size();
}

float SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	if (_animations.find(clipName) != _animations.end())
		return _animations.at(clipName).GetClipStartTime();

	return 0.0f;
}

float SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	if (_animations.find(clipName) != _animations.end())
		return _animations.at(clipName).GetClipEndTime();

	return 0.0f;
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, std::vector<DirectX::XMFLOAT4X4>& boneOffsets, std::unordered_map<std::string, AnimationClip>& animations)
{
	_boneHierarchy.insert(_boneHierarchy.end(), boneHierarchy.begin(), boneHierarchy.end());
	_boneOffsets.insert(_boneOffsets.end(), boneOffsets.begin(), boneOffsets.end());

	for (auto iter = animations.begin(); iter != animations.end(); ++iter)
	{
		_animations[iter->first] = iter->second;
	}
}

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DirectX::XMFLOAT4X4>& finalTransforms) const
{
	if (_animations.find(clipName) == _animations.end())
		return;

	AnimationClip aniClip = _animations.at(clipName);

	int boneCount = BoneCount();
	std::vector<DirectX::XMFLOAT4X4> boneTransforms(boneCount);

	aniClip.Interpolate(timePos, boneTransforms);

	std::vector<DirectX::XMFLOAT4X4> toRootTransforms(boneCount);

	// 각 계산시 부모의 트랜스폼이 필요하므로, 부모에서 자식으로 내려가면서 계산한다.
	toRootTransforms[0] = boneTransforms[0];
	for (int i = 1; i < boneCount; ++i)
	{
		XMMATRIX parentTransform = XMLoadFloat4x4(&boneTransforms[i]);

		int parentIndex = _boneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(parentTransform, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// 각 본의 최종 트랜스폼은 본의 오프셋과 루트까지의 트랜스폼을 곱해서 구한다.
	for (UINT i = 0; i < boneCount; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&_boneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
	}
}
