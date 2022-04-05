#include "Texture3DResource.h"
#include "../Manager/Manager.h"
extern Manager* manager;

Texture3DResource::Texture3DResource()
{

}

void Texture3DResource::CreateRtvDescriptors()
{
	auto desc = Resource->GetDesc();
	int count = desc.DepthOrArraySize;
	int mRtvDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (int i = 0; i < count; i++)
	{
		int rtvIndex = manager->textureManager->GetRtvDescriptorIndex();
		this->rtvIndexs.push_back(rtvIndex);
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(manager->textureManager->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), rtvIndex, mRtvDescriptorSize);
		this->handles.push_back(hDescriptor);
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = desc.Format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;
		manager->commonInterface.md3dDevice->CreateRenderTargetView(Resource.Get(), &rtvDesc, hDescriptor);
	}
}

D3D12_SRV_DIMENSION Texture3DResource::GetDimension()
{
	return D3D12_SRV_DIMENSION_TEXTURECUBE;
}