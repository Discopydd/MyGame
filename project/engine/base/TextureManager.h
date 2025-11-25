#pragma once
#include "Windows.h"
#include <string>
#include"../externals/DirectXTex/DirectXTex.h"
#include"../externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"
#include <unordered_map>
#include <SrvManager.h>

class TextureManager
{
private:

	static TextureManager* instance;


	 bool initialized_ = false;
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	//テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource>resource;
		Microsoft::WRL::ComPtr<ID3D12Resource>intermediateResource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
	   // ======== GIF 用アニメーションデータ ========
	struct AnimatedTexture {
		std::vector<TextureData> frames;   // 各フレームの TextureData
		std::vector<float> delays;         // 各フレームの表示時間（秒）
		bool loop = true;

		size_t current = 0;
		float timer = 0.0f;
	};
public:

	//シングルトンインタンス
	static TextureManager* GetInstance();

	//終了
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	//テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);
	//SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	//SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);

	//テクスチャ番号からCPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// ======== GIF 追加 API ========
    void LoadGif(const std::string& filePath); // GIF 拆帧 + 上传
    bool IsGifLoaded(const std::string& filePath) const;
    const DirectX::TexMetadata& GetGifMetaData(const std::string& filePath) const;

    // dt でアニメを進めて、現在フレームの SRV ハンドルを返す
    D3D12_GPU_DESCRIPTOR_HANDLE GetGifSrvHandleGPU(const std::string& filePath, float dt);

	static uint32_t kSRVIndexTop;

private:

	//テクスチャデータ
	std::unordered_map<std::string,TextureData>textureDatas;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	// GIF アニメーションデータ
    std::unordered_map<std::string, AnimatedTexture> gifDatas_;
};