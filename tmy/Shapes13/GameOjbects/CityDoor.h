#pragma once
#include "../Compoents/GameObject.h"
class CityDoor : public GameObject
{
public:
	CityDoor(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~CityDoor();
};

