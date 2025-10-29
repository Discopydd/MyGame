#pragma once
#include <vector>
#include <cassert>
#include "../math/Matrix4x4.h"
#include "../math/Quaternion.h"
#include "../math/Vector3.h"
#include "../math/MyMath.h"  // 你现有的 MakeAffine / Lerp / Slerp 等

#include "Animation.h"
#include "Skeleton.h"

namespace SkinningCPU {

// 在某个 BoneTrack 上根据时间 t 线性采样（缺省返回身份TRS）
inline Matrix4x4 SampleTrack(const BoneTrack& track, float t) {
    if (track.keys.empty()) {
        return Math::MakeIdentity4x4();
    }
    if (track.keys.size() == 1) {
        const auto& k = track.keys[0];
        return Math::MakeAffineMatrix(k.s, Math::QuaternionToEuler(k.r), k.t); // 如有 MakeAffine(T,Rquat,S) 可直接用
    }

    // 二分/顺序查找相邻关键帧
    int k1 = 0, k2 = 0;
    if (t <= track.keys.front().time) {
        k1 = 0; k2 = 1;
    } else if (t >= track.keys.back().time) {
        k1 = int(track.keys.size()) - 2;
        k2 = int(track.keys.size()) - 1;
    } else {
        for (int i = 0; i < int(track.keys.size()) - 1; ++i) {
            if (t >= track.keys[i].time && t <= track.keys[i+1].time) {
                k1 = i; k2 = i+1; break;
            }
        }
    }

    const auto& a = track.keys[k1];
    const auto& b = track.keys[k2];
    float span = (b.time - a.time);
    float alpha = (span > 1e-6f) ? (t - a.time) / span : 0.0f;

    Vector3   T = Math::Lerp(a.t, b.t, alpha);
    Quaternion R = Math::Slerp(a.r, b.r, alpha);
    Vector3   S = Math::Lerp(a.s, b.s, alpha);

    // 你若有 MakeAffine(T,Rquat,S) 就用它；这里演示：把四元数转欧拉，再生成矩阵
    return Math::MakeAffineMatrix(S, Math::QuaternionToEuler(R), T);
}

// 采样整条 Clip，得到每条骨骼的“当前局部矩阵”
inline void SampleClip(const AnimationClip& clip, float t, std::vector<Matrix4x4>& outLocal) {
    outLocal.resize(clip.tracks.size());
    for (size_t i = 0; i < clip.tracks.size(); ++i) {
        outLocal[i] = SampleTrack(clip.tracks[i], t);
    }
}

// 从根到叶，计算“模型空间骨骼矩阵 * inverseBind”（即最终蒙皮矩阵）
inline void BuildBonePalette(const Skeleton& skel,
                             const std::vector<Matrix4x4>& local,
                             const Matrix4x4& modelMatrix,
                             std::vector<Matrix4x4>& outBonePalette)
{
    const size_t n = skel.bones.size();
    assert(local.size() == n);
    outBonePalette.resize(n);

    // 先计算每个骨骼的模型空间矩阵 Mbone（不乘 inverseBind）
    std::vector<Matrix4x4> modelFromJoint(n, Math::MakeIdentity4x4());

    // 多根支持：从每个 root 出发
    auto dfs = [&](auto&& self, int idx) -> void {
        int parent = skel.bones[idx].parent;
        if (parent < 0) {
            modelFromJoint[idx] = modelMatrix * local[idx];
        } else {
            modelFromJoint[idx] = modelFromJoint[parent] * local[idx];
        }
        // 子节点
        for (int j = 0; j < (int)n; ++j) {
            if (skel.bones[j].parent == idx) self(self, j);
        }
    };
    if (skel.roots.empty()) {
        // 没指定 root，默认找 parent==-1 的
        for (int i = 0; i < (int)n; ++i) if (skel.bones[i].parent < 0) dfs(dfs, i);
    } else {
        for (int root : skel.roots) dfs(dfs, root);
    }

    // 蒙皮矩阵 = 模型空间骨骼矩阵 * inverseBind
    for (size_t i = 0; i < n; ++i) {
        outBonePalette[i] = modelFromJoint[i] * skel.bones[i].inverseBind;
    }
}

} // namespace SkinningCPU
