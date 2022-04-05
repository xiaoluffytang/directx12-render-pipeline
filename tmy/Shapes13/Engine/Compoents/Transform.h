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

	int NumFramesDirty = gNumFrameResources;     //是否更新这个游戏物体的一些数据的标识符，如果需要更新，则将这个值设置为 gNumFrameResources

	DirectX::XMFLOAT4X4 getLocalMatrix();
	void SetPosition(float x, float y, float z);   //设置位置
	void SetRotation(float x, float y, float z);  //设置旋转角度
	void SetScale(float x, float y, float z);   //设置缩放
	vector<Transform*> children;    //所有的子物体
	Transform* parent;    //父物体
private:
	DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };   //位置
	DirectX::XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };    //旋转的欧拉角
	DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };       //缩放
	DirectX::XMFLOAT3 GetPosition();   //得到位置
	DirectX::XMFLOAT3 GetRotation();   //得到角度
	DirectX::XMFLOAT3 GetScale();    //得到缩放
};

