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
	float4 ShadowPosH : POSITION0;
	float3 PosW    : POSITION1;
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
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //�ֲ����������������˵õ���������
	vout.PosW = posW.xyz;    //���������긳ֵ������ṹ��
	float4 normalW = mul(float4(vin.NormalL, 0.0f), world);   //���ֲ�����ת�����編��
	vout.NormalW = normalW.xyz;    //�����編�߸�ֵ������ṹ��
	vout.PosH = mul(posW, gViewProj);   //����������ת�ɲü��ռ������
	MaterialData material = gMaterialData[instData.MaterialIndex];
	float4x4 uvMatrix = mul(texTransform, material.MatTransform);
	vout.TexC = mul(float4(vin.TexC, 0.0f, 1.0f), uvMatrix).xy;
	vout.ShadowPosH = mul(posW, gShadowTransform);
	return vout;
}

float Noise(float3 x, int noiseTexIndex)
{
	float3 p = floor(x);
	float3 f = frac(x);
	f = f *f * f*(3.0 - 2.0*f);
	
	//����1
	//float2 uv = (p.xy + float2(37, 17)*p.z) + f.xy;
	//uv = (uv + 0.5) / 256;
	////uv.y = 1 - uv.y;
	//float2 rg = gDiffuseMap[noiseTexIndex].Sample(gsamLinearWrap, uv).yx;
	//return lerp(rg.x, rg.y, f.z);
	//����2
	float2 uv = (p.xy) + f.xy + x.z / 2;
	uv = (uv + 0.5) / 256;
	uv.y = 1 - uv.y;
	float4 noise = gDiffuseMap[noiseTexIndex].Sample(gsamLinearWrap, uv);
	return lerp(noise.x, noise.y, noise.z);
}

float WaterMap(float3 pos, int noiseTexIndex)
{
	float3 p = float3(pos.xz, gTotalTime * 2);
	float f = 0;
	float3x3 m = float3x3(0.00, 0.80, 0.60,
		-0.80, 0.36, -0.48,
		-0.60, -0.48, 0.64);

	f += 0.5 * Noise(p, noiseTexIndex);  p = mul(p, m) * 2;
	f += 0.25 * Noise(p, noiseTexIndex);  p = mul(p, m) * 2;
	f += 0.125 * Noise(p, noiseTexIndex);  p = mul(p, m) * 2;
	f += 0.0625 * Noise(p, noiseTexIndex);
	return f / 0.9375 * 1;
}

//��ǰˮ���صķ���
float3 WaterNormal(float3 pos, float dis, int noiseTexIndex)
{
	float EPSILON = 0.008 * dis;
	float3 dx = float3(EPSILON, 0., 0.);
	float3 dz = float3(0., 0., EPSILON);

	float3	normal = float3(0, 1, 0);
	float bumpfactor = 0.6 * (1. - smoothstep(0., 1, EPSILON));//���ݾ�������Bump����

	normal.x = -bumpfactor * (WaterMap(pos + dx, noiseTexIndex) - WaterMap(pos - dx, noiseTexIndex)) / (2. * EPSILON);
	normal.z = -bumpfactor * (WaterMap(pos + dz, noiseTexIndex) - WaterMap(pos - dz, noiseTexIndex)) / (2. * EPSILON);
	return normalize(normal);
}

float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];

	float3 worldLightDir =  normalize(DirectLights[0].Direction);    //��Դ����
	float3 v = gEyePosW - pin.PosW;
	float3 viewDir = normalize(v);   //���߷���
	float dis = length(v);   //��ʾ������͸�����������ж��ٸ���λ����
	float3 worldNormal = WaterNormal(pin.PosW, dis, matData.DiffuseMapIndex);
	float ndotr = dot(worldNormal, -viewDir);
	float fresnel = pow(1.0 - abs(ndotr), 6.);//���� 
	float3 reflectRd = reflect(viewDir, worldNormal);

	float3 sky = float3(1, 1, 1);
	float3 baseColor = float3(0.23, 0.49, 0.53);
	float3 seaWaterColor = float3(0.2, 0.3, 0.4);
	float3 diff = pow(dot(worldNormal, worldLightDir) * 1 + 0.1, 2);
	float3 refractCol = baseColor + diff * seaWaterColor * 0.12;

	float3 col = lerp(refractCol, sky, fresnel);

	float spec = pow(max(dot(reflectRd, worldLightDir), 0.0), 4) * 0.3;
	col += float3(spec, spec, spec);

	return float4(col,1);
}


