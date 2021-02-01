#include "GameObject.h"

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

void GameObject::Update(const GameTimer& gt)
{

}
