
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

	ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;    //图片资源描述符堆
	ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;    //
	ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;    //

	Texture2DResource* depthStencil = nullptr;
	ShadowMap* shadowMap = nullptr;  //阴影贴图
	ScreenPostMap* bloomMap = nullptr;    //bloom特效
	ScreenPostMap* screenPostMap = nullptr;    //屏幕后处理贴图
	Blur* blur = nullptr;   //高斯模糊
	CubeRenderTarget* cubeTarget = nullptr;   //环境映射
	FFTOcean* fftOcean = nullptr;   //海洋
	SkinnedData* skin = nullptr;    //蒙皮动画
	ParticleEffect* particle = nullptr;   //粒子特效
	UINT mCbvSrvUavDescriptorSize = 0;
	int setAside = 0;    //预留一部分位置，用于存放一些特效贴图  （第一个是阴影贴图的描述符，第二个是后台缓冲区的描述符）
	
	int LoadTexture2D(string path);
	int LoadTexture3D(string path);
	int GetSrvDescriptorIndex();
	int GetRtvDescriptorIndex();
	int GetDsvDescriptorIndex();
	void DeleteSrvDescriptorIndex(int index);   //删除一些描述符堆的位置
	void DeleteRtvDescriptorIndex(int index);   //删除一些描述符堆的位置
	void DeleteDsvDescriptorIndex(int index);   //删除一些描述符堆的位置
	Texture2DResource* getTexture(string path);
	void CreateDepthStencil();
private:
	std::unordered_map<string, Texture2DResource*> dics;
	int srvDesctiprotCount = 0;    //渲染资源描述符数量
	std::vector<int> nullSrvDescriptorDic;    //一些空的描述符堆的位置
	int rtvDesctiprotCount = 0;    //渲染资源描述符数量
	int firstRtvAside = SwapChainBufferCount;
	std::vector<int> nullRtvDescriptorDic;    //一些空的描述符堆的位置
	int dsvDesctiprotCount = 0;    //渲染资源描述符数量
	int firstDsvAside = 0;
	std::vector<int> nullDsvDescriptorDic;    //一些空的描述符堆的位置
};
