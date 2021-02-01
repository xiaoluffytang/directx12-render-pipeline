#include "PSOManager.h"
#include "Manager.h"


PSOManager::PSOManager(Manager* manager)
{
	this->manager = manager;
	BuildDefaultRootSignature();
	BuildDefaultShadersAndInputLayout();
	BuildDefaultPSO();

	BuildOceanRootSignature();
	BuildOceanComputePSO();

	BuildParticleRootSignature();
	BuildParticleComputePSO();

	BuildScriptRootSignature();
	BuildScriptComputePSO();
}

PSOManager::~PSOManager()
{
}

void PSOManager::BuildDefaultRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;     //2D贴图
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	CD3DX12_DESCRIPTOR_RANGE texTable2;    //立方体贴图
	texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1,2, 0);
	CD3DX12_ROOT_PARAMETER slotRootParameter[8];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);  //用于每个游戏物体渲染的实例化数据
	slotRootParameter[1].InitAsShaderResourceView(1, 1);  //用于场景材质的传递
	slotRootParameter[2].InitAsConstantBufferView(0);    //用于场景常量传递
	//要使用D3D12_SHADER_VISIBILITY_ALL，D3D12_SHADER_VISIBILITY_PIXEL不能在顶点着色器中采样
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_ALL);//用于shader图片资源的传递，2d
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_ALL);//用于shader图片资源的传递，3d
	slotRootParameter[5].InitAsConstantBufferView(1);    //用于场景中动画数据的传递
	slotRootParameter[6].InitAsConstantBufferView(2);    //用于高斯模糊
	slotRootParameter[7].InitAsConstantBufferView(3);    //用于海洋渲染

	//图片的静态采样器集合
	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(8, slotRootParameter, (UINT)staticSamplers.size(),
		staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void PSOManager::BuildDefaultShadersAndInputLayout()
{
	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};
	const D3D_SHADER_MACRO StrokeDefineds[] =
	{
		"STROKE" , "1",
		NULL,NULL
	};
	//创建所有的shader文件
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["uiSpritePS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "uiSpritePS", "ps_5_1");
	mShaders["uiTextPS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "uiTextPS", "ps_5_1");
	mShaders["shadowMapVS"] = d3dUtil::CompileShader(L"Shaders\\shadowMap.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["shadowMapPS"] = d3dUtil::CompileShader(L"Shaders\\shadowMap.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["skyBoxVS"] = d3dUtil::CompileShader(L"Shaders\\skyBox.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skyBoxPS"] = d3dUtil::CompileShader(L"Shaders\\skyBox.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["screenVS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["defaultPS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["FogPS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "FogPS", "ps_5_1");
	mShaders["BriSatConPS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "BriSatConPS", "ps_5_1");
	mShaders["GetBloomPS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "GetBloomPS", "ps_5_1");
	mShaders["AddBloomPS"] = d3dUtil::CompileShader(L"Shaders\\screen.hlsl", nullptr, "AddBloomPS", "ps_5_1");
	mShaders["BlurHorizontalFirstPS"] = d3dUtil::CompileShader(L"Shaders\\blur.hlsl", nullptr, "BlurHorizontalFirstPS", "ps_5_1");
	mShaders["BlurVerticalFirstPS"] = d3dUtil::CompileShader(L"Shaders\\blur.hlsl", nullptr, "BlurVerticalFirstPS", "ps_5_1");
	mShaders["BlurHorizontalPS"] = d3dUtil::CompileShader(L"Shaders\\blur.hlsl", nullptr, "BlurHorizontalPS", "ps_5_1");
	mShaders["BlurVerticalPS"] = d3dUtil::CompileShader(L"Shaders\\blur.hlsl", nullptr, "BlurVerticalPS", "ps_5_1");
	mShaders["BlurCopyPS"] = d3dUtil::CompileShader(L"Shaders\\blur.hlsl", nullptr, "BlurCopyPS", "ps_5_1");
	
	mShaders["lakeVS"] = d3dUtil::CompileShader(L"Shaders\\lake.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["lakePS"] = d3dUtil::CompileShader(L"Shaders\\lake.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["oceanVS"] = d3dUtil::CompileShader(L"Shaders\\fftOcean.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["oceanPS"] = d3dUtil::CompileShader(L"Shaders\\fftOcean.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["animatorVS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", skinnedDefines, "VS", "vs_5_1");
	mShaders["shadowMapAnimatorVS"] = d3dUtil::CompileShader(L"Shaders\\shadowMap.hlsl", skinnedDefines, "VS", "vs_5_1");

	mShaders["cubeTargetReflectVS"] = d3dUtil::CompileShader(L"Shaders\\cubeTargetReflect.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["cubeTargetReflectPS"] = d3dUtil::CompileShader(L"Shaders\\cubeTargetReflect.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["skullVS"] = d3dUtil::CompileShader(L"Shaders\\skull.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skullGS"] = d3dUtil::CompileShader(L"Shaders\\skull.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["skullPS"] = d3dUtil::CompileShader(L"Shaders\\skull.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["strokeVS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", StrokeDefineds, "VS", "vs_5_1");
	mShaders["strokePS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "strokePS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },	
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	
	mAnimatorInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 68, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void PSOManager::BuildDefaultPSO()
{
	//默认的不透明渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = manager->commonInterface.mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = manager->commonInterface.m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = manager->commonInterface.m4xMsaaState ? (manager->commonInterface.m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = manager->commonInterface.mDepthStencilFormat;
	//opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_back"])));

	//默认的透明混合状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentNonePsoDesc = opaquePsoDesc;
	transparentNonePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	transparentNonePsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&transparentNonePsoDesc, IID_PPV_ARGS(&mPSOs["Transparent"])));

	//默认的不透明，不背面剔除渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueNonePsoDesc = opaquePsoDesc;
	opaqueNonePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&opaqueNonePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_None"])));

	// 镜面模板缓冲区状态,模板测试总是通过
	D3D12_GRAPHICS_PIPELINE_STATE_DESC MirrorStencilDesc = opaquePsoDesc;
	//设置混合状态，关闭颜色写入
	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;
	MirrorStencilDesc.BlendState = mirrorBlendState;
	//设置深度模板状态
	D3D12_DEPTH_STENCIL_DESC mirror;   //这个结构体里的所有的参数都需要赋值
	mirror.DepthEnable = true;    //开启深度测试
	mirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;   //关闭深度写入
	mirror.DepthFunc = D3D12_COMPARISON_FUNC_LESS;    //小于当前深度值才会通过模板测试
	mirror.StencilEnable = true;   //开启模板测试
	mirror.StencilReadMask = 0xff;    //模板值读取
	mirror.StencilWriteMask = 0xff;    //模板值写入
	//先是模板测试，然后是深度测试
	mirror.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试失败怎么办，当前像素模板值保持不变
	mirror.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试成功，深度测试失败怎么办，当前像素模板值保持不变
	mirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;  //深度和模板测试都通过怎么办，更新像素的模板值为指定值
	mirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;    //模板测试总是通过
	mirror.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试失败怎么办，当前像素模板值保持不变
	mirror.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试成功，深度测试失败怎么办，当前像素模板值保持不变
	mirror.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;  //深度和模板测试都通过怎么办，更新像素的模板值为指定值
	mirror.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;    //模板测试总是通过
	MirrorStencilDesc.DepthStencilState = mirror;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&MirrorStencilDesc, IID_PPV_ARGS(&mPSOs["Stencil_Always"])));

	// 镜子里的物体的模板缓冲区状态,模板测试相等时通过
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GameObjectInMirrorStencilDesc = opaquePsoDesc;
	D3D12_DEPTH_STENCIL_DESC gameObjectInMirror;
	gameObjectInMirror.DepthEnable = false;    //开启深度测试
	gameObjectInMirror.DepthFunc = D3D12_COMPARISON_FUNC_LESS;    //小于当前深度值才会通过模板测试
	gameObjectInMirror.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;   //关闭深度写入
	gameObjectInMirror.StencilEnable = true;   //开启模板测试
	gameObjectInMirror.StencilReadMask = 0xff;    //模板值读取
	gameObjectInMirror.StencilWriteMask = 0xff;    //模板值写入
	//先是模板测试，然后是深度测试
	gameObjectInMirror.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试失败怎么办，当前像素模板值保持不变
	gameObjectInMirror.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试成功，深度测试失败怎么办，当前像素模板值保持不变
	gameObjectInMirror.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;  //深度和模板测试都通过怎么办，更新像素的模板值为指定值
	gameObjectInMirror.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;    //模板测试相等时通过
	gameObjectInMirror.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试失败怎么办，当前像素模板值保持不变
	gameObjectInMirror.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;    //模板测试成功，深度测试失败怎么办，当前像素模板值保持不变
	gameObjectInMirror.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;  //深度和模板测试都通过怎么办，更新像素的模板值为指定值
	gameObjectInMirror.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;    //模板测试相等时通过
	GameObjectInMirrorStencilDesc.DepthStencilState = gameObjectInMirror;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&GameObjectInMirrorStencilDesc, IID_PPV_ARGS(&mPSOs["Stencil_Equal"])));

	//阴影贴图的渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ShadowMapDesc = opaquePsoDesc;
	ShadowMapDesc.RasterizerState.DepthBias = 100000;
	ShadowMapDesc.RasterizerState.DepthBiasClamp = 0.0f;
	ShadowMapDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	ShadowMapDesc.pRootSignature = mRootSignature.Get();
	ShadowMapDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowMapVS"]->GetBufferPointer()),
		mShaders["shadowMapVS"]->GetBufferSize()
	};
	ShadowMapDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowMapPS"]->GetBufferPointer()),
		mShaders["shadowMapPS"]->GetBufferSize()
	};
	ShadowMapDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	ShadowMapDesc.NumRenderTargets = 0;   //阴影贴图不需要渲染目标
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&ShadowMapDesc, IID_PPV_ARGS(&mPSOs["shadowMap"])));

	//动画模型的阴影贴图的渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC animatorShadowMapDesc = opaquePsoDesc;
	animatorShadowMapDesc.RasterizerState.DepthBias = 100000;
	animatorShadowMapDesc.RasterizerState.DepthBiasClamp = 0.0f;
	animatorShadowMapDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	animatorShadowMapDesc.pRootSignature = mRootSignature.Get();
	animatorShadowMapDesc.InputLayout = { mAnimatorInputLayout.data(), (UINT)mAnimatorInputLayout.size() };
	animatorShadowMapDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowMapAnimatorVS"]->GetBufferPointer()),
		mShaders["shadowMapAnimatorVS"]->GetBufferSize()
	};
	animatorShadowMapDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["shadowMapPS"]->GetBufferPointer()),
		mShaders["shadowMapPS"]->GetBufferSize()
	};
	animatorShadowMapDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	animatorShadowMapDesc.NumRenderTargets = 0;   //阴影贴图不需要渲染目标
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&animatorShadowMapDesc, IID_PPV_ARGS(&mPSOs["animatorShadowMap"])));

	//天空盒的渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyBoxDesc = opaquePsoDesc;
	skyBoxDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyBoxDesc.VS =
	{ 
		reinterpret_cast<BYTE*>(mShaders["skyBoxVS"]->GetBufferPointer()),
		mShaders["skyBoxVS"]->GetBufferSize()
	};
	skyBoxDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyBoxPS"]->GetBufferPointer()),
		mShaders["skyBoxPS"]->GetBufferSize()
	};
	skyBoxDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&skyBoxDesc, IID_PPV_ARGS(&mPSOs["skyBox"])));

	//立方体环境反射
	D3D12_GRAPHICS_PIPELINE_STATE_DESC cubeTargetReflectDesc = opaquePsoDesc;
	cubeTargetReflectDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["cubeTargetReflectVS"]->GetBufferPointer()),
		mShaders["cubeTargetReflectVS"]->GetBufferSize()
	};
	cubeTargetReflectDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["cubeTargetReflectPS"]->GetBufferPointer()),
		mShaders["cubeTargetReflectPS"]->GetBufferSize()
	};
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&cubeTargetReflectDesc, IID_PPV_ARGS(&mPSOs["cubeTargetReflect"])));

	//屏幕后处理--无特效，单纯复制
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenDefaultDesc = opaquePsoDesc;
	screenDefaultDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenDefaultDesc.DepthStencilState.DepthEnable = false;
	screenDefaultDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenDefaultDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["defaultPS"]->GetBufferPointer()),
		mShaders["defaultPS"]->GetBufferSize()
	};
	screenDefaultDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenDefaultDesc, IID_PPV_ARGS(&mPSOs["screenDefault"])));

	//屏幕后处理--雾效
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenFogDesc = opaquePsoDesc;
	screenFogDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenFogDesc.DepthStencilState.DepthEnable = false;
	screenFogDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenFogDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["FogPS"]->GetBufferPointer()),
		mShaders["FogPS"]->GetBufferSize()
	};
	screenFogDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenFogDesc, IID_PPV_ARGS(&mPSOs["screenFog"])));

	//屏幕后处理--饱和度对比度和亮度
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenBSCDesc = opaquePsoDesc;
	screenBSCDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenBSCDesc.DepthStencilState.DepthEnable = false;
	screenBSCDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenBSCDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BriSatConPS"]->GetBufferPointer()),
		mShaders["BriSatConPS"]->GetBufferSize()
	};
	screenBSCDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenBSCDesc, IID_PPV_ARGS(&mPSOs["screenBSC"])));

	//屏幕后处理--Bloom特效(提取高亮像素)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenGetBloomDesc = opaquePsoDesc;
	screenGetBloomDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenGetBloomDesc.DepthStencilState.DepthEnable = false;
	screenGetBloomDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenGetBloomDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["GetBloomPS"]->GetBufferPointer()),
		mShaders["GetBloomPS"]->GetBufferSize()
	};
	screenGetBloomDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenGetBloomDesc, IID_PPV_ARGS(&mPSOs["screenGetBloom"])));

	//屏幕后处理--Bloom特效(添加高亮像素)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenAddBloomDesc = opaquePsoDesc;
	screenAddBloomDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenAddBloomDesc.DepthStencilState.DepthEnable = false;
	screenAddBloomDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenAddBloomDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["AddBloomPS"]->GetBufferPointer()),
		mShaders["AddBloomPS"]->GetBufferSize()
	};
	screenAddBloomDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenAddBloomDesc, IID_PPV_ARGS(&mPSOs["screenAddBloom"])));

	//高斯模糊横向模糊
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenBlurHorizontalDesc = opaquePsoDesc;
	screenBlurHorizontalDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenBlurHorizontalDesc.DepthStencilState.DepthEnable = false;
	screenBlurHorizontalDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenBlurHorizontalDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BlurHorizontalPS"]->GetBufferPointer()),
		mShaders["BlurHorizontalPS"]->GetBufferSize()
	};
	screenBlurHorizontalDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenBlurHorizontalDesc, IID_PPV_ARGS(&mPSOs["BlurHorizontal"])));

	//高斯模糊纵向模糊
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenBlurVerticalDesc = opaquePsoDesc;
	screenBlurVerticalDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenBlurVerticalDesc.DepthStencilState.DepthEnable = false;
	screenBlurVerticalDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenBlurVerticalDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BlurVerticalPS"]->GetBufferPointer()),
		mShaders["BlurVerticalPS"]->GetBufferSize()
	};
	screenBlurVerticalDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenBlurVerticalDesc, IID_PPV_ARGS(&mPSOs["BlurVertical"])));

	//高斯模糊横向模糊第一次
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenBlurHorizontalFirstDesc = opaquePsoDesc;
	screenBlurHorizontalFirstDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenBlurHorizontalFirstDesc.DepthStencilState.DepthEnable = false;
	screenBlurHorizontalFirstDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenBlurHorizontalFirstDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BlurHorizontalFirstPS"]->GetBufferPointer()),
		mShaders["BlurHorizontalFirstPS"]->GetBufferSize()
	};
	screenBlurHorizontalFirstDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenBlurHorizontalFirstDesc, IID_PPV_ARGS(&mPSOs["BlurHorizontalFirst"])));

	//高斯模糊纵向模糊第一次
	D3D12_GRAPHICS_PIPELINE_STATE_DESC screenBlurVerticalFirstDesc = opaquePsoDesc;
	screenBlurVerticalFirstDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	screenBlurVerticalFirstDesc.DepthStencilState.DepthEnable = false;
	screenBlurVerticalFirstDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	screenBlurVerticalFirstDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BlurVerticalFirstPS"]->GetBufferPointer()),
		mShaders["BlurVerticalFirstPS"]->GetBufferSize()
	};
	screenBlurVerticalFirstDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&screenBlurVerticalFirstDesc, IID_PPV_ARGS(&mPSOs["BlurVerticalFirst"])));

	//高斯模糊结果复制
	D3D12_GRAPHICS_PIPELINE_STATE_DESC BlurCopyDesc = opaquePsoDesc;
	BlurCopyDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	BlurCopyDesc.DepthStencilState.DepthEnable = false;
	BlurCopyDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["screenVS"]->GetBufferPointer()),
		mShaders["screenVS"]->GetBufferSize()
	};
	BlurCopyDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["BlurCopyPS"]->GetBufferPointer()),
		mShaders["BlurCopyPS"]->GetBufferSize()
	};
	BlurCopyDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&BlurCopyDesc, IID_PPV_ARGS(&mPSOs["BlurCopy"])));

	//ui图片的渲染
	D3D12_GRAPHICS_PIPELINE_STATE_DESC uiDesc = opaquePsoDesc;
	uiDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	uiDesc.DepthStencilState.DepthEnable = false;
	uiDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	uiDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["uiSpritePS"]->GetBufferPointer()),
		mShaders["uiSpritePS"]->GetBufferSize()
	};
	uiDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	uiDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	uiDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	//uiDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&uiDesc, IID_PPV_ARGS(&mPSOs["uiSprite"])));

	//ui文本的渲染
	D3D12_GRAPHICS_PIPELINE_STATE_DESC uiTextDesc = uiDesc;
	uiDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["uiTextPS"]->GetBufferPointer()),
		mShaders["uiTextPS"]->GetBufferSize()
	};
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&uiDesc, IID_PPV_ARGS(&mPSOs["uiText"])));

	//湖的渲染
	D3D12_GRAPHICS_PIPELINE_STATE_DESC lakeDesc = opaquePsoDesc;
	lakeDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["lakeVS"]->GetBufferPointer()),
		mShaders["lakeVS"]->GetBufferSize()
	};
	lakeDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["lakePS"]->GetBufferPointer()),
		mShaders["lakePS"]->GetBufferSize()
	};
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&lakeDesc, IID_PPV_ARGS(&mPSOs["lake"])));

	//海洋的渲染
	D3D12_GRAPHICS_PIPELINE_STATE_DESC oceanDesc = opaquePsoDesc;
	oceanDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["oceanVS"]->GetBufferPointer()),
		mShaders["oceanVS"]->GetBufferSize()
	};
	oceanDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["oceanPS"]->GetBufferPointer()),
		mShaders["oceanPS"]->GetBufferSize()
	};
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&oceanDesc, IID_PPV_ARGS(&mPSOs["ocean"])));

	//动画模型的渲染
	D3D12_GRAPHICS_PIPELINE_STATE_DESC animatorDesc = opaquePsoDesc;
	animatorDesc.InputLayout = { mAnimatorInputLayout.data(), (UINT)mAnimatorInputLayout.size() };
	animatorDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["animatorVS"]->GetBufferPointer()),
		mShaders["animatorVS"]->GetBufferSize()
	};
	animatorDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	animatorDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT;    //这个动画文件有问题，尽然是正面剔除的模型
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&animatorDesc, IID_PPV_ARGS(&mPSOs["animator"])));

	//头骨粒子特效
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skullDesc = opaquePsoDesc;
	skullDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skullVS"]->GetBufferPointer()),
		mShaders["skullVS"]->GetBufferSize()
	};
	skullDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["skullGS"]->GetBufferPointer()),
		mShaders["skullGS"]->GetBufferSize()
	};
	skullDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["skullPS"]->GetBufferPointer()),
		mShaders["skullPS"]->GetBufferSize()
	};
	skullDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	skullDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&skullDesc, IID_PPV_ARGS(&mPSOs["skull"])));

	//描边
	D3D12_GRAPHICS_PIPELINE_STATE_DESC strokeDesc = opaquePsoDesc;
	strokeDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	strokeDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["strokeVS"]->GetBufferPointer()),
		mShaders["strokeVS"]->GetBufferSize()
	};
	strokeDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["strokePS"]->GetBufferPointer()),
		mShaders["strokePS"]->GetBufferSize()
	};
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&strokeDesc, IID_PPV_ARGS(&mPSOs["stroke"])));

	//默认的使用网格线渲染状态
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
}

void PSOManager::BuildOceanRootSignature()
{
	const int paramCount = 11;
	int texCountDesCount = paramCount - 2;
	CD3DX12_ROOT_PARAMETER slotRootParameter[paramCount];
	
	std::vector<CD3DX12_DESCRIPTOR_RANGE> tables;
	tables.resize(texCountDesCount);
	for (int i = 0; i < texCountDesCount; i++)
	{
		tables[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, i);
		slotRootParameter[i].InitAsDescriptorTable(1, &tables[i]);
	}
	slotRootParameter[texCountDesCount].InitAsConstantBufferView(0);
	slotRootParameter[texCountDesCount + 1].InitAsConstants(1, 1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(paramCount, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mOceanRootSignature.GetAddressOf())));
}

void PSOManager::BuildOceanComputePSO()
{
	std::vector<string> str = { "ComputeGaussianRandom","CreateHeightDisplaceSpectrum",
	"FFTHorizontal","FFTHorizontalEnd","FFTVertical",
	"FFTVerticalEnd","TextureGenerationDisplace","TextureGenerationNormalBubbles", };
	for (int i = 0; i < str.size(); i++)
	{
		string& name = str[i];
		mShaders[name] = d3dUtil::CompileShader(L"Shaders\\fftOceanCS.hlsl", nullptr, name, "cs_5_1");
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = mOceanRootSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(mShaders[name]->GetBufferPointer()),
			mShaders[name]->GetBufferSize()
		};
		ThrowIfFailed(manager->commonInterface.md3dDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mPSOs[name])));
	}
}

void PSOManager::BuildParticleRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE uav1;
	uav1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uav1);
	slotRootParameter[2].InitAsShaderResourceView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mParticleSignature.GetAddressOf())));
}

void PSOManager::BuildParticleComputePSO()
{
	std::vector<string> str = { "ParticleDieAndMove1","ParticleDieAndMove2" };
	for (int i = 0; i < str.size(); i++)
	{
		string& name = str[i];
		mShaders[name] = d3dUtil::CompileShader(L"Shaders\\particleCS.hlsl", nullptr, name, "cs_5_1");
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = mParticleSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(mShaders[name]->GetBufferPointer()),
			mShaders[name]->GetBufferSize()
		};
		ThrowIfFailed(manager->commonInterface.md3dDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mPSOs[name])));
	}
}

void PSOManager::BuildScriptRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	slotRootParameter[0].InitAsUnorderedAccessView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(manager->commonInterface.md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mScriptLandSignature.GetAddressOf())));
}

void PSOManager::BuildScriptComputePSO()
{
	std::vector<string> str = { "Moutain","Moutain2" };
	for (int i = 0; i < str.size(); i++)
	{
		string& name = str[i];
		mShaders[name] = d3dUtil::CompileShader(L"Shaders\\scriptLand.hlsl", nullptr, name, "cs_5_1");
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = mScriptLandSignature.Get();
		desc.CS =
		{
			reinterpret_cast<BYTE*>(mShaders[name]->GetBufferPointer()),
			mShaders[name]->GetBufferSize()
		};
		ThrowIfFailed(manager->commonInterface.md3dDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mPSOs[name])));
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> PSOManager::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow
	};
}