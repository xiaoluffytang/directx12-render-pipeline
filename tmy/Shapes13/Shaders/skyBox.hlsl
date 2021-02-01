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
	float3 PosL    : POSITION;

	nointerpolation uint MatIndex  : MATINDEX;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //����һ��Ĭ�ϵ����
	InstanceData instData = gInstanceData[instanceID];    //�õ���������ʵ��������
	vout.PosL = vin.PosL;    //�þֲ������λ����Ϊ������ͼ�Ĳ�������
	float4x4 world = instData.World;    //�õ�ʵ���������������������
	vout.MatIndex = instData.MaterialIndex;   //�õ����ʵ��������ʹ�õĲ�������������ֵ������ṹ��
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //�ֲ����������������˵õ���������
	posW.xyz += gEyePosW;
	vout.PosH = mul(posW, gViewProj).xyww;   //��z=w������ʹ��պе����ֵ��ԶΪ1
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
    return cubeTexture[diffuseTexIndex].Sample(gsamLinearWrap, pin.PosL);
}


