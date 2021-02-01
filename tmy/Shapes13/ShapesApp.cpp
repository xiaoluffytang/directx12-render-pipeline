#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "FrameResource.h"
#include "Manager/Manager.h"
#include "ModelObj.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment ( lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")

const int gNumFrameResources = 3;

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
	void createScreenPostDepthStencilResource(int srvIndex,int dsvIndex);     //创建一个用于屏幕后处理来使用的深度模板缓冲区
	Microsoft::WRL::ComPtr<ID3D12Resource> screenPostDSResource = nullptr;
private:
	Manager* manager = nullptr;    //游戏管理器

    bool mIsWireframe = false;    //是否使用线框模式来渲染
	BoundingFrustum mCamFrustum;     //用于包围体检测

	int postEffectState = 0;   //屏幕后处理特效
	int waterState = 0;   //使用哪种方式渲染水面
	bool renderEnvironment = false;   //是否渲染环境映射到球体
	void InitFont(int index);
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
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 30;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//深度模板缓冲区视图，这里创建多个，第一个是真正的深度模板缓冲区描述符，第二个是阴影贴图描述符，第三个也是一个深度模板缓冲区描述符
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 10;
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
	//创建游戏管理器
	manager = new Manager(commonInterface);
	//游戏场景初始化
	manager->sceneManager->Init();
	//游戏场景初始化完毕后，创建一些堆资源和相关描述符资源
	manager->heapResourceManager->BuildFrameResources();    //创建帧资源
    manager->heapResourceManager->BuildDescriptorHeaps();   //创建描述符堆
	manager->heapResourceManager->BuildSrvViews(mCbvSrvUavDescriptorSize);    //创建图片资源描述符
    manager->heapResourceManager->mCurrFrameResource = manager->heapResourceManager->mFrameResources[manager->heapResourceManager->mCurrFrameResourceIndex].get();
	//初始化shadowMa（着色器资源描述符堆第0号位，深度缓冲区描述符堆第1号位）
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 0, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0 , mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowDsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(),1, mDsvDescriptorSize);
	manager->textureManager->shadowMap->BuildDescriptors(shadowCpuSrv, shadowGpuSrv, shadowDsv);
	//屏幕后处理纹理资源（着色器资源描述符堆第1号位，渲染目标缓冲区描述符堆第SwapChainBufferCount号位）
	CD3DX12_CPU_DESCRIPTOR_HANDLE screenPostCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 1, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE screenPostGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 1, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE screenPostRtv(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount, mRtvDescriptorSize);
	manager->textureManager->screenPostMap->BuildDescriptors(screenPostCpuSrv, screenPostGpuSrv, screenPostRtv);
	//新建一个深度缓冲区作为纹理资源（着色器资源描述符堆第2号位，深度缓冲区描述符堆第2号位）
	createScreenPostDepthStencilResource(2,2);
	//高斯模糊设置后台缓冲区的描述符 （着色器资源描述符堆第3号位和第4号位，渲染目标缓冲区描述符堆第SwapChainBufferCount + 1号位和SwapChainBufferCount + 2号位）
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuSrv0(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE blurGpuSrv0(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuDsv0(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 1, mRtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuSrv1(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE blurGpuSrv1(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 4, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuDsv1(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 2, mRtvDescriptorSize);
	manager->textureManager->blur->BuildDescriptors(blurCpuSrv0, blurGpuSrv0, blurCpuDsv0, blurCpuSrv1, blurGpuSrv1, blurCpuDsv1);
	//设置字体纹理资源的描述符（着色器资源描述符堆第5号位）
	InitFont(5);
	//设置环境映射的资源描述符（着色器资源描述符堆第6号位，渲染目标缓冲区描述符堆第SwapChainBufferCount + 3号位到SwapChainBufferCount + 8号位）
	CD3DX12_CPU_DESCRIPTOR_HANDLE cubeRtvHandles[6];
	auto rtvCpuStart = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	int rtvOffset = SwapChainBufferCount + 3;
	for (int i = 0; i < 6; ++i)
		cubeRtvHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvCpuStart, rtvOffset + i, mRtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cubeTargetCpuDsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mDsvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cubeTargetCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 6, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cubeTargetGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 6, mCbvSrvUavDescriptorSize);
	manager->textureManager->cubeTarget->BuildDescriptors(cubeTargetCpuSrv, cubeTargetGpuSrv, cubeRtvHandles, cubeTargetCpuDsv);
	manager->textureManager->cubeTarget->BuildCubeFaceCamera(0.0f,10.0f,-25.0f);
	//设置bloom特效（着色器资源描述符堆第7号位，渲染目标缓冲区描述符堆第SwapChainBufferCount号位）
	CD3DX12_CPU_DESCRIPTOR_HANDLE bloomCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE bloomGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 7, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE bloomRtv(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 9, mRtvDescriptorSize);
	manager->textureManager->bloomMap->BuildDescriptors(bloomCpuSrv, bloomGpuSrv, bloomRtv);
	//设置海洋（着色器堆的第8号位到（7 + 5(5个纹理资源) * 2(1个着色器资源视图，1个无序访问视图) = 17）17号位）
	manager->textureManager->fftOcean->BuildDescriptors(manager->heapResourceManager->mSrvHeap.Get(), 8, mCbvSrvUavDescriptorSize);
	//设置动画贴图（着色器堆的第18号位）
	manager->textureManager->skin->BuildAniTex(md3dDevice.Get(), mCommandList.Get(), manager->heapResourceManager->mSrvHeap.Get(),18, mCbvSrvUavDescriptorSize);
	//设置粒子特效贴图（着色器堆的第19号位）
	manager->textureManager->particle->BuildParticleTex(md3dDevice.Get(), mCommandList.Get(), manager->heapResourceManager->mSrvHeap.Get(), 19, mCbvSrvUavDescriptorSize);
	//初始化完毕
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
    FlushCommandQueue();
	OnResize();

    return true;
}

//创建一个用于屏幕后处理的深度模板缓冲区
void MyRenderDemoApp::createScreenPostDepthStencilResource(int srvIndex, int dsvIndex)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mClientWidth;
	texDesc.Height = mClientHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 0.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&screenPostDSResource)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE DSCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), srvIndex, mCbvSrvUavDescriptorSize);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(screenPostDSResource.Get(), &srvDesc, DSCpuSrv);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(), dsvIndex, mDsvDescriptorSize);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(screenPostDSResource.Get(), &dsvDesc, dsv);
}
 
void MyRenderDemoApp::OnResize()
{
    D3DApp::OnResize();
	if (manager != nullptr)
	{
		//窗口变化后，摄像机的一些参数也要变化
		manager->cameraManager->mainCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		manager->cameraManager->uiCamera->size = mClientWidth;
		manager->cameraManager->uiCamera->SetLens(0, AspectRatio(), 1.0f, 1000.0f);
		
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
	manager->heapResourceManager->UpdateObjectCBs(gt, mCamFrustum);
	manager->heapResourceManager->UpdateMaterials(gt);
	manager->heapResourceManager->UpdateShadowTransform(gt);
	manager->heapResourceManager->UpdateShadowPassCB(gt);
	manager->heapResourceManager->UpdateMainPassCB(gt);
	manager->heapResourceManager->UpdateUIPassCB(gt);
	manager->heapResourceManager->UpdateCubeTargetPassCB(gt);
}

void MyRenderDemoApp::Draw(const GameTimer& gt)
{
	
    auto cmdListAlloc = manager->heapResourceManager->mCurrFrameResource->CmdListAlloc;
    ThrowIfFailed(cmdListAlloc->Reset());
	//设置初始PSO
	string psoName = mIsWireframe ? "opaque_wireframe" : "opaque_back";
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), manager->psoManager->mPSOs[psoName]));
	//设置描述符堆
	ID3D12DescriptorHeap* descriptorHeaps[] = { manager->heapResourceManager->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	//设置根签名
	mCommandList->SetGraphicsRootSignature(manager->psoManager->mRootSignature.Get());
	//把所有的材质数据的描述符传给gpu，
    //材质数据所存储的地方是连续的，这里就不用描述符表，用根描述符，因为描述符表需要指定这个表有多少个描述符，不够动态
	auto matBuffer = manager->heapResourceManager->mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());	
	//把这个场景所需要的图片资源的描述符传给gpu
	//图片资源所存储的地方不是连续的，就不能用根描述符，
	mCommandList->SetGraphicsRootDescriptorTable(3, manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(4, manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
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
	manager->renderManager->DrawShadowMap();
	//绘制环境到立方体贴图上
	if (renderEnvironment) manager->renderManager->DrawSceneToCubeMap();
	//绘制场景中的物体
    mCommandList->RSSetViewports(1, &mScreenViewport);    //设置视口
    mCommandList->RSSetScissorRects(1, &mScissorRect);    //设置裁剪
	//将后台缓冲区变为渲染目标状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	//将屏幕后处理贴图变为渲染目标状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	//清理后台缓冲区
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	//清理屏幕后处理贴图
	mCommandList->ClearRenderTargetView(manager->textureManager->screenPostMap->Rtv(), Colors::LightSteelBlue, 0, nullptr);
	//清理模板缓冲区
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mDsvDescriptorSize);
    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(screenPostDSResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	//设置渲染目标
    mCommandList->OMSetRenderTargets(1, &manager->textureManager->screenPostMap->Rtv(), true, &dsv);
	//把这个场景所需要的一些公关数据传给gpu
	auto passCB = manager->heapResourceManager->mCurrFrameResource->PassCB->Resource();
	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 0 * passCBByteSize);

	//绘制描边
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["stroke"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList2, false);
	//绘制所有的物体
	//先把所有不透明物体渲染完
	//mCommandList->SetPipelineState(manager->psoManager->mPSOs["opaque_wireframe"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList1,true);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList2, true);
	manager->renderManager->RenderList(manager->renderManager->OpaqueAnimatorList1, true);
	
	//manager->renderManager->RenderList(manager->renderManager->OpaqueList3, false);
	//将模板缓冲区中的镜面像素标记为1
	mCommandList->OMSetStencilRef(1);
	//开始绘制镜面，将看得见的镜面模板值变为上面设置的模板值
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["Stencil_Always"]);
	manager->renderManager->RenderList(manager->renderManager->TransparentList2,false);
	//绘制镜面里面的游戏物体
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["Stencil_Equal"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList3,false);
	//和模板缓冲有关的渲染完毕了，将模板值恢复默认
	mCommandList->OMSetStencilRef(0);
	
	
	manager->renderManager->RenderList(manager->renderManager->TransparentList1, true);
	manager->renderManager->RenderList(manager->renderManager->TransparentList2, true);
	manager->renderManager->RenderList(manager->renderManager->TransparentList3, true);
	if (renderEnvironment) manager->renderManager->RenderList(manager->renderManager->EnvironmentCube, true);
	manager->renderManager->RenderList(manager->renderManager->SkyBoxList, true);
	

	//将屏幕特效贴图从渲染目标状态变成普通状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(screenPostDSResource.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
	//一系列的屏幕后处理特效
	if (postEffectState == 2) //高斯模糊，将屏幕后处理贴图模糊一遍
	{
		manager->textureManager->blur->Execute(manager, mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			 manager->textureManager->screenPostMap->Resource(), manager->textureManager->screenPostMap->Rtv(), 3,1,1, &mScreenViewport, &mScissorRect);
		mCommandList->RSSetViewports(1, &mScreenViewport);    //设置视口
		mCommandList->RSSetScissorRects(1, &mScissorRect);    //设置裁剪
	}
	else if (postEffectState == 4)   //bloom特效
	{
		// 先提取较亮的像素
		mCommandList->OMSetRenderTargets(1, &manager->textureManager->bloomMap->Rtv(), true, &dsv);   
		mCommandList->SetPipelineState(manager->psoManager->mPSOs["screenGetBloom"]);
		manager->renderManager->RenderList(manager->renderManager->postEffect, false);
		manager->textureManager->blur->Execute(manager, mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			manager->textureManager->bloomMap->Resource(), manager->textureManager->bloomMap->Rtv(), 3, 7, 3, &mScreenViewport, &mScissorRect);    //将这个较亮的像素高斯模糊一遍
	}
	//最后在将屏幕后处理贴图按照选定的特效在处理一遍，并渲染到后台缓冲区
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
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
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 2 * passCBByteSize);
	manager->renderManager->RenderList(manager->renderManager->UIList, true);

	//将后台缓冲区从渲染目标状态变成普通状态
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
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

void MyRenderDemoApp::InitFont(int index)
{
	Font* font = manager->fontManager->font;
	CD3DX12_CPU_DESCRIPTOR_HANDLE DSCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), index, mCbvSrvUavDescriptorSize);
	font->CreateSrv(DSCpuSrv);
}

void MyRenderDemoApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	manager->sceneManager->OnMouseDown(btnState, x, y);
}

void MyRenderDemoApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	manager->sceneManager->OnMouseUp(btnState, x, y);
}

void MyRenderDemoApp::OnMouseMove(WPARAM btnState, int x, int y)
{
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
