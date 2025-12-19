cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
    
};
cbuffer AnimMeshBuffer : register(b4)
{
    float4x4 bones[256];
};
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint4 BoneIDs : BONEIDS;
    float4 BoneWeights : BONEWEIGHTS;
};

struct PS_INPUT
{
    float4 Pos : SV_Position;
    float2 TexCoords : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
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
    float4 pos = input.Pos;
    float4x4 transform;
    transform = bones[input.BoneIDs[0]] * input.BoneWeights[0];
    transform += bones[input.BoneIDs[1]] * input.BoneWeights[1];
    transform += bones[input.BoneIDs[2]] * input.BoneWeights[2];
    transform += bones[input.BoneIDs[3]] * input.BoneWeights[3];
    
    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) W));
    
    output.Pos = mul(pos, transform);
    //output.Pos = input.Pos;
    output.Pos = mul(output.Pos, W);
    output.Pos = mul(output.Pos, VP);
    output.Normal = mul(input.Normal, (float3x3) transform);
    output.Normal = normalize(output.Normal);
    output.Normal = mul(output.Normal, normalMatrix);
    //output.Normal = mul(input.Normal, (float3x3) W);
    output.Normal = normalize(output.Normal);
    output.Tangent = mul(input.Tangent, (float3x3) transform);
    output.Tangent = normalize(output.Tangent);
    output.Tangent = mul(output.Tangent, normalMatrix);
    //output.Tangent = mul(input.Tangent, (float3x3) W);
    output.Tangent = normalize(output.Tangent);
    output.TexCoords = input.TexCoords;
    
    output.Bitangent = normalize(cross(output.Normal, output.Tangent) * 1.0f);
    
    return output;
}