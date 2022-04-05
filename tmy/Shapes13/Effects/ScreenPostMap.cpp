#include "ScreenPostMap.h"
#include "../Engine/Manager/Manager.h"
extern Manager* manager;

ScreenPostMap::ScreenPostMap(UINT width, UINT height)
{
	mWidth = width;
	mHeight = height;
}

ScreenPostMap::~ScreenPostMap()
{
}

UINT ScreenPostMap::Width()const
{
	return mWidth;
}

UINT ScreenPostMap::Height()const
{
	return mHeight;
}

ID3D12Resource*  ScreenPostMap::Resource()
{
	return texture->Resource.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ScreenPostMap::Srv()const
{
	return texture->GetSrvGpuHandle();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ScreenPostMap::Rtv()const
{
	return texture->GetRtvCpuHandle();
}

void ScreenPostMap::OnResize(UINT newWidth, UINT newHeight)
{
	if ((mWidth != newWidth) || (mHeight != newHeight))
	{

		mWidth = newWidth;
		mHeight = newHeight;
		if (texture != nullptr)
		{
			manager->textureManager->DeleteSrvDescriptorIndex(texture->srvIndex);
			manager->textureManager->DeleteRtvDescriptorIndex(texture->rtvIndex);
		}
		BuildResource();
	}
}

void ScreenPostMap::BuildDescriptors()
{
	BuildResource();
}

void ScreenPostMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	CD3DX12_CLEAR_VALUE optClear(DXGI_FORMAT_R16G16B16A16_FLOAT, normalClearColor);
	texture = new Texture2DResource();
	texture->LoadTextureByDesc(&texDesc, &optClear);
	texture->CreateRtvIndexDescriptor();
}