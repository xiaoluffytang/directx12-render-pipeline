#pragma once
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
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
	ParticleEffect(ID3D12Device* device);
	~ParticleEffect();

	XMFLOAT3* pos;    //�������ӳ�ʼλ��
	int posCount = 0;   //��������
	
	void InitPos(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList);
	void BuildParticleTex(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* heap, int startIndex, int mCbvSrvUavDescriptorSize);
	void ComputePos(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);
	std::unique_ptr<UploadBuffer<ParticleConstant>> particleData = nullptr;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE uavHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE posUavHandle;
	ID3D12Resource* tex = nullptr;    //��������Ч��Ϣ��ͼ
	Microsoft::WRL::ComPtr<ID3D12Resource> poss;  //���ӳ�ʼλ��
	Microsoft::WRL::ComPtr<ID3D12Resource> temp;
	ParticleConstant data;
};

