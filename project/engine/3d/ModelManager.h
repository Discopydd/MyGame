#pragma once

#include "Model.h"
#include <map>
#include <string>
#include "../base/DirectXCommon.h"
class ModelManager
{
public:

	// シングルトンインスタンスの取得
	static ModelManager& Instance();
	// 既存コード互換（ポインタが欲しい場合）
	static ModelManager* GetInstants() { return &Instance(); }
	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxcommon);

	// モデルの読み込み
	void LoadModel(const std::string& filePath);

	// モデル検索
	Model* FindModel(const std::string& filePath);
private:
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
 	ModelManager& operator=(const ModelManager&) = delete;
 	ModelManager(ModelManager&&) = delete;
 	ModelManager& operator=(ModelManager&&) = delete;
	//モデルデータ
	std::map<std::string, std::unique_ptr < Model>> models;
	std::unique_ptr<ModelCommon> modelCommon;

};