#include "FrameResource.h"
#include "../Manager/Manager.h"
extern Manager* manager;

FrameResource::FrameResource(UINT passCount, UINT materialCount)
{
	ID3D12Device* device = manager->commonInterface.md3dDevice.Get();
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));
	commonCB = std::make_unique<UploadBuffer<CommonConstants>>(device, 1, true);
	if(materialCount > 0)
		MaterialBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, false);
}

FrameResource::~FrameResource()
{

}