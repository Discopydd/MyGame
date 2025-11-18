#include "SrvManager.h"
const uint32_t SrvManager::kMaxSRVCount = 512;

SrvManager* SrvManager::GetInstance() {
    static SrvManager instance;
    return &instance;
}
void SrvManager::Initialize(DirectXCommon* dxCommon)
{
	this->directXCommon = dxCommon;
	//デスクリプタヒープの生成
	descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount,true);
	//デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//SRVの設定
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = MipLevels;

	directXCommon->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(
	uint32_t srvIndex,
	ID3D12Resource* pResource,
	UINT numElements,
	UINT structureByteStride)
{
	assert(pResource);            // 安全のため
	assert(numElements > 0);
	assert(structureByteStride > 0);
	// もし自分で最大数を決めているなら:
	// assert(srvIndex < kMaxSrvCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;                   // ★重要
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements;        // 粒子数など
	srvDesc.Buffer.StructureByteStride = structureByteStride;// 1粒子のサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	auto cpuHandle = GetCPUDescriptorHandle(srvIndex);
	directXCommon->GetDevice()->CreateShaderResourceView(
		pResource, &srvDesc, cpuHandle);
}


void SrvManager::PreDraw()
{
	//描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directXCommon->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	directXCommon->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}
	bool SrvManager::Securedcheck()
	{
		return useIndex < kMaxSRVCount;
	}
	uint32_t SrvManager::Allocate()
{
	//上限に達していないかチェック
	assert(useIndex < kMaxSRVCount);

	//returnする番号を一旦記録しておく
	int index = useIndex;
	//次のため番号を進める
	useIndex++;
	//上で記録した番号をreturn
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void SrvManager::Finalize() {
    // 释放描述符堆（这通常是活对象的根源之一）
    descriptorHeap.Reset();
    // 复位内部状态，避免下一次运行的“脏状态”
    useIndex = 0;
    descriptorSize = 0;
    directXCommon = nullptr;
}