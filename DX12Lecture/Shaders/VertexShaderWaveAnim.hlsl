struct GerstnerWave
{
    float directionX; // 波的传播方向（归一化）
    float directionY;
    float amplitude; // 振幅（波高，可关联波长：amplitude = steepness * wavelength / (2π)，更物理）
    float wavelength; // 波长（替代原frequency，频率由波长计算）
    float phaseOffset; // 相位偏移（随机，避免波同步）
    float steepness; // 陡度（控制波的卷曲程度，0~1）
    float padding[2]; // 内存对齐
};

// ===== 新增：水面常量缓冲区（register(b3)，需对应根签名的新索引）=====


cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};
cbuffer WaterBuffer : register(b3)
{
    GerstnerWave waves[16];
    // 自动配置参数
    int waveCount; // 实际使用的波数量（如8、12）
    float wavelengthMin; // 波长最小值
    float wavelengthMax; // 波长最大值
    float steepnessMin; // 陡度最小值
    float steepnessMax; // 陡度最大值
    float2 baseDirection; // 基础方向（如(1,0)，波的主要传播方向）
    float randomDirection; // 方向随机度（0~1，0=完全沿基础方向，1=完全随机）
    float time; // 时间（用于相位更新）
    float scale; // 水面缩放系数
    float waveHeightGain; // wave height scale
    int seed; // 随机数种子（避免每次运行波的方向都一样）
    float padding; // 内存对齐（保证16字节对齐）
};
float Random(float seed)
{
    // dot(seed向量, 固定向量) → 正弦值 → 小数部分 → 映射到[-1,1]
    return frac(sin(dot(float2(seed, 2.0), float2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0;
}

void ComputeGerstnerWaves(float3 inputPos, out float3 outputPos, out float3 outputNormal)
{
    // 初始位置：水面平面的x-z坐标（y=0），缩放系数调整范围
    float2 pos = inputPos.xz * scale;
    outputPos = inputPos;
    float3 normal = float3(0.0, 1.0, 0.0); // 初始法线（向上）

    // 限制波的数量（避免超出数组范围）
    int actualWaveCount = min(waveCount, 16);

    // 叠加多个波（自动计算每个波的参数）
    for (int i = 0; i < actualWaveCount; i++)
    {
        // ===== 步骤1：计算当前波的插值因子step（0~1）=====
        float step = (float) i / (float) (actualWaveCount - 1); // 从0到1均匀分布（避免最后一个波参数极端）
        // 若需要更非线性的分布，可对step做曲线映射（如step = pow(step, 1.5)）

        // ===== 步骤2：线性插值波长和陡度 =====
        float wavelength = lerp(wavelengthMin, wavelengthMax, step);
        float steepness = lerp(steepnessMin, steepnessMax, step);

        // ===== 步骤3：生成随机方向（结合基础方向和随机度）=====
        // 随机数种子：用i + seed避免每次运行波的方向一致
        float randomSeedX = (float) (i + seed);
        float randomSeedY = (float) (2 * (i + seed));
        float2 randomDir = float2(Random(randomSeedX), Random(randomSeedY));
        randomDir = normalize(randomDir); // 归一化随机方向
        // 线性插值：基础方向 ←→ 随机方向（由randomDirection控制）
        float2 baseDir = normalize(baseDirection);
        float2 dir = normalize(lerp(baseDir, randomDir, randomDirection));

        // ===== 步骤4：计算波的物理参数 =====
        float frequency = 2.0 * 3.1415926535 / wavelength; // 频率 = 2π/波长
        float amplitude = steepness * wavelength / (2.0 * 3.1415926535); // 振幅与波长关联（避免波过高）
        // 相位：时间*频率 + 随机相位偏移（让波的起始位置不一样）
        float phase = time * frequency + Random((float) (3 * (i + seed))) * 2.0 * 3.1415926535;

        // ===== 步骤5：Gerstner波核心计算（与原有逻辑一致）=====
        if (amplitude < 1e-6)
            continue;

        float theta = 2.0 * 3.1415926535 * (dot(dir, pos) * frequency + phase);
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        // 顶点位置扰动
        float2 horzDisplacement = steepness * amplitude * dir * cosTheta / frequency;
        float vertDisplacement = amplitude * sinTheta * waveHeightGain;
        outputPos.x += horzDisplacement.x;
        outputPos.z += horzDisplacement.y;
        outputPos.y += vertDisplacement;

        // 法线扰动
        float dHdx = amplitude * frequency * dir.x * cosTheta;
        float dHdz = amplitude * frequency * dir.y * cosTheta;
        normal.x -= steepness * dHdx;
        normal.z -= steepness * dHdz;
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
    float3 outLocalPos;
    ComputeGerstnerWaves(localPos, outLocalPos, localNormal); // 扰动位置和法线

    // 本地→世界空间
    float4 worldPos = mul(float4(outLocalPos, 1.0), worldMatrix);
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