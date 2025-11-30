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
#include "Common.hlsl"
//#include "LightUtils.hlsl"

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
    float4x4 world = World;
    float4x4 texTransform = TexTransform;
    uint matIndex = MaterialIndex;
    
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
    
    float3 r = reflect(-toEyeW, pin.NormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, pin.NormalW, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}
