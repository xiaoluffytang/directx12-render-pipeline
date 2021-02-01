#pragma once
#include "../../../Common/d3dUtil.h"
#pragma comment ( lib, "D3D12.lib")
#pragma comment(lib, "d3dcompiler.lib")
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class Manager;
class PSOManager
{
public:
	PSOManager(Manager* manager);
	~PSOManager();
	Manager* manager;
	
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;   //顶点的输入格式
	std::vector<D3D12_INPUT_ELEMENT_DESC> mAnimatorInputLayout;   //顶点的输入格式
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;     //所有的shader文件
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;   //根签名
	ComPtr<ID3D12RootSignature> mOceanRootSignature = nullptr;   //海洋根签名
	ComPtr<ID3D12RootSignature> mParticleSignature = nullptr;   //粒子系统根签名
	ComPtr<ID3D12RootSignature> mScriptLandSignature = nullptr;   //程序地形根签名
	std::unordered_map<std::string, ID3D12PipelineState*> mPSOs;    //所有的渲染状态的集合
private:
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
	void BuildDefaultRootSignature();    //创建默认根签名，用于shader文件所用的资源中，除了顶点缓冲区和索引缓冲区外，其他所有资源的描述
	void BuildDefaultShadersAndInputLayout();   //创建默认的输入布局，用于shader文件中，顶点缓冲区和索引缓冲区的描述
	void BuildDefaultPSO();
	//快速傅里叶变换海洋
	void BuildOceanRootSignature();
	void BuildOceanComputePSO();
	//粒子特效计算
	void BuildParticleRootSignature();
	void BuildParticleComputePSO();
	//程序地形
	void BuildScriptRootSignature();
	void BuildScriptComputePSO();
};

