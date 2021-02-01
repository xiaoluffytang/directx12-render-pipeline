#include "HeapResourceManager.h"
#include "Manager.h"
#include "../../../Common/d3dx12.h"


HeapResourceManager::HeapResourceManager(Manager* manager)
{
	this->manager = manager;
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = 50;
}


HeapResourceManager::~HeapResourceManager()
{

}

void HeapResourceManager::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mSrvHeap)));
}

void HeapResourceManager::BuildSrvViews(UINT mCbvSrvUavDescriptorSize)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
	hDescriptor.Offset(manager->textureManager->setAside, mCbvSrvUavDescriptorSize);   //预留前面的一部分
	for (int i = 0; i < manager->textureManager->mDiffuseTextures.size(); i++)
	{
		Texture* texture = manager->textureManager->mDiffuseTextures[i];
		auto resource = texture->Resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = resource->GetDesc().Format;
		srvDesc.ViewDimension = i == 0 ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;    //图片数组里的第一个是天空盒贴图
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		manager->commonInterface.md3dDevice->CreateShaderResourceView(resource.Get(), &srvDesc, hDescriptor);
		hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	}

}

void HeapResourceManager::BuildFrameResources()
{
	vector<int> list;
	for (int i = 0; i < manager->gameobjectManager->gameobjectList.size(); i++)
	{
		list.push_back(manager->gameobjectManager->gameobjectList[i]->instanceCount);
	}
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		//有9个场景常量buff，第一个是主摄像机信息，第二个是创建阴影贴图所需场景信息，第三个是UI摄像机,还有6个是立方体贴图视角的摄像机视角信息
		mFrameResources.push_back(std::make_unique<FrameResource>(manager->commonInterface.md3dDevice.Get(),
			9, list, (UINT)manager->materialManager->mMaterials.size()));
	}
}

void HeapResourceManager::UpdateUseNextFrameResource()
{
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
}

void HeapResourceManager::UpdateShadowTransform(const GameTimer& gt)
{
	XMVECTOR lightPos = XMVectorSet(35, 35, 25, 0.0f);
	XMVECTOR targetPos = XMVectorSet(0, 0, 0, 0.0f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);
	XMStoreFloat3(&mLightPosW, lightPos);
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;

	mLightNearZ = n;
	mLightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj*T;
	XMStoreFloat4x4(&mLightView, lightView);
	XMStoreFloat4x4(&mLightProj, lightProj);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void HeapResourceManager::UpdateMainPassCB(const GameTimer& gt)
{
	UpdateCameraCB(gt, manager->cameraManager->mainCamera, 0);
}

void HeapResourceManager::UpdateUIPassCB(const GameTimer& gt)
{
	UpdateCameraCB(gt, manager->cameraManager->uiCamera, 2);
}

void HeapResourceManager::UpdateCubeTargetPassCB(const GameTimer& gt)
{
	auto mCubeMapCamera = manager->textureManager->cubeTarget->mCubeMapCamera;
	float CubeMapSize = manager->textureManager->cubeTarget->CubeMapSize;
	for (int i = 0; i < 6; ++i)
	{
		PassConstants cubeFacePassCB = mMainPassCB;

		XMMATRIX view = mCubeMapCamera[i].GetView();
		XMMATRIX proj = mCubeMapCamera[i].GetProj();

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&cubeFacePassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&cubeFacePassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&cubeFacePassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&cubeFacePassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&cubeFacePassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&cubeFacePassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		cubeFacePassCB.EyePosW = mCubeMapCamera[i].GetPosition3f();
		cubeFacePassCB.RenderTargetSize = XMFLOAT2(CubeMapSize, CubeMapSize);
		cubeFacePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / CubeMapSize, 1.0f / CubeMapSize);

		auto currPassCB = mCurrFrameResource->PassCB.get();

		// Cube map pass cbuffers are stored in elements 1-6.
		currPassCB->CopyData(3 + i, cubeFacePassCB);
	}
}

void HeapResourceManager::UpdateCameraCB(const GameTimer& gt, Camera* camera,int index)
{
	XMMATRIX view = camera->GetView();
	XMMATRIX proj = camera->GetProj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&mMainPassCB.ShadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&mShadowTransform)));
	mMainPassCB.EyePosW = camera->GetPosition3f();;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)manager->commonInterface.mClientWidth, (float)manager->commonInterface.mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / manager->commonInterface.mClientWidth, 1.0f / manager->commonInterface.mClientHeight);
	mMainPassCB.NearZ = camera->GetNearZ();
	mMainPassCB.FarZ = camera->GetFarZ();
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.5f, 0.5f, 0.5f, 1.0f };
	mMainPassCB.DirectLights[0].Direction = { -0.58f, -0.58f, -0.58f };
	mMainPassCB.DirectLights[0].Strength = { 0.9f, 0.8f, 0.7f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(index, mMainPassCB);
}

void HeapResourceManager::UpdateShadowPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = manager->textureManager->shadowMap->Width();
	UINT h = manager->textureManager->shadowMap->Height();
	XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mShadowPassCB.EyePosW = mLightPosW;
	mShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	mShadowPassCB.NearZ = mLightNearZ;
	mShadowPassCB.FarZ = mLightFarZ;
	mShadowPassCB.TotalTime = gt.TotalTime();
	mShadowPassCB.DeltaTime = gt.DeltaTime();
	mShadowPassCB.AmbientLight = { 0.5f, 0.5f, 0.5f, 1.0f };
	mShadowPassCB.DirectLights[0].Direction = { -0.58f, -0.58f, -0.58f };
	mShadowPassCB.DirectLights[0].Strength = { 0.9f, 0.8f, 0.7f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(1, mShadowPassCB);
}

void HeapResourceManager::UpdateObjectCBs(const GameTimer& gt, const BoundingFrustum& mCamFrustum)
{
	XMMATRIX view = manager->cameraManager->mainCamera->GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	auto instanceDatas = mCurrFrameResource->InstanceBuffer;
	for (auto& gameobject : manager->gameobjectManager->gameobjectList)
	{
		int index = gameobject->ObjCBIndex;   //表示这个游戏物体是第几个游戏物体
		auto instanceBuffer = instanceDatas[index];
		if (instanceBuffer == nullptr)
		{
			continue;
		}
		int visibleInstanceCount = 0;
		for (int i = 0; i < gameobject->instanceCount; i++)
		{
			bool needUpdate = true;
			
			InstanceData& data = gameobject->Instances[i];
			if (gameobject->needCheckCulling)
			{
				//XMMATRIX world = XMLoadFloat4x4(&data.World);
				//XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);
				//XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);
				//BoundingFrustum localSpaceFrustum;
				//mCamFrustum.Transform(localSpaceFrustum, viewToLocal);
				//needUpdate = localSpaceFrustum.Contains(gameobject->meshrender->GetSubmesh().Bounds) != ContainmentType::DISJOINT;
			}

			if (needUpdate)
			{
				instanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}
		gameobject->renderCount = visibleInstanceCount;
	}
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
			if (material->DiffuseSrvHeapIndex == 4)
			{
				int l = 0;
			}
			ms->CopyData(material->MatCBIndex, mc);
			material->NumFramesDirty--;
		}
	}
}