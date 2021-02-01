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
	float3 CenterW : POSITION;
	float2 SizeW : TEXCOORD;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	uint   PrimID  : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID,uint vertexID : SV_VertexID)
{
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
	MaterialData matData = gMaterialData[instData.MaterialIndex];

	float u = 1 / 512 + fmod(vertexID, 256) / 256;
	float v = 1 / 512 + floor(vertexID / 256) / 256;
	float4 offset = gDiffuseMap[matData.DiffuseMapIndex].SampleLevel(gsamPointWrap, float2(u,v), 0.0f);
	float4 posW = mul(float4(vin.PosL + offset.xyz, 1.0f), instData.World);    //局部坐标和世界坐标相乘得到世界坐标
	vout.CenterW = posW.xyz;
	vout.SizeW = float2(0.04,0.04);
	return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{

	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePosW - gin[0].CenterW;
	look.y = 0.0f;
	look = normalize(look);
	float3 right = cross(up, look);

	float halfWidth = 0.5f*gin[0].SizeW.x;
	float halfHeight = 0.5f*gin[0].SizeW.y;

	float4 v[4];
	v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);

	float2 texC[4] =
	{
		float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
	};

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.PosH = mul(v[i], gViewProj);
		gout.PosW = v[i].xyz;
		gout.NormalW = look;
		gout.TexC = texC[i];
		gout.PrimID = primID;

		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
	return float4(1,0,0,1);
}


