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
	VertexOut vout = (VertexOut)0.0f;   //创建一个默认的输出
	InstanceData instData = gInstanceData[instanceID];    //得到这个物体的实例化数据
#ifdef SKINNED
	//开始  动画部分
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
	//结束
#endif

	float4x4 world = instData.World;    //得到实例化的这个物体的世界矩阵
	float4x4 texTransform = instData.TexTransform;   //得到这个实例化物体的UV偏移矩阵
	vout.MatIndex = instData.MaterialIndex;   //得到这个实例化物体使用的材质索引，并赋值给输出结构体
	float4 posW = mul(float4(vin.PosL, 1.0f), world);    //局部坐标和世界坐标相乘得到世界坐标
	vout.PosW = posW.xyz;    //将世界坐标赋值给输出结构体
	float4 normalW = mul(float4(vin.NormalL, 0.0f), world);   //将局部法线转成世界法线
	vout.NormalW = normalW.xyz;    //将世界法线赋值给输出结构体
	vout.PosH = mul(posW, gViewProj);   //将世界坐标转成裁剪空间的坐标
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
	float4 diffuseAlbedo = matData.DiffuseAlbedo;    //材质的漫反射反照率
	float3 fresnelR0 = matData.FresnelR0;    //当一束光线垂直照射下来时反射的光亮，（一种物理材质属性）
	float roughness = matData.Roughness;    //粗糙度
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	//将漫反射贴图上的反照率 与 材质里的漫反射反照率相乘得到 真正的漫反射反照率
	diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);
	diffuseAlbedo *= float4(pin.Color,1);
	//在光栅化阶段，顶点属性插值的过程，可能会导致法线的长度不为1，这里需要归一化
	pin.NormalW = normalize(pin.NormalW);   
	//在世界空间中，摄像机坐标-该目标点坐标，得到摄像机的视角
	float3 toEyeW = normalize(gEyePosW - pin.PosW);  
	//这里使用常用的光照模型
	//环境光 + 漫反射 + 镜面反射（高光）
	float4 ambient = gAmbientLight * diffuseAlbedo;    //环境光
	const float shininess = 1.0f - roughness;   //得到光滑度
	Material mat = { diffuseAlbedo, fresnelR0, shininess };     //新建一个材质传给光照函数
	float shadowFactor = CalcShadowFactor(pin.ShadowPosH) * 1;    //阴影系数
	float4 directLight = ComputeLighting(DirectLights, mat, pin.PosW,   //光照函数，返回的是漫反射光 + 镜面光
		pin.NormalW, toEyeW, float3(shadowFactor, 1, 1));
	float4 litColor = ambient + directLight;    //环境光 + 漫反射光 + 镜面光
	//litColor.a = diffuseAlbedo.a;
	litColor.a = 0.5f;

	return litColor;
}

//普通的ui图片PS
float4 uiSpritePS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamPointWrap, pin.TexC);
//	renderTex *= float4(pin.Color,1);
//	renderTex.a = 1;
	return renderTex;
} 

//普通的ui文本PS
float4 uiTextPS(VertexOut pin) : SV_Target
{
	MaterialData matData = gMaterialData[pin.MatIndex];
	uint diffuseTexIndex = matData.DiffuseMapIndex;      //用第几张贴图
	float4 renderTex = gDiffuseMap[diffuseTexIndex].Sample(gsamPointWrap, pin.TexC);
	renderTex = float4(pin.Color.rgb,renderTex.r);
	return renderTex;
}

//描边
float4 strokePS(VertexOut pin) : SV_Target
{
	return float4(1,0,0,1);
}
