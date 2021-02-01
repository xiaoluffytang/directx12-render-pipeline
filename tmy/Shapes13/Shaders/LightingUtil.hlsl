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
	float3 Strength;      //����ǿ��
	float FalloffStart; // point/spot light only
	float3 Direction;   // ���շ���
	float FalloffEnd;   // point/spot light only
	float3 Position;    //  �������
	float SpotPower;    // spot light only
};

struct Material
{
	float4 DiffuseAlbedo;     //�����䷴����
	float3 FresnelR0;    //������ЧӦֵ��һ����ʯ��������Ʊ�ʾ
	float Shininess;    //�⻬��
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
	return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

//ʹ��ʯ��˼��㷨�����������ֵ
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	//RF(a) = RF(0) + (1 - RF(0)) * pow(cos(a),5);
	float cosIncidentAngle = saturate(dot(normal, lightVec));    //��������ֵ
	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0)*pow(f0,5);
	return reflectPercent;
}

//����ǿ�ȣ����շ��򣬺�۱��淨�ߣ���������۲��ķ��򣬲���
float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
	const float m = mat.Shininess * 256.0f;     //���⻬�ȴӷ�Χ1��Ϊ��Χ256
	float3 halfVec = normalize(toEye + lightVec);     //������շ���͹۲췽����м�����
	//���ݹ⻬�ȣ��������ϵ������ʽ����
	float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	//���������ϵ����ʹ��ʯ��� ���Ƽ���
	float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, normal, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);
	
	return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
	//�Ƚ����շ����Ϊ�෴����
	float3 lightVec = -L.Direction;
	//������ߺ͹��շ��������ֵ
	float ndotl = max(dot(lightVec, normal), 0.0f);
	//����Lambert�ʲ����Ҷ��������෴�������ǿ�ȵĹ�
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
