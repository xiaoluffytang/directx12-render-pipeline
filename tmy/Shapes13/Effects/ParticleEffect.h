#pragma once
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
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
	ParticleEffect(ID3D12Device* device);
	~ParticleEffect();

	XMFLOAT3* pos;    //所有粒子初始位置
	int posCount = 0;   //粒子总数
	
	void InitPos(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList);
	void BuildParticleTex(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* heap, int startIndex, int mCbvSrvUavDescriptorSize);
	void ComputePos(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);
	std::unique_ptr<UploadBuffer<ParticleConstant>> particleData = nullptr;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE uavHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE posUavHandle;
	ID3D12Resource* tex = nullptr;    //；粒子特效信息贴图
	Microsoft::WRL::ComPtr<ID3D12Resource> poss;  //粒子初始位置
	Microsoft::WRL::ComPtr<ID3D12Resource> temp;
	ParticleConstant data;
};

