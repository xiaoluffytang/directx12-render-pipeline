#include "Transform.h"



Transform::Transform()
{
}


Transform::~Transform()
{
}

//得到这个transform的世界矩阵
DirectX::XMFLOAT4X4 Transform::getLocalMatrix()
{
	XMMATRIX m1 = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX m2 = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX m3 = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX m = m3 * m2 * m1;
	DirectX::XMFLOAT4X4* matrix = &MathHelper::Identity4x4();
	XMStoreFloat4x4(matrix, m);
	return *matrix;
}

void Transform::SetPosition(float x,float y,float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Transform::SetRotation(float x, float y, float z)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

