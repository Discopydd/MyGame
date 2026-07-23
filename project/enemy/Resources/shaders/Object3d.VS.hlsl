#include"Object3d.hlsli"
/// <summary>
/// 3D頂点をワールド空間およびスクリーン空間へ変換する行列を保持する構造体。
/// </summary>
struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;

};
ConstantBuffer<TransformationMatrix> gTransformtionMatrix : register(b0);

/// <summary>
/// 3Dオブジェクト用頂点シェーダーへ入力する頂点情報を保持する構造体。
/// </summary>
struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformtionMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformtionMatrix.World));
    output.worldPosition = mul(input.position, gTransformtionMatrix.World).xyz;
    return output;
}