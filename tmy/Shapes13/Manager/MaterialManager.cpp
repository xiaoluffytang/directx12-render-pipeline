#include "MaterialManager.h"



MaterialManager::MaterialManager(Manager* manager)
{
	this->manager = manager;
}


MaterialManager::~MaterialManager()
{
}

Material* MaterialManager::CreateMaterial(string name)
{
	auto m = new Material();
	m->Name = name;
	string n = name;
	mMaterials[name] = m;
	return m;
}

Material* MaterialManager::GetMaterial(string name,bool returnDefault)
{
	if (returnDefault)
	{
		return mMaterials.count(name) > 0 ? mMaterials[name] : mMaterials["plane"];
	}
	else
	{
		return mMaterials[name];
	}
}
