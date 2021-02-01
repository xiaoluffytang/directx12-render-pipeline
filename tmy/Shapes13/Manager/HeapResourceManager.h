#pragma once
#include "../FrameResource.h"
#include "../../../Common/GameTimer.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class Manager;
class Camera;
class HeapResourceManager
{
public:
	HeapResourceManager(Manager* manager);
	~HeapResourceManager();

	PassConstants mMainPassCB;    //整个游戏场景中需要给shader用的一些数据集合
	PassConstants mShadowPassCB;    //阴影光源的一些常量数据集合
	ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;    //图片资源描述符堆

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;   //场景中的帧资源
	FrameResource* mCurrFrameResource = nullptr;    //当前是使用的帧资源
	int mCurrFrameResourceIndex = 0;    //当前帧资源下标
	
	Manager* manager;

	void BuildDescriptorHeaps();    //创建描述符堆
	void BuildSrvViews(UINT mCbvSrvUavDescriptorSize);   //创建图片资源的描述符
	void BuildFrameResources();     //创建帧资源
	void UpdateMainPassCB(const GameTimer& gt);    //更新场景常量信息
	void UpdateUIPassCB(const GameTimer& gt);    //更新场景常量信息
	void UpdateCubeTargetPassCB(const GameTimer& gt);    //更新场景的立方体渲染的场景信息
	
	void UpdateShadowTransform(const GameTimer& gt);   //更新场景的光源位置
	void UpdateShadowPassCB(const GameTimer& gt);   //更新场景光源信息
	void UpdateObjectCBs(const GameTimer& gt,const BoundingFrustum& mCamFrustum);    //更新场景中的游戏物体的常量信息
	void UpdateMaterials(const GameTimer& gt);   //更新场景中的材质信息
	void UpdateUseNextFrameResource();   //使用下一个帧资源

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

	int n = 1;

private:
	void UpdateCameraCB(const GameTimer& gt,Camera* camera,int index);   
};

