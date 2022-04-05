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

	PassConstants mShadowPassCB;    //阴影光源的一些常量数据集合

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;   //场景中的帧资源
	FrameResource* mCurrFrameResource = nullptr;    //当前是使用的帧资源
	int mCurrFrameResourceIndex = 0;    //当前帧资源下标

	void BuildFrameResources();     //创建帧资源
	void UpdateMaterials(const GameTimer& gt);   //更新场景中的材质信息
	void UpdateUseNextFrameResource();   //使用下一个帧资源
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

