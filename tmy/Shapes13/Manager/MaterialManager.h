#include "TextureManager.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma once
struct Material
{
	std::string Name;    //���ʵ�����
	int MatCBIndex = -1;    //��ʾ������������в����е������Ƕ���
	int DiffuseSrvHeapIndex = 0;    //��ʾ���������ʹ�õ����������ͼ������
	int NormalSrvHeapIndex = 0;     //��ʾ���������ʹ�õķ�����ͼ������
	//��ʾ������ʵ������Ƿ�仯�ˣ����ΪgNumFrameResource,����Ҫ��������ʵ����ݸ��µ�֡��������
	int NumFramesDirty = gNumFrameResources;
	ID3D12PipelineState* pso;    //�������ʹ����һ��pso����Ⱦ

	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };    //�����䷴���ʣ��������ͨ������ �����䷴������ͼ ��� ��ʹ��
	DirectX::XMFLOAT3 FresnelR0 = { 0.5f, 0.5f, 0.5f };   //��һ�����ߴ�ֱ��������ʱ����Ĺ�������һ������������ԣ�
	float Roughness = 0.25f;    //�ֲڶ�  ��һ������������ԣ�
	float Alpha = 0;   //͸����  
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};
class Manager;
class MaterialManager
{
public:
	MaterialManager(Manager* manager);
	~MaterialManager();

	Material* CreateMaterial(string name);
	Material* GetMaterial(string name, bool returnDefault);
	std::unordered_map<std::string, Material*> mMaterials;    //���еĲ���
	Manager* manager;
};


