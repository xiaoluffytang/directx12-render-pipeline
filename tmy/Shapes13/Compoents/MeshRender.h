#include "../Manager/MaterialManager.h"
#include "Component.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;
#pragma once
//����ģ�����
class MeshRender
{
public:
	MeshRender();
	~MeshRender();

	Material* material;   //����
	MeshGeometry* mesh;    //ģ����������������ݿ��ܴ���Ǻܶ��ģ�ͺϲ������������
	string submeshName = "";  //����������������ҵ���ǰģ�ʹ洢������
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();    //�����Ϸ�������ͼ����
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;    //����ʽ

	SubmeshGeometry GetSubmesh();
};