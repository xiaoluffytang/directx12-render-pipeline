#include "TextureManager.h"
#include "Manager.h"
extern Manager* manager;

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
}

void TextureManager::Init()
{
	//创建三种描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 200;    //这个数字得合适 不能太大，否则64位会爆，也不能小，否则64位创建描述符会崩
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mSrvHeap)));
	mCbvSrvUavDescriptorSize = manager->commonInterface.md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto device = manager->commonInterface.md3dDevice.Get();
	auto command = manager->commonInterface.mCommandList.Get();
	auto width = manager->commonInterface.mClientWidth;
	auto height = manager->commonInterface.mClientHeight;

	//创建深度模板缓冲区
	CreateDepthStencil();
	this->shadowMap = new ShadowMap(1024, 1024);
	this->bloomMap = new ScreenPostMap(width, height);
	this->screenPostMap = new ScreenPostMap(width, height);
	this->blur = new Blur(width / 2, height / 2);
	this->cubeTarget = new CubeRenderTarget(512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);
	this->fftOcean = new FFTOcean();
	this->skin = new SkinnedData();
	this->particle = new ParticleEffect();
}

void TextureManager::CreateDepthStencil()
{
	if (depthStencil != nullptr)
	{
		DeleteSrvDescriptorIndex(depthStencil->srvIndex);
		DeleteDsvDescriptorIndex(depthStencil->dsvIndex);
	}
	// Create the depth/stencil buffer and view.
	bool m4xMsaaState = false;
	int m4xMsaaQuality = 0;
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = manager->commonInterface.mClientWidth;
	depthStencilDesc.Height = manager->commonInterface.mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	depthStencil = new Texture2DResource();
	depthStencil->LoadTextureByDesc(&depthStencilDesc, &optClear);
	depthStencil->CreateDsvIndexDescriptor();
}

int TextureManager::LoadTexture2D(string file)
{
	file = manager->getPathPrefix(file);
	auto texture = new Texture2DResource();
	texture->LoadTextureByFile(file);
	dics[file] = texture;
	return texture->srvIndex;
}

int TextureManager::LoadTexture3D(string file)
{
	file = manager->getPathPrefix(file);
	auto texture = new Texture3DResource();
	texture->LoadTextureByFile(file);
	dics[file] = texture;
	return texture->srvIndex;
}

Texture2DResource* TextureManager::getTexture(string path)
{
	path = manager->getPathPrefix(path);
	return dics[path];
}

int TextureManager::GetSrvDescriptorIndex()
{
	int first = setAside;
	int result = 0;
	if (nullSrvDescriptorDic.size() > 0)
	{
		result = nullSrvDescriptorDic[nullSrvDescriptorDic.size() - 1];
		nullSrvDescriptorDic.pop_back();
	}
	else
	{
		result = srvDesctiprotCount + first;
		srvDesctiprotCount++;
	}
	return result;
}

int TextureManager::GetRtvDescriptorIndex()
{
	int first = firstRtvAside;
	int result = 0;
	if (nullRtvDescriptorDic.size() > 0)
	{
		result = nullRtvDescriptorDic[nullRtvDescriptorDic.size() - 1];
		nullRtvDescriptorDic.pop_back();
	}
	else
	{
		result = rtvDesctiprotCount + first;
		rtvDesctiprotCount++;
	}
	return result;
}

int TextureManager::GetDsvDescriptorIndex()
{
	int first = firstDsvAside;
	int result = 0;
	if (nullDsvDescriptorDic.size() > 0)
	{
		result = nullDsvDescriptorDic[nullDsvDescriptorDic.size() - 1];
		nullDsvDescriptorDic.pop_back();
	}
	else
	{
		result = dsvDesctiprotCount + first;
		dsvDesctiprotCount++;
	}
	return result;
}

void TextureManager::DeleteSrvDescriptorIndex(int index)
{
	nullSrvDescriptorDic.push_back(index);
}

void TextureManager::DeleteRtvDescriptorIndex(int index)
{
	nullRtvDescriptorDic.push_back(index);
}

void TextureManager::DeleteDsvDescriptorIndex(int index)
{
	nullDsvDescriptorDic.push_back(index);
}