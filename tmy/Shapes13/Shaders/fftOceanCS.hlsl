#define PI 3.14159274f
#define G 9.81f

cbuffer cbSettings : register(b0)
{
	int N;					//fft�����С
	float OceanLength;		//���󳤶�
	float A;				//phillips�ײ�����Ӱ�첨�˸߶�	
	float Time;				//ʱ��
	float Lambda;			//ƫ��Ӱ��
	float HeightScale;		//�߶�Ӱ��
	float BubblesScale;	    //��ĭǿ��
	float BubblesThreshold; //��ĭ��ֵ
	float2 WindAndSeed;		//���������� xyΪ��, zwΪ�����������
}

cbuffer cbSettings : register(b1)
{
	int Ns;					//Ns = pow(2,m-1); mΪ�ڼ��׶�
}

RWTexture2D<float4> GaussianRandomRT : register(u0);		//��˹�����
RWTexture2D<float4> HeightSpectrumRT : register(u1);		//�߶�Ƶ��
RWTexture2D<float4> DisplaceXSpectrumRT : register(u2);	//Xƫ��Ƶ��
RWTexture2D<float4> DisplaceZSpectrumRT : register(u3);	//Zƫ��Ƶ��
RWTexture2D<float4> OutputRT : register(u4);				//���
RWTexture2D<float4> DisplaceRT : register(u5);				//������ɵ�ƫ������
RWTexture2D<float4> NormalRT : register(u6);				//��������
RWTexture2D<float4> BubblesRT : register(u7);				//��ĭ����
RWTexture2D<float4> InputRT : register(u8);				//����

float DonelanBannerDirectionalSpreading(float2 k);
float PositiveCosineSquaredDirectionalSpreading(float2 k);
float phillips(float2 k);
float dispersion(float2 k);
float2 gaussian(float2 id);
uint wangHash(uint seed);
uint rand(uint rngState);
float2 complexMultiply(float2 c1, float2 c2);

//�����˹�������
[numthreads(8, 8, 1)]
void ComputeGaussianRandom(uint3 id: SV_DispatchThreadID)
{
	float2 g = gaussian(id.xy);

	GaussianRandomRT[id.xy] = float4(g, 0, 0);
}


//���ɸ߶Ⱥ�ƫ��Ƶ��
[numthreads(8, 8, 1)]
void CreateHeightDisplaceSpectrum(uint3 id: SV_DispatchThreadID)
{
	float2 k = float2(2.0f * PI * id.x / N - PI, 2.0f * PI * id.y / N - PI);

	float2 gaussian = GaussianRandomRT[id.xy].xy;
	float phillipsValue = phillips(k);
	float banner1 = DonelanBannerDirectionalSpreading(k);
	float2 hTilde0 = gaussian * sqrt(abs(phillipsValue * banner1) / 2.0f);
	float2 hTilde0Conj = gaussian * sqrt(abs(phillips(-k) * DonelanBannerDirectionalSpreading(-k)) / 2.0f);

	hTilde0Conj.y *= -1.0f;

	float omegat = dispersion(k) * Time;
	float c = cos(omegat);
	float s = sin(omegat);

	float2 h1 = complexMultiply(hTilde0, float2(c, s));
	float2 h2 = complexMultiply(hTilde0Conj, float2(c, -s));
	float2 HTilde = h1 + h2;
	//�߶�Ƶ��
	HeightSpectrumRT[id.xy] = float4(HTilde, 0, 0);
	//ƫ��Ƶ��
	k /= max(0.001f, length(k));
	float2 KxHTilde = complexMultiply(float2(0, -k.x), HTilde);
	float2 kzHTilde = complexMultiply(float2(0, -k.y), HTilde);
	DisplaceXSpectrumRT[id.xy] = float4(KxHTilde, 0, 0);
	DisplaceZSpectrumRT[id.xy] = float4(kzHTilde, 0, 0);
}
//����FFT����,ֻ��Ե�m-1�׶Σ����һ�׶���Ҫ���⴦��
[numthreads(8, 8, 1)]
void FFTHorizontal(uint3 id: SV_DispatchThreadID)
{
	int2 idxs = id.xy;
	idxs.x = floor(id.x / (Ns * 2.0f)) * Ns + id.x % Ns;
	float angle = 2.0f * PI * (id.x / (Ns * 2.0f));
	float2 w = float2(cos(angle), sin(angle));

	float2 x0 = InputRT[idxs].xy;
	float2 x1 = InputRT[int2(idxs.x + N * 0.5f, idxs.y)].xy;

	float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
	OutputRT[id.xy] = float4(output, 0, 0);
	
}
//����FFT���׶μ���,��Ҫ�����ر���
[numthreads(8, 8, 1)]
void FFTHorizontalEnd(uint3 id: SV_DispatchThreadID)
{
	int2 idxs = id.xy;
	idxs.x = floor(id.x / (Ns * 2.0f)) * Ns + id.x % Ns;
	float angle = 2.0f * PI * (id.x / (Ns * 2.0f));
	float2 w = float2(cos(angle), sin(angle));

	/*********�޸�����***********/
	w *= -1;
	/***************************/

	float2 x0 = InputRT[idxs].xy;
	float2 x1 = InputRT[int2(idxs.x + N * 0.5f, idxs.y)].xy;

	float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
	/*********�޸�����***********/
	int x = id.x - N * 0.5f;
	output *= ((x + 1) % 2.0f) * 1 + (x % 2.0f) * (-1);
	/***************************/
	OutputRT[id.xy] = float4(output, 0, 0);
}
//����FFT����,ֻ��Ե�m-1�׶Σ����һ�׶���Ҫ���⴦��
[numthreads(8, 8, 1)]
void FFTVertical(uint3 id: SV_DispatchThreadID)
{
	int2 idxs = id.xy;
	idxs.y = floor(id.y / (Ns * 2.0f)) * Ns + id.y % Ns;
	float angle = 2.0f * PI * (id.y / (Ns * 2.0f));
	float2 w = float2(cos(angle), sin(angle));

	float2 x0 = InputRT[idxs].xy;
	float2 x1 = InputRT[int2(idxs.x, idxs.y + N * 0.5f)].xy;

	float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
	OutputRT[id.xy] = float4(output, 0, 0);
}
//����FFT���׶μ���,��Ҫ�����ر���
[numthreads(8, 8, 1)]
void FFTVerticalEnd(uint3 id: SV_DispatchThreadID)
{
	int2 idxs = id.xy;
	idxs.y = floor(id.y / (Ns * 2.0f)) * Ns + id.y % Ns;
	float angle = 2.0f * PI * (id.y / (Ns * 2.0f));
	float2 w = float2(cos(angle), sin(angle));

	/*********�޸�����***********/
	w *= -1;
	/***************************/

	float2 x0 = InputRT[idxs].xy;
	float2 x1 = InputRT[int2(idxs.x, idxs.y + N * 0.5f)].xy;

	float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
	/*********�޸�����***********/
	int x = id.y - N * 0.5f;
	output *= ((x + 1) % 2.0f) * 1 + (x % 2.0f) * (-1);
	/***************************/
	OutputRT[id.xy] = float4(output, 0, 0);
}
//����ƫ������
[numthreads(8, 8, 1)]
void TextureGenerationDisplace(uint3 id: SV_DispatchThreadID)
{
	float y = length(HeightSpectrumRT[id.xy].xy) / (N * N) * HeightScale;//�߶�
	float x = length(DisplaceXSpectrumRT[id.xy].xy) / (N * N) * Lambda;//x��ƫ��
	float z = length(DisplaceZSpectrumRT[id.xy].xy) / (N * N) * Lambda;//z��ƫ��

	//HeightSpectrumRT[id.xy] = float4(y, y, y, 0);
	//DisplaceXSpectrumRT[id.xy] = float4(x, x, x, 0);
	//DisplaceZSpectrumRT[id.xy] = float4(z, z, z, 0);
	DisplaceRT[id.xy] = float4(x, y, z, 0);
}
//���ɷ��ߺ���ĭ����
[numthreads(8, 8, 1)]
void TextureGenerationNormalBubbles(uint3 id: SV_DispatchThreadID)
{
	//���㷨��
	float uintLength = OceanLength / (N - 1.0f);//����䵥λ����

	//��ȡ��ǰ�㣬��Χ4�����uv����
	uint2 uvX1 = uint2((id.x - 1.0f + N) % N, id.y);
	uint2 uvX2 = uint2((id.x + 1.0f + N) % N, id.y);
	uint2 uvZ1 = uint2(id.x, (id.y - 1.0f + N) % N);
	uint2 uvZ2 = uint2(id.x, (id.y + 1.0f + N) % N);

	//�Ե�ǰ��Ϊ���ģ���ȡ��Χ4�����ƫ��ֵ
	float3 x1D = DisplaceRT[uvX1].xyz;//��x�� ��һ�����ƫ��ֵ
	float3 x2D = DisplaceRT[uvX2].xyz;//��x�� �ڶ������ƫ��ֵ
	float3 z1D = DisplaceRT[uvZ1].xyz;//��z�� ��һ�����ƫ��ֵ
	float3 z2D = DisplaceRT[uvZ2].xyz;//��z�� �ڶ������ƫ��ֵ
	
	//�Ե�ǰ��Ϊԭ�㣬������Χ4���������
	float3 x1 = float3(x1D.x - uintLength, x1D.yz);//��x�� ��һ���������
	float3 x2 = float3(x2D.x + uintLength, x2D.yz);//��x�� �ڶ����������
	float3 z1 = float3(z1D.xy, z1D.z - uintLength);//��z�� ��һ���������
	float3 z2 = float3(z1D.xy, z1D.z + uintLength);//��z�� �ڶ����������

	//��������������
	float3 tangentX = x2 - x1;
	float3 tangentZ = z2 - z1;

	//���㷨��
	float3 normal = normalize(cross(tangentZ, tangentX));


	//������ĭ
	float3 ddx = x2D - x1D;
	float3 ddz = z2D - z1D;
	//�ſɱ�����ʽ
	float jacobian = (1.0f + ddx.x) * (1.0f + ddz.z) - ddx.z * ddz.x;

	jacobian = saturate(max(0, BubblesThreshold - saturate(jacobian)) * BubblesScale);

	NormalRT[id.xy] = float4(normal, 0);
	BubblesRT[id.xy] = float4(jacobian, jacobian, jacobian, 0);
}

//Donelan-Banner������չ
float DonelanBannerDirectionalSpreading(float2 k)
{
	float betaS;
	float omegap = 0.855f * G / length(WindAndSeed.xy);
	float ratio = dispersion(k) / omegap;

	if (ratio < 0.95f)
	{
		betaS = 2.61f * pow(ratio, 1.3f);
	}
	if (ratio >= 0.95f && ratio < 1.6f)
	{
		betaS = 2.28f * pow(ratio, -1.3f);
	}
	if (ratio > 1.6f)
	{
		float epsilon = -0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio));
		betaS = pow(10, epsilon);
	}
	float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);

	return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}
//������ƽ��������չ
float PositiveCosineSquaredDirectionalSpreading(float2 k)
{
	float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);
	if (theta > -PI / 2.0f && theta < PI / 2.0f)
	{
		return 2.0f / PI * pow(cos(theta), 2);
	}
	else
	{
		return 0;
	}
}


//����phillips��
float phillips(float2 k)
{
	float kLength = length(k);
	kLength = max(0.001f, kLength);
	float kLength2 = kLength * kLength;
	float kLength4 = kLength2 * kLength2;

	float windLength = length(WindAndSeed.xy);
	float len = windLength * windLength / G;
	float l2 = len * len;

	float damping = 0.000001f;
	float L2 = l2 * damping;

	//phillips��
	return  A * exp(-1.0f / (kLength2 * l2)) / kLength4 * exp(-kLength2 * L2);
}
//�������
uint wangHash(uint seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}
//������ȷֲ������[0,1)
uint rand(uint rngState)
{
	rngState ^= (rngState << 13);
	rngState ^= (rngState >> 17);
	rngState ^= (rngState << 5);
	return rngState;
}

//�����˹�����
float2 gaussian(float2 id)
{
	//���ȷֲ������
	uint rngState = wangHash(id.y * N + id.x);     //����Ҫ��uint��������int
	rngState = rand(rngState);
	float x1 = rngState / 4294967296.0f;
	rngState = rand(rngState);
	float x2 = rngState / 4294967296.0f;

	x1 = max(1e-6f, x1);
	x2 = max(1e-6f, x2);
	//���������໥�����ĸ�˹�����
	float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
	float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);

	return float2(g1, g2);
}
//������ɢ
float dispersion(float2 k)
{
	return sqrt(G * length(k));
}
//�������
float2 complexMultiply(float2 c1, float2 c2)
{
	return float2(c1.x * c2.x - c1.y * c2.y,
		c1.x * c2.y + c1.y * c2.x);
}
