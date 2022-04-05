#pragma once
#include "../Common/Camera.h"
#include "../Compoents/GameObject.h"
#include "../Texture/Texture2DResource.h"

//渲染通道
class RenderPassBase
{
public:
	Camera* camera;    //使用哪个摄像机
	D3D12_VIEWPORT viewport;   //视口参数
	D3D12_RECT rect;    //窗口裁剪参数
	std::vector<std::vector<GameObject*>> list;    //渲染哪些物体
	Texture2DResource* renderTarget;   //渲染到哪个渲染目标上
	Texture2DResource* depthStencil;   //使用哪个深度模板缓冲区
	ID3D12PipelineState* targetPso;    //使用指定的渲染状态
	virtual ID3D12GraphicsCommandList* GetCommandList();
	virtual void BeginDraw();    //开始渲染
	virtual void DrawOneGameObject(GameObject* go);
	virtual void ClearRenderTarget(XMVECTORF32 color);    //清理渲染目标
	virtual void ClearDepthStencil(float depth, UINT8 stencil);    //清理深度模板缓冲区
	virtual void SetViewports();
	virtual void SetScissorRects();
	virtual void SetResourceState(Texture2DResource* texture, D3D12_RESOURCE_STATES state1, D3D12_RESOURCE_STATES state2);
	virtual void SetRenderTargets();
	virtual void SetTargetPso(ID3D12PipelineState* pso);
	virtual void RenderList();
};

