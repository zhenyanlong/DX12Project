struct GerstnerWave
{
    float directionX; 
    float directionY;
    float amplitude;  
    float wavelength; 
    float phaseOffset; 
    float steepness; 
    float padding[2]; 
};




cbuffer staticMeshBuffer : register(b0)
{
    float4x4 W;
    float4x4 VP;
};
cbuffer WaterBuffer : register(b3)
{
    GerstnerWave waves[16];
    // Automatic configuration parameters
    int waveCount; // wave count
    float wavelengthMin; // Min wave length
    float wavelengthMax; // Max wave length
    float steepnessMin; // Min steepness
    float steepnessMax; // Max steepness
    float2 baseDirection; // main direction
    float randomDirection; // 0 = Completely along the main direction£¬1 = Completely random
    float time; 
    float scale; 
    float waveHeightGain; // wave height scale
    int seed; // random seed
    float padding; 
};
float Random(float seed)
{
    // dot(seed vector, fixed vector) ¡ú sine value ¡ú fractional part ¡ú mapped to [-1, 1]
    return frac(sin(dot(float2(seed, 2.0), float2(12.9898, 78.233))) * 43758.5453) * 2.0 - 1.0;
}

void ComputeGerstnerWaves(float3 inputPos, out float3 outputPos, out float3 outputNormal)
{
    // init pos
    float2 pos = inputPos.xz * scale;
    outputPos = inputPos;
    float3 normal = float3(0.0, 1.0, 0.0); 

    // clamp wave count
    int actualWaveCount = min(waveCount, 16);

    // overlay multiple waves£¨Automatically calculate the parameters of each wave£©
    for (int i = 0; i < actualWaveCount; i++)
    {
        
        float step = (float) i / (float) (actualWaveCount - 1); 
        

        
        float wavelength = lerp(wavelengthMin, wavelengthMax, step);
        float steepness = lerp(steepnessMin, steepnessMax, step);

        // spawn random direction
        float randomSeedX = (float) (i + seed);
        float randomSeedY = (float) (2 * (i + seed));
        float2 randomDir = float2(Random(randomSeedX), Random(randomSeedY));
        randomDir = normalize(randomDir); 
        
        float2 baseDir = normalize(baseDirection);
        float2 dir = normalize(lerp(baseDir, randomDir, randomDirection));

        
        float frequency = 2.0 * 3.1415926535 / wavelength; 
        float amplitude = steepness * wavelength / (2.0 * 3.1415926535); 
        
        float phase = time * frequency + Random((float) (3 * (i + seed))) * 2.0 * 3.1415926535;

        // Gerstner wave calculation
        if (amplitude < 1e-6)
            continue;

        float theta = 2.0 * 3.1415926535 * (dot(dir, pos) * frequency + phase);
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        // modify vertex position
        float2 horzDisplacement = steepness * amplitude * dir * cosTheta / frequency;
        float vertDisplacement = amplitude * sinTheta * waveHeightGain;
        outputPos.x += horzDisplacement.x;
        outputPos.z += horzDisplacement.y;
        outputPos.y += vertDisplacement;

        // modify normal
        float dHdx = amplitude * frequency * dir.x * cosTheta;
        float dHdz = amplitude * frequency * dir.y * cosTheta;
        normal.x -= steepness * dHdx;
        normal.z -= steepness * dHdz;
    }

    // normalize
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
    float3 WorldPos : WORLDPOS; 
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
    


    float3 localPos = input.Pos.xyz; 
    float3 localNormal;
    float3 outLocalPos;
    ComputeGerstnerWaves(localPos, outLocalPos, localNormal); 


    float4 worldPos = mul(float4(outLocalPos, 1.0), worldMatrix);
    output.Pos = mul(worldPos, VP); 
    output.WorldPos = worldPos.xyz;


    float3x3 normalMatrix = transpose(Inverse3x3((float3x3) worldMatrix));
    output.WorldNormal = normalize(mul(localNormal, normalMatrix));
    output.WorldTangent = normalize(mul(input.Tangent, normalMatrix));
    output.TexCoords = input.TexCoords;


    output.WorldBitangent = normalize(cross(output.WorldNormal, output.WorldTangent) * 1.0f);

    return output;
}