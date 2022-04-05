#pragma once
#include "../Engine/Common/d3dUtil.h"
#include "../Engine/Common/FrameResource.h"
#include "../Engine/Texture/Texture2DResource.h"
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
	FFTOcean();
	~FFTOcean();

	float OceanLength = 1024;  //�����ж��
	int FFTPow = 10;     //���ɺ�������Ĵ�С��2��n�η�
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

	void BuildDescriptors();

	void ComputeOcean(ID3D12GraphicsCommandList* command, ID3D12RootSignature* rootSig, std::unordered_map<std::string, ID3D12PipelineState*> mPsos,
		float time);

public:
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuSrvs;
	std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> mhGpuUavs;

	int fftSize;    //fft�����С = pow(2,FFTPow)
	float computeCount;
	float time = 0;    //ʱ��

	std::vector<Texture2DResource*> textures;
	BYTE* _pTexBuf;
	std::unique_ptr<UploadBuffer<OceanData>> oceanData = nullptr;
	std::unique_ptr<UploadBuffer<OceanRenderConstant>> oceanRenderIndexs = nullptr;
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
};

