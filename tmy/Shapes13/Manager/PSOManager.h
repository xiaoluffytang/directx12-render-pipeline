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
	
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;   //����������ʽ
	std::vector<D3D12_INPUT_ELEMENT_DESC> mAnimatorInputLayout;   //����������ʽ
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;     //���е�shader�ļ�
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;   //��ǩ��
	ComPtr<ID3D12RootSignature> mOceanRootSignature = nullptr;   //�����ǩ��
	ComPtr<ID3D12RootSignature> mParticleSignature = nullptr;   //����ϵͳ��ǩ��
	ComPtr<ID3D12RootSignature> mScriptLandSignature = nullptr;   //������θ�ǩ��
	std::unordered_map<std::string, ID3D12PipelineState*> mPSOs;    //���е���Ⱦ״̬�ļ���
private:
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
	void BuildDefaultRootSignature();    //����Ĭ�ϸ�ǩ��������shader�ļ����õ���Դ�У����˶��㻺�����������������⣬����������Դ������
	void BuildDefaultShadersAndInputLayout();   //����Ĭ�ϵ����벼�֣�����shader�ļ��У����㻺����������������������
	void BuildDefaultPSO();
	//���ٸ���Ҷ�任����
	void BuildOceanRootSignature();
	void BuildOceanComputePSO();
	//������Ч����
	void BuildParticleRootSignature();
	void BuildParticleComputePSO();
	//�������
	void BuildScriptRootSignature();
	void BuildScriptComputePSO();
};

