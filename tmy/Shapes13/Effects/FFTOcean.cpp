#include "FFTOcean.h"


FFTOcean::FFTOcean(ID3D12Device* device)
{
	md3dDevice = device;
	BuildResources();

	//��������
	oceanData = std::make_unique<UploadBuffer<OceanData>>(device, 1, true);
	oceanRenderIndexs = std::make_unique<UploadBuffer<OceanRenderConstant>>(device, 1, true);
}


FFTOcean::~FFTOcean()
{
}

void FFTOcean::BuildDescriptors(ID3D12DescriptorHeap* heap,
	int index, int descriptorSize)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	int srvIndex = index;
	int uavIndex = index + texCount;
	for (int i = 0; i < texCount; i++)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrv(heap->GetCPUDescriptorHandleForHeapStart(), srvIndex, descriptorSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrv(heap->GetGPUDescriptorHandleForHeapStart(), srvIndex++, descriptorSize);
		mhGpuSrvs.push_back(gpuSrv);
		md3dDevice->CreateShaderResourceView(resources[i], &srvDesc, cpuSrv);
		
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuUav(heap->GetCPUDescriptorHandleForHeapStart(), uavIndex, descriptorSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuUav(heap->GetGPUDescriptorHandleForHeapStart(), uavIndex++, descriptorSize);
		mhGpuUavs.push_back(gpuUav);
		md3dDevice->CreateUnorderedAccessView(resources[i], nullptr, &uavDesc, cpuUav);
	}
	this->startIndex = index;
}

//����������Դ
void FFTOcean::BuildResources()
{
	fftSize = pow(2, FFTPow);
	computeCount = fftSize / 8;
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = fftSize;
	texDesc.Height = fftSize;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 2;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	for (int i = 0; i < texCount; i++)
	{
		BuildOneResource(texDesc);
	}
}

void FFTOcean::BuildOneResource(D3D12_RESOURCE_DESC& texDesc)
{
	ID3D12Resource* r;
	resources.push_back(r);
	int index = resources.size() - 1;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&resources[index])));
}


void FFTOcean::ComputeOcean(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
	float time)
{
	heightSpectrumRTIndex = 1;
	DisplaceXSpectrumRTIndex = 2;
	DisplaceZSpectrumRTIndex = 3;
	outputRTIndex = 4;

	this->time = time * TimeScale;
	command->SetComputeRootSignature(rootSig);
	//ע����������ĵ���ʱ����Ҫ��mCommandList->SetDescriptorHeaps����֮����ܵ���
	
	setOceanData(command);
	command->SetComputeRootDescriptorTable(0, mhGpuUavs[0]);
	//���ɸ�˹���������ֻ�����һ�Σ�
	if (!hasInit)
	{
		hasInit = true;
		command->SetPipelineState(mPsos["ComputeGaussianRandom"]);
		command->Dispatch(computeCount, computeCount, 1);
	}
	
	//���ɸ߶�Ƶ�׺�ƫ��Ƶ��
	command->SetComputeRootDescriptorTable(1, mhGpuUavs[heightSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(2, mhGpuUavs[DisplaceXSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(3, mhGpuUavs[DisplaceZSpectrumRTIndex]);
	command->SetPipelineState(mPsos["CreateHeightDisplaceSpectrum"]);
	command->Dispatch(computeCount, computeCount, 1);

	//����FFT
	for (int i = 1; i <= FFTPow; i++)
	{
		ns = pow(2, i - 1);
		setFFTData(command);
		if (i != FFTPow)
		{
			ComputeFFT(command, mPsos["FFTHorizontal"], heightSpectrumRTIndex);  
			ComputeFFT(command, mPsos["FFTHorizontal"], DisplaceXSpectrumRTIndex); 			
			ComputeFFT(command, mPsos["FFTHorizontal"], DisplaceZSpectrumRTIndex); 		
		}
		else
		{
			ComputeFFT(command, mPsos["FFTHorizontalEnd"], heightSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTHorizontalEnd"], DisplaceXSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTHorizontalEnd"], DisplaceZSpectrumRTIndex);
		}
	}
	//����FFT
	for (int i = 1; i <= FFTPow; i++)
	{
		ns = pow(2, i - 1);
		setFFTData(command);
		if (i != FFTPow)
		{
			ComputeFFT(command, mPsos["FFTVertical"], heightSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTVertical"], DisplaceXSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTVertical"], DisplaceZSpectrumRTIndex);
		}
		else
		{
			ComputeFFT(command, mPsos["FFTVerticalEnd"], heightSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTVerticalEnd"], DisplaceXSpectrumRTIndex);
			ComputeFFT(command, mPsos["FFTVerticalEnd"], DisplaceZSpectrumRTIndex);
		}
	}

	//��������ƫ��
	command->SetComputeRootDescriptorTable(1, mhGpuUavs[heightSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(2, mhGpuUavs[DisplaceXSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(3, mhGpuUavs[DisplaceZSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(5, mhGpuUavs[outputRTIndex]);
	command->SetPipelineState(mPsos["TextureGenerationDisplace"]);
	command->Dispatch(computeCount, computeCount, 1);

	//���㷨�ߺ���ĭ����
	command->SetComputeRootDescriptorTable(6, mhGpuUavs[heightSpectrumRTIndex]);
	command->SetComputeRootDescriptorTable(7, mhGpuUavs[DisplaceXSpectrumRTIndex]);
	command->SetPipelineState(mPsos["TextureGenerationNormalBubbles"]);
	command->Dispatch(computeCount, computeCount, 1);

	SetOceanTextureIndex(command, 7);
}

void FFTOcean::ComputeFFT(ID3D12GraphicsCommandList* command, ID3D12PipelineState* state, int& input)
{
	//1234����ͼ������Ϊ������������
	command->SetComputeRootDescriptorTable(8, mhGpuUavs[input]);
	command->SetComputeRootDescriptorTable(4, mhGpuUavs[outputRTIndex]);
	command->SetPipelineState(state);
	command->Dispatch(computeCount, computeCount, 1);

	int temp = input;
	input = outputRTIndex;
	outputRTIndex = temp;
}

//���ú���������
void FFTOcean::setOceanData(ID3D12GraphicsCommandList* command)
{
	OceanData data;
	data.N = fftSize;
	data.OceanLength = OceanLength;
	data.A = A;
	//���÷�ķ�����������
	XMFLOAT3 d;
	XMStoreFloat3(&d, XMVector3Normalize(XMVectorSet(1, 1, 0, 0)));
	data.WindAndSeed = XMFLOAT2(ceil(d.x * WindScale),ceil(d.y * WindScale));

	data.Time = time;
	data.Lambda = lambda;
	data.HeightScale = HeightScale;
	data.BubblesScale = BubblesScale;
	data.BubblesThreshold = BubblesThreshold;

	oceanData->CopyData(0, data);
	command->SetComputeRootConstantBufferView(9, oceanData->Resource()->GetGPUVirtualAddress());
}

//����FFT����
void FFTOcean::setFFTData(ID3D12GraphicsCommandList* command)
{	
	//ֻ����SetComputeRoot32BitConstants��������SetComputeRootConstantBufferView,��ΪSetComputeRootConstantBufferView���ó���������ʱ������GPU��ֵ���ܻᱻ����
	command->SetComputeRoot32BitConstants(10, 1, &ns, 0);
}

//���ú���ļ������������
void FFTOcean::SetOceanTextureIndex(ID3D12GraphicsCommandList* command,int rootSignatureParamIndex)
{
	OceanRenderConstant data;
	data.displaceIndex = startIndex + outputRTIndex;
	data.normalIndex = startIndex + heightSpectrumRTIndex;
	data.buddlesIndex = startIndex + DisplaceXSpectrumRTIndex;

	oceanRenderIndexs->CopyData(0, data);
	command->SetGraphicsRootConstantBufferView(rootSignatureParamIndex, oceanRenderIndexs->Resource()->GetGPUVirtualAddress());
}
