#pragma once
#include "../Compoents/GameObject.h"
#include "../font/Font.h"
class UIText : public GameObject
{
public:
	UIText(MeshGeometry* mesh, int index, string submeshName, Material* material, Font* font);
	~UIText();
	Font* font;
	void SetText(string str);
};
