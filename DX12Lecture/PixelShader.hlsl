
//cbuffer bufferName : register(b0)
//{
//    float time;
//    float2 lights[4];
//};



//float dist(float2 screenspace_pos, float2 light)
//{
//	return (length(screenspace_pos - light) / (50.0 * abs(cos(time))));
//}
struct PS_INPUT
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
float4 PS(PS_INPUT input) : SV_Target0
{
    return float4(abs(normalize(input.Normal)) * 0.9f, 1.0);
    //return float4(float3(1.f, 1.f, 1.f), 1.f);
}
