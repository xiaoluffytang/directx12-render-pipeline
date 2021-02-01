#include "TextureManager.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma once
struct Material
{
	std::string Name;    //材质的名字
	int MatCBIndex = -1;    //表示这个材质在所有材质中的索引是多少
	int DiffuseSrvHeapIndex = 0;    //表示这个材质所使用的漫反射的贴图的索引
	int NormalSrvHeapIndex = 0;     //表示这个材质所使用的法线贴图的索引
	//表示这个材质的属性是否变化了，如果为gNumFrameResource,则需要把这个材质的数据更新到帧缓冲区中
	int NumFramesDirty = gNumFrameResources;
	ID3D12PipelineState* pso;    //这个材质使用哪一个pso来渲染

	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };    //漫反射反照率，这个属性通常会与 漫反射反照率贴图 相乘 来使用
	DirectX::XMFLOAT3 FresnelR0 = { 0.5f, 0.5f, 0.5f };   //当一束光线垂直照射下来时反射的光亮，（一种物理材质属性）
	float Roughness = 0.25f;    //粗糙度  （一种物理材质属性）
	float Alpha = 0;   //透明度  
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
	std::unordered_map<std::string, Material*> mMaterials;    //所有的材质
	Manager* manager;
};


