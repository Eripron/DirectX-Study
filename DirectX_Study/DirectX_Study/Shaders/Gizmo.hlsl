//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gViewProj;
};
 
struct VertexIn
{
	float3 PosL     : POSITION;
    float4 Color    : COLOR;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float4 Color    : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld); // 월드 변환
    vout.PosH = mul(posW, gViewProj); // 뷰, 투영 변환
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}
