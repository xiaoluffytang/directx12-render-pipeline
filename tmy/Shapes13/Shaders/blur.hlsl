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
	float2 TexC    : TEXCOORD;
	nointerpolation uint MatIndex  : MATINDEX;
};

//ģ������
float4 BlurFun(uint index, VertexOut pin, bool horizontal)
{
	float4 old = gDiffuseMap[index].Sample(gsamLinearWrap, pin.TexC);
	uint width, height, numMips;
	//�õ�ͼƬ��ȸ߶Ⱥ�mip�㼶
	gDiffuseMap[index].GetDimensions(0, width, height, numMips);
	float x = pin.TexC.x;
	float y = pin.TexC.y;
	float oneWidth = 1.0f / width;
	float weights[] = { 0.0545f,0.2442f,0.4026f,0.2442,0.0545f }; 
	float4 renderTex = float4(0, 0, 0, 0);
	for (int i = 0; i < 5; i++)
	{
		float tx = x + (i - 2) * oneWidth * blurRadius;
		float ty = y;
		float2 texC = float2(tx,ty);
		float4 col = gDiffuseMap[index].Sample(gsamLinearWrap, texC);
		renderTex += col * weights[i];
	}
	return renderTex;
}

//ģ������
float4 BlurFun2(uint index, VertexOut pin, bool horizontal)
{
	float4 old = gDiffuseMap[index].Sample(gsamLinearWrap, pin.TexC);
	uint width, height, numMips;
	//�õ�ͼƬ��ȸ߶Ⱥ�mip�㼶
	gDiffuseMap[index].GetDimensions(0, width, height, numMips);
	float x = pin.TexC.x;
	float y = pin.TexC.y;
	double oneHeight = 1.0f / height;
	float weights[] = { 0.0545f,0.2442f,0.4026f,0.2442,0.0545f };
	float4 renderTex = float4(0, 0, 0, 0);
	for (int i = 0; i < 5; i++)
	{
		float tx = x;
		float ty = y + (i - 2) * oneHeight * blurRadius;
		float2 texC = float2(tx, ty);
		float4 col = gDiffuseMap[index].Sample(gsamLinearWrap, texC);
		renderTex += col * weights[i];
	}
	return renderTex;
}

//��˹ģ��ˮƽ���򣨵�һ�Σ�
float4 BlurHorizontalFirstPS(VertexOut pin) : SV_Target
{
	return BlurFun(blurIndex,pin,true);
}

//��˹����ֱ���򣨵�һ�Σ�
float4 BlurVerticalFirstPS(VertexOut pin) : SV_Target
{
	return BlurFun2(3,pin,false);
}

//��˹ģ��ˮƽ���򣨵�n�Σ�n > 1��
float4 BlurHorizontalPS(VertexOut pin) : SV_Target
{
	return BlurFun(4,pin,true);
}

//��˹����ֱ���򣨵�n�Σ�n > 1��
float4 BlurVerticalPS(VertexOut pin) : SV_Target
{
	return BlurFun2(3,pin,false);
}


//���Ƹ�˹ģ�����
float4 BlurCopyPS(VertexOut pin) : SV_Target
{
	float4 renderTex = gDiffuseMap[3].Sample(gsamLinearWrap, pin.TexC);
	return renderTex;
}