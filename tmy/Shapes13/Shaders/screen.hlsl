#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION0;
	float2 TexC    : TEXCOORD;
	nointerpolation uint MatIndex  : MATINDEX;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
	if (vin.PosL.x > -0.5 && vin.PosL.x < 0.5)
	{
		vin.PosL.z = 0;
	}
	vout.PosH = float4(vin.PosL.x, vin.PosL.z, 0, 1);
	vout.TexC = float2(vin.TexC.x, vin.TexC.y);
	vout.PosW = float3(vin.PosL.x, vin.PosL.z, 0);
	vout.MatIndex = instData.MaterialIndex;   //得到这个实例化物体使用的材质索引，并赋值给输出结构体
	return vout;
}

//普通的
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);
	//renderTex = float4(0.5,0,0,1);
	return renderTex;
}

//雾效
float4 FogPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //后台缓冲区的颜色
	float4 depthStenicTex = gDiffuseMap[depthStencilMapIndex].Sample(gsamLinearWrap, pin.TexC);    //深度值
	float d = depthStenicTex.r;
	float z = 1 / (-(gFarZ - gNearZ) * d + gFarZ);
	float4 fogColor = float4(1, 0.9, 0.9, 1);
	float start = 0.001f;
	float end = 0.05f;
	if (z > end)
	{
		return fogColor;
	}
	else if (z > start)
	{
		return lerp(renderTex, fogColor, (z - start) / (end - start));
	}
	else
	{
		return renderTex;
	}
	
	return renderTex;
}

//调整屏幕亮度，饱和度和对比度
float4 BriSatConPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);

	float BrightNess = 2.0f;   //亮度（一般是0-3）
	float Saturation = 0.50f;   //饱和度（一般是0-3）
	float Contrast = 2.0f;     //对比度（一般是0-3）

	float3 finColor = renderTex.xyz * BrightNess;    //颜色*亮度
	//应用饱和度
	float s = 0.2125f * renderTex.r + 0.7154f * renderTex.g + 0.0721f * renderTex.b;
	finColor = lerp(float3(s, s, s), finColor, Saturation);
	//应用对比度
	finColor = lerp(float3(0.5f, 0.5f, 0.5f), finColor, Contrast);

	return float4(finColor,1.0f);
}

//Bloom特效（让亮的地方更亮，并且让亮的地方的周围也更亮）
float4 GetBloomPS(VertexOut pin):SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //后台缓冲区的颜色
	float light = getLightValue(renderTex.rgb);
	light = clamp(light - 0.4, 0, 0.2);
	return light * renderTex;
}

//Bloom特效（让亮的地方更亮，并且让亮的地方的周围也更亮）
float4 AddBloomPS(VertexOut pin) :SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //后台缓冲区的颜色

	float4 bloomTex = gDiffuseMap[bloomMapIndex].Sample(gsamLinearWrap, pin.TexC);    //bloom贴图的高亮颜色
	return bloomTex + renderTex;
}