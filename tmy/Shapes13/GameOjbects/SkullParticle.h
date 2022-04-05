#pragma once
#include "../Engine/Compoents/GameObject.h"
class SkullParticle : public GameObject
{
public:
	SkullParticle(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~SkullParticle();
};

