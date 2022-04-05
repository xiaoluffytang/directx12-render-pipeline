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
	void Pick(int x, int y);    //���߼��
	Microsoft::WRL::ComPtr<ID3D12Resource> screenPostDSResource = nullptr;
private:
	//Manager* manager = nullptr;    //��Ϸ������

    bool mIsWireframe = false;    //�Ƿ�ʹ���߿�ģʽ����Ⱦ
	BoundingFrustum mCamFrustum;     //���ڰ�Χ����

	int postEffectState = 0;   //��Ļ������Ч
	int waterState = 1;   //ʹ�����ַ�ʽ��Ⱦˮ��
	bool renderEnvironment = false;   //�Ƿ���Ⱦ����ӳ�䵽����
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
	//��̨����������ȾĿ����ͼ
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 100;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//���ģ�建������ͼ�����ﴴ���������һ�������������ģ�建�������������ڶ�������Ӱ��ͼ��������������Ҳ��һ�����ģ�建����������
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
	
	//����һ�������ӿ�
	DeviceCommonInterface commonInterface;
	commonInterface.md3dDevice = md3dDevice;
	commonInterface.mCommandList = mCommandList;
	commonInterface.mScreenViewport = mScreenViewport;
	commonInterface.mScissorRect = mScissorRect;
	commonInterface.mClientWidth = mClientWidth;
	commonInterface.mClientHeight = mClientHeight;
	commonInterface.isDebugModel = getIsDebugModel();
	//������Ϸ������
	manager = new Manager(commonInterface, mRtvHeap, mDsvHeap);
	manager->Init();
	//��Ϸ������ʼ��
	manager->sceneManager->Init();
	
	//��Ϸ������ʼ����Ϻ󣬴���һЩ����Դ�������������Դ
	manager->heapResourceManager->BuildFrameResources();    //����֡��Դ
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
	//��ʼ�����
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
		//���ڱ仯���������һЩ����ҲҪ�仯
		manager->cameraManager->mainCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		manager->cameraManager->uiCamera->size = mClientWidth;
		manager->cameraManager->uiCamera->SetLens(0, AspectRatio(), 1.0f, 1000.0f);
		//��ͼ��С�ı�
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
	//���ó�ʼPSO
	string psoName = mIsWireframe ? "opaque_wireframe" : "opaque_back";
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), manager->psoManager->mPSOs[psoName]));
	//������������
	ID3D12DescriptorHeap* descriptorHeaps[] = { manager->textureManager->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	
	//���ø�ǩ��
	mCommandList->SetGraphicsRootSignature(manager->psoManager->mRootSignature.Get());
	//�����еĲ������ݵ�����������gpu��
    //�����������洢�ĵط��������ģ�����Ͳ������������ø�����������Ϊ����������Ҫָ��������ж��ٸ���������������̬
	auto matBuffer = manager->heapResourceManager->mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());	
	//�������������Ҫ��ͼƬ��Դ������������gpu
	//ͼƬ��Դ���洢�ĵط����������ģ��Ͳ����ø���������
	mCommandList->SetGraphicsRootDescriptorTable(3, manager->textureManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(4, manager->textureManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootConstantBufferView(8, manager->heapResourceManager->mCurrFrameResource->commonCB->Resource()->GetGPUVirtualAddress());
	//�������ݴ���
	manager->textureManager->skin->SetSkinData(mCommandList.Get(), 5);
	//����
	for (int i = 0; i < manager->renderManager->TransparentList3.size(); i++)
	{
		manager->renderManager->TransparentList3[i]->meshrender->material->pso = manager->psoManager->mPSOs[waterState == 0 ? "lake" : "ocean"];
	}
	if (waterState == 1)  //����
	{
		manager->textureManager->fftOcean->ComputeOcean(mCommandList.Get(), manager->psoManager->mOceanRootSignature.Get(), manager->psoManager->mPSOs,
			gt.TotalTime());
	}
	//������Ч����
	manager->textureManager->particle->ComputePos(mCommandList.Get(), manager->psoManager->mParticleSignature.Get(), manager->psoManager->mPSOs,gt.TotalTime());
	//���Ƴ����е���Ӱ
	manager->renderManager->renderPassDic["render_shadow_opaque"]->BeginDraw();
	manager->renderManager->renderPassDic["render_shadow_opaque_animator"]->BeginDraw();
	if (renderEnvironment)    //���Ʒ�����
	{
		mCommandList->DiscardResource(manager->textureManager->getTexture("../../Textures/tree01S.dds")->Resource.Get(), nullptr);
		//���ƻ�������������ͼ��
		manager->renderManager->DrawSceneToCubeMap();
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = manager->textureManager->depthStencil->GetDsvCpuHandle();

	manager->renderManager->renderPassDic["default"]->BeginDraw();
	manager->renderManager->renderPassDic["render_opaque"]->BeginDraw();
	manager->renderManager->renderPassDic["stroke"]->BeginDraw();
	manager->renderManager->renderPassDic["tab_mirror"]->BeginDraw();
	manager->renderManager->renderPassDic["render_mirror"]->BeginDraw();
	manager->renderManager->renderPassDic["render_transparent"]->BeginDraw();

	if (renderEnvironment)    //���Ʒ�����
	{
		//���ƻ�������������ͼ��
		manager->renderManager->RenderList(manager->renderManager->EnvironmentCube, true);
	}
	

	//����Ļ��Ч��ͼ����ȾĿ��״̬�����ͨ״̬
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->depthStencil->Resource.Get(),
	//	D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	//һϵ�е���Ļ������Ч
	if (postEffectState == 2) //��˹ģ��������Ļ������ͼģ��һ��
	{
		manager->textureManager->blur->Execute(mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			 manager->textureManager->screenPostMap->Resource(), manager->textureManager->screenPostMap->Rtv(), 3,
			manager->textureManager->screenPostMap->texture->srvIndex,1, &mScreenViewport, &mScissorRect);
		mCommandList->RSSetViewports(1, &mScreenViewport);    //�����ӿ�
		mCommandList->RSSetScissorRects(1, &mScissorRect);    //���òü�
	}
	else if (postEffectState == 4)   //bloom��Ч
	{
		// ����ȡ����������
		mCommandList->OMSetRenderTargets(1, &manager->textureManager->bloomMap->Rtv(), true, &dsv);
		mCommandList->SetPipelineState(manager->psoManager->mPSOs["screenGetBloom"]);
		manager->renderManager->RenderList(manager->renderManager->postEffect, false);
		manager->textureManager->blur->Execute(mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			manager->textureManager->bloomMap->Resource(), manager->textureManager->bloomMap->Rtv(), 3, 
			manager->textureManager->bloomMap->texture->srvIndex, 3, &mScreenViewport, &mScissorRect);  //��������������ظ�˹ģ��һ��
	}
	//����ڽ���Ļ������ͼ����ѡ������Ч�ڴ���һ�飬����Ⱦ����̨������
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
	//��Ⱦui
	mCommandList->SetGraphicsRootConstantBufferView(2, 
		manager->cameraManager->uiCamera->UpdateAndGetConstantCB(manager->heapResourceManager->mCurrFrameResourceIndex));
	manager->renderManager->RenderList(manager->renderManager->UIList, true);

	////����̨����������ȾĿ��״̬�����ͨ״̬
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//�ر������б�
    ThrowIfFailed(mCommandList->Close());
	//�ύ�����б��������
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	//������̨������
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
