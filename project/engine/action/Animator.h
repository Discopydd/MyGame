#pragma once
#include "Animation.h"
#include "Skeleton.h"

struct Animator {
    const Skeleton*     skeleton = nullptr;
    const AnimationClip* clip     = nullptr;

    float time  = 0.0f;
    float speed = 1.0f;
    bool  loop  = true;

    void Advance(float dt) {
        if (!clip) return;
        time += dt * speed;
        if (loop && clip->duration > 0.0f) {
            // 允许大步进，使用 while/fmod
            while (time >= clip->duration) time -= clip->duration;
            while (time < 0.0f)            time += clip->duration;
        } else {
            if (time < 0.0f) time = 0.0f;
            if (time > clip->duration) time = clip->duration;
        }
    }
};
