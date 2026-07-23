#pragma once
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <type_traits>

namespace MyEngine {

#if __cpp_concepts
template <class V>
concept Vec3Like = requires(V v) {
    { v.x } -> std::convertible_to<float>;
    { v.y } -> std::convertible_to<float>;
    { v.z } -> std::convertible_to<float>;
};
#endif

/// <summary>
/// Quaternionで使用する関連データをまとめて保持する構造体です。
/// </summary>
struct Quaternion {
    // 四元数 q = w + xi + yj + zk
    float w, x, y, z;

    // ===== 構築 =====
    /// <summary>
    /// Quaternionのインスタンスを生成します。
    /// </summary>
    constexpr Quaternion() : w(1.0f), x(0), y(0), z(0) {}
    /// <summary>
    /// Quaternionのインスタンスを生成します。
    /// </summary>
    /// <param name="_w">処理に使用する_wの値。</param>
    /// <param name="_x">処理に使用する_xの値。</param>
    /// <param name="_y">処理に使用する_yの値。</param>
    /// <param name="_z">処理に使用する_zの値。</param>
    constexpr Quaternion(float _w, float _x, float _y, float _z)
        : w(_w), x(_x), y(_y), z(_z) {}

    /// <summary>
    /// Identity処理を実行します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    static constexpr Quaternion Identity() { return Quaternion(1,0,0,0); }

#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
    /// <summary>
    /// From Axis Angle処理を実行します。
    /// </summary>
    /// <param name="axis">移動または回転の軸。</param>
    /// <param name="angleRad">回転角（ラジアン）。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion FromAxisAngle(const V& axis, float angleRad) {
        float ax = static_cast<float>(axis.x);
        float ay = static_cast<float>(axis.y);
        float az = static_cast<float>(axis.z);
        float len = std::sqrt(ax*ax + ay*ay + az*az);
        if (len < FLT_EPSILON) return Identity();
        float inv = 1.0f / len;
        float s = std::sinf(angleRad * 0.5f);
        float c = std::cosf(angleRad * 0.5f);
        return Quaternion(c, ax*inv*s, ay*inv*s, az*inv*s);
    }

    /// <summary>
    /// From Yaw Pitch Roll処理を実行します。
    /// </summary>
    /// <param name="yawY">処理に使用するyawYの値。</param>
    /// <param name="pitchX">処理に使用するpitchXの値。</param>
    /// <param name="rollZ">処理に使用するrollZの値。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion FromYawPitchRoll(float yawY, float pitchX, float rollZ) {
        float cy = std::cosf(yawY * 0.5f);
        float sy = std::sinf(yawY * 0.5f);
        float cp = std::cosf(pitchX * 0.5f);
        float sp = std::sinf(pitchX * 0.5f);
        float cr = std::cosf(rollZ * 0.5f);
        float sr = std::sinf(rollZ * 0.5f);

        // 采用 Z * X * Y 组合
        Quaternion qy(cy, 0, sy, 0);
        Quaternion qx(cp, sp, 0, 0);
        Quaternion qz(cr, 0, 0, sr);
        return (qz * qx * qy).Normalized();
    }

    // Yaw-Pitch-Roll を抽出する（上記の前提と合わせる）。返り値は (yawY, pitchX, rollZ)
    /// <summary>
    /// To Yaw Pitch Roll処理を実行します。
    /// </summary>
    /// <param name="outYawY">処理に使用する参照値。</param>
    /// <param name="outPitchX">処理に使用する参照値。</param>
    /// <param name="outRollZ">処理に使用する参照値。</param>
    inline void ToYawPitchRoll(float& outYawY, float& outPitchX, float& outRollZ) const {
        // 先に回転行列を求め、そこからオイラー角（YXZ）を逆算する
        float m[3][3];
        ToRotationMatrix3x3(m);

        // 基于 YXZ 分解（一般的方案、避ける奇异時数值不稳）
        // pitchX = asin(clamp(-m[2][1], -1, 1))
        float sy = -m[2][1];
        sy = std::clamp(sy, -1.0f, 1.0f);
        float pitchX = std::asinf(sy);

        float yawY, rollZ;
        if (std::fabs(sy) < 0.999999f) {
            // yawY = atan2(m[2][0], m[2][2])
            yawY  = std::atan2f(m[2][0], m[2][2]);
            // rollZ = atan2(m[0][1], m[1][1])
            rollZ = std::atan2f(m[0][1], m[1][1]);
        } else {
            // 特異点近傍: 片方の角度を 0 にして、もう片方へまとめる
            yawY  = 0.0f;
            rollZ = std::atan2f(-m[1][0], m[0][0]);
        }
        outYawY   = yawY;
        outPitchX = pitchX;
        outRollZ  = rollZ;
    }

    // ===== 基本运算 =====
    /// <summary>
    /// 演算子「+」による計算結果を生成します。
    /// </summary>
    /// <param name="r">演算対象の右辺値。</param>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion operator+(const Quaternion& r) const {
        return { w + r.w, x + r.x, y + r.y, z + r.z };
    }
    /// <summary>
    /// 演算子「-」による計算結果を生成します。
    /// </summary>
    /// <param name="r">演算対象の右辺値。</param>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion operator-(const Quaternion& r) const {
        return { w - r.w, x - r.x, y - r.y, z - r.z };
    }
    /// <summary>
    /// 演算子「*」による計算結果を生成します。
    /// </summary>
    /// <param name="s">演算に使用するスカラー値。</param>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion operator*(float s) const {
        return { w*s, x*s, y*s, z*s };
    }
    /// <summary>
    /// 演算子「*」による計算結果を生成します。
    /// </summary>
    /// <param name="s">演算に使用するスカラー値。</param>
    /// <param name="q">演算対象のクォータニオン。</param>
    /// <returns>計算または取得した結果。</returns>
    friend constexpr Quaternion operator*(float s, const Quaternion& q) {
        return q * s;
    }
    /// <summary>
    /// 演算子「/」による計算結果を生成します。
    /// </summary>
    /// <param name="s">演算に使用するスカラー値。</param>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion operator/(float s) const {
        return { w/s, x/s, y/s, z/s };
    }

    // クォータニオン乗算（複合回転。交換法則は成り立たない）
    /// <summary>
    /// 演算子「*」による計算結果を生成します。
    /// </summary>
    /// <param name="r">演算対象の右辺値。</param>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion operator*(const Quaternion& r) const {
        return {
            w*r.w - x*r.x - y*r.y - z*r.z,
            w*r.x + x*r.w + y*r.z - z*r.y,
            w*r.y - x*r.z + y*r.w + z*r.x,
            w*r.z + x*r.y - y*r.x + z*r.w
        };
    }
    Quaternion& operator*=(const Quaternion& r) {
        *this = (*this) * r;
        return *this;
    }

    // 点积 / 范数
    /// <summary>
    /// Dot処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <returns>計算または取得した数値。</returns>
    constexpr static float Dot(const Quaternion& a, const Quaternion& b) {
        return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
    }
    /// <summary>
    /// Length Sq処理を実行します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    constexpr float LengthSq() const { return Dot(*this, *this); }
    /// <summary>
    /// Length処理を実行します。
    /// </summary>
    /// <returns>計算または取得した数値。</returns>
    float Length() const { return std::sqrt(LengthSq()); }

    // 单位化
    /// <summary>
    /// dを正規化します。
    /// </summary>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    /// <returns>計算または取得した結果。</returns>
    Quaternion Normalized(float eps = 1e-8f) const {
        float lsq = LengthSq();
        if (lsq <= eps) return Identity();
        float inv = 1.0f / std::sqrt(lsq);
        return (*this) * inv;
    }
    /// <summary>
    /// Normalize処理を実行します。
    /// </summary>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    void Normalize(float eps = 1e-8f) { *this = Normalized(eps); }

    // 共轭 / 逆
    /// <summary>
    /// Conjugate処理を実行します。
    /// </summary>
    /// <returns>計算または取得した結果。</returns>
    constexpr Quaternion Conjugate() const { return { w, -x, -y, -z }; }
    /// <summary>
    /// Inverse処理を実行します。
    /// </summary>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    /// <returns>計算または取得した結果。</returns>
    Quaternion Inverse(float eps = 1e-8f) const {
        float lsq = LengthSq();
        if (lsq <= eps) return Identity();
        return Conjugate() / lsq;
    }

    // 回転補間
    // Nlerp: 線性後正規化（速度常量近似）
    /// <summary>
    /// Nlerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <param name="shortestPath">対象ファイルまたはリソースのパス。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion Nlerp(const Quaternion& a, const Quaternion& b, float t, bool shortestPath=true) {
        Quaternion bb = b;
        float d = Dot(a,b);
        if (shortestPath && d < 0.0f) bb = {-b.w, -b.x, -b.y, -b.z};
        Quaternion q = a*(1.0f - t) + bb*t;
        return q.Normalized();
    }

    // Slerp: 等角速度插值
    /// <summary>
    /// Slerp処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="t">補間係数。</param>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, float eps=1e-6f) {
        float d = Dot(a,b);
        Quaternion bb = b;
        // 走最短弧
        if (d < 0.0f) { d = -d; bb = {-b.w, -b.x, -b.y, -b.z}; }

        if (1.0f - d < eps) {
            // 非常接近: 退化为 Nlerp
            return Nlerp(a, bb, t, /*shortestPath*/false);
        }
        float theta = std::acosf(std::clamp(d, -1.0f, 1.0f));
        float s0 = std::sinf((1.0f - t) * theta) / std::sinf(theta);
        float s1 = std::sinf(t * theta) / std::sinf(theta);
        return (a*s0 + bb*s1).Normalized();
    }

    // クォータニオンを 3x3 回転行列へ変換する（行優先 m[row][col]）
    /// <summary>
    /// To Rotation Matrix 3 x 3処理を実行します。
    /// </summary>
    /// <param name="m">処理に使用するmの値。</param>
    void ToRotationMatrix3x3(float m[3][3]) const {
        float xx = x + x, yy = y + y, zz = z + z;
        float xy = x * yy, xz = x * zz, yz = y * zz;
        float wx = w * xx, wy = w * yy, wz = w * zz;
        float xx2 = x * xx, yy2 = y * yy, zz2 = z * zz;

        m[0][0] = 1.0f - (yy2 + zz2);
        m[0][1] = xy + wz;
        m[0][2] = xz - wy;

        m[1][0] = xy - wz;
        m[1][1] = 1.0f - (xx2 + zz2);
        m[1][2] = yz + wx;

        m[2][0] = xz + wy;
        m[2][1] = yz - wx;
        m[2][2] = 1.0f - (xx2 + yy2);
    }

    // 4x4 行列の回転部分を設定し、残りは単位行列にする（columnMajor に従って配置）
    /// <summary>
    /// To Rotation Matrix 4 x 4処理を実行します。
    /// </summary>
    /// <param name="m">処理に使用するmの値。</param>
    /// <param name="columnMajor">処理に使用するcolumnMajorの値。</param>
    void ToRotationMatrix4x4(float m[16], bool columnMajor = true) const {
        float r[3][3];
        ToRotationMatrix3x3(r);
        if (columnMajor) {
            // 列優先として扱う（DirectXMath は行優先が基本だが、列ベクトル形式を受け取る API もあるため両方を用意する）
            m[0] = r[0][0]; m[4] = r[0][1]; m[8]  = r[0][2]; m[12] = 0.0f;
            m[1] = r[1][0]; m[5] = r[1][1]; m[9]  = r[1][2]; m[13] = 0.0f;
            m[2] = r[2][0]; m[6] = r[2][1]; m[10] = r[2][2]; m[14] = 0.0f;
            m[3] = 0.0f;    m[7] = 0.0f;    m[11] = 0.0f;    m[15] = 1.0f;
        } else {
            // 行優先
            m[0] = r[0][0]; m[1] = r[0][1]; m[2]  = r[0][2]; m[3]  = 0.0f;
            m[4] = r[1][0]; m[5] = r[1][1]; m[6]  = r[1][2]; m[7]  = 0.0f;
            m[8] = r[2][0]; m[9] = r[2][1]; m[10] = r[2][2]; m[11] = 0.0f;
            m[12]= 0.0f;    m[13]= 0.0f;    m[14] = 0.0f;    m[15] = 1.0f;
        }
    }

    // 3x3 回転行列から構築する（行優先 m[row][col]）
    /// <summary>
    /// From Rotation Matrix 3 x 3処理を実行します。
    /// </summary>
    /// <param name="m">処理に使用するmの値。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion FromRotationMatrix3x3(const float m[3][3]) {
        float trace = m[0][0] + m[1][1] + m[2][2];
        if (trace > 0.0f) {
            float s = std::sqrtf(trace + 1.0f) * 2.0f; // s = 4*w
            float inv = 1.0f / s;
            float w = 0.25f * s;
            float x = (m[2][1] - m[1][2]) * inv;
            float y = (m[0][2] - m[2][0]) * inv;
            float z = (m[1][0] - m[0][1]) * inv;
            return Quaternion(w,x,y,z).Normalized();
        } else {
            // 选主对角最大项
            if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
                float s = std::sqrtf(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;
                float inv = 1.0f / s;
                float w = (m[2][1] - m[1][2]) * inv;
                float x = 0.25f * s;
                float y = (m[0][1] + m[1][0]) * inv;
                float z = (m[0][2] + m[2][0]) * inv;
                return Quaternion(w,x,y,z).Normalized();
            } else if (m[1][1] > m[2][2]) {
                float s = std::sqrtf(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;
                float inv = 1.0f / s;
                float w = (m[0][2] - m[2][0]) * inv;
                float x = (m[0][1] + m[1][0]) * inv;
                float y = 0.25f * s;
                float z = (m[1][2] + m[2][1]) * inv;
                return Quaternion(w,x,y,z).Normalized();
            } else {
                float s = std::sqrtf(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f;
                float inv = 1.0f / s;
                float w = (m[1][0] - m[0][1]) * inv;
                float x = (m[0][2] + m[2][0]) * inv;
                float y = (m[1][2] + m[2][1]) * inv;
                float z = 0.25f * s;
                return Quaternion(w,x,y,z).Normalized();
            }
        }
    }

    // ベクトルを回転する: v' = q * v * q^{-1}
#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
    /// <summary>
    /// Rotate処理を実行します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    /// <returns>計算または取得した結果。</returns>
    V Rotate(const V& v) const {
        // より高效的ベクトル回転: v + 2*cross(q.xyz, cross(q.xyz, v) + q.w*v)
        V out{};
        float qx = x, qy = y, qz = z, qw = w;
        float uvx = qy * v.z - qz * v.y;
        float uvy = qz * v.x - qx * v.z;
        float uvz = qx * v.y - qy * v.x;

        float uuvx = qy * uvz - qz * uvy;
        float uuvy = qz * uvx - qx * uvz;
        float uuvz = qx * uvy - qy * uvx;

        uvx *= (2.0f * qw);
        uvy *= (2.0f * qw);
        uvz *= (2.0f * qw);

        out.x = v.x + 2.0f * (uvx + uuvx);
        out.y = v.y + 2.0f * (uvy + uuvy);
        out.z = v.z + 2.0f * (uvz + uuvz);
        return out;
    }

    // ベクトルを「増分回転」の軸角として扱う（長さ = 角度、方向 = 軸）→ クォータニオン
#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
    /// <summary>
    /// From Scaled Axis処理を実行します。
    /// </summary>
    /// <param name="v">設定または計算に使用する値。</param>
    /// <returns>計算または取得した結果。</returns>
    static Quaternion FromScaledAxis(const V& v) {
        float angle = std::sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (angle < 1e-8f) return Identity();
        float inv = 1.0f / angle;
        return FromAxisAngle(v * inv, angle);
    }

    // 近等判断（用于避免归一化抖動）
    /// <summary>
    /// Almost Equal処理を実行します。
    /// </summary>
    /// <param name="a">計算に使用する1つ目の値。</param>
    /// <param name="b">計算に使用する2つ目の値。</param>
    /// <param name="eps">数値誤差判定に使用する許容値。</param>
    /// <returns>判定結果。</returns>
    static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float eps = 1e-5f) {
        // クォータニオン q と -q は同じ回転を表すため、比較時は両方を考慮する
        Quaternion d1 = a - b;
        Quaternion d2 = a + b;
        return (d1.LengthSq() <= eps*eps) || (d2.LengthSq() <= eps*eps);
    }
};

// ===== 便捷函数 =====
/// <summary>
/// Lerp処理を実行します。
/// </summary>
/// <param name="a">計算に使用する1つ目の値。</param>
/// <param name="b">計算に使用する2つ目の値。</param>
/// <param name="t">補間係数。</param>
/// <returns>計算または取得した結果。</returns>
inline Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t) {
    // 普通 Lerp（不保证单位長度）；通常より推荐 Nlerp
    return a*(1.0f - t) + b*t;
}


} // namespace MyEngine
