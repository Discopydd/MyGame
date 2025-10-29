#pragma once

#include "Model.h"
#include <map>
#include <string>
#include "../base/DirectXCommon.h"
class ModelManager
{

	static ModelManager* instance;
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = default;
	ModelManager& operator=(ModelManager&) = delete;

public:

	// シングルトンインスタンスの取得
	static ModelManager* GetInstants();
	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxcommon);

	// モデルの読み込み
	void LoadModel(const std::string& filePath);

	// モデル検索
	Model* FindModel(const std::string& filePath);
private:

	//モデルデータ
	std::map<std::string, std::unique_ptr < Model>> models;
	ModelCommon* modelCommon = nullptr;

};