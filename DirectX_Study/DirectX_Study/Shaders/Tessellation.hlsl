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

Texture2D gDiffuseMap : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
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
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
};

cbuffer cbMaterial : register(b2)
{
	float4 gDiffuseAlbedo;  // 반사율(md)
    float3 gFresnelR0;      // 매질(R0)
    float  gRoughness;      // 거칠기(m)
	float4x4 gMatTransform;
};
 
struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
    float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
    vout.PosL = vin.PosL;

    return vout;
}

// tesselation stages
struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

struct PatchTessTri
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

struct PatchTessIsoline
{
    float EdgeTess[2] : SV_TessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
	
    float3 centerL = 0.25f * (patch[0].PosL + patch[1].PosL + patch[2].PosL + patch[3].PosL);
    float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
	
    float d = distance(centerW, gEyePosW);

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.
	
    const float d0 = 20.0f;
    const float d1 = 100.0f;
    float tess = 64.0f * saturate((d1 - d) / (d1 - d0));

	// Uniformly tessellate the patch.

    pt.EdgeTess[0] = tess / 2;
    pt.EdgeTess[1] = tess / 3;
    pt.EdgeTess[2] = tess / 4;
    pt.EdgeTess[3] = tess / 5;
	
    pt.InsideTess[0] = tess / 2;
    pt.InsideTess[1] = tess;
	
    return pt;
}

PatchTessIsoline ConstantHSIsoline(InputPatch<VertexOut, 2> patch, uint patchID : SV_PrimitiveID)
{
    PatchTessIsoline pt;
	
    float3 centerL = 0.25f * (patch[0].PosL + patch[1].PosL);
    float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
	
    float d = distance(centerW, gEyePosW);

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.
	
    const float d0 = 20.0f;
    const float d1 = 100.0f;
    float tess = 64.0f * saturate((d1 - d) / (d1 - d0));
    
    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
	
    return pt;
}

PatchTessTri ConstantHSTri(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchTessTri pt;
	
    float3 centerL = 0.25f * (patch[0].PosL + patch[1].PosL + patch[2].PosL);
    float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
	
    float d = distance(centerW, gEyePosW);

    const float d0 = 20.0f;
    const float d1 = 100.0f;
    float tess = 64.0f * saturate((d1 - d) / (d1 - d0));

    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
	
    pt.InsideTess = tess;
	
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
};

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("ConstantHSIsoline")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 2> controlPoints, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOut hout;
	
    hout.PosL = controlPoints[i].PosL;
	
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

[domain("isoline")]
DomainOut DS(PatchTessIsoline patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 2> quad)
{
    DomainOut dout;
	
    // isoline
    float u = uv.x; // 선을 따라가는 비율
    float v = uv.y; // 라인 index

    // 시작점과 끝점 사이를 보간
    float3 p0 = quad[0].PosL;
    float3 p1 = quad[1].PosL;

    float3 p = lerp(p0, p1, u);
    
	// quad
    //float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
    //float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
    //float3 p = lerp(v1, v2, uv.y);
	
    // tri
    //float3 p = patch[0].PosL * bary.x + patch[1].PosL * bary.y + patch[2].PosL * bary.z;
    
	// Displacement mapping
    p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z)) + v;
	
    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);
	
    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
