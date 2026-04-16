#include "map/MovingPlatform.h"
#include <algorithm>
#include <cmath>
namespace {
    bool IsSolidForPlatform(MapChipType t) {
        switch (t) {
        case MapChipType::kBlock:
        case MapChipType::kBlock2:
        case MapChipType::kSpike:
        case MapChipType::kPortal:
        case MapChipType::kEnemy:
        case MapChipType::kItem:
            return true;
        default:
            return false;
        }
    }
}

void MovingPlatform::Initialize(Object3dCommon* common, Camera* camera,
    const Vector3& startPos, Axis axis, float speed, int lengthInTiles)
{
    axis_ = axis;
    position_ = startPos;
    prevPosition_ = position_;

    lengthInTiles_ = (std::max)(1, lengthInTiles);

    speed_ = std::fabs(speed);
    if (axis_ == Axis::Horizontal) {
        dir_ = { (speed >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f };
    } else {
        dir_ = { 0.0f, (speed >= 0.0f) ? 1.0f : -1.0f, 0.0f };
    }

    halfW_ = MapChipField::kBlockWidth  * 0.5f * static_cast<float>(lengthInTiles_);
    halfH_ = MapChipField::kBlockHeight * 0.5f;

    // この列の小ブロックを生成
    tiles_.clear();          // ★ unique_ptr が古いオブジェクトを自動で解放する
    tileOffsets_.clear();

    tiles_.reserve(lengthInTiles_);
    tileOffsets_.reserve(lengthInTiles_);

    const float step        = MapChipField::kBlockWidth;
    const float offsetStart = -step * 0.5f * static_cast<float>(lengthInTiles_ - 1);

    for (int i = 0; i < lengthInTiles_; ++i) {
        auto tile = std::make_unique<Object3d>();
        tile->Initialize(common);
        tile->SetModel("cube/cube.obj");
        tile->SetCamera(camera);
        tile->SetTranslate(startPos);

        tiles_.push_back(std::move(tile));

        Vector3 offset{};
        offset.x = offsetStart + step * static_cast<float>(i);
        offset.y = 0.0f;
        offset.z = 0.0f;
        tileOffsets_.push_back(offset);
    }
}


MapChipField::Rect MovingPlatform::GetRect() const
{
    MapChipField::Rect r{};
    r.left   = position_.x - halfW_;
    r.right  = position_.x + halfW_;
    r.bottom = position_.y - halfH_;
    r.top    = position_.y + halfH_;
    return r;
}

void MovingPlatform::HandleBlockCollision(const MapChipField& field)
{
    // 現在の軸方向にのみ移動
    if (axis_ == Axis::Horizontal) {
        float nextX = position_.x;
        float left   = nextX - halfW_;
        float right  = nextX + halfW_;
        float bottom = position_.y - halfH_;
        float top    = position_.y + halfH_;

        auto minIdx = field.GetMapChipIndexByPosition({ left,  bottom, 0 });
        auto maxIdx = field.GetMapChipIndexByPosition({ right, top,    0 });

        bool hit = false;
        float fixX = nextX;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = field.GetMapChipTypeByIndex(x, y);
                if (!IsSolidForPlatform(t)) {
                    continue;
                }
                auto r = field.GetRectByIndex(x, y);

                bool overlapY = !(top <= r.bottom || bottom >= r.top);
                bool overlapX = !(right <= r.left || left >= r.right);
                if (overlapX && overlapY) {
                    hit = true;
                    if (dir_.x > 0)      fixX = r.left  - halfW_;
                    else if (dir_.x < 0) fixX = r.right + halfW_;
                }
            }
        }

        if (hit) {
            position_.x = fixX;
            dir_.x *= -1.0f;   // 反転
        }
    } else { // Vertical
        float nextY = position_.y;
        float left   = position_.x - halfW_;
        float right  = position_.x + halfW_;
        float bottom = nextY - halfH_;
        float top    = nextY + halfH_;

        auto minIdx = field.GetMapChipIndexByPosition({ left,  bottom, 0 });
        auto maxIdx = field.GetMapChipIndexByPosition({ right, top,    0 });

        bool hit = false;
        float fixY = nextY;

        for (uint32_t y = minIdx.yIndex; y <= maxIdx.yIndex; ++y) {
            for (uint32_t x = minIdx.xIndex; x <= maxIdx.xIndex; ++x) {
                MapChipType t = field.GetMapChipTypeByIndex(x, y);
                if (!IsSolidForPlatform(t)) {
                    continue;
                }
                auto r = field.GetRectByIndex(x, y);

                bool overlapX = !(right <= r.left || left >= r.right);
                bool overlapY = !(top   <= r.bottom || bottom >= r.top);
                if (overlapX && overlapY) {
                    hit = true;
                    if (dir_.y > 0)      fixY = r.bottom - halfH_;
                    else if (dir_.y < 0) fixY = r.top    + halfH_;
                }
            }
        }

        if (hit) {
            position_.y = fixY;
            dir_.y *= -1.0f;   // 反転
        }
    }
}

void MovingPlatform::HandlePlatformCollision(const std::vector<MovingPlatform*>& allPlatforms)
{
    MapChipField::Rect self = GetRect();
    for (auto* p : allPlatforms) {
        if (p == this) continue;
        MapChipField::Rect r = p->GetRect();

        bool overlapX = !(self.right <= r.left || self.left >= r.right);
        bool overlapY = !(self.top   <= r.bottom || self.bottom >= r.top);
        if (overlapX && overlapY) {
            // 簡易処理: 前フレーム位置に戻して反転
            position_ = prevPosition_;
            if (axis_ == Axis::Horizontal) {
                dir_.x *= -1.0f;
            } else {
                dir_.y *= -1.0f;
            }
            break;
        }
    }
}

void MovingPlatform::Update(float dt,
                            const MapChipField& field,
                            const std::vector<MovingPlatform*>& allPlatforms)
{
    prevPosition_ = position_;

    Vector3 v = dir_ * speed_;
    position_ = position_ + v * dt;

    // 静的ブロック／トゲ／扉と衝突
    HandleBlockCollision(field);
    // 他の移動床と衝突
    HandlePlatformCollision(allPlatforms);

    for (size_t i = 0; i < tiles_.size(); ++i) {
        if (!tiles_[i]) continue;
        tiles_[i]->SetTranslate(position_ + tileOffsets_[i]);
        tiles_[i]->Update();
    }
}

void MovingPlatform::Draw()
{
    for (auto& tile : tiles_) {
        if (tile) {
            tile->Draw();
        }
    }
}

