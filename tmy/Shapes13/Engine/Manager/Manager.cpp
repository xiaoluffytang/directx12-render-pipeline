#include "Manager.h"


Manager::Manager(DeviceCommonInterface commonInterface, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap)
{
	this->commonInterface = commonInterface;
	psoManager = new PSOManager();   //��Ⱦ״̬���ȳ�ʼ��
	gameobjectManager = new GameObjectManager();
	materialManager = new MaterialManager();
	textureManager = new TextureManager();
	cameraManager = new CameraManager();
	heapResourceManager = new HeapResourceManager();
	renderManager = new RenderManager();
	fontManager = new FontManager();
	textureManager->mRtvHeap = mRtvHeap;
	textureManager->mDsvHeap = mDsvHeap;
	//sceneManager ����ʼ��
	sceneManager = new SceneManager();
}

void Manager::Init()
{
	psoManager->Init();
	gameobjectManager->Init();
	materialManager->Init();
	textureManager->Init();
	cameraManager->Init();
	heapResourceManager->Init();
	renderManager->Init();
	fontManager->Init();
}

Manager::~Manager()
{
}
