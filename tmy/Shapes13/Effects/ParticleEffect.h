#pragma once
#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Common/FrameResource.h"
#include "../Engine/Texture/Texture2DResource.h"
using namespace DirectX;

struct ParticleConstant
{
	float nowTime;   //时刻
	float cycleTime;   //一个周期的时间
	int oneLineCount;   //纹理有多宽
	int totalCount;    //总共有多少粒子
};

class ParticleEffect
{
public:
	ParticleEffect();
	~ParticleEffect();

	XMFLOAT3* pos;    //所有粒子初始位置
	int posCount = 0;   //粒子总数
	
	void InitPos();
	void BuildParticleTex();
	void ComputePos(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);
	std::unique_ptr<UploadBuffer<ParticleConstant>> particleData = nullptr;

public:
	Texture2DResource* texture = nullptr;   //粒子特效信息贴图
	Microsoft::WRL::ComPtr<ID3D12Resource> poss;  //粒子初始位置
	Microsoft::WRL::ComPtr<ID3D12Resource> temp;
	ParticleConstant data;
};

