
cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};


struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};
struct PS_INPUT
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
};


PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(input.Pos,W); // 局部->世界（HLSL：float4 * float4x4 = 列向量*矩阵）
    output.Pos = mul(worldPos,VP); // 世界->裁剪（VP=View*Proj）
    //output.Pos = output.Pos / output.Pos.w;
    
    output.Normal = mul(input.Normal, (float3x3) W);
    
    output.Tangent = mul(input.Tangent, (float3x3) W);
    output.TexCoords = input.TexCoords;
    output.TexCoords = input.TexCoords;
    return output;
}
