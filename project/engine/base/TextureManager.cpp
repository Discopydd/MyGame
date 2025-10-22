#include "TextureManager.h"
#include "StringUtility.h"

TextureManager* TextureManager::instance = nullptr;

//ImGuiで0盤を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

//シングルインスタンスの取得
TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}
//終了
void TextureManager::Finalize() {
	if (!instance) return;
    instance->textureDatas.clear();
    instance->initialized_ = false;
    delete instance;
    instance = nullptr;
}
//初期化
void TextureManager::Initialize(DirectXCommon*dxCommon, SrvManager* srvManager) {
	if (initialized_) {
		return;
	}
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	initialized_ = true;
}
//テクスチャファイルの読み込み
void TextureManager::LoadTexture(const std::string& filePath) {
	//読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		return;
	}
	//基盤？
	//テクスチャ枚数上限チェック
	assert(srvManager_->Securedcheck());

	DirectX::ScratchImage image{};
	//テクスチャファイルを読んでプログラムで抑えるようにする
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages{};
	//ミップマップの作成
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//テクスチャデータを追加して書きこむ
	TextureData& textureData = textureDatas[filePath];

	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);
	textureData.intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);

	//テクスチャデータの要素数番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	DXGI_FORMAT format = textureData.metadata.format;
	if (format == DXGI_FORMAT_R8G8B8A8_UNORM) {
		format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(),
		format, UINT(textureData.metadata.mipLevels));

}
//SRVインデックスの開始番号
uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	//読み込み済みテクスチャデータを検索
	auto it = textureDatas.find(filePath);

	if (it != textureDatas.end()) {
		return it->second.srvIndex;
	}

	return 0;
}
//SRVインデックスの取得
uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	TextureData& textureData = textureDatas[filePath];
	return textureData.srvIndex;
}
//テクスチャ番号からGPUハンドルを取得
D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	//範囲外指定違反チェック
	assert(srvManager_->Securedcheck());

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}
//メタデータを取得
const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
    //範囲外指定違反チェック
	assert(srvManager_->Securedcheck());

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}