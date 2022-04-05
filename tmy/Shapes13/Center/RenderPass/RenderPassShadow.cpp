#include "RenderPassShadow.h"
#include "../../Engine/Manager/Manager.h"
extern Manager* manager;

void RenderPassShadow::BeginDraw()
{
	//标记镜面，将模板缓冲区中的镜面像素标记为1
	manager->commonInterface.mCommandList->OMSetStencilRef(1);
	//开始绘制镜面，将看得见的镜面模板值变为上面设置的模板值
	SetTargetPso(targetPso);
	RenderList();
}