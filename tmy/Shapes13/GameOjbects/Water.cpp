#include "Water.h"



Water::Water(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	int count = 1;
	float dis = 2;
	float scale = 1;
	Instances = new InstanceData[count];
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, 0.0f,
			0.0f, scale, 0.0f, -0.01,
			0.0f, 0.0f, scale, 0.0f,
			0, 0, 0, 1.0f);
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


Water::~Water()
{
}
