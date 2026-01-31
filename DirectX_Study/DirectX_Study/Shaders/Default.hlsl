//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "Common.hlsl"

struct VertexIn
{
	float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 TexC     : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float4 SsaoPosH : POSITIONT1;
    float3 PosW     : POSITION1;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC     : TEXCOORD;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
	VertexOut vout = (VertexOut)0.0f;
    
    MaterialData matData = gMaterialData[MaterialIndex];
    
    // world 좌표로 변환
    float4 posW = mul(float4(vin.PosL, 1.0f), World);
    vout.PosW = posW.xyz;

    // view, projection 좌표로 변환
    vout.PosH = mul(posW, gViewProj);
    
    vout.SsaoPosH = mul(posW, gViewProjTex);
    
    // normal vector (local) -> (world)
    float3x3 invTrans = transpose(inverse((float3x3) World));
    vout.NormalW = normalize(mul(vin.NormalL, invTrans));
    
    // tangent vector (local) -> (world)
    vout.TangentW = mul(vin.TangentU, (float3x3) World);
    
    // Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), TexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
    
    vout.ShadowPosH = mul(posW, gShadowTransform);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // material data
    MaterialData matData = gMaterialData[MaterialIndex];
    
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseTexIndex = matData.DiffuseMapIndex;
    uint normalMapIndex = matData.NormalMapIndex;
    
    // texture sampling
    diffuseAlbedo *= gDiffuseMap[diffuseTexIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
    // normal sampling
    pin.NormalW = normalize(pin.NormalW);
    float4 normalMapSample = gDiffuseMap[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    
    // Vector from point being lit to eye. 
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize
    
    pin.SsaoPosH /= pin.SsaoPosH.w;
    float ambientAccess = gSsaoMap.Sample(gsamLinearClamp, pin.SsaoPosH.xy, 0.0f).r;
    
	//  ambient 계산 = 간접광의 양 * 반사율 * Occlusion 값
    float4 ambient = ambientAccess * gAmbientLight * diffuseAlbedo;

    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);
    //shadowFactor[0] = 0.0f;
    
    const float shininess = (1.0f - roughness) * normalMapSample.a;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, bumpedNormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
    
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
    
    litColor.a = diffuseAlbedo.a;
    
    return litColor;
}
