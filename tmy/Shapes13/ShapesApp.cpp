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
	void Pick(int x, int y);    //���߼��
	void createScreenPostDepthStencilResource(int srvIndex,int dsvIndex);     //����һ��������Ļ������ʹ�õ����ģ�建����
	Microsoft::WRL::ComPtr<ID3D12Resource> screenPostDSResource = nullptr;
private:
	Manager* manager = nullptr;    //��Ϸ������

    bool mIsWireframe = false;    //�Ƿ�ʹ���߿�ģʽ����Ⱦ
	BoundingFrustum mCamFrustum;     //���ڰ�Χ����

	int postEffectState = 0;   //��Ļ������Ч
	int waterState = 0;   //ʹ�����ַ�ʽ��Ⱦˮ��
	bool renderEnvironment = false;   //�Ƿ���Ⱦ����ӳ�䵽����
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
	//��̨����������ȾĿ����ͼ
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 30;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	//���ģ�建������ͼ�����ﴴ���������һ�������������ģ�建�������������ڶ�������Ӱ��ͼ��������������Ҳ��һ�����ģ�建����������
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
	
	//����һ�������ӿ�
	DeviceCommonInterface commonInterface;
	commonInterface.md3dDevice = md3dDevice;
	commonInterface.mCommandList = mCommandList;
	//������Ϸ������
	manager = new Manager(commonInterface);
	//��Ϸ������ʼ��
	manager->sceneManager->Init();
	//��Ϸ������ʼ����Ϻ󣬴���һЩ����Դ�������������Դ
	manager->heapResourceManager->BuildFrameResources();    //����֡��Դ
    manager->heapResourceManager->BuildDescriptorHeaps();   //������������
	manager->heapResourceManager->BuildSrvViews(mCbvSrvUavDescriptorSize);    //����ͼƬ��Դ������
    manager->heapResourceManager->mCurrFrameResource = manager->heapResourceManager->mFrameResources[manager->heapResourceManager->mCurrFrameResourceIndex].get();
	//��ʼ��shadowMa����ɫ����Դ�������ѵ�0��λ����Ȼ������������ѵ�1��λ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 0, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0 , mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE shadowDsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(),1, mDsvDescriptorSize);
	manager->textureManager->shadowMap->BuildDescriptors(shadowCpuSrv, shadowGpuSrv, shadowDsv);
	//��Ļ����������Դ����ɫ����Դ�������ѵ�1��λ����ȾĿ�껺�����������ѵ�SwapChainBufferCount��λ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE screenPostCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 1, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE screenPostGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 1, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE screenPostRtv(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount, mRtvDescriptorSize);
	manager->textureManager->screenPostMap->BuildDescriptors(screenPostCpuSrv, screenPostGpuSrv, screenPostRtv);
	//�½�һ����Ȼ�������Ϊ������Դ����ɫ����Դ�������ѵ�2��λ����Ȼ������������ѵ�2��λ��
	createScreenPostDepthStencilResource(2,2);
	//��˹ģ�����ú�̨�������������� ����ɫ����Դ�������ѵ�3��λ�͵�4��λ����ȾĿ�껺�����������ѵ�SwapChainBufferCount + 1��λ��SwapChainBufferCount + 2��λ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuSrv0(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE blurGpuSrv0(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 3, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuDsv0(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 1, mRtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuSrv1(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE blurGpuSrv1(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 4, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE blurCpuDsv1(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 2, mRtvDescriptorSize);
	manager->textureManager->blur->BuildDescriptors(blurCpuSrv0, blurGpuSrv0, blurCpuDsv0, blurCpuSrv1, blurGpuSrv1, blurCpuDsv1);
	//��������������Դ������������ɫ����Դ�������ѵ�5��λ��
	InitFont(5);
	//���û���ӳ�����Դ����������ɫ����Դ�������ѵ�6��λ����ȾĿ�껺�����������ѵ�SwapChainBufferCount + 3��λ��SwapChainBufferCount + 8��λ��
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
	//����bloom��Ч����ɫ����Դ�������ѵ�7��λ����ȾĿ�껺�����������ѵ�SwapChainBufferCount��λ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE bloomCpuSrv(manager->heapResourceManager->mSrvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mCbvSrvUavDescriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE bloomGpuSrv(manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart(), 7, mCbvSrvUavDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE bloomRtv(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount + 9, mRtvDescriptorSize);
	manager->textureManager->bloomMap->BuildDescriptors(bloomCpuSrv, bloomGpuSrv, bloomRtv);
	//���ú�����ɫ���ѵĵ�8��λ����7 + 5(5��������Դ) * 2(1����ɫ����Դ��ͼ��1�����������ͼ) = 17��17��λ��
	manager->textureManager->fftOcean->BuildDescriptors(manager->heapResourceManager->mSrvHeap.Get(), 8, mCbvSrvUavDescriptorSize);
	//���ö�����ͼ����ɫ���ѵĵ�18��λ��
	manager->textureManager->skin->BuildAniTex(md3dDevice.Get(), mCommandList.Get(), manager->heapResourceManager->mSrvHeap.Get(),18, mCbvSrvUavDescriptorSize);
	//����������Ч��ͼ����ɫ���ѵĵ�19��λ��
	manager->textureManager->particle->BuildParticleTex(md3dDevice.Get(), mCommandList.Get(), manager->heapResourceManager->mSrvHeap.Get(), 19, mCbvSrvUavDescriptorSize);
	//��ʼ�����
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
    FlushCommandQueue();
	OnResize();

    return true;
}

//����һ��������Ļ��������ģ�建����
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
		//���ڱ仯���������һЩ����ҲҪ�仯
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
	//���ó�ʼPSO
	string psoName = mIsWireframe ? "opaque_wireframe" : "opaque_back";
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), manager->psoManager->mPSOs[psoName]));
	//������������
	ID3D12DescriptorHeap* descriptorHeaps[] = { manager->heapResourceManager->mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	//���ø�ǩ��
	mCommandList->SetGraphicsRootSignature(manager->psoManager->mRootSignature.Get());
	//�����еĲ������ݵ�����������gpu��
    //�����������洢�ĵط��������ģ�����Ͳ������������ø�����������Ϊ����������Ҫָ��������ж��ٸ���������������̬
	auto matBuffer = manager->heapResourceManager->mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(1, matBuffer->GetGPUVirtualAddress());	
	//�������������Ҫ��ͼƬ��Դ������������gpu
	//ͼƬ��Դ���洢�ĵط����������ģ��Ͳ����ø���������
	mCommandList->SetGraphicsRootDescriptorTable(3, manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(4, manager->heapResourceManager->mSrvHeap->GetGPUDescriptorHandleForHeapStart());
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
	manager->renderManager->DrawShadowMap();
	//���ƻ�������������ͼ��
	if (renderEnvironment) manager->renderManager->DrawSceneToCubeMap();
	//���Ƴ����е�����
    mCommandList->RSSetViewports(1, &mScreenViewport);    //�����ӿ�
    mCommandList->RSSetScissorRects(1, &mScissorRect);    //���òü�
	//����̨��������Ϊ��ȾĿ��״̬
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	//����Ļ������ͼ��Ϊ��ȾĿ��״̬
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	//�����̨������
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	//������Ļ������ͼ
	mCommandList->ClearRenderTargetView(manager->textureManager->screenPostMap->Rtv(), Colors::LightSteelBlue, 0, nullptr);
	//����ģ�建����
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(mDsvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mDsvDescriptorSize);
    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(screenPostDSResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	//������ȾĿ��
    mCommandList->OMSetRenderTargets(1, &manager->textureManager->screenPostMap->Rtv(), true, &dsv);
	//�������������Ҫ��һЩ�������ݴ���gpu
	auto passCB = manager->heapResourceManager->mCurrFrameResource->PassCB->Resource();
	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 0 * passCBByteSize);

	//�������
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["stroke"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList2, false);
	//�������е�����
	//�Ȱ����в�͸��������Ⱦ��
	//mCommandList->SetPipelineState(manager->psoManager->mPSOs["opaque_wireframe"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList1,true);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList2, true);
	manager->renderManager->RenderList(manager->renderManager->OpaqueAnimatorList1, true);
	
	//manager->renderManager->RenderList(manager->renderManager->OpaqueList3, false);
	//��ģ�建�����еľ������ر��Ϊ1
	mCommandList->OMSetStencilRef(1);
	//��ʼ���ƾ��棬�����ü��ľ���ģ��ֵ��Ϊ�������õ�ģ��ֵ
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["Stencil_Always"]);
	manager->renderManager->RenderList(manager->renderManager->TransparentList2,false);
	//���ƾ����������Ϸ����
	mCommandList->SetPipelineState(manager->psoManager->mPSOs["Stencil_Equal"]);
	manager->renderManager->RenderList(manager->renderManager->OpaqueList3,false);
	//��ģ�建���йص���Ⱦ����ˣ���ģ��ֵ�ָ�Ĭ��
	mCommandList->OMSetStencilRef(0);
	
	
	manager->renderManager->RenderList(manager->renderManager->TransparentList1, true);
	manager->renderManager->RenderList(manager->renderManager->TransparentList2, true);
	manager->renderManager->RenderList(manager->renderManager->TransparentList3, true);
	if (renderEnvironment) manager->renderManager->RenderList(manager->renderManager->EnvironmentCube, true);
	manager->renderManager->RenderList(manager->renderManager->SkyBoxList, true);
	

	//����Ļ��Ч��ͼ����ȾĿ��״̬�����ͨ״̬
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(manager->textureManager->screenPostMap->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(screenPostDSResource.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
	//һϵ�е���Ļ������Ч
	if (postEffectState == 2) //��˹ģ��������Ļ������ͼģ��һ��
	{
		manager->textureManager->blur->Execute(manager, mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			 manager->textureManager->screenPostMap->Resource(), manager->textureManager->screenPostMap->Rtv(), 3,1,1, &mScreenViewport, &mScissorRect);
		mCommandList->RSSetViewports(1, &mScreenViewport);    //�����ӿ�
		mCommandList->RSSetScissorRects(1, &mScissorRect);    //���òü�
	}
	else if (postEffectState == 4)   //bloom��Ч
	{
		// ����ȡ����������
		mCommandList->OMSetRenderTargets(1, &manager->textureManager->bloomMap->Rtv(), true, &dsv);   
		mCommandList->SetPipelineState(manager->psoManager->mPSOs["screenGetBloom"]);
		manager->renderManager->RenderList(manager->renderManager->postEffect, false);
		manager->textureManager->blur->Execute(manager, mCommandList.Get(), manager->psoManager->mRootSignature.Get(),
			manager->textureManager->bloomMap->Resource(), manager->textureManager->bloomMap->Rtv(), 3, 7, 3, &mScreenViewport, &mScissorRect);    //��������������ظ�˹ģ��һ��
	}
	//����ڽ���Ļ������ͼ����ѡ������Ч�ڴ���һ�飬����Ⱦ����̨������
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
	//��Ⱦui
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 2 * passCBByteSize);
	manager->renderManager->RenderList(manager->renderManager->UIList, true);

	//����̨����������ȾĿ��״̬�����ͨ״̬
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
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
