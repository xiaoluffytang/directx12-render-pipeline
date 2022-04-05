#pragma once
#include "../Common/d3dApp.h"

using namespace std;
#include "../../libs/freetype-2.5.3/include/ft2build.h"
#include FT_FREETYPE_H
#include "../Texture/Texture2DResource.h"

class Font
{
public:
	typedef struct _CHAR_TEX
	{
		unsigned long c;
		bool used;
		long useCount;
		long row; //第几行
		long col;   //第几列
		float width;//字符宽度
		float height;//字符高度
		float deltaX;//左边间距
		float deltaY;//下边间距
		float advanceX;  //显示该字符总共占用多宽的空间（包括上下左右空白的地方）
		float advanceY;  //显示该字符总共占用多高的空间（包括上下左右空白的地方）
		float leftTopU;   //左上u坐标
		float leftTopV;    //左上v坐标
		float rightBottomU; //右下u坐标
		float rightBottomV; //右下v坐标
	}CHAR_TEX, *LPCHAR_TEX;
public:
	Font(ID3D12Device* md3dDevice, ID3D12GraphicsCommandList* commandList, string path);
	~Font(void);
	void DrawSimpleText(const std::string& str);
	Font::LPCHAR_TEX getCharInfo(int charcode);
private:
	void init(const std::string& fontFile, const int& fontSize);
	bool copyCharToTexture(LPCHAR_TEX& pCharTex);
	void BuildFontResource();

	const int TEXTURE_SIZE = 512;
protected:
	ID3D12Device* m_pDevice;
	FT_Library			m_FT2Lib;
	FT_Face				m_FT_Face;
public:
	Texture2DResource* texture = nullptr;
	//ID3D12Resource* fontMap = nullptr;     //字体资源
	ID3D12Resource* fontTempMap = nullptr;     //临时字体资源
	char* _pTexBuf;
	bool _bOutLine;
	//纹理采用，先进先出的淘汰原则
	long _texUnitSize;  //单元纹理的大小
	long _texUnitLen;   //单元纹理的个数，横坐标或者纵坐标
	long _texUnitIndex;//单元纹理使用的索引
	LPCHAR_TEX* _pTexIndices;   //纹理对应字符索引
	LPCHAR_TEX* _pCharIndices;  //字符对应纹理索引

	bool _isInit;
	ID3D12GraphicsCommandList* commandList;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle;
	string path;
};



