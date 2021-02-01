//***************************************************************************************
// Blur.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Blur.h"
#include "../Manager/Manager.h"

Blur::Blur(ID3D12Device* device,
	UINT width, UINT height)
{
	md3dDevice = device;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };
	BuildResources();
	PassCB = std::make_unique<UploadBuffer<BlurParam>>(md3dDevice, 1, true);
}

ID3D12Resource* Blur::Output()
{
	return mBlurMap0.Get();
}

void Blur::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor0,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor0,
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv0,
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor1,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor1,
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv1)
{
	mBlur0CpuSrv = hCpuDescriptor0;
	mBlur0GpuSrv = hGpuDescriptor0;
	mBlur0CpuDsv = hCpuRtv0;

	mBlur1CpuSrv = hCpuDescriptor1;
	mBlur1GpuSrv = hGpuDescriptor1;
	mBlur1CpuDsv = hCpuRtv1;

	BuildDescriptors();
}

void Blur::OnResize(UINT newWidth, UINT newHeight)
{
	if ((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResources();
		BuildDescriptors();
	}
}

void Blur::Execute(
	Manager* manager,
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12Resource* input,
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle,
	int blurCount, int blurResourceIndex, float blurRadius,
	const D3D12_VIEWPORT* mScreenViewport,
	const D3D12_RECT* mScreenScissorRect)
{
	if (blurCount < 1)
	{
		return;
	}
	cmdList->SetComputeRootSignature(rootSig);

	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);

	BlurParam param;
	param.blurResourceIndex = blurResourceIndex;
	param.blurRadius = blurRadius;
	param.scaleX = manager->commonInterface.mClientWidth / mWidth;
	param.scaleY = manager->commonInterface.mClientHeight / mHeight;
	PassCB->CopyData(0, param);
	cmdList->SetGraphicsRootConstantBufferView(6, PassCB->Resource()->GetGPUVirtualAddress());

	//第一次模糊
	BlurHorizontal(cmdList, manager, manager->psoManager->mPSOs["BlurHorizontalFirst"]);
	BlurVertical(cmdList, manager, manager->psoManager->mPSOs["BlurVerticalFirst"]);
	for (int i = 1; i < blurCount - 1; i++)
	{
		BlurHorizontal(cmdList, manager, manager->psoManager->mPSOs["BlurHorizontal"]);
		BlurVertical(cmdList, manager, manager->psoManager->mPSOs["BlurVertical"]);
	}

	cmdList->RSSetViewports(1, mScreenViewport);
	cmdList->RSSetScissorRects(1, mScreenScissorRect);
	//将结果复制
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->OMSetRenderTargets(1, &handle, true, nullptr);
	cmdList->SetPipelineState(manager->psoManager->mPSOs["BlurCopy"]);
	manager->renderManager->RenderList(manager->renderManager->postEffect, false);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

//横向模糊
void Blur::BlurHorizontal(ID3D12GraphicsCommandList* cmdList, Manager* manager, ID3D12PipelineState* horzBlurPSO)
{
	//横向模糊，0作为渲染目标，1作为资源
	cmdList->SetPipelineState(horzBlurPSO);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ClearRenderTargetView(mBlur0CpuDsv, Colors::Black, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &mBlur0CpuDsv, true, nullptr);
	manager->renderManager->RenderList(manager->renderManager->postEffect, false);
}

//横向模糊
void Blur::BlurVertical(ID3D12GraphicsCommandList* cmdList, Manager* manager, ID3D12PipelineState* vertical)
{
	//横向模糊，1作为渲染目标，0作为资源
	cmdList->SetPipelineState(vertical);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ClearRenderTargetView(mBlur1CpuDsv, Colors::Black, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &mBlur1CpuDsv, true, nullptr);
	manager->renderManager->RenderList(manager->renderManager->postEffect, false);
}

void Blur::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(mBlurMap0.Get(), &srvDesc, mBlur0CpuSrv);
	md3dDevice->CreateShaderResourceView(mBlurMap1.Get(), &srvDesc, mBlur1CpuSrv);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateRenderTargetView(mBlurMap0.Get(), nullptr, mBlur0CpuDsv);
	md3dDevice->CreateRenderTargetView(mBlurMap1.Get(), nullptr, mBlur1CpuDsv);
}

void Blur::BuildResources()
{
	//创建2个资源
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

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_PRESENT,
		nullptr,
		IID_PPV_ARGS(&mBlurMap0)));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_PRESENT,
		nullptr,
		IID_PPV_ARGS(&mBlurMap1)));
	
}