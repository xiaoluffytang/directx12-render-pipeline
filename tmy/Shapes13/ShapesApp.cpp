#include "Engine/Common/d3dApp.h"
#include "Engine/Common/MathHelper.h"
#include "Engine/Common/UploadBuffer.h"
#include "Engine/Common/GeometryGenerator.h"
#include "Engine/Common/FrameResource.h"
#include "Engine/Manager/Manager.h"
#include "Engine/Global.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment ( lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")

Manager* manager;

class MyRenderDemoApp : public D3DApp
{
public:
    MyRenderDemoApp(HINSTANCE hInstance);
    MyRenderDemoApp(const MyRenderDemoApp& rhs) = delete;
    MyRenderDemoApp& operator=(const MyRenderDemoApp& rhs) = delete;
    ~MyRenderDemoApp();

    virtual bool Initialize()override;
	virtual void CreateRtvAndDsvDescriptorHeaps()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
    void OnKeyboardInput(const GameTimer& gt);
	int t = 0;
	void Pick(int x, int y);    //射线检测
	Microsoft::WRL::ComPtr<ID3D12Resource> screenPostDSResource = nullptr;
private:
	//Manager* manager = nullptr;    //游戏管理器

    bool mIsWireframe = false;    //是否使用线框模式来渲染
	BoundingFrustum mCamFrustum;     //用于包围体检测

	int postEffectState = 0;   //屏幕后处理特效
	int waterState = 1;   //使用哪种方式渲染水面
	bool renderEnvironment = false;   //是否渲染环境映射到球体
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd){
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        MyRenderDemoApp theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
		
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

MyRenderDemoApp::MyRenderDemoApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

MyRenderDemoApp::~MyRenderDemoApp()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

void MyRenderDemoApp::CreateRtvAndDsvDescriptorHeaps()
{
	//后台缓冲区的渲染目标视图
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 100;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//深度模板缓冲区视图，这里创建多个，第一个是真正的深度模板缓冲区描述符，第二个是阴影贴图描述符，第三个也是一个深度模板缓冲区描述符
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 100;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

bool MyRenderDemoApp::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	
	//创建一个公共接口
	DeviceCommonInterface commonInterface;
	commonInterface.md3dDevice = md3dDevice;
	commonInterface.mCommandList = mCommandList;
	commonInterface.mScreenViewport = mScreenViewport;
	commonInterface.mScissorRect = mScissorRect;
	commonInterface.mClientWidth = mClientWidth;
	commonInterface.mClientHeight = mClientHeight;
	commonInterface.isDebugModel = getIsDebugModel();
	//创建游戏管理器
	manager = new Manager(commonInterface, mRtvHeap, mDsvHeap);
	manager->Init();
	//游戏场景初始化
	manager->sceneManager->Init();
	
	//游戏场景初始化完毕后，创建一些堆资源和相关描述符资源
	manager->heapResourceManager->BuildFrameResources();    //创建帧资源
    manager->heapResourceManager->mCurrFrameResource = manager->heapResourceManager->mFrameResources[manager->heapResourceManager->mCurrFrameResourceIndex].get();
	manager->textureManager->shadowMap->BuildDescriptors();
	manager->textureManager->screenPostMap->BuildDescriptors();
	manager->textureManager->blur->BuildDescriptors();
	manager->textureManager->cubeTarget->BuildDescriptors();
	manager->textureManager->cubeTarget->BuildCubeFaceCamera(0.0f,10.0f,-25.0f);
	manager->textureManager->bloomMap->BuildDescriptors();
	manager->textureManager->fftOcean->BuildDescriptors();
	manager->textureManager->skin->BuildAniTex();
	manager->textureManager->particle->BuildParticleTex();
	
	manager->renderManager->CreateAnderPass();
	//初始化完毕
	manager->sceneManager->InitEnd();
	
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
    FlushCommandQueue();
	OnResize();

    return true;
}
 
void MyRenderDemoApp::OnResize()
{
    D3DApp::OnResize();
	if (manager != nullptr)
	{
		manager->commonInterface.mClientWidth = mClientWidth;
		manager->commonInterface.mClientHeight = mClientHeight;
		//窗口变化后，摄像机的一些参数也要变化
		manager->cameraManager->mainCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		manager->cameraManager->uiCamera->size = mClientWidth;
		manager->cameraManager->uiCamera->SetLens(0, AspectRatio(), 1.0f, 1000.0f);
		//贴图大小改变
		manager->textureManager->screenPostMap->OnResize(mClientWidth, mClientHeight);
		manager->textureManager->bloomMap->OnResize(mClientWidth, mClientHeight);
		manager->renderManager->renderPassDic["default"]->viewport = mScreenViewport;
		manager->renderManager->renderPassDic["default"]->rect = mScissorRect;
		manager->renderManager->renderPassDic["default"]->renderTarget == manager->textureManager->screenPostMap->texture;
		manager->textureManager->CreateDepthStencil();
	}	
}

void MyRenderDemoApp::Update(const GameTimer& gt)
{
	manager->sceneManager->Update(gt);
    OnKeyboardInput(gt);
	manager->heapResourceManager->UpdateUseNextFrameResource();
    if(manager->heapResourceManager->mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < manager->heapResourceManager->mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(manager->heapResourceManager->mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
	BoundingFrustum::CreateFromMatrix(mCamFrustum, manager->cameraManager->mainCamera->GetProj());
	manager->gameobjectManager->Update(gt);
	manager->heapResourceManager->UpdateMaterials(gt);
	manager->heapResourceManager->UpdateCommonConstantsCB(gt);
}

void MyRenderDemoApp::Draw(const GameTimer& gt)
{
	
    auto cmdListAlloc = manager->heapResourceManager->mCurrFrameResource->CmdListAlloc;
    ThrowIfFailed(cmdListAlloc->Reset());
	//设置初始PSO
	string psoName = mIsWireframe ? "opaque_wireframe" : "opaque_back";
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), manager->psoManager->mPSOs[psoName]));
	//设置描述符堆
	ID3D12DescriptorHeap* descriptorHeaps[] = { manager->textureManager->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	
	//设置根签名
	mCommandList->SetGraphicsRootSignature(manager->psoManager->mRootSignature.Get());
	//把所有的材质数据的描述符传给gpu，
    //材质数据所存储的地方是连续的，这里就不用描述符表，用根描述符，因为描述符表需要指定这个表有多少个描述符，不够动态
	auto matBuffer = manager->heapResourceManager->mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());	
	//把这个场景所需要的图片资源的描述符传给gpu
	//图片资源所存储的地方不是连续的，就不能用根描述符，
	mCommandList->SetGraphicsRootDescriptorTable(3, manager->textureManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(4, manager->textureManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootConstantBufferView(8, manager->heapResourceManager->mCurrFrameResource->commonCB->Resource()->GetGPUVirtualAddress());
	//动画数据传递
	manager->textureManager->skin->SetSkinData(mCommandList.Get(), 5);
	//海洋
	for (int i = 0; i < manager->renderManager->TransparentList3.size(); i++)
	{
		manager->renderManager->TransparentList3[i]->meshrender->material->pso = manager->psoManager->mPSOs[waterState == 0 ? "lake" : "ocean"];
	}
	if (waterState == 1)  //海洋
	{
		manager->textureManager->fftOcean->ComputeOcean(mCommandList.Get(), manager->psoManager->mOceanRootSignature.Get(), manager->psoManager->mPSOs,
			gt.TotalTime());
	}
	//粒子特效计算
	manager->textureManager->particle->ComputePos(mCommandList.Get(), manager->psoManager->mParticleSignature.Get(), manager->psoManager->mPSOs,gt.TotalTime());
	//绘制场景中的阴影
	manager->renderManager->renderPassDic["render_shadow_opaque"]->BeginDraw();
	manager->renderManager->renderPassDic["render_shadow_opaque_animator"]->BeginDraw();
	if (renderEnvironment)    //绘制反射球
	{
		mCommandList->DiscardResource(manager->textureManager->getTexture("../../Textures/tree01S.dds")->Resource.Get(), nullptr);
		//绘制环境到立方体贴图上
		manager->renderManager->DrawSceneToCubeMap();
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = manager->textureManager->depthStencil->GetDsvCpuHandle();

	manager->renderManager->renderPassDic["default"]->BeginDraw();
	manager->renderManager->renderPassDic["render_opaque"]->BeginDraw();
	manager->renderManager->renderPassDic["stroke"]->BeginDraw();
	manager->renderManager->renderPassDic["tab_mirror"]->BeginDraw();
	manager->renderManager->renderPassDic["render_mirror"]->BeginDraw();
	manager->renderManager->renderPassDic["render_transparent"]->BeginDraw();

	if (renderEnvironment)    //绘制反射球
	{
		//绘制环境到立方体贴图上
		manager->renderManager->RenderList(manager->renderManager->EnvironmentCube, true);
	}
	

	//将屏幕特效贴图从渲染目标状态变成普通状态
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->depthStencil->Resource.Get(),
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	//一系列的屏幕后处理特效
	if (postEffectState == 2) //高斯模糊，将屏幕后处理贴图模糊一遍
	{
		manager->textureManager->blur->Execute(mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			 manager->textureManager->screenPostMap->Resource(), manager->textureManager->screenPostMap->Rtv(), 3,
			manager->textureManager->screenPostMap->texture->srvIndex,1, &mScreenViewport, &mScissorRect);
		mCommandList->RSSetViewports(1, &mScreenViewport);    //设置视口
		mCommandList->RSSetScissorRects(1, &mScissorRect);    //设置裁剪
	}
	else if (postEffectState == 4)   //bloom特效
	{
		// 先提取较亮的像素
		mCommandList->OMSetRenderTargets(1, &manager->textureManager->bloomMap->Rtv(), true, &dsv);
		mCommandList->SetPipelineState(manager->psoManager->mPSOs["screenGetBloom"]);
		manager->renderManager->RenderList(manager->renderManager->postEffect, false);
		manager->textureManager->blur->Execute(mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			manager->textureManager->bloomMap->Resource(), manager->textureManager->bloomMap->Rtv(), 3, 
			manager->textureManager->bloomMap->texture->srvIndex, 3, &mScreenViewport, &mScissorRect);  //将这个较亮的像素高斯模糊一遍
	}
	//最后在将屏幕后处理贴图按照选定的特效在处理一遍，并渲染到后台缓冲区
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &dsv);
	for (int i = 0; i < manager->renderManager->postEffect.size(); i++)
	{
		Material* m = manager->renderManager->postEffect[i]->meshrender->material;
		if (postEffectState == 1)
		{
			m->pso = manager->psoManager->mPSOs["screenFog"];
		}
		else if (postEffectState == 3)
		{
			m->pso = manager->psoManager->mPSOs["screenBSC"]; 
		}
		else if (postEffectState == 4)
		{
			m->pso = manager->psoManager->mPSOs["screenAddBloom"];
		}
		else
		{
			m->pso = manager->psoManager->mPSOs["screenDefault"];
		}
	}
	manager->renderManager->RenderList(manager->renderManager->postEffect, true);
	//渲染ui
	mCommandList->SetGraphicsRootConstantBufferView(2, 
		manager->cameraManager->uiCamera->UpdateAndGetConstantCB(manager->heapResourceManager->mCurrFrameResourceIndex));
	manager->renderManager->RenderList(manager->renderManager->UIList, true);

	////将后台缓冲区从渲染目标状态变成普通状态
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//关闭命令列表
    ThrowIfFailed(mCommandList->Close());
	//提交命令列表到命令队列
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	//交换后台缓冲区
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	manager->heapResourceManager->mCurrFrameResource->Fence = ++mCurrentFence;
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void MyRenderDemoApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if (manager == nullptr || manager->sceneManager == nullptr)
	{
		return;
	}
	manager->sceneManager->OnMouseDown(btnState, x, y);
}

void MyRenderDemoApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	if (manager == nullptr || manager->sceneManager == nullptr)
	{
		return;
	}
	manager->sceneManager->OnMouseUp(btnState, x, y);
}

void MyRenderDemoApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (manager == nullptr || manager->sceneManager == nullptr)
	{
		return;
	}
	manager->sceneManager->OnMouseMove(btnState, x, y);
}
 
void MyRenderDemoApp::OnKeyboardInput(const GameTimer& gt)
{
	manager->sceneManager->OnKeyboardInput(gt);
	if (GetAsyncKeyState('Z') & 0x8000)
		postEffectState = 0;
	else if (GetAsyncKeyState('X') & 0x8000)
		postEffectState = 1;
	else if (GetAsyncKeyState('C') & 0x8000)
		postEffectState = 2;
	else if (GetAsyncKeyState('V') & 0x8000)
		postEffectState = 3;
	else if (GetAsyncKeyState('B') & 0x8000)
		postEffectState = 4;
	else if (GetAsyncKeyState('O') & 0x8000)
		waterState = 0;
	else if (GetAsyncKeyState('P') & 0x8000)
		waterState = 1;
	else if (GetAsyncKeyState('L') & 0x8000)
		renderEnvironment = true;
	else if (GetAsyncKeyState('K') & 0x8000)
		renderEnvironment = false;

}
