#pragma once
#include "../../Engine/RenderPipeline/RenderPipeline.h"
class MyForwardRenderPipeline:public RenderPipeline
{
public:
	MyForwardRenderPipeline();
	~MyForwardRenderPipeline();
	void Init();
	void Update();
	void Draw();
};

