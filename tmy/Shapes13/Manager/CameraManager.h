#pragma once
#include "../../../Common/Camera.h"
class Manager;
class CameraManager
{
public:
	CameraManager(Manager* manager);
	~CameraManager();
	Manager* manager;
	Camera* mainCamera;
	Camera* uiCamera;
};

