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
};

void skin(VertexIn vin, inout float3 posL, int boneIndex, float weight, float firstU, float firstV)
{
#ifdef SKINNED
	float2 uv = float2(firstU, firstV + boneIndex * animationDisV);
	float4x4 m;
	m[0] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv, 0.0f);
	m[1] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(animationDisU, 0), 0.0f);
	m[2] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(2 * animationDisU, 0), 0.0f);
	m[3] = gDiffuseMap[animationTexIndex].SampleLevel(gsamPointWrap, uv + float2(3 * animationDisU, 0), 0.0f);
	posL += weight * mul(float4(vin.PosL, 1.0f), m).xyz;
#endif
}

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据

#ifdef SKINNED
	float3 ws = vin.BoneWeights;
	float3 posL = float3(0.0f, 0.0f, 0.0f);

	float t = gTotalTime * instData.param3 + instData.param1;
	t = fmod(t, animationWholeTimes);
	float firstU = animationFirstU + floor(t * animationSecondFrameCount) * animationDisU * 4;
	float firstV = animationFirstV + instData.param2 * animationBoneCount * animationDisV;

	skin(vin, posL, vin.BoneIndices[0], ws.x, firstU, firstV);
	skin(vin, posL, vin.BoneIndices[1], ws.y, firstU, firstV);
	skin(vin, posL, vin.BoneIndices[2], ws.z, firstU, firstV);
	skin(vin, posL, vin.BoneIndices[3], 1 - ws.x - ws.y - ws.z, firstU, firstV);
	vin.PosL = posL;
#endif

	float4x4 world = instData.World;    //得到实例化的这个物体的世界矩阵
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //局部坐标和世界坐标相乘得到世界坐标	
	vout.PosH = mul(posW, gViewProj);   //将世界坐标转成裁剪空间的坐标
	return vout;
}

void PS(VertexOut pin)
{

}



