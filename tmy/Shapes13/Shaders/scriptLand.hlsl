struct Vertex
{
	float3 Pos;
	float3 Normal;
	float2 UV;
	float3 TangentL;
	float3 Color;
};

RWStructuredBuffer<Vertex> datas : register(u0);

float Hash12(float2 co)
{
	return frac(sin(co.x*157.1147 + co.y*13.713) * 43751.1353);
}

float VNoise(float2 p)
{
	float2 pi = floor(p);
	float2 pf = p - pi;

	float2 w = pf * pf * (3.0 - 2.0 * pf);

	return lerp(lerp(Hash12(pi + float2(0.0, 0.0)), Hash12(pi + float2(1.0, 0.0)), w.x),
		lerp(Hash12(pi + float2(0.0, 1.0)), Hash12(pi + float2(1.0, 1.0)), w.x),
		w.y);
}


//一个噪声发生器
float Noise1(int x, int y)
{
	x = x % 25;
	y = y % 25;
	int n = x + y * 57;
	n = (n << 13) ^ n;
	return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

//一个光滑噪声发生器
float SmoothNoise_1(int x, int y)
{
	float corners = (Noise1(x - 1, y - 1) + Noise1(x + 1, y - 1) + Noise1(x - 1, y + 1) + Noise1(x + 1, y + 1)) / 16.0f;
	float sides = (Noise1(x - 1, y) + Noise1(x + 1, y) + Noise1(x, y - 1) + Noise1(x, y + 1)) / 8.0f;
	float center = Noise1(x, y) / 4.0f;
	return corners + sides + center;
}

//使用cosin插值函数
float Cosine_Interpolate(float a, float b, float x)
{
	double ft = x * 3.1415927;
	double f = (1 - cos(ft)) * 0.5f;

	return  a * (1 - f) + b * f;

}

//插值噪声发生器
float InterpolatedNoise_1(float x, float y)
{

	int integer_X = int(x);
	float fractional_X = x - integer_X;

	int integer_Y = int(y);
	float fractional_Y = y - integer_Y;

	float v1 = SmoothNoise_1(integer_X, integer_Y);
	float v2 = SmoothNoise_1(integer_X + 1, integer_Y);
	float v3 = SmoothNoise_1(integer_X, integer_Y + 1);
	float v4 = SmoothNoise_1(integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, fractional_X);
	float i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

//最终的PERLIN NOISE
float PerlinNoise_2D(float x, float y)
{
	float total = 0.0f;
	float p = 0.45f;
	int n = 2;

	for (int i = 0; i <= n; i++)
	{
		float frequency = pow((float)2, i);
		float amplitude = pow(p, i);

		total = total + InterpolatedNoise_1(x * frequency, y * frequency) * amplitude;
	}

	return total;
}




[numthreads(8, 8, 1)]
void Moutain(uint3 id: SV_DispatchThreadID)
{
	int index = id.y * 64 + id.x;
	float3 pos = datas[index].Pos;

	float2 p = pos.xz * 0.9 / 10;
	float a = 0;
	float b = 0.491;
	float2 d = float2(0, 0);
	for (int i = 0; i < 5; i++)
	{
		float n = VNoise(p);
		a += b * n;
		b *= 0.49;
		p = p * 2.01;
	}
	datas[index].Pos.x *= 2;
	datas[index].Pos.y *= 2;
	datas[index].Pos.y = 200 * a;
}

[numthreads(8, 8, 1)]
void Moutain2(uint3 id: SV_DispatchThreadID)
{
	int index = id.y * 64 + id.x;

	datas[index].Pos.y = 100 * PerlinNoise_2D((id.x + 10000) / 10, (id.z + 10000) / 10);
}