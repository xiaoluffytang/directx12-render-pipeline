#include "../FrameResource.h"
#pragma once
class Component
{
public:
	Component();
	~Component();
	const std::string componentName = "Component";
	void Start();
	void OnEnable();
	/*void Update(GameTimer& gt);*/
	void OnDisable();
	void OnDestroy();
};

