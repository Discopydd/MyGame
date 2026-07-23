#pragma once
#include "Windows.h"
#include <string>
#include"../externals/DirectXTex/DirectXTex.h"
#include"../externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"
#include <unordered_map>
#include <SrvManager.h>

namespace MyEngine {

/// <summary>
/// TextureManagerに関する処理と状態を管理するクラスです。
/// </summary>
class TextureManager
{
private:

	static TextureManager* instance;


	 bool initialized_ = false;
	/// <summary>
	/// TextureManagerのインスタンスを生成します。
	/// </summary>
	TextureManager() = default;
	/// <summary>
	/// TextureManagerが保持するリソースを破棄します。
	/// </summary>
	~TextureManager() = default;
	/// <summary>
	/// TextureManagerのインスタンスを生成します。
	/// </summary>
	TextureManager(TextureManager&) = default;
	/// <summary>
	/// 演算子「=」による計算結果を生成します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	TextureManager& operator=(TextureManager&) = delete;

	//テクスチャ1枚分のデータ
	/// <summary>
	/// TextureDataで使用する関連データをまとめて保持する構造体です。
	/// </summary>
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
	/// <summary>
	/// AnimatedTextureで使用する関連データをまとめて保持する構造体です。
	/// </summary>
	struct AnimatedTexture {
		std::vector<TextureData> frames;   // 各フレームの TextureData
		std::vector<float> delays;         // 各フレームの表示時間（秒）
		bool loop = true;

		size_t current = 0;
		float timer = 0.0f;
	};
public:

	//シングルトンインタンス
	/// <summary>
	/// Instanceを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	static TextureManager* GetInstance();

	//終了
	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();

	// 初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="dxCommon">処理対象のオブジェクトへのポインタ。</param>
	/// <param name="srvManager">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	//メタデータを取得
	/// <summary>
	/// Meta Dataを取得します。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>保持している値への参照。</returns>
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	//テクスチャファイルの読み込み
	/// <summary>
	/// Textureを読み込みます。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	void LoadTexture(const std::string& filePath);
	//SRVインデックスの開始番号
	/// <summary>
	/// Texture Index By File Pathを取得します。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>計算または取得した数値。</returns>
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	//SRVインデックスの取得
	/// <summary>
	/// Srv Indexを取得します。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>計算または取得した数値。</returns>
	uint32_t GetSrvIndex(const std::string& filePath);

	//テクスチャ番号からCPUハンドルを取得
	/// <summary>
	/// Srv Handle GPUを取得します。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>計算または取得した結果。</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// ======== GIF 追加 API ========
    /// <summary>
    /// Gifを読み込みます。
    /// </summary>
    /// <param name="filePath">読み込むファイルのパス。</param>
    void LoadGif(const std::string& filePath); // GIF 拆帧 + 上伝
    /// <summary>
    /// Gif Loadedかを判定します。
    /// </summary>
    /// <param name="filePath">読み込むファイルのパス。</param>
    /// <returns>条件を満たす場合は true、それ以外は false。</returns>
    bool IsGifLoaded(const std::string& filePath) const;
    /// <summary>
    /// Gif Meta Dataを取得します。
    /// </summary>
    /// <param name="filePath">読み込むファイルのパス。</param>
    /// <returns>保持している値への参照。</returns>
    const DirectX::TexMetadata& GetGifMetaData(const std::string& filePath) const;

    // dt でアニメを進めて、現在フレームの SRV ハンドルを返す
    /// <summary>
    /// Gif Srv Handle GPUを取得します。
    /// </summary>
    /// <param name="filePath">読み込むファイルのパス。</param>
    /// <param name="dt">前フレームからの経過時間（秒）。</param>
    /// <returns>計算または取得した結果。</returns>
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

} // namespace MyEngine
