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
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
	vout.PosL = vin.PosL;    //用局部顶点的位置作为立方体图的查找向量
	float4x4 world = instData.World;    //得到实例化的这个物体的世界矩阵
	vout.MatIndex = instData.MaterialIndex;   //得到这个实例化物体使用的材质索引，并赋值给输出结构体
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //局部坐标和世界坐标相乘得到世界坐标
	posW.xyz += gEyePosW;
	vout.PosH = mul(posW, gViewProj).xyww;   //让z=w，可以使天空盒的深度值永远为1
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
    return cubeTexture[diffuseTexIndex].Sample(gsamLinearWrap, pin.PosL);
}


