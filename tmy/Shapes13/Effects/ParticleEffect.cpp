#include "ParticleEffect.h"
#include "../Engine/Manager/Manager.h"
extern Manager* manager;


ParticleEffect::ParticleEffect()
{
	particleData = std::make_unique<UploadBuffer<ParticleConstant>>(manager->commonInterface.md3dDevice.Get(), 1, true);
}


ParticleEffect::~ParticleEffect()
{
}

void ParticleEffect::InitPos()
{
	poss = d3dUtil::CreateDefaultBuffer(manager->commonInterface.md3dDevice.Get(), manager->commonInterface.mCommandList.Get(), pos, posCount * sizeof(XMFLOAT3), temp);
}

void ParticleEffect::BuildParticleTex()
{
	int width = 256;
	int height = 256; 
	//创建关键帧资源
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	texture = new Texture2DResource();
	texture->LoadTextureByDesc(&texDesc, nullptr);
	texture->CreateUavIndexDescriptor();
}

void ParticleEffect::ComputePos(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
	float time)
{
	if (posCount <= 0) return;
	data.cycleTime = 10;
	int cycleCount = floor(time / data.cycleTime);
	data.nowTime = time - cycleCount * data.cycleTime;
	data.oneLineCount = 256;
	data.totalCount = posCount;
	particleData->CopyData(0, data);

	command->SetComputeRootSignature(rootSig);
	command->SetComputeRootConstantBufferView(0, particleData->Resource()->GetGPUVirtualAddress());
	command->SetComputeRootDescriptorTable(1, texture->GetUavGpuHandle());
	command->SetComputeRootShaderResourceView(2, poss->GetGPUVirtualAddress());

	std::string pso = (cycleCount % 2 == 0) ? "ParticleDieAndMove1" : "ParticleDieAndMove2";
	command->SetPipelineState(mPsos[pso]);
	command->Dispatch(4, 256, 1);
}