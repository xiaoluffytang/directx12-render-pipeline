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
		long row; //�ڼ���
		long col;   //�ڼ���
		float width;//�ַ����
		float height;//�ַ��߶�
		float deltaX;//��߼��
		float deltaY;//�±߼��
		float advanceX;  //��ʾ���ַ��ܹ�ռ�ö��Ŀռ䣨�����������ҿհ׵ĵط���
		float advanceY;  //��ʾ���ַ��ܹ�ռ�ö�ߵĿռ䣨�����������ҿհ׵ĵط���
		float leftTopU;   //����u����
		float leftTopV;    //����v����
		float rightBottomU; //����u����
		float rightBottomV; //����v����
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
	//ID3D12Resource* fontMap = nullptr;     //������Դ
	ID3D12Resource* fontTempMap = nullptr;     //��ʱ������Դ
	char* _pTexBuf;
	bool _bOutLine;
	//������ã��Ƚ��ȳ�����̭ԭ��
	long _texUnitSize;  //��Ԫ����Ĵ�С
	long _texUnitLen;   //��Ԫ����ĸ��������������������
	long _texUnitIndex;//��Ԫ����ʹ�õ�����
	LPCHAR_TEX* _pTexIndices;   //�����Ӧ�ַ�����
	LPCHAR_TEX* _pCharIndices;  //�ַ���Ӧ��������

	bool _isInit;
	ID3D12GraphicsCommandList* commandList;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle;
	string path;
};



