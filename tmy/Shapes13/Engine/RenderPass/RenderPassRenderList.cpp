#include "RenderPassRenderList.h"
void RenderPassRenderList::BeginDraw()
{
	SetTargetPso(targetPso);
	RenderList();
}