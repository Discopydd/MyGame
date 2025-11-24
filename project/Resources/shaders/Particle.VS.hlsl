#include "Particle.hlsli"
struct TransformationMatrix
{
    float32_t4x4 World;
    float32_t4x4 WVP;
};
struct ParticleForGPU
{
    float32_t4x4 World;
    float32_t4x4 WVP;
    float32_t4 color;
};
StructuredBuffer<ParticleForGPU> gParticle : register(t1);

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