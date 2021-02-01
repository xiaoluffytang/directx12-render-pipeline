#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "../../../Common/MathHelper.h"
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
using Microsoft::WRL::ComPtr;
//一个骨骼的某一个关键帧信息
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	DirectX::XMFLOAT3 Translation;
    DirectX::XMFLOAT3 Scale;
    DirectX::XMFLOAT4 RotationQuat;
};


struct ToParent
{
	int startFrame;   //开始帧
	int endFrame;     //结束帧
	float percent;    //比例
};


//一根骨骼的所有关键帧信息
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;
	XMFLOAT4 slerp(XMFLOAT4 start, XMFLOAT4 end, float percent)const;
	XMMATRIX XMMatrixAffineTransformati
	(
		XMFLOAT4 Scaling,
		XMFLOAT4 RotationQuaternion,
		GXMVECTOR Translation
	)const;
	XMMATRIX TO
	(
		XMFLOAT4 quaternion
	)const;
	std::vector<Keyframe> Keyframes; 	
};

//一个动画片段的所有骨骼的所有关键帧信息
class AnimationClip
{
public:
	float GetClipStartTime()const;
	float GetClipEndTime()const;

    void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;
	
    std::vector<BoneAnimation> BoneAnimations; 	
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<int>& boneHierarchy, 
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations, ID3D12Device* device);

	void BuildAniTex(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList,ID3D12DescriptorHeap* heap, int startIndex,int mCbvSrvUavDescriptorSize);

	//得到当前动画的某一时刻的所有骨骼的关键帧信息
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;
	std::unordered_map<std::string, AnimationClip> mAnimations;

	void SetSkinData(ID3D12GraphicsCommandList* commandList, int paramIndex);
private:
    //每一个骨骼的父骨骼
	std::vector<int> mBoneHierarchy;
	//每一个骨骼的初始偏移
	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;

	ID3D12Resource* frameTex;    //骨骼动画关键帧信息贴图
	ID3D12Resource* temp;    //临时资源

	D3D12_GPU_DESCRIPTOR_HANDLE frameTexGpuSrvHandle;

	int alignNum(int n);
	SkinnedConstants skin;
	std::unique_ptr<UploadBuffer<SkinnedConstants>> skinData = nullptr;
};
 
#endif // SKINNEDDATA_H