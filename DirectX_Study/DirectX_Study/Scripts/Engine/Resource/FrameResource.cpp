#include "FrameResource.h"

using namespace DK;

FrameResource::FrameResource(ID3D12Device* device, UINT renderPassCount, UINT objectCount, UINT materialCount)
{
	THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	if(renderPassCount > 0) 
		RenderPassCB = std::make_unique<UploadBuffer<RenderPassConstants>>(device, renderPassCount, true);

	if (objectCount > 0)
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);

	SsaoCB = std::make_unique<UploadBuffer<SsaoConstants>>(device, 1, false);

	if (materialCount > 0)
		MaterialBuffer = std::make_unique<UploadBuffer<MaterialData>>(device, materialCount + 10, false);
}

FrameResource::~FrameResource()
{
}
