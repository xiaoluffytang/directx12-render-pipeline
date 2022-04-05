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
	float4 ShadowPosH : POSITION0;
	float3 PosW    : POSITION1;
	float3 PosL    : POSITION2;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	nointerpolation uint MatIndex  : MATINDEX;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //����һ��Ĭ�ϵ����

	InstanceData instData = gInstanceData[instanceID];    //�õ���������ʵ��������
	float4x4 world = instData.World;    //�õ�ʵ���������������������
	float4x4 texTransform = instData.TexTransform;   //�õ����ʵ���������UVƫ�ƾ���
	vout.MatIndex = instData.MaterialIndex;   //�õ����ʵ��������ʹ�õĲ�������������ֵ������ṹ��
	float4 posW = mul(float4(vin.PosL, 1), world);    //�ֲ����������������˵õ���������
	vout.PosW = posW.xyz;    //���������긳ֵ������ṹ��
	vout.NormalW = mul(float4(vin.NormalL,0), world).xyz;    //���ֲ�����ת�����編��,�����編�߸�ֵ������ṹ��
	vout.PosH = mul(posW, gViewProj);   //����������ת�ɲü��ռ������
	MaterialData material = gMaterialData[instData.MaterialIndex];
	float4x4 uvMatrix = mul(texTransform, material.MatTransform);
	vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), uvMatrix).xy;
	vout.ShadowPosH = mul(posW, gShadowTransform);
	vout.PosL = vin.PosL;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo * 0.3;
	float3 fresnelR0 = matData.FresnelR0;
	float  roughness = matData.Roughness;
	uint diffuseTexIndex = matData.DiffuseMapIndex;
	pin.NormalW = normalize(pin.NormalW);
	float3 toEyeW = normalize(gEyePosW - pin.PosW);
	float4 ambient = gAmbientLight * diffuseAlbedo;
	const float shininess = 1.0f - roughness;
	Material mat = { diffuseAlbedo, fresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(DirectLights, mat, pin.PosW,   //���պ��������ص���������� + �����
		pin.NormalW, toEyeW, shadowFactor);
	float4 litColor = ambient + directLight;
	float3 r = reflect(-toEyeW, pin.NormalW);    //ʹ�÷�������
	float4 reflectionColor = cubeTexture[diffuseTexIndex].Sample(gsamLinearWrap, r);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, pin.NormalW, r);
	litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
	litColor.a = diffuseAlbedo.a;
	return litColor;
}
