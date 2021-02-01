#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentL : TANGENT;
	float3 Color : COLOR;
#ifdef SKINNED
	float3 BoneWeights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
#endif
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float4 ShadowPosH : POSITION0;
	float3 PosW    : POSITION1;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 Color   : COLOR;
	nointerpolation uint MatIndex  : MATINDEX;
};

void skin(VertexIn vin, inout float3 posL, inout float3 normalL, inout float3 tangentL, int boneIndex, float weight, float firstU, float firstV)
{
#ifdef SKINNED
	float2 uv = float2(firstU, firstV + boneIndex * animationDisV);
	float4x4 m;
	m[0] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv, 0.0f);
	m[1] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(animationDisU, 0), 0.0f);
	m[2] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(2 * animationDisU, 0), 0.0f);
	m[3] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(3 * animationDisU, 0), 0.0f);
	posL += weight * mul(float4(vin.PosL, 1.0f), m).xyz;
	normalL += weight * mul(vin.NormalL, (float3x3)m);
	tangentL += weight * mul(vin.TangentL.xyz, (float3x3)m);
#endif
}

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //����һ��Ĭ�ϵ����
	InstanceData instData = gInstanceData[instanceID];    //�õ���������ʵ��������
#ifdef SKINNED
	//��ʼ  ��������
	float3 ws = vin.BoneWeights;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);

	float t = gTotalTime * instData.param3 + instData.param1;
	t = fmod(t, animationWholeTimes);
	float firstU = animationFirstU + floor(t * animationSecondFrameCount) * animationDisU * 4;
	float firstV = animationFirstV + instData.param2 * animationBoneCount * animationDisV;

	skin(vin, posL, normalL, tangentL, vin.BoneIndices[0], ws.x, firstU, firstV);
	skin(vin, posL, normalL, tangentL, vin.BoneIndices[1], ws.y, firstU, firstV);
	skin(vin, posL, normalL, tangentL, vin.BoneIndices[2], ws.z, firstU, firstV);
	skin(vin, posL, normalL, tangentL, vin.BoneIndices[3], 1 - ws.x - ws.y - ws.z, firstU, firstV);

	vin.PosL = posL;
	vin.NormalL = normalL;
	vin.TangentL.xyz = tangentL;
	//����
#endif

	float4x4 world = instData.World;    //�õ�ʵ���������������������
	float4x4 texTransform = instData.TexTransform;   //�õ����ʵ���������UVƫ�ƾ���
	vout.MatIndex = instData.MaterialIndex;   //�õ����ʵ��������ʹ�õĲ�������������ֵ������ṹ��
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //�ֲ����������������˵õ���������
	vout.PosW = posW.xyz;    //���������긳ֵ������ṹ��
	float4 normalW = mul(float4(vin.NormalL, 0.0f), world);   //���ֲ�����ת�����編��
	vout.NormalW = normalW.xyz;    //�����編�߸�ֵ������ṹ��
	vout.PosH = mul(posW, gViewProj);   //����������ת�ɲü��ռ������
	MaterialData material = gMaterialData[instData.MaterialIndex];
	float4x4 uvMatrix = mul(texTransform, material.MatTransform);
	vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), uvMatrix).xy;
	vout.ShadowPosH = mul(posW, gShadowTransform);
	vout.Color = vin.Color;
#ifdef STROKE
	//float3 viewNormal = mul(vout.NormalW, (float3x3)gView);
	//float2 projectNormal = mul(viewNormal.xy, (float2x2)gProj);
	float2 projectNormal = mul(vout.NormalW, (float3x3)gViewProj).xy;
	vout.PosH.xy += projectNormal;
	vout.PosH.z += 0.0005f * vout.PosH.w;
#endif
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;    //���ʵ������䷴����
	float3 fresnelR0 = matData.FresnelR0;    //��һ�����ߴ�ֱ��������ʱ����Ĺ�������һ������������ԣ�
	float roughness = matData.Roughness;    //�ֲڶ�
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	//����������ͼ�ϵķ����� �� ������������䷴������˵õ� �����������䷴����
	diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);
	diffuseAlbedo *= float4(pin.Color,1);
	//�ڹ�դ���׶Σ��������Բ�ֵ�Ĺ��̣����ܻᵼ�·��ߵĳ��Ȳ�Ϊ1��������Ҫ��һ��
	pin.NormalW = normalize(pin.NormalW);   
	//������ռ��У����������-��Ŀ������꣬�õ���������ӽ�
	float3 toEyeW = normalize(gEyePosW - pin.PosW);  
	//����ʹ�ó��õĹ���ģ��
	//������ + ������ + ���淴�䣨�߹⣩
	float4 ambient = gAmbientLight * diffuseAlbedo;    //������
	const float shininess = 1.0f - roughness;   //�õ��⻬��
	Material mat = { diffuseAlbedo, fresnelR0, shininess };     //�½�һ�����ʴ������պ���
	float shadowFactor = CalcShadowFactor(pin.ShadowPosH) * 1;    //��Ӱϵ��
	float4 directLight = ComputeLighting(DirectLights, mat, pin.PosW,   //���պ��������ص���������� + �����
		pin.NormalW, toEyeW, float3(shadowFactor, 1, 1));
	float4 litColor = ambient + directLight;    //������ + ������� + �����
	//litColor.a = diffuseAlbedo.a;
	litColor.a = 0.5f;

	return litColor;
}

//��ͨ��uiͼƬPS
float4 uiSpritePS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamPointWrap, pin.TexC);
//	renderTex *= float4(pin.Color,1);
//	renderTex.a = 1;
	return renderTex;
} 

//��ͨ��ui�ı�PS
float4 uiTextPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //�õڼ�����ͼ
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamPointWrap, pin.TexC);
	renderTex = float4(pin.Color.rgb,renderTex.r);
	return renderTex;
}

//���
float4 strokePS(VertexOut pin) : SV_Target
{
	return float4(1,0,0,1);
}
