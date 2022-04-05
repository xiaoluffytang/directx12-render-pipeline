//***************************************************************************************
// Blur.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Blur.h"
#include "../Engine/Manager/Manager.h"
extern Manager* manager;

Blur::Blur(UINT width, UINT height)
{
	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	PassCB = std::make_unique<UploadBuffer<BlurParam>>(manager->commonInterface.md3dDevice.Get(), 1, true);
}

void Blur::BuildDescriptors()
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
	blurMap0 = new Texture2DResource();
	blurMap0->LoadTextureByDesc(&texDesc, nullptr);
	blurMap0->CreateRtvIndexDescriptor();
	blurMap1 = new Texture2DResource();
	blurMap1->LoadTextureByDesc(&texDesc, nullptr);
	blurMap1->CreateRtvIndexDescriptor();
}

void Blur::OnResize(UINT newWidth, UINT newHeight)
{
	if ((mWidth != newWidth) || (mHeight != newHeight))
	{
		//mWidth = newWidth;
		//mHeight = newHeight;

		////BuildResources();
		//BuildDescriptors();
	}
}

void Blur::Execute(
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
	param.horIndex = blurMap1->srvIndex;
	param.vecIndex = blurMap0->srvIndex;
	PassCB->CopyData(0, param);
	cmdList->SetGraphicsRootConstantBufferView(6, PassCB->Resource()->GetGPUVirtualAddress());

	//第一次模糊
	BlurHorizontal(cmdList,  manager->psoManager->mPSOs["BlurHorizontalFirst"]);
	BlurVertical(cmdList,  manager->psoManager->mPSOs["BlurVerticalFirst"]);
	for (int i = 1; i < blurCount - 1; i++)
	{
		BlurHorizontal(cmdList,  manager->psoManager->mPSOs["BlurHorizontal"]);
		BlurVertical(cmdList,  manager->psoManager->mPSOs["BlurVertical"]);
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
void Blur::BlurHorizontal(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* horzBlurPSO)
{
	//横向模糊，0作为渲染目标，1作为资源
	cmdList->SetPipelineState(horzBlurPSO);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(blurMap1->Resource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(blurMap0->Resource.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ClearRenderTargetView(blurMap0->GetRtvCpuHandle(), Colors::Black, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &blurMap0->GetRtvCpuHandle(), true, nullptr);
	manager->renderManager->RenderList(manager->renderManager->postEffect, false);
}

//横向模糊
void Blur::BlurVertical(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* vertical)
{
	//横向模糊，1作为渲染目标，0作为资源
	cmdList->SetPipelineState(vertical);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(blurMap0->Resource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(blurMap1->Resource.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	cmdList->ClearRenderTargetView(blurMap1->GetRtvCpuHandle(), Colors::Black, 0, nullptr);
	cmdList->OMSetRenderTargets(1, &blurMap1->GetRtvCpuHandle(), true, nullptr);
	manager->renderManager->RenderList(manager->renderManager->postEffect, false);
}
