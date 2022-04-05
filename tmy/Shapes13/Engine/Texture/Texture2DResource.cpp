#include "Texture2DResource.h"
#include "../Manager/Manager.h"
extern Manager* manager;

Texture2DResource::Texture2DResource()
{

}

void Texture2DResource::LoadTextureByFile(const string& filename)
{
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), StringToWString(filename).c_str(),
		Resource, UploadHeap));
	CreateSrvIndexDescriptor();
	height = Resource->GetDesc().Height;
	width = Resource->GetDesc().Width;
}

void Texture2DResource::LoadTextureByDesc(const D3D12_RESOURCE_DESC* texDesc, const D3D12_CLEAR_VALUE* clear_desc)
{
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		texDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		clear_desc,
		IID_PPV_ARGS(&Resource)));
	CreateSrvIndexDescriptor();
	height = Resource->GetDesc().Height;
	width = Resource->GetDesc().Width;
}

void Texture2DResource::CreateSrvIndexDescriptor()
{
	if (srvIndex >= 0)
		return;
	srvIndex = manager->textureManager->GetSrvDescriptorIndex();
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(manager->textureManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex, mCbvSrvUavDescriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	auto format = Resource->GetDesc().Format;
	if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)   //这个格式的图片资源得转换下
	{
		format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	}
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = format;
	srvDesc.ViewDimension = GetDimension();
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = Resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	//wstring e = L"DDD";
	//MessageBox(nullptr, e.c_str(), L"卧槽", MB_OK);
	manager->commonInterface.md3dDevice->CreateShaderResourceView(Resource.Get(), &srvDesc, hDescriptor);
}

void Texture2DResource::CreateRtvIndexDescriptor()
{
	if (rtvIndex >= 0)
		return;
	rtvIndex = manager->textureManager->GetRtvDescriptorIndex();
	int mRtvDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(manager->textureManager->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), rtvIndex, mRtvDescriptorSize);
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = Resource->GetDesc().Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	manager->commonInterface.md3dDevice->CreateRenderTargetView(Resource.Get(), &rtvDesc, hDescriptor);
}

void Texture2DResource::CreateDsvIndexDescriptor()
{
	if (dsvIndex >= 0)
		return;
	dsvIndex = manager->textureManager->GetDsvDescriptorIndex();
	int mRtvDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(manager->textureManager->mDsvHeap->GetCPUDescriptorHandleForHeapStart(), dsvIndex, mRtvDescriptorSize);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = Resource->GetDesc().Format;
	dsvDesc.Texture2D.MipSlice = 0;
	manager->commonInterface.md3dDevice->CreateDepthStencilView(Resource.Get(), nullptr, hDescriptor);
}

void Texture2DResource::CreateUavIndexDescriptor()
{
	if (uavIndex >= 0)
		return;
	uavIndex = manager->textureManager->GetSrvDescriptorIndex();
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(manager->textureManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), uavIndex, mCbvSrvUavDescriptorSize);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = Resource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	manager->commonInterface.md3dDevice->CreateUnorderedAccessView(Resource.Get(), nullptr, &uavDesc, hDescriptor);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Texture2DResource::GetSrvCpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto heap = manager->textureManager->mSrvHeap.Get();
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), srvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Texture2DResource::GetSrvGpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto heap = manager->textureManager->mSrvHeap.Get();
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), srvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Texture2DResource::GetRtvCpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto heap = manager->textureManager->mRtvHeap.Get();
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), rtvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Texture2DResource::GetRtvGpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto heap = manager->textureManager->mRtvHeap.Get();
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), rtvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Texture2DResource::GetDsvCpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	auto heap = manager->textureManager->mDsvHeap.Get();
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), dsvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Texture2DResource::GetDsvGpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	auto heap = manager->textureManager->mDsvHeap.Get();
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), dsvIndex, mCbvSrvUavDescriptorSize);
	return handle;
}


CD3DX12_CPU_DESCRIPTOR_HANDLE Texture2DResource::GetUavCpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto heap = manager->textureManager->mSrvHeap.Get();
	auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), uavIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Texture2DResource::GetUavGpuHandle()
{
	int mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto heap = manager->textureManager->mSrvHeap.Get();
	auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), uavIndex, mCbvSrvUavDescriptorSize);
	return handle;
}

D3D12_SRV_DIMENSION Texture2DResource::GetDimension()
{
	return D3D12_SRV_DIMENSION_TEXTURE2D;
}