struct GerstnerWave
{
    float2 direction; // 波的传播方向（归一化）
    float amplitude; // 振幅（波高）
    float frequency; // 频率（2π/波长）
    float phase; // 相位（随时间变化，控制波的移动）
    float steepness; // 陡度（控制波的卷曲程度，0~1）
};

// ===== 新增：水面常量缓冲区（register(b3)，需对应根签名的新索引）=====
cbuffer WaterBuffer : register(b3)
{
    GerstnerWave waves[4]; // 4个波叠加，效果更自然
    float time; // 时间（用于更新相位）
    float scale; // 水面缩放系数
};

cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};

void ComputeGerstnerWaves(float3 inputPos, out float3 outputPos, out float3 outputNormal)
{
    // 初始位置：水面平面的x-z坐标（y=0），缩放系数调整范围
    float2 pos = inputPos.xz * scale;
    outputPos = inputPos;
    float3 normal = float3(0.0, 1.0, 0.0); // 初始法线（向上）

    // 叠加4个Gerstner波
    for (int i = 0; i < 4; i++)
    {
        GerstnerWave wave = waves[i];
        if (wave.amplitude < 1e-6)
            continue; // 跳过无效波

        // 归一化波的方向
        float2 dir = normalize(wave.direction);
        // 波的相位：2π*频率*（方向·位置） + 相位 + 时间*频率（随时间移动）
        float theta = 2.0 * 3.1415926535 * (dot(dir, pos) * wave.frequency + wave.phase + time * wave.frequency);
        // 计算余弦和正弦值
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        // ===== 顶点位置扰动 =====
        // 水平位移（x-z）：陡度*振幅*方向*cos(theta)/频率
        float2 horzDisplacement = wave.steepness * wave.amplitude * dir * cosTheta / wave.frequency;
        // 垂直位移（y）：振幅*sin(theta)
        float vertDisplacement = wave.amplitude * sinTheta;
        // 叠加到输出位置
        outputPos.x += horzDisplacement.x;
        outputPos.z += horzDisplacement.y;
        outputPos.y += vertDisplacement;

        // ===== 法线扰动（通过偏导数计算）=====
        // 法线的偏导数：d(位移)/d(x), d(位移)/d(z)
        float dHdx = wave.amplitude * wave.frequency * dir.x * cosTheta;
        float dHdz = wave.amplitude * wave.frequency * dir.y * cosTheta;
        // 扰动后的法线：(-dHdx, 1, -dHdz) 归一化
        normal.x -= wave.steepness * dHdx;
        normal.z -= wave.steepness * dHdz;
    }

    // 归一化法线
    outputNormal = normalize(normal);
}

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
    float2 TexCoords : TEXCOORD;
    float3 WorldNormal : NORMAL;
    float3 WorldTangent : TANGENT;
    float3 WorldBitangent : BITANGENT;
    float3 WorldPos : WORLDPOS; // 新增：世界位置（用于水面计算）
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
    float4x4 worldMatrix;

    
    worldMatrix = W;
    

    // ===== 核心：Gerstner波顶点动画 =====
    float3 localPos = input.Pos.xyz; // 本地平面顶点位置
    float3 localNormal;
    ComputeGerstnerWaves(localPos, localPos, localNormal); // 扰动位置和法线

    // 本地→世界空间
    float4 worldPos = mul(float4(localPos, 1.0), worldMatrix);
    output.Pos = mul(worldPos, VP); // 世界→裁剪空间
    output.WorldPos = worldPos.xyz;

    // 法线/切线的世界空间变换（使用扰动后的本地法线）
    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) worldMatrix));
    output.WorldNormal = normalize(mul(localNormal, normalMatrix));
    output.WorldTangent = normalize(mul(input.Tangent, normalMatrix));
    output.TexCoords = input.TexCoords;

    // 副切线计算（保留原有逻辑）
    output.WorldBitangent = normalize(cross(output.WorldNormal, output.WorldTangent) * 1.0f);

    return output;
}