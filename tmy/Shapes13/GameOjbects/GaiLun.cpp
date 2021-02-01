#include "GaiLun.h"



GaiLun::GaiLun(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	needCheckCulling = true;
	int count = 1;
	Instances = new InstanceData[count];
	float dis = 2;
	int index = 0;
	float scale = 0.02f;
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, 0 + dis * k,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f, scale, -18,
			0, 0, 0, 1.0f);
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


GaiLun::~GaiLun()
{
}
