
#include "../FrameResource.h"
#include "../Effects/ShadowMap.h"
#include "../Effects/ScreenPostMap.h"
#include "../Effects/Ssao.h"
#include "../Effects/Blur.h"
#include "../Effects/CubeRenderTarget.h"
#include "../Effects/FFTOcean.h"
#include "../Animator/SkinnedData.h"
#include "../Effects/ParticleEffect.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

#pragma once
class Manager;
class TextureManager
{
public:
	TextureManager(Manager* manager);
	~TextureManager();

	ShadowMap* shadowMap = nullptr;  //��Ӱ��ͼ
	ScreenPostMap* bloomMap = nullptr;    //bloom��Ч
	ScreenPostMap* screenPostMap = nullptr;    //��Ļ������ͼ
	Ssao* ssao = nullptr;    //��Ļ�������ڱ�
	Blur* blur = nullptr;   //��˹ģ��
	CubeRenderTarget* cubeTarget = nullptr;   //����ӳ��
	FFTOcean* fftOcean = nullptr;   //����
	SkinnedData* skin = nullptr;    //��Ƥ����
	ParticleEffect* particle = nullptr;   //������Ч

	int setAside = 100;    //Ԥ��һ����λ�ã����ڴ��һЩ��Ч��ͼ  ����һ������Ӱ��ͼ�����������ڶ����Ǻ�̨����������������
	std::vector<Texture*> mDiffuseTextures;     //��������ͼ����
	std::unordered_map<string, Texture*> mDiffuseTexturesDic;    //��������ͼ�ֵ�
	int LoadTexture(string path);
	Manager* manager;
private:
	wstring StringToWString(const std::string & str);
};

