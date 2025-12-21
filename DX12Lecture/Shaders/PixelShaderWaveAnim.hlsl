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
    float3 WorldPos : WORLDPOS; 
};

// Lambert diffuse
float3 LambertBRDF(float3 albedo, float NdotL)
{
    // Lambert¹«Ê½£º(albedo / ¦Ð) * max(NdotL, 0)
    return (albedo / 3.1415926535) * max(NdotL, 0.0);
}


float4 PS(PS_INPUT input) : SV_Target0
{

    
    float3 albedoTexel = float3(0.f, 0.5f, 0.8f);
  
    float3 linearAlbedo = albedoTexel.rgb;




    float3 L = normalize(lightDir);
    float NdotL = max(dot(input.WorldNormal, L), 0.0);

    float3 diffuse = LambertBRDF(linearAlbedo, NdotL);
    float3 finalColor = diffuse * lightColor * lightIntensity + linearAlbedo * 0.8;
    


    return float4(finalColor, 1.0);
}