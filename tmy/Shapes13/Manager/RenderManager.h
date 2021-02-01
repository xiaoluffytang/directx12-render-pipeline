#pragma once
#include "../../../Common/GameTimer.h"
#include "../../../Common/GeometryGenerator.h"
#include "../FrameResource.h"
#include "../ModelObj.h"
#include "GameObjectManager.h"
class Manager;
class RenderManager
{
public:
	RenderManager(Manager* manager);
	~RenderManager();

	Manager* manager;

	vector<GameObject*> OpaqueList1;  //不透明列表
	vector<GameObject*> OpaqueList2;  //不透明列表
	vector<GameObject*> OpaqueList3;  //不透明列表
	vector<GameObject*> OpaqueAnimatorList1;    //不透明的动画角色
	vector<GameObject*> TransparentList1;   //透明列表(普通透明物体)
	vector<GameObject*> TransparentList2;   //透明列表(镜面渲染)
	vector<GameObject*> TransparentList3;   //透明列表(水的渲染)
	vector<GameObject*> EnvironmentCube;   //立方体映射环境
	vector<GameObject*> SkyBoxList;   //天空盒
	vector<GameObject*> postEffect;   //屏幕特效
	vector<GameObject*> UIList;   //UI列表

	void RenderList(vector<GameObject*> list, bool useSelfPso);
	void DrawOneGameObject(GameObject* go, bool useSelfPso);
	void DrawShadowMap();
	void DrawSceneToCubeMap();
	void DrawStroke();
};

