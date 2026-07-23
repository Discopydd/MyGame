#pragma once

#include "Model.h"
#include <map>
#include <string>
#include "../base/DirectXCommon.h"
namespace MyEngine {

/// <summary>
/// 3Dモデルの読み込み、検索、共有を一元管理するクラス。
/// </summary>
class ModelManager
{
public:

	// シングルトンインスタンスの取得
	/// <summary>
	/// Instance処理を実行します。
	/// </summary>
	/// <returns>保持している値への参照。</returns>
	static ModelManager& Instance();
	// 既存コード互換（ポインタが欲しい場合）
	/// <summary>
	/// Instantsを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	static ModelManager* GetInstants() { return &Instance(); }
	// 終了
	/// <summary>
	/// 使用しているリソースを解放し、終了処理を行います。
	/// </summary>
	void Finalize();

	// 初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="dxcommon">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(DirectXCommon* dxcommon);

	// モデルの読み込み
	/// <summary>
	/// Modelを読み込みます。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	void LoadModel(const std::string& filePath);

	// モデル検索
	/// <summary>
	/// Modelを検索します。
	/// </summary>
	/// <param name="filePath">読み込むファイルのパス。</param>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	Model* FindModel(const std::string& filePath);
private:
	/// <summary>
	/// ModelManagerのインスタンスを生成します。
	/// </summary>
	ModelManager() = default;
	/// <summary>
	/// ModelManagerが保持するリソースを破棄します。
	/// </summary>
	~ModelManager() = default;
	/// <summary>
	/// ModelManagerのインスタンスを生成します。
	/// </summary>
	ModelManager(const ModelManager&) = delete;
 	/// <summary>
 	/// 演算子「=」による計算結果を生成します。
 	/// </summary>
 	/// <returns>保持している値への参照。</returns>
 	ModelManager& operator=(const ModelManager&) = delete;
 	/// <summary>
 	/// ModelManagerのインスタンスを生成します。
 	/// </summary>
 	ModelManager(ModelManager&&) = delete;
 	/// <summary>
 	/// 演算子「=」による計算結果を生成します。
 	/// </summary>
 	/// <returns>保持している値への参照。</returns>
 	ModelManager& operator=(ModelManager&&) = delete;
	//モデルデータ
	std::map<std::string, std::unique_ptr < Model>> models;
	std::unique_ptr<ModelCommon> modelCommon;

};

} // namespace MyEngine
