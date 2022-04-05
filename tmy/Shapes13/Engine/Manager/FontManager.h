#pragma once
#include "../font/Font.h"
class FontManager
{
public:
	FontManager();
	~FontManager();
	Font* font;
	void Init();
};

