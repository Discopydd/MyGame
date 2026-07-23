#pragma once
#include "Matrix4x4.h"
namespace MyEngine {

/// <summary>
/// TransformationMatrixで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

} // namespace MyEngine
