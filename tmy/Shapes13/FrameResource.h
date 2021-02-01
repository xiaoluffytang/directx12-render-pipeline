#pragma once

#include "../../Common/d3dUtil.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

//ÿ��֡��Դ�е����峣���ṹ�壬���ܺ�������峣�������Բ�ͬ
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};
//ÿ��ʵ��������������Ҫ������
struct InstanceData
{
	float param1;
	float param2;
	float param3;
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT MaterialIndex;
};
//ÿ��֡��Դ�еĲ��ʽṹ�壬���ܺ���Ĳ��ʵ����Բ�ͬ
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
	Light DirectLights[MaxLights]; //���������е�ƽ�й�
	//Light PointLights[MaxLights];  //���������еĵ��Դ
	//Light SpotLights[MaxLights];  //���������еľ۹��
};
//��������
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
	int aniTexIndex = 0;   //������ͼ������
	float firstU;   //��������ص�u����
	float disU;     //������������֮��ľ���u
	float firstV;   //���ϱ����ص�v����
	float disV;     //������������֮��ľ���v
	int boneCount;   //��������ܹ��ж��ٸ�����
	int secondFrameCount;  //һ���Ӵ��˶���֡����
	float wholeTimes;   //����������ʱ��
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

	//ÿ��֡��Դ����һ��������������
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;   
	//ÿ��֡��Դ��� ÿ����Ҫ��Ⱦ����Ϸ���� ����һ���Լ���ʵ��������buff��
	//����ʾ�����Ϸ���������������е��±ֵ꣬��ʾ�����Ϸ�����ʵ����buff��
	std::vector<UploadBuffer<InstanceData>*> InstanceBuffer;
	//ÿ��֡��Դ�� ���еĲ�������
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialBuffer = nullptr;   //���еĲ���
    //ÿ��֡��Դ��Χ��
    UINT64 Fence = 0;
};

