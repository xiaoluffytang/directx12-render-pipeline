#include "MeshRender.h"



MeshRender::MeshRender()
{
}


MeshRender::~MeshRender()
{
}

SubmeshGeometry MeshRender::GetSubmesh()
{
	return mesh->DrawArgs[submeshName];
}
