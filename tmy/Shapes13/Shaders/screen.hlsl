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
	VertexOut vout = (VertexOut)0.0f;   //����һ��Ĭ�ϵ����
	InstanceData instData = gInstanceData[instanceID];    //�õ���������ʵ��������
	if (vin.PosL.x > -0.5 && vin.PosL.x < 0.5)
	{
		vin.PosL.z = 0;
	}
	vout.PosH = float4(vin.PosL.x, vin.PosL.z, 0, 1);
	vout.TexC = float2(vin.TexC.x, vin.TexC.y);
	vout.PosW = float3(vin.PosL.x, vin.PosL.z, 0);
	vout.MatIndex = instData.MaterialIndex;   //�õ����ʵ��������ʹ�õĲ�������������ֵ������ṹ��
	return vout;
}

//��ͨ��
float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);
	//renderTex = float4(0.5,0,0,1);
	return renderTex;
}

//��Ч
float4 FogPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //��̨����������ɫ
	float4 depthStenicTex = gDiffuseMap[depthStencilMapIndex].Sample(gsamLinearWrap, pin.TexC);    //���ֵ
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

//������Ļ���ȣ����ͶȺͶԱȶ�
float4 BriSatConPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);

	float BrightNess = 2.0f;   //���ȣ�һ����0-3��
	float Saturation = 0.50f;   //���Ͷȣ�һ����0-3��
	float Contrast = 2.0f;     //�Աȶȣ�һ����0-3��

	float3 finColor = renderTex.xyz * BrightNess;    //��ɫ*����
	//Ӧ�ñ��Ͷ�
	float s = 0.2125f * renderTex.r + 0.7154f * renderTex.g + 0.0721f * renderTex.b;
	finColor = lerp(float3(s, s, s), finColor, Saturation);
	//Ӧ�öԱȶ�
	finColor = lerp(float3(0.5f, 0.5f, 0.5f), finColor, Contrast);

	return float4(finColor,1.0f);
}

//Bloom��Ч�������ĵط����������������ĵط�����ΧҲ������
float4 GetBloomPS(VertexOut pin):SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //��̨����������ɫ
	float light = getLightValue(renderTex.rgb);
	light = clamp(light - 0.4, 0, 0.2);
	return light * renderTex;
}

//Bloom��Ч�������ĵط����������������ĵط�����ΧҲ������
float4 AddBloomPS(VertexOut pin) :SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);    //��̨����������ɫ

	float4 bloomTex = gDiffuseMap[bloomMapIndex].Sample(gsamLinearWrap, pin.TexC);    //bloom��ͼ�ĸ�����ɫ
	return bloomTex + renderTex;
}