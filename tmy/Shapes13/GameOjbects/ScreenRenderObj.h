#pragma once
#include "../Compoents/GameObject.h"
class ScreenRenderObj : public GameObject
{
public:
	ScreenRenderObj(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~ScreenRenderObj();
};

