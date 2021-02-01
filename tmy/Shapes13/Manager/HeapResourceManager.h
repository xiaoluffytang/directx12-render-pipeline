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

	PassConstants mMainPassCB;    //������Ϸ��������Ҫ��shader�õ�һЩ���ݼ���
	PassConstants mShadowPassCB;    //��Ӱ��Դ��һЩ�������ݼ���
	ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;    //ͼƬ��Դ��������

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;   //�����е�֡��Դ
	FrameResource* mCurrFrameResource = nullptr;    //��ǰ��ʹ�õ�֡��Դ
	int mCurrFrameResourceIndex = 0;    //��ǰ֡��Դ�±�
	
	Manager* manager;

	void BuildDescriptorHeaps();    //������������
	void BuildSrvViews(UINT mCbvSrvUavDescriptorSize);   //����ͼƬ��Դ��������
	void BuildFrameResources();     //����֡��Դ
	void UpdateMainPassCB(const GameTimer& gt);    //���³���������Ϣ
	void UpdateUIPassCB(const GameTimer& gt);    //���³���������Ϣ
	void UpdateCubeTargetPassCB(const GameTimer& gt);    //���³�������������Ⱦ�ĳ�����Ϣ
	
	void UpdateShadowTransform(const GameTimer& gt);   //���³����Ĺ�Դλ��
	void UpdateShadowPassCB(const GameTimer& gt);   //���³�����Դ��Ϣ
	void UpdateObjectCBs(const GameTimer& gt,const BoundingFrustum& mCamFrustum);    //���³����е���Ϸ����ĳ�����Ϣ
	void UpdateMaterials(const GameTimer& gt);   //���³����еĲ�����Ϣ
	void UpdateUseNextFrameResource();   //ʹ����һ��֡��Դ

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

