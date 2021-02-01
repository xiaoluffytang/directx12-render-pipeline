#include "Grass1.h"



Grass1::Grass1(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	needCheckCulling = true;
	int count = 100;
	Instances = new InstanceData[count];
	float dis = 1;
	float scale = 0.2f;
	int oneLineCount = 4;
	for (int k = 0; k < count; k++)
	{
		float x = 20 + dis * (k % oneLineCount);
		float z = 10 - dis * (k / oneLineCount);
		Instances[k] = InstanceData();
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, x,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f, scale, z,
			0.0f, 0, 0, 1.0f);
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


Grass1::~Grass1()
{
}
