#include "RenderPassShadow.h"
#include "../../Engine/Manager/Manager.h"
extern Manager* manager;

void RenderPassShadow::BeginDraw()
{
	//��Ǿ��棬��ģ�建�����еľ������ر��Ϊ1
	manager->commonInterface.mCommandList->OMSetStencilRef(1);
	//��ʼ���ƾ��棬�����ü��ľ���ģ��ֵ��Ϊ�������õ�ģ��ֵ
	SetTargetPso(targetPso);
	RenderList();
}