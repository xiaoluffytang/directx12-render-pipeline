#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "../../../Common/MathHelper.h"
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
using Microsoft::WRL::ComPtr;
//һ��������ĳһ���ؼ�֡��Ϣ
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
	int startFrame;   //��ʼ֡
	int endFrame;     //����֡
	float percent;    //����
};


//һ�����������йؼ�֡��Ϣ
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

//һ������Ƭ�ε����й��������йؼ�֡��Ϣ
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

	//�õ���ǰ������ĳһʱ�̵����й����Ĺؼ�֡��Ϣ
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;
	std::unordered_map<std::string, AnimationClip> mAnimations;

	void SetSkinData(ID3D12GraphicsCommandList* commandList, int paramIndex);
private:
    //ÿһ�������ĸ�����
	std::vector<int> mBoneHierarchy;
	//ÿһ�������ĳ�ʼƫ��
	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;

	ID3D12Resource* frameTex;    //���������ؼ�֡��Ϣ��ͼ
	ID3D12Resource* temp;    //��ʱ��Դ

	D3D12_GPU_DESCRIPTOR_HANDLE frameTexGpuSrvHandle;

	int alignNum(int n);
	SkinnedConstants skin;
	std::unique_ptr<UploadBuffer<SkinnedConstants>> skinData = nullptr;
};
 
#endif // SKINNEDDATA_H