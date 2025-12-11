
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

float3x3 Inverse3x3(float3x3 m)
{
    float det = determinant(m);
    
    if (abs(det) < 1e-8)
        return float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);

    float3x3 adj;
    adj[0] = cross(m[1], m[2]);
    adj[1] = cross(m[2], m[0]);
    adj[2] = cross(m[0], m[1]);
    
    adj = transpose(adj);
    
    return adj / det;
}

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(input.Pos,W); // 局部->世界（HLSL：float4 * float4x4 = 列向量*矩阵）
    output.Pos = mul(worldPos,VP); // 世界->裁剪（VP=View*Proj）
    //output.Pos = output.Pos / output.Pos.w;
    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) W));
    output.Normal = mul(input.Normal, normalMatrix);
    
    output.Tangent = mul(input.Tangent, normalMatrix);
    output.TexCoords = input.TexCoords;
    output.TexCoords = input.TexCoords;
    return output;
}
