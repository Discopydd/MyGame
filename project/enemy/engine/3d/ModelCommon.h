#pragma once

#include "../base/DirectXCommon.h"

namespace MyEngine {

/// <summary>
/// 3Dモデル描画で共通使用するDirectX関連リソースを管理するクラス。
/// </summary>
class ModelCommon
{

public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};

} // namespace MyEngine
