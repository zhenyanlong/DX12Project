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

// Lambert diffuse
float3 LambertBRDF(float3 albedo, float NdotL)
{
    
    return (albedo / 3.1415926535) * max(NdotL, 0.0);
}

float4 PS(PS_INPUT input) : SV_Target0
{
    float4 albedoTexel = tex.Sample(samplerLinear, input.TexCoords);
    
    if (albedoTexel.a < 0.5)
        discard; // Alpha testing
    float3 linearAlbedo = pow(albedoTexel.rgb, 1.0/2.2); 
    //float3 linearAlbedo = albedoTexel.rgb;
    
    // Sample the normal map and convert it to the normal in the tangent space£¨[0,1] ¡ú [-1,1]£©
    float3 tangentNormal = normalTex.Sample(samplerLinear, input.TexCoords).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0); 

    // Construct the TBN matrix (from tangent space to world space)
    float3 T = normalize(input.WorldTangent);
    float3 B = normalize(input.WorldBitangent);
    float3 N = normalize(input.WorldNormal);
    float3x3 TBN = float3x3(T, B, N); 

    // Convert the normal vector to the world space
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    
    float3 L = normalize(lightDir);
    
    float NdotL = max(dot(worldNormal, L), 0.0);
    
    float3 diffuse;
    diffuse = LambertBRDF(linearAlbedo, NdotL);
    //diffuse = (linearAlbedo / 3.1415926535) * max(NdotL, 0.0);
    
    // Color blending
    float3 finalColor = diffuse * lightColor * lightIntensity + linearAlbedo * 0.5;

    
    return float4(finalColor, 1.0);
}
