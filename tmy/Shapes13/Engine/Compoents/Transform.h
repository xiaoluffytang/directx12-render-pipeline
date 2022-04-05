#include "Component.h"
#include "../Global.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment ( lib, "D3D12.lib")
#pragma once
class Transform
{
public:
	Transform();
	~Transform();

	int NumFramesDirty = gNumFrameResources;     //�Ƿ���������Ϸ�����һЩ���ݵı�ʶ���������Ҫ���£������ֵ����Ϊ gNumFrameResources

	DirectX::XMFLOAT4X4 getLocalMatrix();
	void SetPosition(float x, float y, float z);   //����λ��
	void SetRotation(float x, float y, float z);  //������ת�Ƕ�
	void SetScale(float x, float y, float z);   //��������
	vector<Transform*> children;    //���е�������
	Transform* parent;    //������
private:
	DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };   //λ��
	DirectX::XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };    //��ת��ŷ����
	DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };       //����
	DirectX::XMFLOAT3 GetPosition();   //�õ�λ��
	DirectX::XMFLOAT3 GetRotation();   //�õ��Ƕ�
	DirectX::XMFLOAT3 GetScale();    //�õ�����
};

