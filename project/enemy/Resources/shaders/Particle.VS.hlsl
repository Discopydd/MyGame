#include "Particle.hlsli"
/// <summary>
/// パーティクル頂点をワールド空間およびスクリーン空間へ変換する行列を保持する構造体。
/// </summary>
struct TransformationMatrix
{
    float32_t4x4 World;
    float32_t4x4 WVP;
};
/// <summary>
/// GPUで描画するパーティクルのワールド行列と色を保持する構造体。
/// </summary>
struct ParticleForGPU
{
    float32_t4x4 World;
    float32_t4x4 WVP;
    float32_t4 color;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t1);

/// <summary>
/// パーティクル用頂点シェーダーへ入力する頂点座標とテクスチャ座標を保持する構造体。
/// </summary>
struct VertexShaderInput
{
    float32_t4 position : POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};


VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instanceID].WVP);
    output.texcoord = input.texcoord;

    output.color = gParticle[instanceID].color;
    return output;
}