#include "FontManager.h"
#include "../Manager/Manager.h"


FontManager::FontManager(Manager* manager)
{
	this->manager = manager;
	font = new Font(manager->commonInterface.md3dDevice.Get(), manager->commonInterface.mCommandList.Get(), "../../font/simhei.ttf");
}


FontManager::~FontManager()
{
}
