#include "GameObject.h"
#include "../Manager/Manager.h"
extern Manager* manager;

GameObject::GameObject(MeshGeometry* mesh, int index, string submeshName,Material* material)
{
	meshrender = new MeshRender();
	this->meshrender->mesh = mesh;
	this->ObjCBIndex = index;
	this->meshrender->submeshName = submeshName;
	this->meshrender->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->meshrender->material = material;
}


GameObject::~GameObject()
{
}

void GameObject::CreateBuffer()
{
	if (hasCreateBuffer)
	{
		return;
	}
	hasCreateBuffer = true;
	if (instanceCount != 1)
	{
		int j = 0;
	}
	for (int i = 0; i < gNumFrameResources; i++)
	{
		auto buffer = new UploadBuffer<InstanceData>(manager->commonInterface.md3dDevice.Get(), instanceCount, false);
		InstanceBuffer.push_back(buffer);
	}
}

void GameObject::CopyData(int frameIndex)
{
	CreateBuffer();
	UploadBuffer<InstanceData>*  buffer = InstanceBuffer[frameIndex];
	renderCount = 0;
	for (int i = 0; i < instanceCount; i++)
	{
		bool needUpdate = true;

		InstanceData& data = Instances[i];
		
		if (needCheckCulling)
		{
			//XMMATRIX world = XMLoadFloat4x4(&data.World);
			//XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
			//XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);
			//BoundingFrustum localSpaceFrustum;
			//mCamFrustum.Transform(localSpaceFrustum, viewToLocal);
			//needUpdate = localSpaceFrustum.Contains(gameobject->meshrender->GetSubmesh().Bounds) != ContainmentType::DISJOINT;
		}

		if (needUpdate)
		{
			buffer->CopyData(renderCount++, data);
		}
	}
}

void GameObject::Update(const GameTimer& gt)
{

}
