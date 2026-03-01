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
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

TextureCube gCubeMap : register(t0);
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1); // material

Texture2D gDiffuseMap[20] : register(t1); // texture

// Constant data that varies per material.
cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
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