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
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出

	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
	float4x4 world = instData.World;    //得到实例化的这个物体的世界矩阵
	float4x4 texTransform = instData.TexTransform;   //得到这个实例化物体的UV偏移矩阵
	vout.MatIndex = instData.MaterialIndex;   //得到这个实例化物体使用的材质索引，并赋值给输出结构体
	float4 posW = mul(float4(vin.PosL, 1), world);    //局部坐标和世界坐标相乘得到世界坐标
	vout.PosW = posW.xyz;    //将世界坐标赋值给输出结构体
	vout.NormalW = mul(float4(vin.NormalL,0), world).xyz;    //将局部法线转成世界法线,将世界法线赋值给输出结构体
	vout.PosH = mul(posW, gViewProj);   //将世界坐标转成裁剪空间的坐标
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
	float4 directLight = ComputeLighting(DirectLights, mat, pin.PosW,   //光照函数，返回的是漫反射光 + 镜面光
		pin.NormalW, toEyeW, shadowFactor);
	float4 litColor = ambient + directLight;
	float3 r = reflect(-toEyeW, pin.NormalW);    //使用反射向量
	float4 reflectionColor = cubeTexture[diffuseTexIndex].Sample(gsamLinearWrap, r);
	float3 fresnelFactor = SchlickFresnel(fresnelR0, pin.NormalW, r);
	litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
	litColor.a = diffuseAlbedo.a;
	return litColor;
}
