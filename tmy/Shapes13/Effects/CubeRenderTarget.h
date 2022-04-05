//***************************************************************************************
// CubeRenderTarget.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Common/Camera.h"
#include "../Engine/Texture/Texture2DResource.h"
#include "../Engine/Texture/Texture3DResource.h"
using namespace DirectX;

enum class CubeMapFace : int
{
	PositiveX = 0,
	NegativeX = 1,
	PositiveY = 2,
	NegativeY = 3,
	PositiveZ = 4,
	NegativeZ = 5
};

class CubeRenderTarget
{
public:
	CubeRenderTarget(UINT width, UINT height, DXGI_FORMAT format);

	CubeRenderTarget(const CubeRenderTarget& rhs) = delete;
	CubeRenderTarget& operator=(const CubeRenderTarget& rhs) = delete;
	~CubeRenderTarget() = default;

	ID3D12Resource* Resource();
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(int faceIndex);
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv();

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	const int CubeMapSize = 512;
	XMFLOAT3 mPos;
	void BuildDescriptors();

	void OnResize(UINT newWidth, UINT newHeight);
	Camera mCubeMapCamera[6];
	void BuildCubeFaceCamera(float x, float y, float z);
private:
	void BuildResource();

public:
	void BuildCubeDepthStencil();

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	Texture3DResource* cubeMap = nullptr;
	Texture2DResource* dsMap = nullptr;
};

