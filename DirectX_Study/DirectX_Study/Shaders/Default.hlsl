//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightUtils.hlsl" 

struct InstanceData
{
    float4x4 World;
    float4x4 TexTransform;
    uint MaterialIndex;
    uint InstPad0;
    uint InstPad1;
    uint InstPad2;
};

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint MatPad0;
    uint MatPad1;
    uint MatPad2;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

Texture2D gDiffuseMap[7] : register(t0);

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

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
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    
	nointerpolation uint MatIndex  : MATINDEX;
};

float3x3 cofactor(float3x3 m)
{
    float3x3 c;

    c[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]);  // +C00
    c[0][1] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]); // -C01
    c[0][2] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]);  // +C02

    c[1][0] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]); // -C10
    c[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]);  // +C11
    c[1][2] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]); // -C12

    c[2][0] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]);  // +C20
    c[2][1] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]); // -C21
    c[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]);  // +C22

    return c;
}

float3x3 inverse(float3x3 m)
{
    float det = determinant(m);
    
    // 부동소수점 오차를 방지하기 위해서 절충값 1e-5 사용
    if(det < 1e-5) 
        return float3x3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    
    float3x3 adj = transpose(cofactor(m));
    
    return adj / det;
}

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Fetch the instance data.
    InstanceData instData = gInstanceData[instanceID];
    float4x4 world = instData.World;
    float4x4 texTransform = instData.TexTransform;
    uint matIndex = instData.MaterialIndex;
    
    vout.MatIndex = matIndex;
    
    MaterialData matData = gMaterialData[matIndex];
    
    // * 월드 변환
    float4 posW = mul(float4(vin.PosL, 1.0f), world);
    vout.PosW = posW.xyz;

    // * 뷰, 투영 변환
    vout.PosH = mul(posW, gViewProj);
    
    // * normal vector 변환
    float3x3 invTrans = transpose(inverse((float3x3) world));
    vout.NormalW = normalize(mul(vin.NormalL, invTrans));

    // Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), texTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Fetch the material data.
    MaterialData matData = gMaterialData[pin.MatIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseTexIndex = matData.DiffuseMapIndex;
    
    diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamLinearWrap, pin.TexC);
    
  #ifdef ALPHA_TEST
	// Discard pixel if texture alpha < 0.1.  We do this test as soon 
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);
#endif
    
    // Vector from point being lit to eye. 
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize
    
	//  ambient 계산 = 간접광의 양 * 반사율
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

#ifdef FOG
    float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
 #endif
    
    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}
