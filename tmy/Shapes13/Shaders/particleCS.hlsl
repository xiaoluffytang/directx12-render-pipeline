cbuffer cbSettings : register(b0)
{
	float nowTime;   //时刻
	float cycleTime;   //一个周期的时间
	int oneLineCount;   //纹理有多宽
	int totalCount;    //总共有多少粒子
}

RWTexture2D<float4> gOutPut : register(u0);
StructuredBuffer<float3> datas : register(t0);

[numthreads(64, 1, 1)]
void ParticleDieAndMove1(uint3 id: SV_DispatchThreadID)
{
	int index = id.y * oneLineCount + id.x;
	if (index >= totalCount)
	{
		return;
	}
	
	float3 pos = datas[index];
	float dieSpeedY = 1;
	float beginDieY = 0;

	float nowDieY = beginDieY + dieSpeedY * nowTime;
	float dieTime = (nowDieY - pos.y) / dieSpeedY;
	dieTime = clamp(dieTime,0, cycleTime);
	float moveX = 8 * dieTime;
	moveX = clamp(moveX,0, 10);
	gOutPut[id.xy] = float4(moveX,0, 0, 1);
}

[numthreads(64, 1, 1)]
void ParticleDieAndMove2(uint3 id: SV_DispatchThreadID)
{
	int index = id.y * oneLineCount + id.x;
	if (index >= totalCount)
	{
		return;
	}

	float3 pos = datas[index];
	float dieSpeedY = 1;
	float beginDieY = 0;

	float nowDieY = beginDieY + dieSpeedY * nowTime;
	float dieTime = (nowDieY - pos.y) / dieSpeedY;
	dieTime = clamp(dieTime, 0, cycleTime);
	float moveX = -8 * dieTime;
	moveX = clamp(moveX, -10, 0);
	gOutPut[id.xy] = float4(10 + moveX, 0, 0, 1);
}