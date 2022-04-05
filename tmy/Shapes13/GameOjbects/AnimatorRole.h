#pragma once
#include "../Engine/Compoents/GameObject.h"
#include "../Engine/Animator/LoadM3d.h"
class AnimatorRole : public GameObject
{
public:
	AnimatorRole(MeshGeometry* mesh, int index, string submeshName, Material* material, std::vector<float>& timeOffsets,
		std::vector<float>& speeds);
	~AnimatorRole();
	SkinnedModelInstance* mSkinnedModelInst;
};

