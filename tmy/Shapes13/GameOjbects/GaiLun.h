#pragma once
#include "../Compoents/GameObject.h"
class GaiLun : public GameObject
{
public:
	GaiLun(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~GaiLun();
};

