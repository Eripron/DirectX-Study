//=============================================================================
// Ssao.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//=============================================================================

cbuffer cbSsao : register(b0)
{
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gProjTex;
    float4 gOffsetVectors[14];

    // For SsaoBlur.hlsl
    float4 gBlurWeights[3];

    float2 gInvRenderTargetSize;

    // Coordinates given in view space.
    float gOcclusionRadius;
    float gOcclusionFadeStart;
    float gOcclusionFadeEnd;
    float gSurfaceEpsilon;
};

cbuffer cbRootConstants : register(b1)
{
    bool gHorizontalBlur;
};
 
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalMap : register(t0);
Texture2D gDepthMap : register(t1);
Texture2D gRandomVecMap : register(t2);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);

static const int gSampleCount = 14;

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
	
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};
 
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float2 TexC : TEXCOORD0;
};

// ssao에서는 풀스크린 쿼드를 사용하여 모든 픽셀에 대해 셰이더를 실행한다.
// : 외부에서 가상의 6개의 정점 ID를 넘겨받아 처리한다.
VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;

	// 해당 정점의 텍스처 좌표
    vout.TexC = gTexCoords[vid];

    // 텍스처 좌표를 NDC 좌표로 변환 (z는 0 = near, w = 1)
    vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
 
    // NDC 좌표를 뷰 공간 좌표로 변환
    float4 ph = mul(vout.PosH, gInvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}

float NdcDepthToViewDepth(float z_ndc)
{
	/*
	P_view = (x, y, z, 1)
	P_clip = P_view × Projection
	| A  0  0   0 |
	| 0  B  0   0 |
	| 0  0  C   D |
	| 0  0  1   0 |

	x_clip = A * x
	y_clip = B * y
	z_clip = C * z + D
	w_clip = z
	*/
	
    // depth 값을 view로 변환은 좌표 변환의 역순으로 계산
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}

float OcclusionFunction(float distZ)
{
	// q의 depth가 p의 depth보다 뒤에 있다면(q가 더 멀다면),
	// q는 p를 가릴 수 없다.
	// 또한 q와 p의 depth 차이가 너무 작으면,
	// 같은 표면에서 발생한 오차로 판단하여 occlusion으로 처리하지 않는다
	// (self-occlusion 방지).
	//
	// 아래 함수는 이러한 조건을 만족하는 경우에만
	// 거리 기반으로 occlusion 값을 계산한다 
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
    float occlusion = 0.0f;
    if (distZ > gSurfaceEpsilon)
    {
         // occlusion이 1에서 0으로 감소하는 구간의 길이
        float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;

		// distZ가 FadeStart일 때 occlusion = 1
		// distZ가 FadeEnd일 때 occlusion = 0
		// 그 사이에서는 선형적으로 감소
        occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
    }
	
    return occlusion;
}
 
float4 PS(VertexOut pin) : SV_Target
{
	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// 현재 출력 픽셀의 tex 좌표에서 view 공간의 법선 벡터를 가져온다.
    float3 n = normalize(gNormalMap.SampleLevel(gsamPointClamp, pin.TexC, 0.0f).xyz);
	
	// depth 맵에서 Depth 값을 가져온다.
    float pz = gDepthMap.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;
	// view 공간의 depth 값 z 변환
    pz = NdcDepthToViewDepth(pz);

	// pin.PosV는 View 공간 위치에 대한 방향
	// 우리가 실제로 원하는 것은 해당 방향의 (?, ?, pz) = pin.PosV * t 이다.
	// pz를 알고 있으니 p.z = t * pin.PosV.z이고 t = p.z / pin.PosV.z
	// depth pz에 해당하는 view 공간 위치 p 계산
    float3 p = (pz / pin.PosV.z) * pin.PosV;
	
	// Extract random vector and map from [0,1] --> [-1, +1].
    float3 randVec = 2.0f * gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f * pin.TexC, 0.0f).rgb - 1.0f;

    float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
    for (int i = 0; i < gSampleCount; ++i)
    {
		// gOffsetVectors라는 미리 균등하게 분포한 Vector를 하나 만들고
		// 동일한 패턴이 반복되지 않도록 랜덤 벡터를 사용하여 반사시킨다.
        float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
	
		// 랜덤한 offset 반향이 법선을 포함한 반구에 있도록 하기 위해서 flip(+1 or -1)을 구한다.
        float flip = sign(dot(offset, n));
		
		// p(view 공간 위치)에서 offset 방향으로 gOcclusionRadius 만큼 떨어진 q 위치를 구한다.
        float3 q = p + flip * gOcclusionRadius * offset;
		
		// q에 대한 texture texture 좌표로 변환
        float4 projQ = mul(float4(q, 1.0f), gProjTex);
        projQ /= projQ.w;

		// q의 텍스처 좌표에서 depth 값을 읽어온다.
        float rz = gDepthMap.SampleLevel(gsamDepthMap, projQ.xy, 0.0f).r;
		// q의 뷰 공간 방향에 대한 실제 z값을 가져온다.
        rz = NdcDepthToViewDepth(rz);

		// r.z = t*q.z ==> t = r.z / q.z
        float3 r = (rz / q.z) * q;
		
        float distZ = p.z - r.z;	// 현재 출력 p와 샘플링한 최소 depth r과의 거리
        float dp = max(dot(n, normalize(r - p)), 0.0f); // 법선과 (r-p) 방향의 각도에 따른 가중치 (법선과 방향이 비슷할수록 빛이 많이 들어옴)

		// 거리 기반 및 각도 기반 차폐도 계산
        float occlusion = dp * OcclusionFunction(distZ);

        occlusionSum += occlusion;
    }
	
	// 평균 차폐도 계산
    occlusionSum /= gSampleCount;
	
	// 최종 도달도 계산
    float access = 1.0f - occlusionSum;

	// statuate = clamp(0,1)
	// pow는 색 보정을 위해서(어두운 부분을 더 어둡게)
    return saturate(pow(access, 6.0f));
}
