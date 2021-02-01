#include "TextureManager.h"
#include "Manager.h"


TextureManager::TextureManager(Manager* manager)
{
	this->manager = manager;
	auto device = manager->commonInterface.md3dDevice.Get();
	auto command = manager->commonInterface.mCommandList.Get();
	auto width = manager->commonInterface.mClientWidth;
	auto height = manager->commonInterface.mClientHeight;
	this->shadowMap = new ShadowMap(device,1024,1024);
	this->bloomMap = new ScreenPostMap(device, width, height);
	this->screenPostMap = new ScreenPostMap(device, width, height);
	this->ssao = new Ssao(device, command, width, height);
	this->blur = new Blur(device, width / 2, height / 2);
	this->cubeTarget = new CubeRenderTarget(device, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM);
	this->fftOcean = new FFTOcean(device);
	this->skin = new SkinnedData();
	this->particle = new ParticleEffect(device);
}

TextureManager::~TextureManager()
{
}

int TextureManager::LoadTexture(string file)
{
	if (mDiffuseTexturesDic[file] != nullptr)
	{
		return mDiffuseTexturesDic[file]->index;
	}
	auto texture = new Texture();
	texture->Name = file;
	texture->Filename = StringToWString(file);
	
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(manager->commonInterface.md3dDevice.Get(),
		manager->commonInterface.mCommandList.Get(), texture->Filename.c_str(),
		texture->Resource, texture->UploadHeap));
	int index = mDiffuseTextures.size() + setAside; 
	texture->index = index;
	mDiffuseTextures.push_back(texture);
	mDiffuseTexturesDic[file] = std::move(texture);
	return index;
}

wstring TextureManager::StringToWString(const std::string& str)
{
	int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t *wide = new wchar_t[num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
	std::wstring w_str(wide);
	delete[] wide;
	return w_str;
}
