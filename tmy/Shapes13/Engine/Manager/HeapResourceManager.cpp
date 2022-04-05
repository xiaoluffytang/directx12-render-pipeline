#include "HeapResourceManager.h"
#include "Manager.h"
extern Manager* manager;

HeapResourceManager::HeapResourceManager()
{

}


HeapResourceManager::~HeapResourceManager()
{

}

void HeapResourceManager::Init()
{
	/*mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = 50;*/
}

void HeapResourceManager::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		//有1个场景常量buf, 第1个是创建阴影贴图所需场景信息
		mFrameResources.push_back(std::make_unique<FrameResource>(1, (UINT)manager->materialManager->mMaterials.size()));
	}
}

void HeapResourceManager::UpdateUseNextFrameResource()
{
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
}

void HeapResourceManager::UpdateCommonConstantsCB(const GameTimer& gt)
{
	CommonConstants cb;
	cb.skullIndex = manager->textureManager->getTexture("../../Textures/star.dds")->srvIndex;
	cb.bloomMapIndex = manager->textureManager->bloomMap->texture->srvIndex;
	cb.shadowMapIndex = manager->textureManager->shadowMap->shadowMap->srvIndex;
	cb.depthStencilMapIndex = manager->textureManager->depthStencil->srvIndex;
	cb.AmbientLight = { 0.5f, 0.5f, 0.5f, 1.0f };
	cb.DirectLights[0].Direction = { -0.58f, -0.58f, -0.58f };
	cb.DirectLights[0].Strength = { 0.9f, 0.8f, 0.7f };
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX view = manager->cameraManager->shadowCamera->GetView();
	XMMATRIX proj = manager->cameraManager->shadowCamera->GetProj();
	XMStoreFloat4x4(&cb.ShadowTransform, XMMatrixTranspose(view * proj * T));
	cb.TotalTime = gt.TotalTime();
	cb.DeltaTime = gt.DeltaTime();
	auto currCb = mCurrFrameResource->commonCB.get();
	currCb->CopyData(0, cb);
}

void HeapResourceManager::UpdateMaterials(const GameTimer& gt)
{
	auto ms = mCurrFrameResource->MaterialBuffer.get();
	for (auto &e : manager->materialManager->mMaterials)
	{
		Material* material = e.second;
		if (material->NumFramesDirty > 0)
		{
			MaterialConstants mc;
			mc.DiffuseAlbedo = material->DiffuseAlbedo;
			mc.FresnelR0 = material->FresnelR0;
			mc.Roughness = material->Roughness;
			mc.MatTransform = material->MatTransform;
			mc.DiffuseMapIndex = material->DiffuseSrvHeapIndex;
			ms->CopyData(material->MatCBIndex, mc);
			material->NumFramesDirty--;
		}
	}
}