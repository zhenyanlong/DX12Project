cbuffer PSLightBuffer : register(b2)
{
    float3 lightDir;
    float lightIntensity;
    float3 lightColor;
    float roughness;
};

struct PS_INPUT
{
    float4 Pos : SV_Position;
    float2 TexCoords : TEXCOORD;
    float3 WorldNormal : NORMAL;
    float3 WorldTangent : TANGENT;
    float3 WorldBitangent : BITANGENT;
    float3 WorldPos : WORLDPOS; // 新增：世界位置（用于水面计算）
};

// 辅助函数：Lambert漫反射BRDF（基础）
float3 LambertBRDF(float3 albedo, float NdotL)
{
    // Lambert公式：(albedo / π) * max(NdotL, 0)
    return (albedo / 3.1415926535) * max(NdotL, 0.0);
}
// 像素着色器保留（可添加水面的菲涅尔效果，增强真实感）

float4 PS(PS_INPUT input) : SV_Target0
{
    // 原有光照逻辑 + 水面菲涅尔效果（可选）
    //float4 albedoTexel = tex.Sample(samplerLinear, input.TexCoords);
    float3 albedoTexel = float3(0.f, 0.8f, 0.8f);
    //if (albedoTexel.a < 0.5)
    //    discard;
    float3 linearAlbedo = albedoTexel.rgb;

    // 菲涅尔计算（水面反射的基础）
    float3 viewDir = normalize(float3(0, 5, 0) - input.WorldPos); // 摄像机位置（需传入，这里临时写死）
    float fresnel = pow(1.0 - max(dot(input.WorldNormal, viewDir), 0.0), 2.0);
    fresnel = saturate(fresnel + 0.1); // 基础反射率

    // 原有光照计算
    float3 L = normalize(lightDir);
    float NdotL = max(dot(input.WorldNormal, L), 0.0);
    float3 diffuse = LambertBRDF(linearAlbedo, NdotL);
    float3 finalColor = diffuse * lightColor * lightIntensity + linearAlbedo * 0.3;

    // 叠加菲涅尔效果（水面更亮）
    //finalColor = lerp(finalColor, float3(1.0, 1.0, 1.0), fresnel);

    return float4(finalColor, 1.0);
}