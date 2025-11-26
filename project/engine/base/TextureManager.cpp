#include "TextureManager.h"
#include "StringUtility.h"
#include <algorithm>
#include <wincodec.h>
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
    if (filePath.empty()) {
        return;
    }
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
// ======== GIF アニメーション ========
bool TextureManager::IsGifLoaded(const std::string& filePath) const {
    return gifDatas_.contains(filePath);
}

const DirectX::TexMetadata& TextureManager::GetGifMetaData(const std::string& filePath) const {
    auto it = gifDatas_.find(filePath);
    assert(it != gifDatas_.end());
    assert(!it->second.frames.empty());
    return it->second.frames[0].metadata;
}

void TextureManager::LoadGif(const std::string& filePath) {
    if (gifDatas_.contains(filePath)) return;
    assert(srvManager_->Securedcheck());

    std::wstring wfilePath = StringUtility::ConvertString(filePath);

    // WIC factory
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory));
    assert(SUCCEEDED(hr));

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(
        wfilePath.c_str(), nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &decoder);
    assert(SUCCEEDED(hr));

    UINT frameCount = 0;
    decoder->GetFrameCount(&frameCount);
    assert(frameCount > 0);

    AnimatedTexture anim{};
    anim.frames.reserve(frameCount);
    anim.delays.reserve(frameCount);

    for (UINT i = 0; i < frameCount; ++i) {
        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(i, &frame);
        assert(SUCCEEDED(hr));

        UINT w, h;
        frame->GetSize(&w, &h);

        // RGBA32 に変換
        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        hr = factory->CreateFormatConverter(&converter);
        assert(SUCCEEDED(hr));

        hr = converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr, 0.0,
            WICBitmapPaletteTypeCustom);
        assert(SUCCEEDED(hr));

        std::vector<uint8_t> pixels(size_t(w) * h * 4);
        hr = converter->CopyPixels(
            nullptr, w * 4,
            static_cast<UINT>(pixels.size()),
            pixels.data());
        assert(SUCCEEDED(hr));

        // DirectXTex scratch に包む
        DirectX::Image img{};
        img.width = w;
        img.height = h;
        img.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        img.rowPitch = w * 4;
        img.slicePitch = img.rowPitch * h;
        img.pixels = pixels.data();

        DirectX::ScratchImage scratch;
        scratch.InitializeFromImage(img);
        DirectX::TexMetadata meta = scratch.GetMetadata();

        // GPU にアップロード
        TextureData tex{};
        tex.metadata = meta;
        tex.resource = dxCommon_->CreateTextureResource(meta);
        tex.intermediateResource = dxCommon_->UploadTextureData(tex.resource, scratch);

        tex.srvIndex = srvManager_->Allocate();
        tex.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(tex.srvIndex);
        tex.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(tex.srvIndex);

        DXGI_FORMAT srvFormat = meta.format;
        if (!DirectX::IsSRGB(srvFormat)) {
            srvFormat = DirectX::MakeSRGB(srvFormat);
        }

        srvManager_->CreateSRVforTexture2D(
            tex.srvIndex,
            tex.resource.Get(),
            srvFormat,
            1);

        anim.frames.push_back(std::move(tex));

        // フレーム delay 取得（/grctlext/Delay、単位 10ms）
        float delaySec = 0.1f; // default 100ms
        Microsoft::WRL::ComPtr<IWICMetadataQueryReader> metaReader;
        if (SUCCEEDED(frame->GetMetadataQueryReader(&metaReader))) {
            PROPVARIANT var; PropVariantInit(&var);
            if (SUCCEEDED(metaReader->GetMetadataByName(L"/grctlext/Delay", &var))) {
                if (var.vt == VT_UI2) {
                    delaySec = var.uiVal / 100.0f;
                }
                PropVariantClear(&var);
            }
        }
        anim.delays.push_back(delaySec);
    }

    gifDatas_[filePath] = std::move(anim);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGifSrvHandleGPU(const std::string& filePath, float dt) {
    auto it = gifDatas_.find(filePath);
    assert(it != gifDatas_.end());

    AnimatedTexture& anim = it->second;
    if (anim.frames.empty()) {
        static D3D12_GPU_DESCRIPTOR_HANDLE dummy{};
        return dummy;
    }

    anim.timer += dt;

    float curDelay = anim.delays[anim.current];
    if (anim.timer >= curDelay) {
        anim.timer -= curDelay;
        anim.current++;
        if (anim.current >= anim.frames.size()) {
            anim.current = anim.loop ? 0 : anim.frames.size() - 1;
        }
    }

    return anim.frames[anim.current].srvHandleGPU;
}