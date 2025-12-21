
cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};



cbuffer instanceBuffer : register(b1)
{
    float4x4 instanceMatrices[100]; 
};


struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint InstanceID : SV_InstanceID; 
};
struct PS_INPUT
{
    float4 Pos : SV_Position;
    float2 TexCoords : TEXCOORD;
    float3 WorldNormal : NORMAL;
    float3 WorldTangent : TANGENT;
    float3 WorldBitangent : BITANGENT;
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
    
    float4x4 instanceW = instanceMatrices[input.InstanceID];
    
    float4 worldPos = mul(input.Pos, instanceW);
    output.Pos = mul(worldPos, VP); 
    
    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) instanceW));
    //float3x3 normalMatrix = (float3x3) instanceW;
    output.WorldNormal = normalize(mul(input.Normal, normalMatrix));
    output.WorldTangent = normalize(mul(input.Tangent, normalMatrix));
    output.TexCoords = input.TexCoords;
    
    output.WorldBitangent = normalize(cross(output.WorldNormal, output.WorldTangent) * 1.0f);
    return output;
}
