#pragma once
#include "../Common/GameTimer.h"
#include "../Common/GeometryGenerator.h"
#include "../Common/FrameResource.h"
#include "../Model3D/ModelObj.h"
#include "GameObjectManager.h"
#include "../RenderPass/RenderPassBase.h"
#include "../RenderPass/RenderPassDefault.h"
#include "../RenderPass/RenderPassRenderList.h"
#include "../../Center/RenderPass/RenderPassTabMirror.h"
#include "../../Center/RenderPass/RenderPassRenderMirror.h"

class RenderManager
{
public:
	RenderManager();
	~RenderManager();
	void Init();

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

	std::unordered_map<string, RenderPassBase*> renderPassDic;

	void RenderList(vector<GameObject*> list, bool useSelfPso);
	void DrawOneGameObject(GameObject* go, bool useSelfPso);
	void DrawSceneToCubeMap();
	void CreateAnderPass();
private:
	void CreateRenderPass1();
	void CreateRenderPass2();
	void CreateRenderPass3();
	void CreateRenderPass4();
	void CreateRenderPass5();
	void CreateRenderPass6();
	void CreateRenderPass7();
	void CreateRenderPass8();
	void CreateRenderPass9();
	void CreateRenderPass10();
	void CreateRenderPass11();
	void CreateRenderPass12();
	void CreateRenderPass13();
	void CreateRenderPass14();
};

