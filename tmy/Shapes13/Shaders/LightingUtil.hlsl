#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif
#define MaxLights 5

struct Light
{
	float3 Strength;      //光照强度
	float FalloffStart; // point/spot light only
	float3 Direction;   // 光照方向
	float FalloffEnd;   // point/spot light only
	float3 Position;    //  光的坐标
	float SpotPower;    // spot light only
};

struct Material
{
	float4 DiffuseAlbedo;     //漫反射反照率
	float3 FresnelR0;    //菲涅尔效应值，一般用石里克莱近似表示
	float Shininess;    //光滑度
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
	return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

//使用石里克计算法，计算菲涅尔值
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	//RF(a) = RF(0) + (1 - RF(0)) * pow(cos(a),5);
	float cosIncidentAngle = saturate(dot(normal, lightVec));    //计算余弦值
	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0)*pow(f0,5);
	return reflectPercent;
}

//光照强度，光照方向，宏观表面法线，摄像机看观察点的方向，材质
float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
	const float m = mat.Shininess * 256.0f;     //将光滑度从范围1变为范围256
	float3 halfVec = normalize(toEye + lightVec);     //算出光照方向和观察方向的中间向量
	//根据光滑度，算出反射系数，公式如下
	float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	//算出菲涅尔系数，使用石里克 近似计算
	float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, normal, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);
	
	return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
	//先将光照方向变为相反方向
	float3 lightVec = -L.Direction;
	//算出法线和光照方向的余弦值
	float ndotl = max(dot(lightVec, normal), 0.0f);
	//根据Lambert朗伯余弦定理，算出最多反射出多少强度的光
	float3 lightStrength = L.Strength * ndotl;
	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	float3 lightVec = L.Position - pos;
	float d = length(lightVec);
	if (d > L.FalloffEnd)
		return 0.0f;
	lightVec /= d;

	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = L.Strength * ndotl;
	float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
	lightStrength *= att;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	float3 lightVec = L.Position - pos;
	float d = length(lightVec);
	if (d > L.FalloffEnd)
		return 0.0f;
	lightVec /= d;
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = L.Strength * ndotl;
	float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
	lightStrength *= att;
	float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
	lightStrength *= spotFactor;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
	float3 pos, float3 normal, float3 toEye, float3 shadowFactor)
{
	float3 result = 0.0f;

	int i = 0;

#if (NUM_DIR_LIGHTS > 0)
	for (i = 0; i < NUM_DIR_LIGHTS; ++i)
	{
		result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
	}
#endif

#if (NUM_POINT_LIGHTS > 0)
	for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
	{
		result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
	}
#endif

#if (NUM_SPOT_LIGHTS > 0)
	for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
	{
		result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
	}
#endif 

	return float4(result, 0.0f);
}
