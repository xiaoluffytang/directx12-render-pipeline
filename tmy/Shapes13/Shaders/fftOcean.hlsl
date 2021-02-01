//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
#ifdef SKINNED
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
#endif
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION1;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
	vout.TexC = vin.TexC;
	float4x4 world = instData.World;    //得到实例化的这个物体的世界矩阵
	vin.PosL += gDiffuseMap[oceanDisplaceIndex].SampleLevel(gsamLinearWrap, vin.TexC, 0.0f).rgb;    // SampleLevel
	vin.PosL.y -= 20;

	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //局部坐标和世界坐标相乘得到世界坐标
	vout.PosW = posW.xyz;    //将世界坐标赋值给输出结构体
	vout.PosH = mul(posW, gViewProj);   //将世界坐标转成裁剪空间的坐标

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float3 normal = gDiffuseMap[oceanNormalIndex].Sample(gsamLinearWrap, pin.TexC).rgb;
	float bubbles = gDiffuseMap[oceanBuddlesIndex].Sample(gsamLinearWrap, pin.TexC).r;

	float3 lightDir = -normalize(DirectLights[0].Direction);    //光源方向
	float3 v = gEyePosW - pin.PosW;
	float3 viewDir = normalize(v);   //视线方向
	float3 reflectDir = reflect(-viewDir, normal);
	//reflectDir *= sign(reflectDir.y);

	half3 sky = float3(0.2,0.4,0.6);    //天空颜色
	float _FresnelScale = 0.02;  //菲涅尔
	float fresnel = saturate(_FresnelScale + (1 - _FresnelScale) * pow(1 - dot(normal, viewDir), 5));

	half facing = saturate(dot(viewDir, normal));
	float3 _OceanColorShallow = float3(0.35, 0.55, 0.65);
	float3 _OceanColorDeep = float3(0.15, 0.25, 0.30);
	float3 oceanColor = lerp(_OceanColorShallow, _OceanColorDeep, facing);

	float3 ambient = gAmbientLight.rgb * 0.2;     //环境光
	float3 _BubblesColor = float3(1,1,1);  //泡沫颜色
	float3 lightColor = DirectLights[0].Strength.rgb;
	float3 bubblesDiffuse = _BubblesColor.rbg * lightColor * saturate(dot(lightDir, normal));
	//海洋颜色
	float3 oceanDiffuse = oceanColor * lightColor * saturate(dot(lightDir, normal));
	float3 halfDir = normalize(lightDir + viewDir);
	float3 _Specular = float3(0.4, 0.4, 0.4);
	float _Gloss = 256;
	float3 specular = lightColor * _Specular.rgb * pow(max(0, dot(normal, halfDir)), _Gloss);

	float3 diffuse = lerp(oceanDiffuse, bubblesDiffuse, bubbles);

	float3 col = ambient + lerp(diffuse, sky, fresnel) + specular;

	return float4(col, 1);
}


