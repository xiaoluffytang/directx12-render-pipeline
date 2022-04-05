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

//骨骼
cbuffer cbPass : register(b1)
{
	int animationTexIndex;   //动画贴图的索引
	float animationFirstU;   //最左边像素的u坐标
	float animationDisU;     //左右两个像素之间的距离u
	float animationFirstV;   //最上边像素的v坐标
	float animationDisV;     //上线两个像素之间的距离v
	int animationBoneCount;   //这个动画总共有多少个骨骼
	int animationSecondFrameCount;  //一秒钟存了多少帧动画
	float animationWholeTimes;   //完整动画的时间
};

//高斯模糊参数
cbuffer cbPass : register(b2)
{
	int blurIndex;   //要模糊的纹理的索引
	float blurRadius;   //模糊半径
	float scaleX;
	float scaleY;
	int horIndex = 0;
	int vecIndex = 0;
}

//海洋渲染贴图索引
cbuffer cbPass : register(b3)
{
	int oceanDisplaceIndex;   //偏移频谱的纹理索引
	int oceanNormalIndex;     //法线频谱的纹理索引
	int oceanBuddlesIndex;    //泡沫频谱的纹理索引
}

//一些常量数据
cbuffer cbPass : register(b4)
{
	float4 gAmbientLight;
	Light DirectLights[MaxLights];
	float4x4 gShadowTransform;
	int depthStencilMapIndex;   //深度模板贴图索引
	int shadowMapIndex;   //阴影贴图索引
	int bloomMapIndex;    //bloom贴图索引
	int skullIndex;     //粒子特效的每个粒子的贴图
	float gTotalTime;
	float gDeltaTime;
}

float CalcShadowFactor(float4 shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;   //纹理坐标
	float depth = shadowPosH.z;   //NDC空间下的深度值
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
		float r = gDiffuseMap[shadowMapIndex].SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;    //第0张贴图是阴影贴图
		percentLit += r;
	}
	float p = percentLit / 9.0f;
	return p;
}

//得到一个rgb的亮度值
float getLightValue(float3 rgb)
{
	return rgb.r * 0.2125 + rgb.g * 0.7154 + rgb.b * 0.0721;
}

