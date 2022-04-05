#include "RenderManager.h"
#include "Manager.h"

extern Manager* manager;

RenderManager::RenderManager()
{

}


RenderManager::~RenderManager()
{
}

void RenderManager::Init()
{
	
}

void RenderManager::CreateAnderPass()
{
	CreateRenderPass1();
	CreateRenderPass2();
	CreateRenderPass3();
	CreateRenderPass4();
	CreateRenderPass5();
	CreateRenderPass6();
	CreateRenderPass7();
	CreateRenderPass8();
	CreateRenderPass9();
}

void RenderManager::CreateRenderPass1()
{
	RenderPassDefaultState* pass = new RenderPassDefaultState();
	pass->camera = manager->cameraManager->mainCamera;
	pass->depthStencil = manager->textureManager->depthStencil;
	pass->renderTarget = manager->textureManager->screenPostMap->texture;
	pass->viewport = manager->commonInterface.mScreenViewport;
	pass->rect = manager->commonInterface.mScissorRect;
	pass->list.push_back(OpaqueList1);
	pass->list.push_back(OpaqueList2);
	pass->list.push_back(OpaqueAnimatorList1);
	renderPassDic["default"] = pass;
}

void RenderManager::CreateRenderPass2()
{
	RenderPassRenderList* pass = new RenderPassRenderList();
	pass->list.push_back(OpaqueList1);
	pass->list.push_back(OpaqueList2);
	pass->list.push_back(OpaqueAnimatorList1);
	renderPassDic["render_opaque"] = pass;
}

void RenderManager::CreateRenderPass3()
{
	RenderPassRenderList* pass = new RenderPassRenderList();
	pass->targetPso = manager->psoManager->mPSOs["stroke"];
	pass->list.push_back(OpaqueList2);
	renderPassDic["stroke"] = pass;
}

void RenderManager::CreateRenderPass4()
{
	RenderPassTabMirror* pass = new RenderPassTabMirror();
	pass->targetPso = manager->psoManager->mPSOs["Stencil_Always"];
	pass->list.push_back(TransparentList2);
	renderPassDic["tab_mirror"] = pass;
}

void RenderManager::CreateRenderPass5()
{
	RenderPassRenderMirror* pass = new RenderPassRenderMirror();
	pass->targetPso = manager->psoManager->mPSOs["Stencil_Equal"];
	pass->list.push_back(OpaqueList3);
	renderPassDic["render_mirror"] = pass;
}

void RenderManager::CreateRenderPass6()
{
	RenderPassRenderList* pass = new RenderPassRenderList();
	pass->list.push_back(TransparentList1);
	pass->list.push_back(TransparentList2);
	pass->list.push_back(TransparentList3);
	pass->list.push_back(SkyBoxList);
	renderPassDic["render_transparent"] = pass;
}

void RenderManager::CreateRenderPass7()
{
	RenderPassDefaultState* pass = new RenderPassDefaultState();
	pass->camera = manager->cameraManager->shadowCamera;
	pass->depthStencil = manager->textureManager->shadowMap->shadowMap;
	pass->renderTarget = nullptr;
	pass->viewport = manager->textureManager->shadowMap->Viewport();
	pass->rect = manager->textureManager->shadowMap->ScissorRect();
	pass->targetPso = manager->psoManager->mPSOs["shadowMap"];
	pass->list.push_back(OpaqueList1);
	pass->list.push_back(OpaqueList2);
	pass->list.push_back(OpaqueList3);
	renderPassDic["render_shadow_opaque"] = pass;
}

void RenderManager::CreateRenderPass8()
{
	RenderPassRenderList* pass = new RenderPassRenderList();
	pass->list.push_back(OpaqueAnimatorList1);
	pass->targetPso = manager->psoManager->mPSOs["animatorShadowMap"];
	renderPassDic["render_shadow_opaque_animator"] = pass;
}

void RenderManager::CreateRenderPass9()
{
	
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

		auto address = manager->textureManager->cubeTarget->mCubeMapCamera[i].UpdateAndGetConstantCB(manager->heapResourceManager->mCurrFrameResourceIndex);
		mCommandList->SetGraphicsRootConstantBufferView(2, address);
		
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
	if (go->renderCount <= 0)
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
	auto instanceBuffer = go->InstanceBuffer[manager->heapResourceManager->mCurrFrameResourceIndex]->Resource();
	cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
	SubmeshGeometry submesh = go->meshrender->GetSubmesh();
	cmdList->DrawIndexedInstanced(submesh.IndexCount, go->renderCount, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
}

