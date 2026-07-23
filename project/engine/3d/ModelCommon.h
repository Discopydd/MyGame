#pragma once

#include "../base/DirectXCommon.h"

namespace MyEngine {

/// <summary>
/// ModelCommonに関する処理と状態を管理するクラスです。
/// </summary>
class ModelCommon
{

public:
	// 初期化
	/// <summary>
	/// 動作に必要な参照とリソースを設定し、初期状態を構築します。
	/// </summary>
	/// <param name="dxCommon">処理対象のオブジェクトへのポインタ。</param>
	void Initialize(DirectXCommon* dxCommon);

	//DXCommon
	/// <summary>
	/// Dx Commonを取得します。
	/// </summary>
	/// <returns>取得した対象へのポインタ。対象が存在しない場合は nullptr。</returns>
	DirectXCommon* GetDxCommon()const { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;
};

} // namespace MyEngine
