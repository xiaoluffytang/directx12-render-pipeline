#pragma once
#include "../Common/Camera.h"
#include "../Compoents/GameObject.h"
#include "../Texture/Texture2DResource.h"

//��Ⱦͨ��
class RenderPassBase
{
public:
	Camera* camera;    //ʹ���ĸ������
	D3D12_VIEWPORT viewport;   //�ӿڲ���
	D3D12_RECT rect;    //���ڲü�����
	std::vector<std::vector<GameObject*>> list;    //��Ⱦ��Щ����
	Texture2DResource* renderTarget;   //��Ⱦ���ĸ���ȾĿ����
	Texture2DResource* depthStencil;   //ʹ���ĸ����ģ�建����
	ID3D12PipelineState* targetPso;    //ʹ��ָ������Ⱦ״̬
	virtual ID3D12GraphicsCommandList* GetCommandList();
	virtual void BeginDraw();    //��ʼ��Ⱦ
	virtual void DrawOneGameObject(GameObject* go);
	virtual void ClearRenderTarget(XMVECTORF32 color);    //������ȾĿ��
	virtual void ClearDepthStencil(float depth, UINT8 stencil);    //�������ģ�建����
	virtual void SetViewports();
	virtual void SetScissorRects();
	virtual void SetResourceState(Texture2DResource* texture, D3D12_RESOURCE_STATES state1, D3D12_RESOURCE_STATES state2);
	virtual void SetRenderTargets();
	virtual void SetTargetPso(ID3D12PipelineState* pso);
	virtual void RenderList();
};

