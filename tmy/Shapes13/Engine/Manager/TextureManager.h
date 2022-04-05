
#include "../Common/FrameResource.h"
#include "../../Effects/ShadowMap.h"
#include "../../Effects/ScreenPostMap.h"
#include "../../Effects/Blur.h"
#include "../../Effects/CubeRenderTarget.h"
#include "../../Effects/FFTOcean.h"
#include "../Animator/SkinnedData.h"
#include "../../Effects/ParticleEffect.h"
#include "../../Engine/EngineUtils.h"
#include "../../Engine/Texture/Texture2DResource.h"
#include "../../Engine/Texture/Texture3DResource.h"
#include "../../Engine/Global.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

#pragma once
class TextureManager
{
public:
	TextureManager();
	~TextureManager();
	void Init();

	ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;    //ͼƬ��Դ��������
	ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;    //
	ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;    //

	Texture2DResource* depthStencil = nullptr;
	ShadowMap* shadowMap = nullptr;  //��Ӱ��ͼ
	ScreenPostMap* bloomMap = nullptr;    //bloom��Ч
	ScreenPostMap* screenPostMap = nullptr;    //��Ļ������ͼ
	Blur* blur = nullptr;   //��˹ģ��
	CubeRenderTarget* cubeTarget = nullptr;   //����ӳ��
	FFTOcean* fftOcean = nullptr;   //����
	SkinnedData* skin = nullptr;    //��Ƥ����
	ParticleEffect* particle = nullptr;   //������Ч
	UINT mCbvSrvUavDescriptorSize = 0;
	int setAside = 0;    //Ԥ��һ����λ�ã����ڴ��һЩ��Ч��ͼ  ����һ������Ӱ��ͼ�����������ڶ����Ǻ�̨����������������
	
	int LoadTexture2D(string path);
	int LoadTexture3D(string path);
	int GetSrvDescriptorIndex();
	int GetRtvDescriptorIndex();
	int GetDsvDescriptorIndex();
	void DeleteSrvDescriptorIndex(int index);   //ɾ��һЩ�������ѵ�λ��
	void DeleteRtvDescriptorIndex(int index);   //ɾ��һЩ�������ѵ�λ��
	void DeleteDsvDescriptorIndex(int index);   //ɾ��һЩ�������ѵ�λ��
	Texture2DResource* getTexture(string path);
	void CreateDepthStencil();
private:
	std::unordered_map<string, Texture2DResource*> dics;
	int srvDesctiprotCount = 0;    //��Ⱦ��Դ����������
	std::vector<int> nullSrvDescriptorDic;    //һЩ�յ��������ѵ�λ��
	int rtvDesctiprotCount = 0;    //��Ⱦ��Դ����������
	int firstRtvAside = SwapChainBufferCount;
	std::vector<int> nullRtvDescriptorDic;    //һЩ�յ��������ѵ�λ��
	int dsvDesctiprotCount = 0;    //��Ⱦ��Դ����������
	int firstDsvAside = 0;
	std::vector<int> nullDsvDescriptorDic;    //һЩ�յ��������ѵ�λ��
};
