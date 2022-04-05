#include "GameObjectManager.h"
#include "Manager.h"
extern Manager* manager;

GameObjectManager::GameObjectManager()
{

}


GameObjectManager::~GameObjectManager()
{
}

void GameObjectManager::Init()
{

}

void GameObjectManager::Update(const GameTimer& gt)
{
	UpdateObjectCBs();
}

void GameObjectManager::UpdateObjectCBs()
{
	for (auto& gameobject : manager->gameobjectManager->gameobjectList)
	{
		gameobject->CopyData(manager->heapResourceManager->mCurrFrameResourceIndex);
	}
}
