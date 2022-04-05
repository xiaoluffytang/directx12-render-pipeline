#include "CameraManager.h"
#include "Manager.h"
extern Manager* manager;

CameraManager::CameraManager()
{
	
}


CameraManager::~CameraManager()
{
}

void CameraManager::Init()
{
	mainCamera = new Camera();
	mainCamera->orthogonal = false;
	mainCamera->SetPosition(0.0f, 10, -40.0f);
	mainCamera->renderTargetSize = XMFLOAT2((float)manager->commonInterface.mClientWidth, (float)manager->commonInterface.mClientHeight);
	mainCamera->invRenderTargetSize = XMFLOAT2(1.0f / manager->commonInterface.mClientWidth, 1.0f / manager->commonInterface.mClientHeight);

	shadowCamera = new Camera();
	shadowCamera->orthogonal = true;
	shadowCamera->size = 100;
	shadowCamera->LookAt(XMFLOAT3(35, 35, 25), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 1.0f, 0));
	shadowCamera->SetLens(0, 1, 5, 105.0f);
	shadowCamera->UpdateViewMatrix();
	shadowCamera->renderTargetSize = XMFLOAT2(manager->textureManager->shadowMap->Width(), manager->textureManager->shadowMap->Height());
	shadowCamera->invRenderTargetSize = XMFLOAT2(1.0f / manager->textureManager->shadowMap->Width(), 1.0f / manager->textureManager->shadowMap->Height());

	uiCamera = new Camera();
	uiCamera->orthogonal = true;
	uiCamera->isLeftUCS = false;
	uiCamera->SetPosition(0.0f, 0, 10);
	uiCamera->size = manager->commonInterface.mClientWidth;
	uiCamera->SetLens(0, manager->commonInterface.mClientWidth / manager->commonInterface.mClientHeight, 1, manager->commonInterface.mClientWidth);
	uiCamera->UpdateViewMatrix();
	uiCamera->renderTargetSize = XMFLOAT2((float)manager->commonInterface.mClientWidth, (float)manager->commonInterface.mClientHeight);
	uiCamera->invRenderTargetSize = XMFLOAT2(1.0f / manager->commonInterface.mClientWidth, 1.0f / manager->commonInterface.mClientHeight);
}
