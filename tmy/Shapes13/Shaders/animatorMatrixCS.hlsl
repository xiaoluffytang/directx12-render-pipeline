struct ToParentData
{
	int startFrame;   //开始帧
	int endFrame;     //结束帧
	float percent;    //比例
};

RWTexture2D<float4> gOutPut : register(u0);
RWTexture2D<float4> frames : register(u1);
RWStructuredBuffer<ToParentData> datas : register(u2);

//四元数球面插值
float4 slerp(float4 starting, float4 ending, float t)
{
	float4 result = float4(0,0,0,0);
	float cosa = starting.x * ending.x + starting.y * ending.y +
		starting.z * ending.z + starting.w * ending.w;
	if (cosa < 0.0f) {
		ending.x = -ending.x;
		ending.y = -ending.y;
		ending.z = -ending.z;
		ending.w = -ending.w;
		cosa = -cosa;
	}
	float k0, k1;
	if (cosa > 0.9999f) {
		k0 = 1.0f - t;
		k1 = t;
	}
	else {
		float sina = sqrt(1.0f - cosa * cosa);
		float a = atan2(sina, cosa);
		float invSina = 1.0f / sina;
		k0 = sin((1.0f - t)*a) * invSina;
		k1 = sin(t*a) * invSina;
	}
	result.x = starting.x * k0 + ending.x * k1;
	result.y = starting.y * k0 + ending.y * k1;
	result.z = starting.z * k0 + ending.z * k1;
	result.w = starting.w * k0 + ending.w * k1;
	return result;
}

//4元数变为旋转矩阵
float4x4 rotation(float4 quaternion)
{
	float xx = quaternion.x * quaternion.x;
	float yy = quaternion.y * quaternion.y;
	float zz = quaternion.z * quaternion.z;
	float xy = quaternion.x * quaternion.y;
	float wz = quaternion.w * quaternion.z;
	float wy = quaternion.w * quaternion.y;
	float xz = quaternion.x * quaternion.z;
	float yz = quaternion.y * quaternion.z;
	float wx = quaternion.w * quaternion.x;

	float r00 = 1 - 2 * (yy + zz);
	float r01 = 2 * (xy - wz);
	float r02 = 2 * (wy + xz);
	float r03 = 0;

	float r10 = 2 * (xy + wz);
	float r11 = 1 - 2 * (xx + zz);
	float r12 = 2 * (yz - wx);
	float r13 = 0;

	float r20 = 2 * (xz - wy);//不是 xy - wy
	float r21 = 2 * (yz + wx);
	float r22 = 1 - 2 * (xx + yy);
	float r23 = 0;

	float r30 = 0;
	float r31 = 0;
	float r32 = 0;
	float r33 = 1;

	return float4x4(r00, r01, r02, r03,
					r10, r11, r12, r13,
					r20, r21, r22, r23,
					r30, r31, r32, r33);
}

float4x4 toMarix(float3 scale, float4 quation, float3 pos)
{
	float4x4 m = {
		scale.x,0,0,0,
		0,scale.y,0,0,
		0,0,scale.z,0,
		0,0,0,1
	};
	float4x4 r = rotation(quation);
	m = mul(m, r);
	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;
	return m;
}

[numthreads(1,64, 1)]
void ComputerToParent(uint3 id: SV_DispatchThreadID)
{
	ToParentData data = datas[id.y];

	int startIndex = data.startFrame * 3;
	float4 startQuation = frames[float2(startIndex,id.y)];
	float3 startScale = frames[float2(startIndex + 1, id.y)].xyz;
	float3 startPos = frames[float2(startIndex + 2, id.y)].xyz;

	int endIndex = data.endFrame * 3;
	float4 endQuation = frames[float2(endIndex, id.y)];
	float3 endScale = frames[float2(endIndex + 1, id.y)].xyz;
	float3 endPos = frames[float2(endIndex + 2, id.y)].xyz;

	float3 scale = lerp(startScale, endScale, data.percent);
	float3 pos = lerp(startPos, endPos, data.percent);
	float4 quation = slerp(startQuation, endQuation,data.percent);

	float4x4 m = toMarix(scale, quation, pos);

	gOutPut[id.xy] = m[0];
	gOutPut[int2(id.x + 1, id.y)] = m[1];
	gOutPut[int2(id.x + 2, id.y)] = m[2];
	gOutPut[int2(id.x + 3, id.y)] = m[3];
}
