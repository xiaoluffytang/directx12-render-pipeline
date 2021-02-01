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

	vector<GameObject*> OpaqueList1;  //��͸���б�
	vector<GameObject*> OpaqueList2;  //��͸���б�
	vector<GameObject*> OpaqueList3;  //��͸���б�
	vector<GameObject*> OpaqueAnimatorList1;    //��͸���Ķ�����ɫ
	vector<GameObject*> TransparentList1;   //͸���б�(��ͨ͸������)
	vector<GameObject*> TransparentList2;   //͸���б�(������Ⱦ)
	vector<GameObject*> TransparentList3;   //͸���б�(ˮ����Ⱦ)
	vector<GameObject*> EnvironmentCube;   //������ӳ�价��
	vector<GameObject*> SkyBoxList;   //��պ�
	vector<GameObject*> postEffect;   //��Ļ��Ч
	vector<GameObject*> UIList;   //UI�б�

	void RenderList(vector<GameObject*> list, bool useSelfPso);
	void DrawOneGameObject(GameObject* go, bool useSelfPso);
	void DrawShadowMap();
	void DrawSceneToCubeMap();
	void DrawStroke();
};

