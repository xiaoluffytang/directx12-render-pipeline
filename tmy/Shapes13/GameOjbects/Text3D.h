#pragma once
#include "../Engine/Compoents/GameObject.h"
class Text3D : public GameObject
{
public:
	Text3D(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~Text3D();
};

