#include "UIText.h"

UIText::UIText(MeshGeometry* mesh, int objIndex, string submeshName, Material* material,Font* font) :GameObject(mesh, objIndex, submeshName, material)
{
	this->font = font;
}


UIText::~UIText()
{
}

void UIText::SetText(string str)
{
	const char* p = str.c_str();
	int count = 0;
	while (*p)
	{
		count++;
		p += (*p & 0x80) ? 2 : 1;
	}
	Instances = new InstanceData[count];
	font->DrawSimpleText(str);
	p = str.c_str();
	
	int charcode = 0;
	XMFLOAT3 scale = XMFLOAT3(0.5,0.5,0.5);
	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
	XMFLOAT3 firstPosition = XMFLOAT3(-450, 280, 0);
	XMFLOAT3 position = firstPosition;
	int index = 0;

	int nowLine = 0;
	float nowLineWidth = 0;
	float oneLineWidth = 900;  //一行有多宽
	float distanceY = 30;  //两行之间的间距

	while (*p)
	{
		charcode = (*p & 0x80) ? *(WORD*)p : *(char*)p;
		p += (*p & 0x80) ? 2 : 1;	
		if (charcode == '|')  //这里用 | 表示换行
		{
			nowLineWidth = 0;
			nowLine++;
			position.x = firstPosition.x;
			continue;
		}
		auto info = font->getCharInfo(charcode);
		if (info == NULL)
		{
			continue;
		}
		float offsetX = (info->width / 2 + info->deltaX / 2) * scale.x;
		float offsetY = -info->deltaY * scale.y - distanceY * nowLine;
		XMMATRIX m1 = XMMatrixTranslation(position.x + offsetX, position.y + offsetY, position.z);
		XMMATRIX m2 = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		XMMATRIX m3 = XMMatrixScaling(info->width * scale.x, -info->height * scale.y, 1);
		XMMATRIX m = m3 * m2 * m1;
		m = XMMatrixTranspose(m);
		DirectX::XMFLOAT4X4* matrix = &MathHelper::Identity4x4();
		XMStoreFloat4x4(matrix, m);
		Instances[index] = InstanceData();
		Instances[index].World = *matrix;
		Instances[index].MaterialIndex = meshrender->material->MatCBIndex;
		Instances[index++].TexTransform = XMFLOAT4X4(
			info->rightBottomU - info->leftTopU, 0, 0, info->leftTopU,
			0, info->rightBottomV - info->leftTopV, 0, info->leftTopV,
			0, 0, 0, 0, 
			0, 0, 0, 0);
		float width = info->advanceX * scale.x;
		position.x += width;
		nowLineWidth += width;
		if (nowLineWidth >= oneLineWidth)
		{
			nowLineWidth = 0;
			nowLine++;
			position.x = firstPosition.x;
		}
	}
	instanceCount = count;
}
