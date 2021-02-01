#pragma once
#include "../Compoents/GameObject.h"
class SkyBox : public GameObject
{
public:
	SkyBox(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~SkyBox();
};

