#include "LightUtils.hlsl"

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint NormalMapIndex;
    uint MatPad1;
    uint MatPad2;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

/*
    register type
    -   b : constant buffer
    -   t : shader resource view (texture, buffer)
    -   s : sampler state
    -   u : unordered access view
*/

TextureCube gCubeMap : register(t0);
Texture2D gShadowMap : register(t1);
Texture2D gDiffuseMap[6] : register(t2); // t2 ~ t7

StructuredBuffer<MaterialData> gMaterialData : register(t0, space1); // material

// Constant data that varies per material.
cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    
    float4x4 gProj;
    float4x4 gInvProj;
    
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    
    float4x4 gShadowTransform;
    
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    Light gLights[MaxLights];
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
};

cbuffer ObjectConstBuffer : register(b1)
{
    float4x4 World;
    float4x4 TexTransform;
    uint MaterialIndex;
};

// normal map sample을 world space로 변환
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 normalWorld, float3 tangentWorld)
{
	// normal map sample을 -1 ~ 1 범위로 수정
    float3 normalT = 2.0f * normalMapSample - 1.0f;

	// TBN 기저 벡터 구하기
    float3 N = normalWorld;
    float3 T = normalize(tangentWorld - dot(tangentWorld, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

	// tangent space -> world space로 벡터 변환
    float3 bumpedNormalW = mul(normalT, TBN);
     
    return bumpedNormalW;
}

/// for shadow mapping
float CalcShadowFactor(float4 shadowPosH)
{
    // 현재 출력 지점의 빛 시점에서의 depth 계산
    shadowPosH.xyz /= shadowPosH.w;
    float depth = shadowPosH.z;

    // GetDimesions는 해당 Texture 2d의 정보를 가져오는 함수
    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        // shadow texture에서 특정 위치(shadowPosH.xy + offsets[i])의 depth와 현재 지점의 depth를 비교해서 
        // 값을 가져온다.
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}