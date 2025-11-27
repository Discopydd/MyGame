#include"Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct Camera
{
    float32_t3 worldPosition;
};

struct PointLight
{
    float32_t4 color;
    float32_t3 position;
    float intensity;
};

struct SpotLight
{
    float32_t4 color;
    float32_t3 position;
    float32_t intensity;
    float32_t3 direction;
    float32_t distance;
    float32_t decay;
    float32_t cosAngle;
    float32_t cosFalloffStart;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    float NDotH = dot(normalize(input.normal), halfVector);
    float speclarPow = pow(saturate(NDotH), gMaterial.shininess);

    // 0：完全不使用光照（原逻辑）
    if (gMaterial.enableLighting == 0)
    {
        output.color = gMaterial.color * textureColor;
    }
    // 2：固定光照模式
    else if (gMaterial.enableLighting == 2)
    {
        float32_t3 baseColor = gMaterial.color.rgb * textureColor.rgb;
        float32_t3 N = normalize(input.normal);
        float32_t3 L = normalize(-gDirectionalLight.direction);

    // --- 漫反射（方向光） ---
        float NdotLFixed = saturate(dot(N, L));
        float32_t3 diffuseFixed =
        baseColor * gDirectionalLight.color.rgb * NdotLFixed * gDirectionalLight.intensity;

    // --- 高光（反光）：用视线方向 + 半角向量 ---
        float32_t3 V = normalize(gCamera.worldPosition - input.worldPosition); // 视线方向
        float32_t3 H = normalize(L + V); // 半角向量
        float NdotH = saturate(dot(N, H));
        float specPow = pow(NdotH, gMaterial.shininess); // 用同一个 shininess

    // 高光颜色：强度不要太大，否则一转镜头就很闪
        float specIntensity = 0.8f; // 想更强可以 0.5f, 太闪就 0.1f
        float32_t3 specularFixed =
        gDirectionalLight.color.rgb * specPow * specIntensity;

    // --- 常量环境光（整体提亮） ---
        float32_t3 ambientFixed = baseColor * 2.0f;

        output.color.rgb = ambientFixed + diffuseFixed + specularFixed;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    // 其它值（1）：保持原来的完整光照逻辑
    else
    {
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);

        // 平行光计算
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        float32_t3 diffuseDirectional =
            gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        float32_t3 specularDirectional =
            gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);

        // 点光源计算
        float32_t distancePoint = length(gPointLight.position - input.worldPosition);
        float32_t factorPoint = 1.0f / (distancePoint * distancePoint + 0.001f);
        float32_t3 pointLightDir = normalize(gPointLight.position - input.worldPosition);
        float NdotLPoint = dot(normalize(input.normal), pointLightDir);
        float cosPoint = pow(NdotLPoint * 0.5f + 0.5f, 2.0f);
        float32_t3 diffusePoint =
            gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cosPoint *
            gPointLight.intensity * factorPoint;

        // 聚光灯计算
        float32_t3 spotLightDir = normalize(gSpotLight.position - input.worldPosition);
        float cosAngle = dot(spotLightDir, gSpotLight.direction);
        float falloffFactor =
            saturate((cosAngle - gSpotLight.cosFalloffStart) /
                     (gSpotLight.cosAngle - gSpotLight.cosFalloffStart));

        float32_t distanceSpot = length(gSpotLight.position - input.worldPosition);
        float32_t factorSpot;
        if (distanceSpot > gSpotLight.distance)
        {
            factorSpot = 0.0f; // 超出距离范围无光照
        }
        else
        {
            factorSpot =
                pow(saturate(1.0f - distanceSpot / gSpotLight.distance), gSpotLight.decay); // 应用 decay 参数
        }

        float NdotLSpot = dot(normalize(input.normal), spotLightDir);
        float cosSpot = pow(NdotLSpot * 0.5f + 0.5f, 2.0f);
        float32_t3 diffuseSpot =
            gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpot *
            gSpotLight.intensity * factorSpot * falloffFactor;

        // 合并所有光照
        output.color.rgb = diffuseDirectional + specularDirectional + diffusePoint + diffuseSpot;
        output.color.a = gMaterial.color.a * textureColor.a; // 保留 Alpha 通道
    }

    return output;
}
