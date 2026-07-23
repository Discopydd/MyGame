#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace MyEngine {

/// <summary>
/// 平行光源の色、方向、明るさをGPUへ渡すための構造体。
/// </summary>
struct DirectionalLight {
	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;//輝度
};

} // namespace MyEngine
