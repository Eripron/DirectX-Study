#include "FrameResource.h"

DK::FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
	THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}

DK::FrameResource::~FrameResource()
{
}
