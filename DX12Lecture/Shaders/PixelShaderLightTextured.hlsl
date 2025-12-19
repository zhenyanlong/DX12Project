cbuffer PSLightBuffer : register(b2)
{
    float3 lightDir; 
    float lightIntensity; 
    float3 lightColor; 
    float roughness; 
};

Texture2D tex : register(t0);
Texture2D normalTex : register(t1); 
SamplerState samplerLinear : register(s0);
struct PS_INPUT
{
    float4 Pos : SV_Position;
    float2 TexCoords : TEXCOORD;
    float3 WorldNormal : NORMAL;
    float3 WorldTangent : TANGENT;
    float3 WorldBitangent : BITANGENT;
};

// 辅助函数：Lambert漫反射BRDF（基础）
float3 LambertBRDF(float3 albedo, float NdotL)
{
    // Lambert公式：(albedo / π) * max(NdotL, 0)
    return (albedo / 3.1415926535) * max(NdotL, 0.0);
}

float4 PS(PS_INPUT input) : SV_Target0
{
    float4 albedoTexel = tex.Sample(samplerLinear, input.TexCoords);
    
    if (albedoTexel.a < 0.5)
        discard; // Alpha测试
    float3 linearAlbedo = pow(albedoTexel.rgb, 1.0/2.2); // SRGB → 线性空间
    //float3 linearAlbedo = albedoTexel.rgb;
    
    // 2. 采样法线贴图，转换为切线空间法线（[0,1] → [-1,1]）
    float3 tangentNormal = normalTex.Sample(samplerLinear, input.TexCoords).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0); // 映射到[-1,1]

    // 3. 构建TBN矩阵（切线空间→世界空间）
    float3 T = normalize(input.WorldTangent);
    float3 B = normalize(input.WorldBitangent);
    float3 N = normalize(input.WorldNormal);
    float3x3 TBN = float3x3(T, B, N); // 列主序：切线、副切线、法线

    // 4. 转换法线到世界空间
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    
    float3 L = normalize(lightDir);
    
    float NdotL = max(dot(worldNormal, L), 0.0);
    
    float3 diffuse;
    diffuse = LambertBRDF(linearAlbedo, NdotL);
    //diffuse = (linearAlbedo / 3.1415926535) * max(NdotL, 0.0);
    
    // 7. 最终颜色计算（光源颜色×强度×漫反射 + 环境光(强度0.1)）
    float3 finalColor = diffuse * lightColor * lightIntensity + linearAlbedo * 0.5;

    
    return float4(finalColor, 1.0);
}
