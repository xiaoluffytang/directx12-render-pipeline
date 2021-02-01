#pragma once
#include "../Compoents/GameObject.h"
class Plane : public GameObject
{
public:
	Plane(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Plane();
};

