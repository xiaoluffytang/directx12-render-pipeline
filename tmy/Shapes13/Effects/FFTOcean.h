#pragma once
#include "../../../Common/d3dUtil.h"
#include "../FrameResource.h"
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

struct OceanData
{
	int N;					//fft�����С
	float OceanLength;		//���󳤶�
	float A;				//phillips�ײ�����Ӱ�첨�˸߶�
	float Time;				//ʱ��
	float Lambda;			//ƫ��Ӱ��
	float HeightScale;		//�߶�Ӱ��
	float BubblesScale;	    //��ĭǿ��
	float BubblesThreshold; //��ĭ��ֵ
	XMFLOAT2 WindAndSeed;		//���������� xyΪ��, zwΪ�����������
};

struct OceanRenderConstant
{
	int displaceIndex;   //ƫ��Ƶ�׵���������
	int normalIndex;     //����Ƶ�׵���������
	int buddlesIndex;    //��ĭƵ�׵���������
};

class FFTOcean
{
public:
	FFTOcean(ID3D12Device* device);
	~FFTOcean();

	float OceanLength = 512;  //�����ж��
	int FFTPow = 9;     //���ɺ�������Ĵ�С��2��n�η�
	float A = 60;     //philips�ײ�����Ӱ�첨�˵ĸ߶�
	float lambda = 8;   //��������ƫ�ƴ�С
	float HeightScale = 28.8;   //�߶�Ӱ��
	float BubblesScale = 1.5;  //��ĭǿ��
	float BubblesThreshold = 1;   //��ĭ��ֵ
	float WindScale = 30;    //��ǿ
	float TimeScale = 2;    //ʱ��Ӱ��
	DirectX::XMFLOAT4 WindAndSeed = { 1, 2, 0, 0 };  //������������ xyΪ��, zwΪ�����������
	int ControlM = 12;    //����m������FFT�任�׶�
	bool isControlH = true;    //�Ƿ���ƺ���FFT,�����������FFT

	void BuildDescriptors(
		ID3D12DescriptorHeap* heap,
		int index,int mCbvSrvUavDescriptorSize);

	void ComputeOcean(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);

public:
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuSrvs;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuUavs;
	ID3D12Device* md3dDevice = nullptr;
	int fftSize;    //fft�����С = pow(2,FFTPow)
	float computeCount;
	float time = 0;    //ʱ��
	
	std::vector<ID3D12Resource*> resources;
	BYTE* _pTexBuf;
	std::unique_ptr<UploadBuffer<OceanData>> oceanData = nullptr;
	std::unique_ptr<UploadBuffer<OceanRenderConstant>> oceanRenderIndexs = nullptr;

	void BuildResources();
	void BuildOneResource(D3D12_RESOURCE_DESC& texDesc);
	bool hasInit = false;

	void setOceanData(ID3D12GraphicsCommandList* command);
	void setFFTData(ID3D12GraphicsCommandList* command);

	const int texCount = 5;    //�ܹ��м�������(0�Ǹ�˹�����������ͼƬ���̶�)
	int heightSpectrumRTIndex = 1;    //�߶�Ƶ����Դ����
	int DisplaceXSpectrumRTIndex = 2;   //Xƫ��Ƶ������
	int DisplaceZSpectrumRTIndex = 3;   //Zƫ��Ƶ������
	int inputRTIndex = 0;    //�������Դ����
	int outputRTIndex = 4;   //��ʱ�����Դ���� 
	int startIndex = 0;    //�������������ַ�Ǵӵڼ�����ʼ��
	int ns;  //Ns = pow(2,m-1); mΪ�ڼ��׶�
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

