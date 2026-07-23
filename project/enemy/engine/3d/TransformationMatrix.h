#pragma once
#include "Matrix4x4.h"
namespace MyEngine {

/// <summary>
/// ワールド変換行列とワールド・ビュー・射影行列をGPUへ渡すための構造体。
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

} // namespace MyEngine
