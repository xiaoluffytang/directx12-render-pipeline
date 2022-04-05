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

	std::unordered_map<std::string, MeshGeometry*> mGeometries;   //���е�������Ϣ��ÿһ��������Դ洢һ������ģ�͵�������Ϣ

	void Init();
	void InitEnd();
	void Update(const GameTimer& gt);

	//һЩ���������¼�
	void OnMouseDown(WPARAM btnState, int x, int y);  //��갴��
	void OnMouseUp(WPARAM btnState, int x, int y);   //���̧��
	void OnMouseMove(WPARAM btnState, int x, int y);   //����ƶ�
	void OnKeyboardInput(const GameTimer& gt);   //��������

private:
	POINT mLastMousePos;     //����ϴε��λ��
	GeometryGenerator geoGen;
	int itemCount = 0;
	SkinnedModelInstance* mSkinnedModelInst;

	void BuildAllGeometry();    //���������е����е�����
	//����һ�������������Ķ������Ϣ��ͨ�����봴����
	void BuildOneGeometry(GeometryGenerator::MeshData& mesh, std::string geoName, std::string materialName, float scale, XMFLOAT3 color);
	//����һ�������������Ķ������Ϣ��ͨ��obj��ʽ��ģ�ʹ�����
	void BuildModel(string path, string textureOppPath, int fToTri, string pso);
	//�������е���Ҫ��Ⱦ����Ϸ����
	void BuildAllRenderGameObject();
	//����һ��3d����ģ��
	void BuildText3D(const string& str);
	//����һ��m3d����ģ��
	void BuildM3dModel(const string& mSkinnedModelFilename);
	//����һ��ģ�ʹ�һ��txt�ļ�
	void BuildParticleFromTxtFile(const string& filepath, const string& gemName);
	//����һЩĬ�ϵ���ͼ
	void BuildDefaultTextures();
	//����һЩĬ�ϵĲ���
	void BuildDefaultMaterials();
	//ʹ�ó������
	void useScriptLand(GeometryGenerator::MeshData& mesh);
};

