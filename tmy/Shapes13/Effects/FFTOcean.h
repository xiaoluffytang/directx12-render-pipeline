#pragma once
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

struct OceanData
{
	int N;					//fft纹理大小
	float OceanLength;		//海洋长度
	float A;				//phillips谱参数，影响波浪高度
	float Time;				//时间
	float Lambda;			//偏移影响
	float HeightScale;		//高度影响
	float BubblesScale;	    //泡沫强度
	float BubblesThreshold; //泡沫阈值
	XMFLOAT2 WindAndSeed;		//风和随机种子 xy为风, zw为两个随机种子
};

struct OceanRenderConstant
{
	int displaceIndex;   //偏移频谱的纹理索引
	int normalIndex;     //法线频谱的纹理索引
	int buddlesIndex;    //泡沫频谱的纹理索引
};

class FFTOcean
{
public:
	FFTOcean(ID3D12Device* device);
	~FFTOcean();

	float OceanLength = 512;  //海洋有多大
	int FFTPow = 9;     //生成海洋纹理的大小，2的n次方
	float A = 60;     //philips谱参数，影响波浪的高度
	float lambda = 8;   //用来控制偏移大小
	float HeightScale = 28.8;   //高度影响
	float BubblesScale = 1.5;  //泡沫强度
	float BubblesThreshold = 1;   //泡沫阈值
	float WindScale = 30;    //风强
	float TimeScale = 2;    //时间影响
	DirectX::XMFLOAT4 WindAndSeed = { 1, 2, 0, 0 };  //风向和随机种子 xy为风, zw为两个随机种子
	int ControlM = 12;    //控制m，控制FFT变换阶段
	bool isControlH = true;    //是否控制横向FFT,否则控制纵向FFT

	void BuildDescriptors(
		ID3D12DescriptorHeap* heap,
		int index,int mCbvSrvUavDescriptorSize);

	void ComputeOcean(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);

public:
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuSrvs;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuUavs;
	ID3D12Device* md3dDevice = nullptr;
	int fftSize;    //fft纹理大小 = pow(2,FFTPow)
	float computeCount;
	float time = 0;    //时间
	
	std::vector<ID3D12Resource*> resources;
	BYTE* _pTexBuf;
	std::unique_ptr<UploadBuffer<OceanData>> oceanData = nullptr;
	std::unique_ptr<UploadBuffer<OceanRenderConstant>> oceanRenderIndexs = nullptr;

	void BuildResources();
	void BuildOneResource(D3D12_RESOURCE_DESC& texDesc);
	bool hasInit = false;

	void setOceanData(ID3D12GraphicsCommandList* command);
	void setFFTData(ID3D12GraphicsCommandList* command);

	const int texCount = 5;    //总共有几张纹理(0是高斯随机，其他的图片不固定)
	int heightSpectrumRTIndex = 1;    //高度频谱资源索引
	int DisplaceXSpectrumRTIndex = 2;   //X偏移频谱索引
	int DisplaceZSpectrumRTIndex = 3;   //Z偏移频谱索引
	int inputRTIndex = 0;    //输入的资源索引
	int outputRTIndex = 4;   //临时输出资源索引 
	int startIndex = 0;    //海洋的描述符地址是从第几个开始的
	int ns;  //Ns = pow(2,m-1); m为第几阶段
	void ComputeFFT(ID3D12GraphicsCommandList* command, ID3D12PipelineState* state, int& input);
	void SetOceanTextureIndex(ID3D12GraphicsCommandList* command, int rootSignatureParamIndex);


	wstring StringToWString(const std::string& str)
	{
		int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t *wide = new wchar_t[num];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
		std::wstring w_str(wide);
		delete[] wide;
		return w_str;
	}
};

