/// <summary>
/// 3Dオブジェクトの頂点シェーダーからピクセルシェーダーへ渡す情報を保持する構造体。
/// </summary>
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : POSITION0;
};
