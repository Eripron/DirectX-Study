#pragma once

#include <memory>

#include "../Utils/D3DUtils.h"
#include "../Utils/MathUtils.h"
#include "../Data/DataTypes.h"
#include "UploadBuffer.h"

namespace DK
{
	struct FrameResource
	{
	public:
		// 복사 및 대입 생성자 삭제
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;

	public:
		FrameResource(ID3D12Device* device, UINT renderPassCount, UINT objectCount, UINT materialCount, UINT skinnedObjectCount);
		~FrameResource();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		std::unique_ptr<UploadBuffer<RenderPassConstants>> RenderPassCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
		std::unique_ptr<UploadBuffer<SkinnedConstants>> SkinnedCB = nullptr;
		std::unique_ptr<UploadBuffer<MaterialData>> MaterialBuffer = nullptr;

		UINT64 Fence = 0;
	};

}