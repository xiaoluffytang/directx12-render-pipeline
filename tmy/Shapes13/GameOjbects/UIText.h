#pragma once
#include "../Engine/Compoents/GameObject.h"
#include "../Engine/font/Font.h"
class UIText : public GameObject
{
public:
	UIText(MeshGeometry* mesh, int index, string submeshName, Material* material, Font* font);
	~UIText();
	Font* font;
	void SetText(string str);
};
