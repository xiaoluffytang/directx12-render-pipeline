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

	string name;    //�����Ϸ���������
	string tag;    //�����Ϸ����ı�ǩ
	UINT ObjCBIndex = -1;     //��ʾ�����Ϸ��������Ϸ�����������ǵڼ���
	bool needCheckCulling = false;   //�����Ϸ�����Ƿ���Ҫ��������޳�
	MeshRender* meshrender;
	InstanceData* Instances;    //��������Ϸ����Ҫ��ʵ������Ⱦ����ʹ�������������Ⱦ
	int instanceCount = 0;  //�ܹ��ж��ٸ���Ϸ����
	int renderCount = 0;   //��ǰҪʵ������Ⱦ���ٸ�����һ�����еĸ������ʵ�������ݶ�����Ⱦ������ֻ��Ⱦһ���֣�
	virtual void Update(const GameTimer& gt);
};



