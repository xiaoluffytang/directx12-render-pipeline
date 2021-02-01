#pragma once
#include "../Compoents/GameObject.h"
class Specular : public GameObject
{
public:
	Specular(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Specular();
};

