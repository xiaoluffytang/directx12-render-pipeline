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

// �豸��Ϣ,�����ӿ�
class DeviceCommonInterface
{
public:
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice; //�豸
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;     //�����б�
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 1000;        //��Ļ���
	int mClientHeight = 600;        //��Ļ�߶�
	bool m4xMsaaState = false;    //�Ƿ������ز���
	UINT m4xMsaaQuality = 0;      //���ز���������
	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;
	bool isDebugModel;
	
};
class Manager
{
public:
	Manager(DeviceCommonInterface md3dDevice, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap);
	~Manager();
	GameObjectManager* gameobjectManager;    //��Ϸ���������
	MaterialManager* materialManager;    //���ʹ�����
	SceneManager* sceneManager;     //����������
	TextureManager* textureManager;    //ͼƬ��Դ������
	CameraManager* cameraManager;    //�����������
	PSOManager* psoManager;    //��Ⱦ״̬����
	HeapResourceManager* heapResourceManager;   //����Դ����
	RenderManager* renderManager;    //��Ⱦ������
	FontManager* fontManager;   //���������

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

