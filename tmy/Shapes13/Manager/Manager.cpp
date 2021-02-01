#include "Manager.h"


Manager::Manager(DeviceCommonInterface commonInterface)
{
	this->commonInterface = commonInterface;
	psoManager = new PSOManager(this);   //渲染状态最先初始化
	gameobjectManager = new GameObjectManager(this);
	materialManager = new MaterialManager(this);
	textureManager = new TextureManager(this);
	cameraManager = new CameraManager(this);
	heapResourceManager = new HeapResourceManager(this);
	renderManager = new RenderManager(this);
	fontManager = new FontManager(this);
	
	//sceneManager 最后初始化
	sceneManager = new SceneManager(this);
}


Manager::~Manager()
{
}
