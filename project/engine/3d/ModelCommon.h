#pragma once

#include "../base/DirectXCommon.h"

namespace MyEngine {

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
