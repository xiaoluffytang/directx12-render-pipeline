#include "Sphere.h"



Sphere::Sphere(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	needCheckCulling = true;
	int count = 1;
	Instances = new InstanceData[count];
	float dis = 2;
	float scale = 10.0f;
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		Instances[k].World = XMFLOAT4X4(
			scale, 0.0f, 0.0f, 0 + dis * k,
			0.0f, scale, 0.0f, 10,
			0.0f, 0.0f, scale, -25,
			0, 0, 0, 1.0f);
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


Sphere::~Sphere()
{
}
