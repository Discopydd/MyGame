#pragma once
#include "Vector3.h"
#include "Vector4.h"
namespace MyEngine {

/// <summary>
/// SpotLightで使用する関連データをまとめて保持する構造体です。
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
