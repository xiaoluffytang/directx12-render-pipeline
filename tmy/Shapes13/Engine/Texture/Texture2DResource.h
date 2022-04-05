#pragma once

#include "../Common/d3dUtil.h"
#include "../EngineUtils.h"
using namespace DirectX;
using namespace std;

class Texture2DResource
{
public:
	Texture2DResource();
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;   //ͼƬbuff��Դ
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
	D3D12_SRV_DIMENSION ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	int width;
	int height;
	void LoadTextureByFile(const string& str);    //�����ļ���ַ������Դ
	void LoadTextureByDesc(const D3D12_RESOURCE_DESC* desc, const D3D12_CLEAR_VALUE* clear_desc);

	int srvIndex = -1;    //��Ϊ��Ⱦ��Դ������������
	int rtvIndex = -1;    //��Ϊ��ȾĿ��������������
	int dsvIndex = -1;    //��Ϊ���ģ��������������
	int uavIndex = -1;    //��Ϊ�������������������
	void CreateSrvIndexDescriptor();   //������Ⱦ��Դ������
	void CreateRtvIndexDescriptor();   //������ȾĿ��������
	void CreateDsvIndexDescriptor();   //�������ģ��������
	void CreateUavIndexDescriptor();   //�������������ͼ������
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetSrvCpuHandle();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetRtvGpuHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvCpuHandle();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetDsvGpuHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetUavCpuHandle();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetUavGpuHandle();
	virtual D3D12_SRV_DIMENSION GetDimension();
};

