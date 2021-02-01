#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, std::vector<int> objectCounts, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	if(materialCount > 0)
		MaterialBuffer = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, false);
	int objectCount = objectCounts.size();
	for (int i = 0; i < objectCount; i++)
	{
		//每个游戏物体创建最多少个buffer缓冲区
		int eleCount = objectCounts[i];
		if (eleCount > 0)
		{
			auto buffer = new UploadBuffer<InstanceData>(device, eleCount, false);
			InstanceBuffer.push_back(buffer);
		}	
		else
		{
			InstanceBuffer.push_back(nullptr);
		}
	}
}

FrameResource::~FrameResource()
{

}