#pragma once

#include "../Common/d3dUtil.h"
#include "../EngineUtils.h"
using namespace DirectX;
using namespace std;

class Texture2DResource
{
public:
	Texture2DResource();
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;   //图片buff资源
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
	D3D12_SRV_DIMENSION ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	int width;
	int height;
	void LoadTextureByFile(const string& str);    //根据文件地址加载资源
	void LoadTextureByDesc(const D3D12_RESOURCE_DESC* desc, const D3D12_CLEAR_VALUE* clear_desc);

	int srvIndex = -1;    //作为渲染资源描述符的索引
	int rtvIndex = -1;    //作为渲染目标描述符的索引
	int dsvIndex = -1;    //作为深度模板描述符的索引
	int uavIndex = -1;    //作为无序访问描述符的索引
	void CreateSrvIndexDescriptor();   //创建渲染资源描述符
	void CreateRtvIndexDescriptor();   //创建渲染目标描述符
	void CreateDsvIndexDescriptor();   //创建深度模板描述符
	void CreateUavIndexDescriptor();   //创建无序访问视图描述符
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

