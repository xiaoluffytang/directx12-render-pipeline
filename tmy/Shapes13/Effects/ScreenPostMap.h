#pragma once
#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Texture/Texture2DResource.h"
class ScreenPostMap
{
public:
	ScreenPostMap(UINT width, UINT height);
	~ScreenPostMap();

	UINT Width()const;
	UINT Height()const;
	ID3D12Resource* Resource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv()const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv()const;
	void BuildDescriptors();
	void OnResize(UINT newWidth, UINT newHeight);

private:
	
	void BuildResource();

public:

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	Texture2DResource* texture;
};

