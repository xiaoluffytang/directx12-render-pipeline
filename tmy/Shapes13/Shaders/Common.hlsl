#include "LightingUtil.hlsl"

struct InstanceData
{
	float     param1;
	float     param2;
	float     param3;
	float4x4 World;
	float4x4 TexTransform;
	uint     MaterialIndex;
};

struct MaterialData
{
	float4   DiffuseAlbedo;
	float3   FresnelR0;
	float    Roughness;
	float4x4 MatTransform;
	uint     DiffuseMapIndex;
	uint     MatPad0;
	uint     MatPad1;
	uint     MatPad2;
};

Texture2D gDiffuseMap[1] : register(t0);
TextureCube cubeTexture[1] : register(t2);

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

cbuffer cbPass : register(b0)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float cbPerObjectPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
};

//����
cbuffer cbPass : register(b1)
{
	int animationTexIndex;   //������ͼ������
	float animationFirstU;   //��������ص�u����
	float animationDisU;     //������������֮��ľ���u
	float animationFirstV;   //���ϱ����ص�v����
	float animationDisV;     //������������֮��ľ���v
	int animationBoneCount;   //��������ܹ��ж��ٸ�����
	int animationSecondFrameCount;  //һ���Ӵ��˶���֡����
	float animationWholeTimes;   //����������ʱ��
};

//��˹ģ������
cbuffer cbPass : register(b2)
{
	int blurIndex;   //Ҫģ�������������
	float blurRadius;   //ģ���뾶
	float scaleX;
	float scaleY;
	int horIndex = 0;
	int vecIndex = 0;
}

//������Ⱦ��ͼ����
cbuffer cbPass : register(b3)
{
	int oceanDisplaceIndex;   //ƫ��Ƶ�׵���������
	int oceanNormalIndex;     //����Ƶ�׵���������
	int oceanBuddlesIndex;    //��ĭƵ�׵���������
}

//һЩ��������
cbuffer cbPass : register(b4)
{
	float4 gAmbientLight;
	Light DirectLights[MaxLights];
	float4x4 gShadowTransform;
	int depthStencilMapIndex;   //���ģ����ͼ����
	int shadowMapIndex;   //��Ӱ��ͼ����
	int bloomMapIndex;    //bloom��ͼ����
	int skullIndex;     //������Ч��ÿ�����ӵ���ͼ
	float gTotalTime;
	float gDeltaTime;
}

float CalcShadowFactor(float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;   //��������
	float depth = shadowPosH.z;   //NDC�ռ��µ����ֵ
	uint width, height, numMips;
	gDiffuseMap[shadowMapIndex].GetDimensions(0, width, height, numMips);
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};
	for (int i = 0; i < 9; ++i)
	{
		float r = gDiffuseMap[shadowMapIndex].SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;    //��0����ͼ����Ӱ��ͼ
		percentLit += r;
	}
	float p = percentLit / 9.0f;
	return p;
}

//�õ�һ��rgb������ֵ
float getLightValue(float3 rgb)
{
	return rgb.r * 0.2125 + rgb.g * 0.7154 + rgb.b * 0.0721;
}

