Texture2D tex : register(t0);
SamplerState samplerLinear : register(s0);
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
float4 PS(PS_INPUT input) : SV_Target0
{
    float4 colour = tex.Sample(samplerLinear, input.TexCoords);
    //colour = tex.Sample(samplerLinear, input.TexCoords);
    float3 linearRGB = pow(colour.rgb, float3(2.2,2.2,2.2));
    // 提亮后转换回sRGB（输出到帧缓冲前）
    float3 sRGB = pow(linearRGB * 3, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    if (colour.a < 0.5)
    {
        discard;
    }
    return float4(colour.rgb, 1.0);
}
