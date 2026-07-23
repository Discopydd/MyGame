#pragma once
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cassert>
#include <Quaternion.h>
namespace MyEngine {

namespace Math {
    const float PI = 3.141592654f;

    /// <summary>
    /// Scale Matrixを生成します。
    /// </summary>
    /// <param name="scale">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeScaleMatrix(const Vector3& scale);

    /// <summary>
    /// Rotate Z Matrixを生成します。
    /// </summary>
    /// <param name="radian">回転角（ラジアン）。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeRotateZMatrix(float radian);

    /// <summary>
    /// Rotate X Matrixを生成します。
    /// </summary>
    /// <param name="radian">回転角（ラジアン）。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeRotateXMatrix(float radian);

    /// <summary>
    /// Translate Matrixを生成します。
    /// </summary>
    /// <param name="translate">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

    /// <summary>
    /// Rotate Y Matrixを生成します。
    /// </summary>
    /// <param name="radian">回転角（ラジアン）。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeRotateYMatrix(float radian);

    /// <summary>
    /// Multiply処理を実行します。
    /// </summary>
    /// <param name="matrix1">処理に使用する参照値。</param>
    /// <param name="matrix2">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2);

    /// <summary>
    /// Affine Matrixを生成します。
    /// </summary>
    /// <param name="scale">処理に使用する参照値。</param>
    /// <param name="rotation">処理に使用する参照値。</param>
    /// <param name="translation">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation);

    /// <summary>
    /// Inverse処理を実行します。
    /// </summary>
    /// <param name="m">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 Inverse(const Matrix4x4& m);

    /// <summary>
    /// Transform処理を実行します。
    /// </summary>
    /// <param name="vector">処理に使用する参照値。</param>
    /// <param name="matrix">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

    /// <summary>
    /// Perspective Fov Matrixを生成します。
    /// </summary>
    /// <param name="fovY">カメラの垂直画角。</param>
    /// <param name="aspectRatio">処理に使用するaspectRatioの値。</param>
    /// <param name="nearClip">処理に使用するnearClipの値。</param>
    /// <param name="farClip">処理に使用するfarClipの値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

    /// <summary>
    /// Orthographic Matrixを生成します。
    /// </summary>
    /// <param name="left">処理に使用するleftの値。</param>
    /// <param name="right">処理に使用するrightの値。</param>
    /// <param name="top">処理に使用するtopの値。</param>
    /// <param name="bottom">処理に使用するbottomの値。</param>
    /// <param name="nearClip">処理に使用するnearClipの値。</param>
    /// <param name="farClip">処理に使用するfarClipの値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeOrthographicMatrix(float left, float right, float top, float bottom, float nearClip, float farClip);

    /// <summary>
    /// Viewport Matrixを生成します。
    /// </summary>
    /// <param name="left">処理に使用するleftの値。</param>
    /// <param name="top">処理に使用するtopの値。</param>
    /// <param name="width">処理に使用するwidthの値。</param>
    /// <param name="height">処理に使用するheightの値。</param>
    /// <param name="minDepth">処理に使用するminDepthの値。</param>
    /// <param name="maxDepth">処理に使用するmaxDepthの値。</param>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

    /// <summary>
    /// Identity 4 x 4を生成します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    Matrix4x4 MakeIdentity4x4();

    /// <summary>
    /// Length処理を実行します。
    /// </summary>
    /// <param name="vec">処理に使用する参照値。</param>
    /// <returns>計算または取得した数値。</returns>
    float Length(const Vector3& vec);

    /// <summary>
    /// Normalize処理を実行します。
    /// </summary>
    /// <param name="vec">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Normalize(const Vector3& vec);

    /// <summary>
    /// Add処理を実行します。
    /// </summary>
    /// <param name="v1">処理に使用する参照値。</param>
    /// <param name="v2">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Add(const Vector3& v1, const Vector3& v2);

    /// <summary>
    /// Subtract処理を実行します。
    /// </summary>
    /// <param name="v1">処理に使用する参照値。</param>
    /// <param name="v2">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Subtract(const Vector3& v1, const Vector3& v2);
    /// <summary>
    /// Multiply処理を実行します。
    /// </summary>
    /// <param name="vec">処理に使用する参照値。</param>
    /// <param name="scalar">乗算または除算に使用するスカラー値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Multiply(const Vector3& vec, float scalar);
    /// <summary>
    /// Coord Localを変換します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    /// <param name="m">処理に使用する参照値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 TransformCoordLocal(const Vector3& v, const Matrix4x4& m);

    /// <summary>
    /// Lerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した数値。</returns>
    float   Lerp(float a, float b, float t);
    /// <summary>
    /// Lerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Lerp(const Vector3& a, const Vector3& b, float t);
    /// <summary>
    /// Lerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector4 Lerp(const Vector4& a, const Vector4& b, float t);
    // 角度(度) -> 弧度(rad)
    /// <summary>
    /// To Radian処理を実行します。
    /// </summary>
    /// <param name="degrees">回転角（度）。</param>
    /// <returns>計算または取得した数値。</returns>
    float ToRadian(float degrees);

    // 軸角（ラジアン）からクォータニオンを構築し、Vector4(quat.x, quat.y, quat.z, quat.w) を返す
    /// <summary>
    /// Axis Angle Quaternionを生成します。
    /// </summary>
    /// <param name="axis">移動または回転の軸。</param>
    /// <param name="angleRad">回転角（ラジアン）。</param>
    /// <returns>計算または取得した結果。</returns>
    Quaternion MakeAxisAngleQuaternion(const Vector3& axis, float angleRad);

    // 線性插值（Nlerp）: デフォルト走最短弧、かつ做正規化
    /// <summary>
    /// Lerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <param name="shortestPath">対象ファイルまたはリソースのパス。</param>
    /// <returns>計算または取得した結果。</returns>
    Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t, bool shortestPath = true);

    // 球面線形補間（Slerp）: デフォルト走最短弧、t∈[0,1]
    /// <summary>
    /// Slerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    /// <returns>計算または取得した結果。</returns>
    Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, float eps = 1e-6f);

    /// <summary>
    /// Quaternion To Euler処理を実行します。
    /// </summary>
    /// <param name="q">演算対象のクォータニオン。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 QuaternionToEuler(const Quaternion& q);

    // （可选）度数版
    /// <summary>
    /// Quaternion To Euler Deg処理を実行します。
    /// </summary>
    /// <param name="q">演算対象のクォータニオン。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 QuaternionToEulerDeg(const Quaternion& q);

    /// <summary>
    /// Cross処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <returns>計算または取得した結果。</returns>
    Vector3 Cross(const Vector3& a, const Vector3& b);
}

} // namespace MyEngine
