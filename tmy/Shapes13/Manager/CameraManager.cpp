#include "CameraManager.h"
#include "Manager.h"


CameraManager::CameraManager(Manager* manager)
{
	this->manager = manager;
	mainCamera = new Camera();
	mainCamera->orthogonal = false;
	mainCamera->SetPosition(0.0f, 10, -40.0f);

	uiCamera = new Camera();
	uiCamera->orthogonal = true;
	uiCamera->SetPosition(0.0f, 0, 10);
	uiCamera->size = manager->commonInterface.mClientWidth;
	uiCamera->SetLens(0, manager->commonInterface.mClientWidth / manager->commonInterface.mClientHeight, 1, manager->commonInterface.mClientWidth);
	uiCamera->UpdateViewMatrix();
}


CameraManager::~CameraManager()
{
}
