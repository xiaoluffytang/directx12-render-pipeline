#include "RenderPassBase.h"
#include "../Manager/Manager.h"
extern Manager* manager;

void RenderPassBase::BeginDraw()
{

}

ID3D12GraphicsCommandList* RenderPassBase::GetCommandList()
{
	if (manager == nullptr)
	{
		return nullptr;
	}
	auto mCommandList = manager->commonInterface.mCommandList.Get();
	return mCommandList;
}

void RenderPassBase::ClearRenderTarget(XMVECTORF32 color)
{
	auto cmdList = GetCommandList();
	if (renderTarget == nullptr || cmdList == nullptr)
	{
		return;
	}
	cmdList->ClearRenderTargetView(renderTarget->GetRtvCpuHandle(), color, 0, nullptr);
}

void RenderPassBase::ClearDepthStencil(float depth, UINT8 stencil)
{
	auto cmdList = GetCommandList();
	if (depthStencil == nullptr || cmdList == nullptr)
	{
		return;
	}
	cmdList->ClearDepthStencilView(depthStencil->GetDsvCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
}

void RenderPassBase::SetViewports()
{
	auto cmdList = GetCommandList();
	if (cmdList == nullptr)
	{
		return;
	}
	cmdList->RSSetViewports(1, &viewport);
}

void RenderPassBase::SetScissorRects()
{
	auto cmdList = GetCommandList();
	if (cmdList == nullptr)
	{
		return;
	}
	cmdList->RSSetScissorRects(1, &rect);
}

void RenderPassBase::SetResourceState(Texture2DResource* texture, D3D12_RESOURCE_STATES state1, D3D12_RESOURCE_STATES state2)
{
	auto cmdList = GetCommandList();
	if (cmdList == nullptr || texture == nullptr)
	{
		return;
	}
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->Resource.Get(), state1, state2));
}

void RenderPassBase::SetRenderTargets()
{
	auto cmdList = GetCommandList();
	if (cmdList == nullptr)
	{
		return;
	}
	int rtv_count = renderTarget == nullptr ? 0: 1;
	D3D12_CPU_DESCRIPTOR_HANDLE* address1 = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE* address2 = nullptr;
	if (renderTarget != nullptr)
	{
		address1 = &renderTarget->GetRtvCpuHandle();
	}
	if (depthStencil != nullptr)
	{
		address2 = &depthStencil->GetDsvCpuHandle();
	}
	cmdList->OMSetRenderTargets(rtv_count, address1, true, address2);
}

void RenderPassBase::SetTargetPso(ID3D12PipelineState* pso)
{
	auto cmdList = GetCommandList();
	if (cmdList == nullptr || pso == nullptr)
	{
		return;
	}
	cmdList->SetPipelineState(pso);
}

void RenderPassBase::RenderList()
{
	for (size_t i = 0; i < list.size(); ++i)
	{
		auto arr = list[i];
		for (size_t i = 0; i < arr.size(); ++i)
		{
			auto ri = arr[i];
			DrawOneGameObject(ri);
		}
	}
}

void RenderPassBase::DrawOneGameObject(GameObject* go)
{
	auto cmdList = GetCommandList();
	if (go == nullptr || cmdList == nullptr)
	{
		return;
	}
	//设置游戏物体的实例化缓冲区，这个没有优化空间
	if (go->renderCount <= 0)
	{
		return;
	}
	if (targetPso == nullptr)
	{
		cmdList->SetPipelineState(go->meshrender->material->pso);
	}
	//经试验，优化顶点视图设置的效果微乎其微，无需优化
	cmdList->IASetVertexBuffers(0, 1, &go->meshrender->mesh->VertexBufferView());
	cmdList->IASetIndexBuffer(&go->meshrender->mesh->IndexBufferView());
	cmdList->IASetPrimitiveTopology(go->meshrender->PrimitiveType);
	auto instanceBuffer = go->InstanceBuffer[manager->heapResourceManager->mCurrFrameResourceIndex]->Resource();
	cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
	SubmeshGeometry submesh = go->meshrender->GetSubmesh();
	cmdList->DrawIndexedInstanced(submesh.IndexCount, go->renderCount, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
}