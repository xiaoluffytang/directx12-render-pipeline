#pragma once
#include "RenderPassBase.h"

//默认的渲染通道
class RenderPassDefaultState :public RenderPassBase
{
public:
	virtual void BeginDraw()override;
};

