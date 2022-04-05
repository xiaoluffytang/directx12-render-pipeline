#pragma once
#include "Texture2DResource.h"

class Texture3DResource: public Texture2DResource
{
public:
	Texture3DResource();
	std::vector<int> rtvIndexs;
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> handles;
	void CreateRtvDescriptors();
	virtual D3D12_SRV_DIMENSION GetDimension()override;
};

