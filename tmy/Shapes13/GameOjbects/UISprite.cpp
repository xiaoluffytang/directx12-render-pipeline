#include "UISprite.h"



UISprite::UISprite(MeshGeometry* mesh, int objIndex, string submeshName, Material* material) :GameObject(mesh, objIndex, submeshName, material)
{
	vector<XMFLOAT3> scales = { XMFLOAT3(100,50,1),XMFLOAT3(80,80,1) ,XMFLOAT3(80,80,1), XMFLOAT3(80,80,1),XMFLOAT3(250,250,1),XMFLOAT3(256,64,1),XMFLOAT3(256,256,1) };
	vector<XMFLOAT3> rotations = { XMFLOAT3(0,0,0),XMFLOAT3(0,0,0) ,XMFLOAT3(0,0,0) ,XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0),XMFLOAT3(0,0,0) };
	vector<XMFLOAT3> positions = { XMFLOAT3(-400,150,0),XMFLOAT3(-400,50,0) ,XMFLOAT3(-400,-50,0),XMFLOAT3(-400,-150,0),XMFLOAT3(-200,0,0),XMFLOAT3(150,150,0),XMFLOAT3(-200,-250,0) };
	vector<int> materialIndexs = { 4,5,6,7,8,15,17};
	int count = scales.size();
	Instances = new InstanceData[count];
	XMFLOAT3 scale;
	XMFLOAT3 rotation;
	XMFLOAT3 position;

	for (int k = 0; k < count; k++)
	{
		Instances[k] = InstanceData();
		scale = scales[k];
		rotation = rotations[k];
		position = positions[k];

		XMMATRIX m1 = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX m2 = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		XMMATRIX m3 = XMMatrixScaling(scale.x, -scale.y, scale.z);
		XMMATRIX m = m3 * m2 * m1;
		m = XMMatrixTranspose(m);
		DirectX::XMFLOAT4X4* matrix = &MathHelper::Identity4x4();
		XMStoreFloat4x4(matrix, m);
		
		Instances[k].World = *matrix;
		Instances[k].MaterialIndex = materialIndexs[k];
		XMStoreFloat4x4(&Instances[k].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	}
	instanceCount = count;
}


UISprite::~UISprite()
{
}
