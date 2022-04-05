#pragma once
#include "../Common/GameTimer.h"
#include "../Common/GeometryGenerator.h"
#include "../Common/FrameResource.h"
#include "../Model3D/ModelObj.h"

#include "../Animator/LoadM3d.h"
#include "../../GameOjbects/CityDoor.h"
#include "../../GameOjbects/Plane.h"
#include "../../GameOjbects/GaiLun.h"
#include "../../GameOjbects/Grass1.h"
#include "../../GameOjbects/Specular.h"
#include "../../GameOjbects/SkyBox.h"
#include "../../GameOjbects/ScreenRenderObj.h"
#include "../../GameOjbects/UISprite.h"
#include "../../GameOjbects/UIText.h"
#include "../font3D/Font3D.h"
#include "../../GameOjbects/Text3D.h"
#include "../../GameOjbects/AnimatorRole.h"
#include "../../GameOjbects/Sphere.h"
#include "../../GameOjbects/Water.h"
#include "../../GameOjbects/SkullParticle.h"
#include "../../GameOjbects/ScriptLand.h"

class Manager;
class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	std::unordered_map<std::string, MeshGeometry*> mGeometries;   //所有的网格信息，每一个网格可以存储一个或多个模型的网格信息

	void Init();
	void InitEnd();
	void Update(const GameTimer& gt);

	//一些外设输入事件
	void OnMouseDown(WPARAM btnState, int x, int y);  //鼠标按下
	void OnMouseUp(WPARAM btnState, int x, int y);   //鼠标抬起
	void OnMouseMove(WPARAM btnState, int x, int y);   //鼠标移动
	void OnKeyboardInput(const GameTimer& gt);   //键盘输入

private:
	POINT mLastMousePos;     //鼠标上次点击位置
	GeometryGenerator geoGen;
	int itemCount = 0;
	SkinnedModelInstance* mSkinnedModelInst;

	void BuildAllGeometry();    //创建场景中的所有的网格
	//创建一个网格，这个网格的顶点等信息是通过代码创建的
	void BuildOneGeometry(GeometryGenerator::MeshData& mesh, std::string geoName, std::string materialName, float scale, XMFLOAT3 color);
	//创建一个网格，这个网格的顶点等信息是通过obj格式的模型创建的
	void BuildModel(string path, string textureOppPath, int fToTri, string pso);
	//创建所有的需要渲染的游戏物体
	void BuildAllRenderGameObject();
	//创建一个3d字体模型
	void BuildText3D(const string& str);
	//创建一个m3d动画模型
	void BuildM3dModel(const string& mSkinnedModelFilename);
	//创建一个模型从一个txt文件
	void BuildParticleFromTxtFile(const string& filepath, const string& gemName);
	//创建一些默认的贴图
	void BuildDefaultTextures();
	//创建一些默认的材质
	void BuildDefaultMaterials();
	//使用程序地形
	void useScriptLand(GeometryGenerator::MeshData& mesh);
};

