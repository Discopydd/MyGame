#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstants()
{
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}


void ModelManager::Finalize()
{
	delete instance;
	instance = nullptr;
}


void ModelManager::Initialize(DirectXCommon* dxcommon)
{
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxcommon);
}


void ModelManager::LoadModel(const std::string& filePath)
{
	//読み込み済みモデルを検索
	if (models.contains(filePath)) {
		//読み込み済みなら早期return
		return;
	}

	 size_t lastSlash = filePath.find_last_of("/\\");
    std::string directory = (lastSlash != std::string::npos) ? 
        "Resources/" + filePath.substr(0, lastSlash) : "Resources";
    
    // 获取文件名（如 "cube.obj"）
    std::string filename = (lastSlash != std::string::npos) ? 
        filePath.substr(lastSlash + 1) : filePath;

    std::unique_ptr<Model> model = std::make_unique<Model>();
    model->Initialize(modelCommon, directory, filename);  // 传递目录和文件名
    models.insert({filePath, std::move(model)});
}


Model* ModelManager::FindModel(const std::string& filePath)
{
	//読み込みモデルを戻り値としてreturn
	if (models.contains(filePath)) {
		return models.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}