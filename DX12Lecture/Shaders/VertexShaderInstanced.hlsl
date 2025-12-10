
cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};


// 实例化常量缓冲区：存储多个实例的世界矩阵
cbuffer instanceBuffer : register(b1)
{
    float4x4 instanceMatrices[100]; // 支持最多100个实例
};

// 新增实例化输入结构（保留原顶点属性，添加实例ID）
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoords : TEXCOORD;
    uint InstanceID : SV_InstanceID; // 实例ID（由GPU自动生成）
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
    // 使用实例ID获取对应的世界矩阵
    float4x4 instanceW = instanceMatrices[input.InstanceID];
    // 局部坐标 -> 世界坐标（实例矩阵）
    float4 worldPos = mul(input.Pos, instanceW);
    output.Pos = mul(worldPos, VP); // 世界->裁剪（VP=View*Proj）
    // 法线/切线的世界空间变换
    output.Normal = mul(input.Normal, (float3x3) instanceW);
    output.Tangent = mul(input.Tangent, (float3x3) instanceW);
    output.TexCoords = input.TexCoords;
    return output;
}
