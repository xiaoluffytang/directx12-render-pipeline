#pragma once
#include "../Compoents/GameObject.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

class GameObjectManager
{
public:
	GameObjectManager();
	~GameObjectManager();
	vector<GameObject*> gameobjectList;
	void Update(const GameTimer& gt);
	void Init();
private:
	void UpdateObjectCBs();
};

