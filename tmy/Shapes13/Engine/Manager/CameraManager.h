#pragma once
#include "../Common/Camera.h"
class CameraManager
{
public:
	CameraManager();
	~CameraManager();
	Camera* mainCamera;
	Camera* shadowCamera;
	Camera* uiCamera;
	void Init();
};

