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
				XMVECTOR Q = XMVectorLerp(q0, q1, lerpPercent);

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
	return 0;
}

float SkinnedData::GetClipStartTime(const std::string& clipName) const
{
	return 0.0f;
}

float SkinnedData::GetClipEndTime(const std::string& clipName) const
{
	return 0.0f;
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, std::vector<DirectX::XMFLOAT4X4>& boneOffsets, std::unordered_map<std::string, AnimationClip>& animations)
{
}

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DirectX::XMFLOAT4X4>& finalTransforms) const
{
}
