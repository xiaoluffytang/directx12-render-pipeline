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

