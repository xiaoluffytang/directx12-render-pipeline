#include "RenderPassRenderMirror.h"
#include "../../Engine/Manager/Manager.h"
extern Manager* manager;

void RenderPassRenderMirror::BeginDraw()
{
	//开始绘制镜面
	SetTargetPso(targetPso);
	RenderList();
	//将模板缓冲区中的镜面像素标记为1
	manager->commonInterface.mCommandList->OMSetStencilRef(0);
}