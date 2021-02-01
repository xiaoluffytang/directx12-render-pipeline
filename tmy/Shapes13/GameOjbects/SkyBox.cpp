#include "SkyBox.h"



SkyBox::SkyBox(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	int count = 1;
	Instances = new InstanceData[count];
	float dis = 0;
	float scale = 1000;
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, 0.0f,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f, scale, 1,
			0, 0, 0, 1.0f);
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


SkyBox::~SkyBox()
{
}
