#include "Common.hlsl"

struct VertexIn
{
    float3 PosL : POSITIONT;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0;
    
    MaterialData matData = gMaterialData[MaterialIndex];
    
    // 동차 좌표로 변환
    float3 posW = mul(float4(vin.PosL, 1.0f), World);
    vout.PosH = mul(float4(posW, 1.0f), gViewProj);
    
    // 비균등 스케일링이 있을 때 normal vector를 올바르게 변환
    float3x3 invTrans = transpose(inverse((float3x3) World));
    vout.NormalW = normalize(mul(vin.NormalL, invTrans));
    
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), TexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;
    
    // tangent vector 변환
    vout.TangentW = mul(vin.TangentU, (float3x3) World);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // view space로 normal vector 변환
    float3 normalV = mul(pin.NormalW, (float3x3)gView);
    return float4(normalV, 0.0f);
}