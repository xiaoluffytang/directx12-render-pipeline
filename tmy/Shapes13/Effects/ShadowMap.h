#pragma once
#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Texture/Texture2DResource.h"
class ShadowMap
{
public:
	ShadowMap(UINT width, UINT height);

	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap& operator=(const ShadowMap& rhs) = delete;
	~ShadowMap() = default;

	UINT Width()const;
	UINT Height()const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv()const;

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void OnResize(UINT newWidth, UINT newHeight);
	void BuildDescriptors();

private:
	void BuildResource();

public:

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	Texture2DResource* shadowMap = nullptr;
};

