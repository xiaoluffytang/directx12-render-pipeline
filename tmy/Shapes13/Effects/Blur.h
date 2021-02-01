#pragma once

#include "../../../Common/d3dUtil.h"
#include "../../../Common/UploadBuffer.h"
class Manager;

struct BlurParam
{
	int blurResourceIndex = 0;
	float blurRadius = 0;
	float scaleX = 1;
	float scaleY = 1;
};

class Blur
{
public:
	Blur(ID3D12Device* device,
		UINT width, UINT height);

	Blur(const Blur& rhs) = delete;
	Blur& operator=(const Blur& rhs) = delete;
	~Blur() = default;

	ID3D12Resource* Output();

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor0,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor0,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv0,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor1,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor1,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv1
		);

	void OnResize(UINT newWidth, UINT newHeight);

	void Execute(
		Manager* manager,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12Resource* input,
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle,
		int blurCount,int blurResourceIndex,float blurRadius,
		const D3D12_VIEWPORT* mScreenViewport,
		const D3D12_RECT* mScreenScissorRect);

private:

	void BuildDescriptors();
	void BuildResources();
	void BlurHorizontal(ID3D12GraphicsCommandList* cmdList, Manager* manager, ID3D12PipelineState* horzBlurPSO);
	void BlurVertical(ID3D12GraphicsCommandList* cmdList, Manager* manager, ID3D12PipelineState* horzBlurPSO);

private:

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	const int MaxBlurRadius = 5;

	ID3D12Device* md3dDevice = nullptr;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur0GpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CpuDsv;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur1GpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CpuDsv;

	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap1 = nullptr;

	std::unique_ptr<UploadBuffer<BlurParam>> PassCB = nullptr;
};

