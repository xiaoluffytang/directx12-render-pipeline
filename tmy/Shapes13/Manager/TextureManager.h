
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

	ShadowMap* shadowMap = nullptr;  //阴影贴图
	ScreenPostMap* bloomMap = nullptr;    //bloom特效
	ScreenPostMap* screenPostMap = nullptr;    //屏幕后处理贴图
	Ssao* ssao = nullptr;    //屏幕环境光遮蔽
	Blur* blur = nullptr;   //高斯模糊
	CubeRenderTarget* cubeTarget = nullptr;   //环境映射
	FFTOcean* fftOcean = nullptr;   //海洋
	SkinnedData* skin = nullptr;    //蒙皮动画
	ParticleEffect* particle = nullptr;   //粒子特效

	int setAside = 100;    //预留一部分位置，用于存放一些特效贴图  （第一个是阴影贴图的描述符，第二个是后台缓冲区的描述符）
	std::vector<Texture*> mDiffuseTextures;     //漫反射贴图数组
	std::unordered_map<string, Texture*> mDiffuseTexturesDic;    //漫反射贴图字典
	int LoadTexture(string path);
	Manager* manager;
private:
	wstring StringToWString(const std::string & str);
};

