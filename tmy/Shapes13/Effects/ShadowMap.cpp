#include "ShadowMap.h"
#include "../Engine/Manager/Manager.h"
extern Manager* manager;

ShadowMap::ShadowMap(UINT width, UINT height)
{

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };
}

UINT ShadowMap::Width()const
{
	return mWidth;
}

UINT ShadowMap::Height()const
{
	return mHeight;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv()const
{
	return shadowMap->GetDsvCpuHandle();
}

D3D12_VIEWPORT ShadowMap::Viewport()const
{
	return mViewport;
}

D3D12_RECT ShadowMap::ScissorRect()const
{
	return mScissorRect;
}

void ShadowMap::OnResize(UINT newWidth, UINT newHeight)
{
	//if ((mWidth != newWidth) || (mHeight != newHeight))
	//{
	//	mWidth = newWidth;
	//	mHeight = newHeight;

	//	BuildResource();

	//	// New resource, so we need new descriptors to that resource.
	//	BuildDescriptors();
	//}
}

void ShadowMap::BuildDescriptors()
{
	BuildResource();
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mWidth;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	shadowMap = new Texture2DResource();
	shadowMap->LoadTextureByDesc(&texDesc, &optClear);
	shadowMap->CreateDsvIndexDescriptor();
}