#pragma once
#include "../Compoents/GameObject.h"
class Sphere : public GameObject
{
public:
	Sphere(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Sphere();
};