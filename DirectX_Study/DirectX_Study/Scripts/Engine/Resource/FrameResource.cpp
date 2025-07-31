#include "FrameResource.h"

using namespace DK;

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT waveVertexCount)
{
	THROW_IF_FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdListAlloc)));

	RenderPassCB = std::make_unique<UploadBuffer<RenderPassConstants>>(device, passCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);

	WaveVB = std::make_unique<UploadBuffer<Vertex>>(device, waveVertexCount, false);
}

FrameResource::~FrameResource()
{
}
