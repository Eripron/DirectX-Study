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
		FrameResource(ID3D12Device* device, UINT renderPassCount, UINT objectCount, UINT materialCount, UINT waveVertexCount);
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;
		~FrameResource();

		// We cannot reset the allocator until the GPU is done processing the commands.
		// So each frame needs their own allocator.
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the commands
		// that reference it.  So each frame needs their own cbuffers.
		std::unique_ptr<UploadBuffer<RenderPassConstants>> RenderPassCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
		std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

		std::unique_ptr<UploadBuffer<Vertex>> WaveVB = nullptr;

		// Fence value to mark commands up to this fence point.  This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64 Fence = 0;
	};

}