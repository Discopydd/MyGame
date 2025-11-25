#include "TextureManager.h"
#include "StringUtility.h"
#include <algorithm>
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
// テクスチャファイルの読み込み
void TextureManager::LoadTexture(const std::string& filePath) {
    // 既に読み込み済みなら何もしない
    if (textureDatas.contains(filePath)) return;

    // テクスチャ枚数上限チェック
    assert(srvManager_->Securedcheck());

    // 拡張子（小文字）
    std::string ext = filePath.substr(filePath.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });

    std::wstring wfilePath = StringUtility::ConvertString(filePath);

    // 1) 読み込み
    DirectX::ScratchImage scratch;
    DirectX::TexMetadata meta{};
    HRESULT hr = S_OK;

    if (ext == "dds") {
        // DDS: そのまま読み込み（BC圧縮やmipが来てもOK）
        hr = DirectX::LoadFromDDSFile(wfilePath.c_str(), DirectX::DDS_FLAGS_NONE, &meta, scratch);
    }
    else {
        // WIC: sRGBとして読み込む
        hr = DirectX::LoadFromWICFile(wfilePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, &meta, scratch);
    }
    assert(SUCCEEDED(hr));

    // 2) 必要な場合のみ mipmap 生成（DDS が既に mip を持っているならスキップ）
    DirectX::ScratchImage finalImg;
    if (meta.mipLevels <= 1) {
        DirectX::ScratchImage mips;
        hr = DirectX::GenerateMipMaps(
            scratch.GetImages(), scratch.GetImageCount(), meta,
            DirectX::TEX_FILTER_SRGB, 0, mips);
        assert(SUCCEEDED(hr));
        finalImg = std::move(mips);
        meta = finalImg.GetMetadata();
    }
    else {
        finalImg = std::move(scratch);
        meta = finalImg.GetMetadata();
    }

    // 3) GPU リソース生成＆アップロード
    TextureData& tex = textureDatas[filePath];
    tex.metadata = meta;
    tex.resource = dxCommon_->CreateTextureResource(tex.metadata);
    tex.intermediateResource = dxCommon_->UploadTextureData(tex.resource, finalImg);

    // 4) SRV 割り当て
    tex.srvIndex = srvManager_->Allocate();
    tex.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(tex.srvIndex);
    tex.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(tex.srvIndex);

    // 5) SRV フォーマット（色テクスチャは sRGB を優先）
    DXGI_FORMAT srvFormat = tex.metadata.format;
    // より安全に：どの入力でも sRGB 変種があればそれを使う
    if (!DirectX::IsSRGB(srvFormat)) {
        srvFormat = DirectX::MakeSRGB(srvFormat);
    }

    srvManager_->CreateSRVforTexture2D(
        tex.srvIndex,
        tex.resource.Get(),
        srvFormat,
        static_cast<UINT>(tex.metadata.mipLevels)
    );
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