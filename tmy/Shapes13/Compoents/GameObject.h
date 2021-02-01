#include "Transform.h"
#include "MeshRender.h"
#include "Component.h"
#include "../FrameResource.h"
#include "../../../Common/GameTimer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment ( lib, "D3D12.lib")
#pragma once
class GameObject
{
public:
	GameObject(MeshGeometry* mesh, int itemCount, string submeshName, Material* material);
	~GameObject();

	unordered_map<string, Component*> components;

	string name;    //这个游戏物体的名字
	string tag;    //这个游戏物体的标签
	UINT ObjCBIndex = -1;     //表示这个游戏物体在游戏物体数组中是第几个
	bool needCheckCulling = false;   //这个游戏物体是否需要主摄像机剔除
	MeshRender* meshrender;
	InstanceData* Instances;    //如果这个游戏物体要被实例化渲染，则使用这个数据来渲染
	int instanceCount = 0;  //总共有多少个游戏物体
	int renderCount = 0;   //当前要实例化渲染多少个（不一定所有的该物体的实例化数据都会渲染，可能只渲染一部分）
	virtual void Update(const GameTimer& gt);
};



