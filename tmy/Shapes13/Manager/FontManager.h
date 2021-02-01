#pragma once
#include "../font/Font.h"
class Manager;
class FontManager
{
public:
	FontManager(Manager* manager);
	~FontManager();
	Manager* manager;
	Font* font;
};

