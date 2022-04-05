#pragma once
#include "GameObjectManager.h";
#include "MaterialManager.h";
#include "SceneManager.h";
#include "TextureManager.h";
#include "CameraManager.h";
#include "PSOManager.h";
#include "HeapResourceManager.h";
#include "RenderManager.h";
#include "FontManager.h";


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

// 设备信息,公共接口
class DeviceCommonInterface
{
public:
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice; //设备
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;     //命令列表
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 1000;        //屏幕宽度
	int mClientHeight = 600;        //屏幕高度
	bool m4xMsaaState = false;    //是否开启多重采样
	UINT m4xMsaaQuality = 0;      //多重采样的质量
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	bool isDebugModel;
	
};
class Manager
{
public:
	Manager(DeviceCommonInterface md3dDevice, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap);
	~Manager();
	GameObjectManager* gameobjectManager;    //游戏物体管理器
	MaterialManager* materialManager;    //材质管理器
	SceneManager* sceneManager;     //场景管理器
	TextureManager* textureManager;    //图片资源管理器
	CameraManager* cameraManager;    //摄像机管理器
	PSOManager* psoManager;    //渲染状态管理
	HeapResourceManager* heapResourceManager;   //堆资源管理
	RenderManager* renderManager;    //渲染管理器
	FontManager* fontManager;   //字体管理器

	DeviceCommonInterface commonInterface;
	void Init();
	string getPathPrefix(const string& path)
	{
		if (commonInterface.isDebugModel)
		{
			return path;
		}
		else
		{
			return "..\\..\\" + path;
		}
	}
};

