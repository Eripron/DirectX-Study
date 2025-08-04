//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightUtils.hlsl" 

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;  // 반사율(md)
    float3 gFresnelR0;      // 매질(R0)
    float  gRoughness;      // 거칠기(m)
	float4x4 gMatTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
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

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
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

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // * 월드 변환
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // * 뷰, 투영 변환
    vout.PosH = mul(posW, gViewProj);
    
    // * normal vector 변환
    float3x3 invTrans = transpose(inverse((float3x3) gWorld));
    vout.NormalW = normalize(mul(vin.NormalL, invTrans));

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

	//  ambient 계산 = 간접광의 양 * 반사율
    float4 ambient = gAmbientLight * gDiffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { gDiffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = gDiffuseAlbedo.a;
    
    return litColor;
}


