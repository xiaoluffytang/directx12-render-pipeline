#include "../Manager/MaterialManager.h"
#include "Component.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;
#pragma once
//绘制模型组件
class MeshRender
{
public:
	MeshRender();
	~MeshRender();

	Material* material;   //材质
	MeshGeometry* mesh;    //模型网格，这个网格数据可能存的是很多个模型合并后的网格数据
	string submeshName = "";  //网格名，这个用于找到当前模型存储的数据
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();    //这个游戏物体的贴图矩阵
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;    //填充格式

	SubmeshGeometry GetSubmesh();
};