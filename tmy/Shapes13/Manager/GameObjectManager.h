#pragma once
#include "../Compoents/GameObject.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

class GameObjectManager
{
public:
	GameObjectManager(Manager* manager);
	~GameObjectManager();
	vector<GameObject*> gameobjectList;
	Manager* manager;
};

