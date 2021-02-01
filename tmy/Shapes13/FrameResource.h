#pragma once

#include "../../Common/d3dUtil.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

//每个帧资源中的物体常量结构体，可能和真的物体常量的属性不同
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};
//每个实例化的物体所需要的数据
struct InstanceData
{
	float param1;
	float param2;
	float param3;
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT MaterialIndex;
};
//每个帧资源中的材质结构体，可能和真的材质的属性不同
struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();

	UINT DiffuseMapIndex = 0;
	UINT MaterialPad0;
	UINT MaterialPad1;
	UINT MaterialPad2;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	Light DirectLights[MaxLights]; //场景中所有的平行光
	//Light PointLights[MaxLights];  //场景中所有的点光源
	//Light SpotLights[MaxLights];  //场景中所有的聚光灯
};
//顶点数据
struct Vertex
{
    DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 TangentL;
	DirectX::XMFLOAT3 Color = {1,1,1};
};

struct SkinnedConstants
{
	int aniTexIndex = 0;   //动画贴图的索引
	float firstU;   //最左边像素的u坐标
	float disU;     //左右两个像素之间的距离u
	float firstV;   //最上边像素的v坐标
	float disV;     //上线两个像素之间的距离v
	int boneCount;   //这个动画总共有多少个骨骼
	int secondFrameCount;  //一秒钟存了多少帧动画
	float wholeTimes;   //完整动画的时间
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:
    
    FrameResource(ID3D12Device* device, UINT passCount, std::vector<int> objectCounts, UINT materialCount);
  //  FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	//每个帧资源都有一个场景公共数据
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;   
	//每个帧资源里的 每个需要渲染的游戏物体 都有一个自己的实例化数据buff区
	//键表示这个游戏物体在所有物体中的下标，值表示这个游戏物体的实例化buff区
	std::vector<UploadBuffer<InstanceData>*> InstanceBuffer;
	//每个帧资源的 所有的材质数据
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialBuffer = nullptr;   //所有的材质
    //每个帧资源的围栏
    UINT64 Fence = 0;
};

