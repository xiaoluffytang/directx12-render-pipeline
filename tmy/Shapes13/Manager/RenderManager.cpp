#include "RenderManager.h"
#include "Manager.h"


RenderManager::RenderManager(Manager* man)
{
	manager = man;
}


RenderManager::~RenderManager()
{
}

//绘制阴影贴图
void RenderManager::DrawShadowMap()
{
	auto mCommandList = manager->commonInterface.mCommandList;
	auto shadowMap = manager->textureManager->shadowMap;
	mCommandList->RSSetViewports(1, &shadowMap->Viewport());
	mCommandList->RSSetScissorRects(1, &shadowMap->ScissorRect());

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	mCommandList->ClearDepthStencilView(shadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(0, nullptr, true, &shadowMap->Dsv());

	auto passCB = manager->heapResourceManager->mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 1 * passCBByteSize);

	mCommandList->SetPipelineState(manager->psoManager->mPSOs["shadowMap"]);

	RenderList(OpaqueList1, false);
	RenderList(OpaqueList2, false);
	RenderList(OpaqueList3, false);

	mCommandList->SetPipelineState(manager->psoManager->mPSOs["animatorShadowMap"]);
	RenderList(OpaqueAnimatorList1, false);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void RenderManager::DrawSceneToCubeMap()
{
	auto mCommandList = manager->commonInterface.mCommandList;
	auto mDynamicCubeMap = manager->textureManager->cubeTarget;
	mCommandList->RSSetViewports(1, &mDynamicCubeMap->Viewport());
	mCommandList->RSSetScissorRects(1, &mDynamicCubeMap->ScissorRect());

	// Change to RENDER_TARGET.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDynamicCubeMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	//std::vector<int> cameraIndexs;
	//XMFLOAT3 mainCameraPos = manager->cameraManager->mainCamera->GetPosition3f();
	//XMFLOAT3 cubePos = mDynamicCubeMap->mPos;
	//XMFLOAT3 mainCameraPosInCubeSpace = XMFLOAT3(mainCameraPos.x - cubePos.x, mainCameraPos.y - cubePos.y, mainCameraPos.z - cubePos.z);   //主摄像机在cubeMap的局部坐标
	//if (mainCameraPosInCubeSpace.x >= 1) cameraIndexs.push_back(0);
	//if (mainCameraPosInCubeSpace.x <= -1) cameraIndexs.push_back(1);
	//if (mainCameraPosInCubeSpace.y >= 1) cameraIndexs.push_back(2);
	//if (mainCameraPosInCubeSpace.y <= -1) cameraIndexs.push_back(3);
	//if (mainCameraPosInCubeSpace.z >= 1) cameraIndexs.push_back(4);
	//if (mainCameraPosInCubeSpace.z <= -1) cameraIndexs.push_back(5);
	
	for (int i = 0; i < 6; ++i)
	{
		int index = i;
		// Clear the back buffer and depth buffer.
		mCommandList->ClearRenderTargetView(mDynamicCubeMap->Rtv(index), Colors::LightSteelBlue, 0, nullptr);
		mCommandList->ClearDepthStencilView(mDynamicCubeMap->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Specify the buffers we are going to render to.
		mCommandList->OMSetRenderTargets(1, &mDynamicCubeMap->Rtv(index), true, &mDynamicCubeMap->Dsv());

		auto passCB = manager->heapResourceManager->mCurrFrameResource->PassCB->Resource();
		mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + (3 + index) * passCBByteSize);
		
		RenderList(OpaqueList1, true);
		RenderList(OpaqueList2, true);
		RenderList(OpaqueList3, true);	
		RenderList(OpaqueAnimatorList1, true);
		RenderList(TransparentList1, true);
		RenderList(TransparentList2, true);
		RenderList(TransparentList3, true);
		RenderList(SkyBoxList, true);
	}

	// Change back to GENERIC_READ so we can read the texture in a shader.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDynamicCubeMap->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void RenderManager::DrawStroke()
{
	auto mCommandList = manager->commonInterface.mCommandList;
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["stroke"]);
	RenderList(OpaqueList1, false);
	RenderList(OpaqueList2, false);
	RenderList(OpaqueList3, false);
}

void RenderManager::RenderList(vector<GameObject*> list,bool useSelfPso)
{
	for (size_t i = 0; i < list.size(); ++i)
	{
		auto ri = list[i];
		DrawOneGameObject(ri,useSelfPso);
	}
}

//绘制一个游戏物体
void RenderManager::DrawOneGameObject(GameObject* go, bool useSelfPso)
{
	if (go == nullptr)
	{
		return;
	}
	//设置游戏物体的实例化缓冲区，这个没有优化空间
	if (manager->heapResourceManager->mCurrFrameResource->InstanceBuffer[go->ObjCBIndex] == nullptr)
	{
		return;
	}
	auto cmdList = manager->commonInterface.mCommandList;
	if (useSelfPso)
	{
		cmdList->SetPipelineState(go->meshrender->material->pso);
	}
	//经试验，优化顶点视图设置的效果微乎其微，无需优化
	cmdList->IASetVertexBuffers(0, 1, &go->meshrender->mesh->VertexBufferView());
	cmdList->IASetIndexBuffer(&go->meshrender->mesh->IndexBufferView());
	cmdList->IASetPrimitiveTopology(go->meshrender->PrimitiveType);
	auto instanceBuffer = manager->heapResourceManager->mCurrFrameResource->InstanceBuffer[go->ObjCBIndex]->Resource();
	cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
	SubmeshGeometry submesh = go->meshrender->GetSubmesh();
	cmdList->DrawIndexedInstanced(submesh.IndexCount, go->renderCount, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
}

