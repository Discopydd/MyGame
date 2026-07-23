#include "ModelManager.h"

namespace MyEngine {

ModelManager& ModelManager::Instance()
{
	static ModelManager instance;
	return instance;
}


void ModelManager::Finalize()
{
	models.clear();
	modelCommon.reset();
}


void ModelManager::Initialize(DirectXCommon* dxcommon)
{
	if (!modelCommon) {
		modelCommon = std::make_unique<ModelCommon>();
	}
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
    
    // 取得ファイル名（ "cube.obj"）
    std::string filename = (lastSlash != std::string::npos) ? 
        filePath.substr(lastSlash + 1) : filePath;

    std::unique_ptr<Model> model = std::make_unique<Model>();
    model->Initialize(modelCommon.get(), directory, filename);  // 伝递目録和文件名
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

} // namespace MyEngine
