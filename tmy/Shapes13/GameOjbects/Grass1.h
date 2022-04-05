#pragma once
#include "../Engine/Compoents/GameObject.h"
class Grass1 : public GameObject
{
public:
	Grass1(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Grass1();
};

