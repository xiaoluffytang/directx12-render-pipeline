#pragma once
#include "../Compoents/GameObject.h"
class ScriptLand : public GameObject
{
public:
	ScriptLand(MeshGeometry* mesh, int index, string submeshName, Material* material);
	~ScriptLand();
};

