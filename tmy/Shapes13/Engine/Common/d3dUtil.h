//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"




inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

/*
#if defined(_DEBUG)
    #ifndef Assert
    #define Assert(x, description)                                  \
    {                                                               \
        static bool ignoreAssert = false;                           \
        if(!ignoreAssert && !(x))                                   \
        {                                                           \
            Debug::AssertResult result = Debug::ShowAssertDialog(   \
            (L#x), description, AnsiToWString(__FILE__), __LINE__); \
        if(result == Debug::AssertIgnore)                           \
        {                                                           \
            ignoreAssert = true;                                    \
        }                                                           \
                    else if(result == Debug::AssertBreak)           \
        {                                                           \
            __debugbreak();                                         \
        }                                                           \
        }                                                           \
    }
    #endif
#else
    #ifndef Assert
    #define Assert(x, description) 
    #endif
#endif 		
    */

class d3dUtil
{
public:
    static bool isDebugModel2;
    static bool IsKeyDown(int vkeyCode);

    static std::string ToString(HRESULT hr);

    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::string& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct SubmeshGeometry
{
	UINT IndexCount = 0;    //���ģ�͵Ķ��������ж��ٸ�
	UINT StartIndexLocation = 0;    //���ģ�͵ĵ�һ�����������ĵ�ַ
	INT BaseVertexLocation = 0;    //���ģ�͵ĵ�һ������ĵ�ַ
	DirectX::BoundingBox Bounds;   //���ģ�͵İ�Χ��
	std::string modelBindMaterialName = "";   //���ģ�ͳ�ʼ����ʱ��󶨵Ĳ�����
};

//����ṹ��洢����Ķ����������ÿһ��������Դ洢һ������ģ�͵�������Ϣ
struct MeshGeometry
{
	//���ģ�͵�����
	std::string Name;

	//���ڱ�����Ĭ�ϻ������е���Դ�������Դ��Ҫ���ϴ�����������
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	//�ϴ����������൱���н飬Ĭ�ϻ���������Դ��Ҫ���ϴ����������ܸ���
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	UINT VertexByteStride = 0;   //һ������ռ�ö����ڴ�
	UINT VertexBufferByteSize = 0;   //������������еĶ����ܹ����˶����ڴ�
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;   //�����洢�ĸ�ʽ
	UINT IndexBufferByteSize = 0;   ////�������������е������ܹ�ռ�˶����ڴ�

	//һ������������Ǻܶ��ģ�͵����ݺϲ��ģ��������򵥵�ģ�ͣ��Ѷ���������ϲ���һ����
	//ע�⣺����ĺϲ�ֻ�ǰѶ���ģ�͵Ķ��㻺���������������������������ѣ����磬�����������������洢������ֵ��û�����ӣ�
	//DrawArgs��ʾÿ��ģ�͵� ������������������������ʼλ�ã����㻺��������ʼλ��
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	//�������㻺�����Ķ�����ͼ
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();    //�������㻺��������ʼλ��
		vbv.StrideInBytes = VertexByteStride;    //һ������ռ�ö����ڴ�
		vbv.SizeInBytes = VertexBufferByteSize;    //������������еĶ����ܹ����˶����ڴ�
		return vbv;
	}

	//���������������������ͼ
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();   //������������������ʼλ��
		ibv.Format = IndexFormat;    //�����Ĵ洢��ʽ
		ibv.SizeInBytes = IndexBufferByteSize;    //�������������е������ܹ�ռ�˶����ڴ�
		return ibv;
	}

	//���ϴ���������Ĭ�ϻ��������������ݺ󣬿����ͷ��ϴ�������������
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 5




struct Texture
{
	// Unique material name for lookup.
	std::string Name;
	int index;
	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.



#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif