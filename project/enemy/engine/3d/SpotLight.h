#pragma once
#include "Vector3.h"
#include "Vector4.h"
namespace MyEngine {

/// <summary>
/// スポットライトの色、位置、方向、照射範囲をGPUへ渡すための構造体。
/// </summary>
struct SpotLight {
    Vector4 color;
    Vector3 position;
    float intensity;
    Vector3 direction;
    float distance;
    float decay;
    float cosAngle;
    float padding[2];  
};

} // namespace MyEngine
