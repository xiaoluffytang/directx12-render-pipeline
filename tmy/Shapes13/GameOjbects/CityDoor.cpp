#include "CityDoor.h"



CityDoor::CityDoor(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	needCheckCulling = true;
	int count = 1;
	Instances = new InstanceData[count];
	float dis = 2;
	float scale = 1;
	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		XMMATRIX m1 = XMMatrixTranslation(2 + dis * k, 0, 18);
		XMMATRIX m2 = XMMatrixRotationRollPitchYaw(0, 135, 0);
		XMMATRIX m3 = XMMatrixScaling(scale, scale, scale);
		XMMATRIX m = m3 * m2 * m1;
		m = XMMatrixTranspose(m);
		DirectX::XMFLOAT4X4* matrix = &MathHelper::Identity4x4();
		XMStoreFloat4x4(matrix, m);
		Instances[k].World = *matrix;
		Instances[k].MaterialIndex = meshrender->material->MatCBIndex;
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


CityDoor::~CityDoor()
{
}
