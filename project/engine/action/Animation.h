#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "../math/Vector3.h"
#include "../math/Quaternion.h"
#include "../math/Matrix4x4.h"

struct KeyframeTRS {
    float     time = 0.0f;
    Vector3   t{0,0,0};
    Quaternion r{0,0,0,1};
    Vector3   s{1,1,1};
};

struct BoneTrack {
    // 对应 Skeleton::bones 的同索引骨骼
    std::vector<KeyframeTRS> keys;   // 允许为空（无动画）
};

struct AnimationClip {
    std::string name;
    float duration = 0.0f;           // 秒
    std::vector<BoneTrack> tracks;   // size == skeleton.bones.size()
};
