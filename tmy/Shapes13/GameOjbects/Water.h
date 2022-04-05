#pragma once
#include "../Engine/Compoents/GameObject.h"
class Water : public GameObject
{
public:
	Water(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Water();
};

