#include "SkinnedData.h"
#include "../Manager/Manager.h"
using namespace DirectX;

extern Manager* manager;

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}
 
float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

//插值得到当前时刻的这个骨骼的关键帧信息，此关键帧信息是相对于父骨骼的
void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	if( t <= Keyframes.front().TimePos )//未开始算刚开始
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if( t >= Keyframes.back().TimePos )   //已结束算刚结束
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for(UINT i = 0; i < Keyframes.size()-1; ++i)
		{
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )    //找到当前时刻是在哪两个关键帧之间
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);   //缩放插值
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);   //位移插值
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);   //旋转插值

				XMFLOAT4 m;
				XMStoreFloat4(&m, Q);
				XMFLOAT4 my = slerp(Keyframes[i].RotationQuat, Keyframes[i + 1].RotationQuat,lerpPercent);
				Q = XMLoadFloat4(&my);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMFLOAT4 sF3;
				XMStoreFloat4(&sF3, S);
				XMFLOAT4 qF3;
				XMStoreFloat4(&qF3, Q);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformati(sF3, qF3, P));

				break;
			}
		}
	}
}

//选择缩放位移 转矩阵
XMMATRIX BoneAnimation::XMMatrixAffineTransformati
(
	XMFLOAT4 Scaling,
	XMFLOAT4 RotationQuaternion,
	GXMVECTOR Translation
)const
{
	XMMATRIX MScaling = XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);

	XMMATRIX MRotation = TO(RotationQuaternion);
	XMVECTOR VTranslation = XMVectorSelect(g_XMSelect1110.v, Translation, g_XMSelect1110.v);
	
	XMMATRIX M;
	M = MScaling;
	M = XMMatrixMultiply(M, MRotation);
	M.r[3] = XMVectorAdd(M.r[3], VTranslation);
	return M;
}

//4元数转矩阵测试
XMMATRIX BoneAnimation::TO(XMFLOAT4 quaternion)const 
{
	float xx = quaternion.x * quaternion.x;
	float yy = quaternion.y * quaternion.y;
	float zz = quaternion.z * quaternion.z;
	float xy = quaternion.x * quaternion.y;
	float wz = quaternion.w * quaternion.z;
	float wy = quaternion.w * quaternion.y;
	float xz = quaternion.x * quaternion.z;
	float yz = quaternion.y * quaternion.z;
	float wx = quaternion.w * quaternion.x;
	
	float r00 = 1 - 2 * (yy + zz);
	float r01 = 2 * (xy - wz);
	float r02 = 2 * (wy + xz);
	float r03 = 0;
	
	float r10 = 2 * (xy + wz);
	float r11 = 1 - 2 * (xx + zz);
	float r12 = 2 * (yz - wx);
	float r13 = 0;
	
	float r20 = 2 * (xz - wy);//不是 xy - wy
	float r21 = 2 * (yz + wx);
	float r22 = 1 - 2 * (xx + yy);
	float r23 = 0;
	
	float r30 = 0;
	float r31 = 0;
	float r32 = 0;
	float r33 = 1;
	
	//return XMMATRIX(r00, r01, r02, r03,
	//					r10, r11, r12, r13,
	//					r20, r21, r22, r23,
	//					r30, r31, r32, r33);
	return XMMATRIX(r00, r10, r20, r30,
		r01, r11, r21, r31,
		r02, r12, r22, r32,
		r03, r13, r23, r33);
}

//四元数球面插值
XMFLOAT4 BoneAnimation::slerp(XMFLOAT4 starting, XMFLOAT4 ending, float t)const
{
	XMFLOAT4 result;
	float cosa = starting.x * ending.x + starting.y * ending.y +
		starting.z * ending.z + starting.w * ending.w;
	if (cosa < 0.0f) {
		ending.x = -ending.x;
		ending.y = -ending.y;
		ending.z = -ending.z;
		ending.w = -ending.w;
		cosa = -cosa;
	}
	float k0, k1;
	if (cosa > 0.9999f) {
		k0 = 1.0f - t;
		k1 = t;
	}
	else {
		float sina = sqrt(1.0f - cosa * cosa);
		float a = atan2(sina, cosa);
		float invSina = 1.0f / sina;
		k0 = sin((1.0f - t)*a) * invSina;
		k1 = sin(t*a) * invSina;
	}
	result.x = starting.x * k0 + ending.x * k1;
	result.y = starting.y * k0 + ending.y * k1;
	result.z = starting.z * k0 + ending.z * k1;
	result.w = starting.w * k0 + ending.w * k1;
	return result;
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = MathHelper::Infinity;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

//计算该动画的所有骨骼在某一时刻相对于父骨骼的矩阵
void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, 
		              std::vector<XMFLOAT4X4>& boneOffsets,
		              std::unordered_map<std::string, AnimationClip>& animations, ID3D12Device* device)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
	skinData = std::make_unique<UploadBuffer<SkinnedConstants>>(device, 1, true);
}
 
void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,  std::vector<XMFLOAT4X4>& finalTransforms)const
{
	UINT numBones = mBoneOffsets.size();
	std::vector<XMFLOAT4X4> toParentTransforms(numBones);
	//计算该动画的所有骨骼相对于父骨骼的矩阵
	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);
	std::vector<XMFLOAT4X4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];
	//计算所有骨骼相对于根骨骼的矩阵
	for(UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}
	//计算所有骨骼的最终矩阵
	for(UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
        XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
	}
}

void SkinnedData::BuildAniTex()
{
	if (mAnimations.size() <= 0)
	{
		return;
	}
	std::vector<XMFLOAT4X4> finalTransforms;
	finalTransforms.resize(BoneCount());

	//计算资源大小
	float speed = 0.5f;   //动画最慢播放速度
	int oneSecondKeepFrame = 60;   //一秒钟的动画保存为多少帧
	int animationCount = mAnimations.size();
	float maxEndTime = 0;   //最慢
	int boneNum = BoneCount();
	int height = boneNum * animationCount;
	int animationIndex = 0;
	for (auto& animation : mAnimations)   //所有的动画
	{
		AnimationClip& clip = animation.second;    //一个动画片段
		float endTime = clip.GetClipEndTime();
		maxEndTime = endTime > maxEndTime ? endTime : maxEndTime;
		skin.wholeTimes = endTime / speed;
	}
	//下面乘以4表示  4个颜色（每个颜色有4个分量） 来表示一个 4X4的矩阵
	int width = ceil(maxEndTime / speed * oneSecondKeepFrame * 4);
	width = alignNum(width);
	height = alignNum(height);
	//创建关键帧资源
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	texture = new Texture2DResource();
	texture->LoadTextureByDesc(&texDesc, nullptr);

	ThrowIfFailed(manager->commonInterface.md3dDevice.Get()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(width * height * 16),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&temp)));
	//设置关键帧资源内容
	float* _pTexBuf = new float[width*height * 4];
	memset(_pTexBuf, 255, width*height * 4);
	int animatorIndex = 0;
	float oneKeyTime = 1.0f / oneSecondKeepFrame * speed;
	for (auto& animation : mAnimations)   //所有的动画
	{
		float endTime = animation.second.GetClipEndTime();
		int aniStartIndex = animatorIndex * width * 4;
		int keyFrame = 0;
		for (float t = 0; t <= endTime; t += oneKeyTime, keyFrame++)
		{
			GetFinalTransforms(animation.first, t, finalTransforms);
			for (int i = 0; i < finalTransforms.size(); i++)
			{
				XMFLOAT4X4& m = finalTransforms[i];
				int startIndex = aniStartIndex + i * width * 4 + keyFrame * 16;
				_pTexBuf[startIndex + 0] = m._11;
				_pTexBuf[startIndex + 1] = m._21;
				_pTexBuf[startIndex + 2] = m._31;
				_pTexBuf[startIndex + 3] = m._41;
				_pTexBuf[startIndex + 4] = m._12;
				_pTexBuf[startIndex + 5] = m._22;
				_pTexBuf[startIndex + 6] = m._32;
				_pTexBuf[startIndex + 7] = m._42;
				_pTexBuf[startIndex + 8] = m._13;
				_pTexBuf[startIndex + 9] = m._23;
				_pTexBuf[startIndex + 10] = m._33;
				_pTexBuf[startIndex + 11] = m._43;
				_pTexBuf[startIndex + 12] = m._14;
				_pTexBuf[startIndex + 13] = m._24;
				_pTexBuf[startIndex + 14] = m._34;
				_pTexBuf[startIndex + 15] = m._44;
			}
		}
	}
	D3D12_SUBRESOURCE_DATA fontTextureData = {};
	fontTextureData.pData = &_pTexBuf[0];
	fontTextureData.RowPitch = width *16;
	fontTextureData.SlicePitch = width * height *16;
	UpdateSubresources(manager->commonInterface.mCommandList.Get(), texture->Resource.Get(), temp, 0, 0, 1, &fontTextureData);
	manager->commonInterface.mCommandList.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));

	skin.aniTexIndex = texture->srvIndex;
	skin.disU = 1.0f / width;
	skin.disV = 1.0f / height;
	skin.firstU = skin.disU / 2.0f;
	skin.firstV = skin.disV / 2.0f;
	skin.boneCount = boneNum;
	skin.secondFrameCount = oneSecondKeepFrame;
	skinData->CopyData(0, skin);

	finalTransforms.clear();
	finalTransforms.shrink_to_fit();
	delete[] _pTexBuf;
}

//对齐为2的n次方
int SkinnedData::alignNum(int n)
{
	int i = 1;
	int temp = n;
	while (n >>= 1)
	{
		i <<= 1;
	}
	return (i < temp) ? i << 1 : i;
}

void SkinnedData::SetSkinData(ID3D12GraphicsCommandList* command,int paramIndex)
{
	if (skinData == nullptr)
	{
		return;
	}
	command->SetGraphicsRootConstantBufferView(paramIndex, skinData->Resource()->GetGPUVirtualAddress());
}