#pragma once
#include "../Common/FrameResource.h"
#include "../Common/GameTimer.h"
#include "../Common/d3dx12.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Camera;
class HeapResourceManager
{
public:
	HeapResourceManager();
	~HeapResourceManager();

	PassConstants mShadowPassCB;    //��Ӱ��Դ��һЩ�������ݼ���

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;   //�����е�֡��Դ
	FrameResource* mCurrFrameResource = nullptr;    //��ǰ��ʹ�õ�֡��Դ
	int mCurrFrameResourceIndex = 0;    //��ǰ֡��Դ�±�

	void BuildFrameResources();     //����֡��Դ
	void UpdateMaterials(const GameTimer& gt);   //���³����еĲ�����Ϣ
	void UpdateUseNextFrameResource();   //ʹ����һ��֡��Դ
	void UpdateCommonConstantsCB(const GameTimer& gt);

	DirectX::BoundingSphere mSceneBounds;
	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

	float mLightRotationAngle = 0.0f;
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];
	void Init();
};

