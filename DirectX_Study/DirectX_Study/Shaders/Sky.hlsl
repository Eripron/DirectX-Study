#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITIONT;
};

VertexOut SkyVS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
    
    vout.PosL = vin.PosL;
    
    float4 posW = mul(float4(vin.PosL, 1.0f), World);
    //posW.xyz += gEyePosW;
    
    vout.PosH = mul(posW, gViewProj).xyzw;
    
    return vout;
}

float4 SkyPS(VertexOut pin) : SV_Target
{
    return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
}
