#pragma once

#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Common/UploadBuffer.h"
#include "../Engine/Texture/Texture2DResource.h"

struct BlurParam
{
	int blurResourceIndex = 0;
	float blurRadius = 0;
	float scaleX = 1;
	float scaleY = 1;
	int horIndex = 0;
	int vecIndex = 0;
};

class Blur
{
public:
	Blur(UINT width, UINT height);

	Blur(const Blur& rhs) = delete;
	Blur& operator=(const Blur& rhs) = delete;
	~Blur() = default;

	void BuildDescriptors();

	void OnResize(UINT newWidth, UINT newHeight);

	void Execute(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12Resource* input,
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle,
		int blurCount,int blurResourceIndex,float blurRadius,
		const D3D12_VIEWPORT* mScreenViewport,
		const D3D12_RECT* mScreenScissorRect);

private:
	void BlurHorizontal(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* horzBlurPSO);
	void BlurVertical(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* horzBlurPSO);

private:

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	const int MaxBlurRadius = 5;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	Texture2DResource* blurMap0;
	Texture2DResource* blurMap1;

	std::unique_ptr<UploadBuffer<BlurParam>> PassCB = nullptr;
};

