#pragma once
#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Common/FrameResource.h"
#include "../Engine/Texture/Texture2DResource.h"
using namespace DirectX;

struct ParticleConstant
{
	float nowTime;   //ʱ��
	float cycleTime;   //һ�����ڵ�ʱ��
	int oneLineCount;   //�����ж��
	int totalCount;    //�ܹ��ж�������
};

class ParticleEffect
{
public:
	ParticleEffect();
	~ParticleEffect();

	XMFLOAT3* pos;    //�������ӳ�ʼλ��
	int posCount = 0;   //��������
	
	void InitPos();
	void BuildParticleTex();
	void ComputePos(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);
	std::unique_ptr<UploadBuffer<ParticleConstant>> particleData = nullptr;

public:
	Texture2DResource* texture = nullptr;   //������Ч��Ϣ��ͼ
	Microsoft::WRL::ComPtr<ID3D12Resource> poss;  //���ӳ�ʼλ��
	Microsoft::WRL::ComPtr<ID3D12Resource> temp;
	ParticleConstant data;
};

