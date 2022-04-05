#include "RenderPassDefault.h"
#include "../Manager/Manager.h"
extern Manager* manager;

void RenderPassDefaultState::BeginDraw()
{
	SetViewports();
	SetScissorRects();
	SetResourceState(renderTarget, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	ClearRenderTarget(Colors::LightSteelBlue);
	ClearDepthStencil(1.0f, 0);
	SetResourceState(depthStencil,D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	SetRenderTargets();
	SetTargetPso(targetPso);
	auto adress = camera->UpdateAndGetConstantCB(manager->heapResourceManager->mCurrFrameResourceIndex);
	manager->commonInterface.mCommandList->SetGraphicsRootConstantBufferView(2, adress);
	//设置摄像机的常量，先空着
	//设置光源的常量，也先空着
	RenderList();
}

