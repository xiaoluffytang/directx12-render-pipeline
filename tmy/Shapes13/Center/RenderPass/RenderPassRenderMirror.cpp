#include "RenderPassRenderMirror.h"
#include "../../Engine/Manager/Manager.h"
extern Manager* manager;

void RenderPassRenderMirror::BeginDraw()
{
	//��ʼ���ƾ���
	SetTargetPso(targetPso);
	RenderList();
	//��ģ�建�����еľ������ر��Ϊ1
	manager->commonInterface.mCommandList->OMSetStencilRef(0);
}