#pragma once
#include <Vector3.h>
namespace MyEngine {

/// <summary>
/// CameraForGPUで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct CameraForGPU {
    Vector3 worldPosition;
    float pad;
};

} // namespace MyEngine
