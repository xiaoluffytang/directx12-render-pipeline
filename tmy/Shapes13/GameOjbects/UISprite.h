#pragma once
#include "../Compoents/GameObject.h"
class UISprite : public GameObject
{
public:
	UISprite(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~UISprite();
};

