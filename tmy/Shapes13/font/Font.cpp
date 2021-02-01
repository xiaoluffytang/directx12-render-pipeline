#include "Font.h"
#include <assert.h>
#include <fstream>


#pragma   warning(   push   ) 
#pragma   warning(   disable   :   4244   )  /*disable long to float*/
#pragma   warning(   disable   :   4267   )  /*disable size_t to int*/

Font::Font(ID3D12Device* pDevice, ID3D12GraphicsCommandList* commandList, string path)
	:m_pDevice(pDevice),
	commandList(commandList),
	m_FT2Lib(NULL),
	m_FT_Face(NULL),
	_bOutLine(false),
	_isInit(false),
	_texUnitSize(0),
	_texUnitLen(0),
	_texUnitIndex(-1)
{
	init(path, 40);
	assert(_isInit);

	_texUnitLen = TEXTURE_SIZE / _texUnitSize;

	_pTexIndices = new LPCHAR_TEX[_texUnitLen * _texUnitLen];
	memset(_pTexIndices, 0, sizeof(_pTexIndices) * _texUnitLen * _texUnitLen);
	_pCharIndices = new LPCHAR_TEX[256 * 256];
	memset(_pCharIndices, 0, sizeof(_pCharIndices) * 256 * 256);
}


Font::~Font(void)
{

}

void Font::init(const std::string& fontFile, const int& fontSize)
{
	path = fontFile;
	_texUnitSize = fontSize;

	if (FT_Init_FreeType(&m_FT2Lib))
	{
		FT_Done_FreeType(m_FT2Lib);
		m_FT2Lib = NULL;
		return;
	}

	if (FT_New_Face(m_FT2Lib, fontFile.c_str(), 0, &m_FT_Face))
	{
		FT_Done_FreeType(m_FT2Lib);
		m_FT2Lib = NULL;
		return;
	}


	FT_Select_Charmap(m_FT_Face, FT_ENCODING_UNICODE);

	FT_Set_Pixel_Sizes(m_FT_Face, fontSize, fontSize);

	BuildFontResource();
	_pTexBuf = new char[TEXTURE_SIZE*TEXTURE_SIZE * 4];
	memset(_pTexBuf, 0, TEXTURE_SIZE*TEXTURE_SIZE * 4);

	_isInit = true;
}

Font::LPCHAR_TEX Font::getCharInfo(int charcode)
{
	auto result = _pCharIndices[charcode];
	return result;
}

void Font::DrawSimpleText(const std::string& str)
{
	if (str.empty())
	{
		return;
	}

	char l = str[0];
	int lineHeight = _texUnitSize + 4;
	unsigned long charcode = 0;
	const char* p = str.c_str();
	long xPos = 0, yPos = 500;
	while (*p)
	{
		if (*p == '\n')
		{
			p++;
			xPos = 0;
			yPos += lineHeight;
			continue;
		}
		if (*p & 0x80)
		{
			charcode = *(WORD*)p;
		}
		else
		{
			charcode = *(char*)p;
		}
		LPCHAR_TEX pCharTex = _pCharIndices[charcode];
		if (pCharTex == NULL)
		{
			_texUnitIndex++;
			if (_pTexIndices[_texUnitIndex] == NULL)
			{
				_pCharIndices[charcode] = new CHAR_TEX();
				_pCharIndices[charcode]->c = charcode;
				_pTexIndices[_texUnitIndex] = _pCharIndices[charcode];
			}
			else
			{
				long tempIndex = _texUnitIndex;
				while (_pTexIndices[_texUnitIndex]->used)
				{
					if (_texUnitIndex == _texUnitLen * _texUnitLen)
					{
						_texUnitIndex = -1;
					}
					_texUnitIndex++;
					if (_texUnitIndex == tempIndex)
					{
						assert(false);
					}
				}
				_pTexIndices[_pTexIndices[_texUnitIndex]->c] = NULL;
				_pTexIndices[charcode] = _pTexIndices[_texUnitIndex];
			}
			_pCharIndices[charcode]->c = charcode;
			_pCharIndices[charcode]->used = true;
			_pCharIndices[charcode]->row = _texUnitIndex / _texUnitLen;
			_pCharIndices[charcode]->col = _texUnitIndex % _texUnitLen;
			copyCharToTexture(_pCharIndices[charcode]);

		}
		if (*p & 0x80)
		{
			p += 2;
		}
		else
		{
			p += 1;
		}
	}

	D3D12_SUBRESOURCE_DATA fontTextureData = {};
	fontTextureData.pData = &_pTexBuf[0];
	fontTextureData.RowPitch = TEXTURE_SIZE;
	fontTextureData.SlicePitch = TEXTURE_SIZE * TEXTURE_SIZE;
	UpdateSubresources(commandList, fontMap, fontTempMap, 0, 0, 1, &fontTextureData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(fontMap, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

bool Font::copyCharToTexture(LPCHAR_TEX& pCharTex)
{
	char*  chr = (char*)&(pCharTex->c);
	wchar_t wchar;

	MultiByteToWideChar(CP_ACP, 0, chr, strlen(chr) + 1, &wchar, 1);
	if (FT_Load_Char(m_FT_Face, wchar, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT))
		return false;

	FT_GlyphSlot& slot = m_FT_Face->glyph;
	FT_Bitmap& bitmap = slot->bitmap;

	int width = bitmap.width;
	int height = bitmap.rows;
	int row = pCharTex->row;
	int col = pCharTex->col;
	pCharTex->width = width;
	pCharTex->height = height;
	pCharTex->deltaX = (slot->metrics.horiBearingX >> 6);
	pCharTex->deltaY = (slot->metrics.vertBearingY >> 6);
	pCharTex->advanceX = (slot->metrics.horiAdvance >> 6);
	pCharTex->advanceY = (slot->metrics.vertAdvance >> 6);
	pCharTex->leftTopU = (row * _texUnitSize) * 1.0f / TEXTURE_SIZE;
	pCharTex->leftTopV = (col * _texUnitSize) * 1.0f / TEXTURE_SIZE;
	pCharTex->rightBottomU = (row * _texUnitSize + width) * 1.0f / TEXTURE_SIZE;
	pCharTex->rightBottomV = (col * _texUnitSize + height) * 1.0f / TEXTURE_SIZE;

	unsigned char* src = bitmap.buffer;

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			unsigned char _vl = src[i + bitmap.width*j];
			int index = (col * _texUnitSize + j) * TEXTURE_SIZE + row * _texUnitSize + i;
			_pTexBuf[index + 0] = _vl;
		}
	}
	return true;
}

void Font::BuildFontResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&fontMap)));

	ThrowIfFailed(m_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(TEXTURE_SIZE * TEXTURE_SIZE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&fontTempMap)));
}

void Font::CreateSrv(CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
{
	this->handle = handle;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_pDevice->CreateShaderResourceView(fontMap, &srvDesc, handle);
}
