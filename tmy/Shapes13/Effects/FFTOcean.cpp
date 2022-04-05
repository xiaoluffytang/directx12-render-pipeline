#include "FFTOcean.h"
#include "../Engine/Manager/Manager.h"
extern Manager* manager;

FFTOcean::FFTOcean()
{
	//��������
	oceanData = std::make_unique<UploadBuffer<OceanData>>(manager->commonInterface.md3dDevice.Get(), 1, true);
	oceanRenderIndexs = std::make_unique<UploadBuffer<OceanRenderConstant>>(manager->commonInterface.md3dDevice.Get(), 1, true);
}


FFTOcean::~FFTOcean()
{
}

void FFTOcean::BuildDescriptors()
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
		Texture2DResource* texture = new Texture2DResource();
		texture->LoadTextureByDesc(&texDesc, nullptr);
		texture->CreateUavIndexDescriptor();
		textures.push_back(texture);
		mhGpuSrvs.push_back(texture->GetSrvGpuHandle());
		mhGpuUavs.push_back(texture->GetUavGpuHandle());
	}
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
	data.displaceIndex = textures[outputRTIndex]->srvIndex;
	data.normalIndex = textures[heightSpectrumRTIndex]->srvIndex;
	data.buddlesIndex = textures[DisplaceXSpectrumRTIndex]->srvIndex;

	oceanRenderIndexs->CopyData(0, data);
	command->SetGraphicsRootConstantBufferView(rootSignatureParamIndex, oceanRenderIndexs->Resource()->GetGPUVirtualAddress());
}
