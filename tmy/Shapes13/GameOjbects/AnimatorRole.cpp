#include "AnimatorRole.h"



AnimatorRole::AnimatorRole(MeshGeometry* mesh, int objIndex, string submeshName, Material* material,std::vector<float>& timeOffsets,
	std::vector<float>& speeds) :GameObject(mesh, objIndex, submeshName, material)
{
	needCheckCulling = true;
	int count = timeOffsets.size();
	Instances = new InstanceData[count];
	float dis = 3;
	float scale = 0.05f;
	int oneLineCount = 10;
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		float x = -15 + dis * (k % oneLineCount);
		float z = -15 + dis * (k / oneLineCount);
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, x,
			0.0f, scale, 0.0f, 0,
			0.0f, 0.0f, scale, z,
			0,0, 0, 1.0f);
		Instances[k].param1 = timeOffsets[k];   //时间偏移
		Instances[k].param2 = 0;   //动画索引
		Instances[k].param3 = speeds[k];   //动画播放速度
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


AnimatorRole::~AnimatorRole()
{
}
