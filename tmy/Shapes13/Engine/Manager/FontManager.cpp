#include "FontManager.h"
#include "../Manager/Manager.h"
extern Manager* manager;

FontManager::FontManager()
{
	
}


FontManager::~FontManager()
{
}

void FontManager::Init()
{
	font = new Font(manager->commonInterface.md3dDevice.Get(), manager->commonInterface.mCommandList.Get(), manager->getPathPrefix("../../font/simhei.ttf"));
}
