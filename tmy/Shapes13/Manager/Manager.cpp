#include "Manager.h"


Manager::Manager(DeviceCommonInterface commonInterface)
{
	this->commonInterface = commonInterface;
	psoManager = new PSOManager(this);   //��Ⱦ״̬���ȳ�ʼ��
	gameobjectManager = new GameObjectManager(this);
	materialManager = new MaterialManager(this);
	textureManager = new TextureManager(this);
	cameraManager = new CameraManager(this);
	heapResourceManager = new HeapResourceManager(this);
	renderManager = new RenderManager(this);
	fontManager = new FontManager(this);
	
	//sceneManager ����ʼ��
	sceneManager = new SceneManager(this);
}


Manager::~Manager()
{
}
