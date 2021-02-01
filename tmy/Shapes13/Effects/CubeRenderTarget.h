//***************************************************************************************
// CubeRenderTarget.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#include "../../../Common/d3dUtil.h"
#include "../../../Common/Camera.h"
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
	CubeRenderTarget(ID3D12Device* device,
		UINT width, UINT height,
		DXGI_FORMAT format);

	CubeRenderTarget(const CubeRenderTarget& rhs) = delete;
	CubeRenderTarget& operator=(const CubeRenderTarget& rhs) = delete;
	~CubeRenderTarget() = default;

	ID3D12Resource* Resource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv();
	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(int faceIndex);
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv();

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	const int CubeMapSize = 512;
	XMFLOAT3 mPos;
	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv[6],
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDSV);

	void OnResize(UINT newWidth, UINT newHeight);
	Camera mCubeMapCamera[6];
	void BuildCubeFaceCamera(float x, float y, float z);
private:
	void BuildDescriptors();
	void BuildResource();

private:
	void BuildCubeDepthStencil();
	
	ID3D12Device* md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuRtv[6];
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDSV;

	Microsoft::WRL::ComPtr<ID3D12Resource> mCubeMap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mCubeDepthStencilBuffer = nullptr;
	
};

