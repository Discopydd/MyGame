#pragma once
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <type_traits>

#if __cpp_concepts
template <class V>
concept Vec3Like = requires(V v) {
    { v.x } -> std::convertible_to<float>;
    { v.y } -> std::convertible_to<float>;
    { v.z } -> std::convertible_to<float>;
};
#endif

struct Quaternion {
    // 四元数 q = w + xi + yj + zk
    float w, x, y, z;

    // ===== 构造 =====
    constexpr Quaternion() : w(1.0f), x(0), y(0), z(0) {}
    constexpr Quaternion(float _w, float _x, float _y, float _z)
        : w(_w), x(_x), y(_y), z(_z) {}

    static constexpr Quaternion Identity() { return Quaternion(1,0,0,0); }

    // 由轴角构造（axis 需非零；内部会单位化），angle 单位：弧度
#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
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

    // Yaw-Pitch-Roll（Yaw 绕 +Y，Pitch 绕 +X，Roll 绕 +Z），弧度
    // 组合顺序：R = Rz(roll) * Rx(pitch) * Ry(yaw)  （常见游戏系：先 yaw，再 pitch，再 roll 的等价实现）
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

    // 提取 Yaw-Pitch-Roll（与上面约定匹配），返回 (yawY, pitchX, rollZ)
    inline void ToYawPitchRoll(float& outYawY, float& outPitchX, float& outRollZ) const {
        // 先得到旋转矩阵，再反推欧拉角（YXZ）
        float m[3][3];
        ToRotationMatrix3x3(m);

        // 基于 YXZ 分解（常见方案，避免奇异时数值不稳）
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
            // 近奇异：将一个角设为 0，合并到另外一个
            yawY  = 0.0f;
            rollZ = std::atan2f(-m[1][0], m[0][0]);
        }
        outYawY   = yawY;
        outPitchX = pitchX;
        outRollZ  = rollZ;
    }

    // ===== 基本运算 =====
    constexpr Quaternion operator+(const Quaternion& r) const {
        return { w + r.w, x + r.x, y + r.y, z + r.z };
    }
    constexpr Quaternion operator-(const Quaternion& r) const {
        return { w - r.w, x - r.x, y - r.y, z - r.z };
    }
    constexpr Quaternion operator*(float s) const {
        return { w*s, x*s, y*s, z*s };
    }
    friend constexpr Quaternion operator*(float s, const Quaternion& q) {
        return q * s;
    }
    constexpr Quaternion operator/(float s) const {
        return { w/s, x/s, y/s, z/s };
    }

    // 四元数乘法（复合旋转，注意非交换）
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
    constexpr static float Dot(const Quaternion& a, const Quaternion& b) {
        return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
    }
    constexpr float LengthSq() const { return Dot(*this, *this); }
    float Length() const { return std::sqrt(LengthSq()); }

    // 单位化
    Quaternion Normalized(float eps = 1e-8f) const {
        float lsq = LengthSq();
        if (lsq <= eps) return Identity();
        float inv = 1.0f / std::sqrt(lsq);
        return (*this) * inv;
    }
    void Normalize(float eps = 1e-8f) { *this = Normalized(eps); }

    // 共轭 / 逆
    constexpr Quaternion Conjugate() const { return { w, -x, -y, -z }; }
    Quaternion Inverse(float eps = 1e-8f) const {
        float lsq = LengthSq();
        if (lsq <= eps) return Identity();
        return Conjugate() / lsq;
    }

    // 旋转插值
    // Nlerp：线性后单位化（速度常量近似）
    static Quaternion Nlerp(const Quaternion& a, const Quaternion& b, float t, bool shortestPath=true) {
        Quaternion bb = b;
        float d = Dot(a,b);
        if (shortestPath && d < 0.0f) bb = {-b.w, -b.x, -b.y, -b.z};
        Quaternion q = a*(1.0f - t) + bb*t;
        return q.Normalized();
    }

    // Slerp：等角速度插值
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, float eps=1e-6f) {
        float d = Dot(a,b);
        Quaternion bb = b;
        // 走最短弧
        if (d < 0.0f) { d = -d; bb = {-b.w, -b.x, -b.y, -b.z}; }

        if (1.0f - d < eps) {
            // 非常接近：退化为 Nlerp
            return Nlerp(a, bb, t, /*shortestPath*/false);
        }
        float theta = std::acosf(std::clamp(d, -1.0f, 1.0f));
        float s0 = std::sinf((1.0f - t) * theta) / std::sinf(theta);
        float s1 = std::sinf(t * theta) / std::sinf(theta);
        return (a*s0 + bb*s1).Normalized();
    }

    // 将四元数转为 3x3 旋转矩阵（行主序 m[row][col]）
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

    // 填充 4x4 矩阵的旋转部分；其余为单位（列主 or 行主均可用，按照 columnMajor 组织）
    void ToRotationMatrix4x4(float m[16], bool columnMajor = true) const {
        float r[3][3];
        ToRotationMatrix3x3(r);
        if (columnMajor) {
            // 列主序（如 DirectXMath 默认行主，但很多 API 接受列向量形式；这里提供两种）
            m[0] = r[0][0]; m[4] = r[0][1]; m[8]  = r[0][2]; m[12] = 0.0f;
            m[1] = r[1][0]; m[5] = r[1][1]; m[9]  = r[1][2]; m[13] = 0.0f;
            m[2] = r[2][0]; m[6] = r[2][1]; m[10] = r[2][2]; m[14] = 0.0f;
            m[3] = 0.0f;    m[7] = 0.0f;    m[11] = 0.0f;    m[15] = 1.0f;
        } else {
            // 行主序
            m[0] = r[0][0]; m[1] = r[0][1]; m[2]  = r[0][2]; m[3]  = 0.0f;
            m[4] = r[1][0]; m[5] = r[1][1]; m[6]  = r[1][2]; m[7]  = 0.0f;
            m[8] = r[2][0]; m[9] = r[2][1]; m[10] = r[2][2]; m[11] = 0.0f;
            m[12]= 0.0f;    m[13]= 0.0f;    m[14] = 0.0f;    m[15] = 1.0f;
        }
    }

    // 由 3x3 旋转矩阵构造（行主序 m[row][col]）
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

    // 旋转向量 v' = q * v * q^{-1}
#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
    V Rotate(const V& v) const {
        // 更高效的向量旋转：v + 2*cross(q.xyz, cross(q.xyz, v) + q.w*v)
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

    // 将向量视作“增量旋转”的轴角（长度为角度，方向为轴）→ 四元数
#if __cpp_concepts
    template <Vec3Like V>
#else
    template <class V>
#endif
    static Quaternion FromScaledAxis(const V& v) {
        float angle = std::sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (angle < 1e-8f) return Identity();
        float inv = 1.0f / angle;
        return FromAxisAngle(v * inv, angle);
    }

    // 近等判断（用于避免归一化抖动）
    static bool AlmostEqual(const Quaternion& a, const Quaternion& b, float eps = 1e-5f) {
        // 四元数 q 与 -q 表示相同旋转；因此比较时考虑两者
        Quaternion d1 = a - b;
        Quaternion d2 = a + b;
        return (d1.LengthSq() <= eps*eps) || (d2.LengthSq() <= eps*eps);
    }
};

// ===== 便捷函数 =====
inline Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t) {
    // 普通 Lerp（不保证单位长度）；通常更推荐 Nlerp
    return a*(1.0f - t) + b*t;
}
