#include "ParticleEffect.h"



ParticleEffect::ParticleEffect(ID3D12Device* device)
{
	particleData = std::make_unique<UploadBuffer<ParticleConstant>>(device, 1, true);
}


ParticleEffect::~ParticleEffect()
{
}

void ParticleEffect::InitPos(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList)
{
	poss = d3dUtil::CreateDefaultBuffer(md3dDevice, commandList, pos, posCount * sizeof(XMFLOAT3), temp);
}

void ParticleEffect::BuildParticleTex(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList
	, ID3D12DescriptorHeap* heap, int startIndex, int mCbvSrvUavDescriptorSize)
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
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&tex)));
	//设置描述符
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle(heap->GetCPUDescriptorHandleForHeapStart(), startIndex, mCbvSrvUavDescriptorSize);
	this->srvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), startIndex, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuUavHandle(heap->GetCPUDescriptorHandleForHeapStart(), startIndex + 1, mCbvSrvUavDescriptorSize);
	this->uavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), startIndex + 1, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE posCpuUavHandle(heap->GetCPUDescriptorHandleForHeapStart(), startIndex + 2, mCbvSrvUavDescriptorSize);
	this->posUavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), startIndex + 2, mCbvSrvUavDescriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(tex, &srvDesc, cpuSrvHandle);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateUnorderedAccessView(tex, nullptr, &uavDesc, cpuUavHandle);
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
	command->SetComputeRootDescriptorTable(1, uavHandle);
	command->SetComputeRootShaderResourceView(2, poss->GetGPUVirtualAddress());

	std::string pso = (cycleCount % 2 == 0) ? "ParticleDieAndMove1" : "ParticleDieAndMove2";
	command->SetPipelineState(mPsos[pso]);
	command->Dispatch(4, 256, 1);
}