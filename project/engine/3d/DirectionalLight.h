#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace MyEngine {

/// <summary>
/// DirectionalLightで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct DirectionalLight {
	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;//輝度
};

} // namespace MyEngine
