#pragma once

#include <vector>
#include <DirectXMath.h>
#include <cstdint>

namespace DK
{
	class GeometryGenerator
	{
	public:

		struct Vertex
		{
			Vertex() {}
			Vertex(
				const DirectX::XMFLOAT3& p, 
				const DirectX::XMFLOAT3& n, 
				const DirectX::XMFLOAT3& t, 
				const DirectX::XMFLOAT2& uv) : 
				Position(p), 
				Normal(n), 
				TangentU(t), 
				TexC(uv) {}

			Vertex(
			float px, float py, float pz, 
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) : 
            Position(px,py,pz), 
            Normal(nx,ny,nz),
			TangentU(tx, ty, tz), 
            TexC(u,v){}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 TangentU;
			DirectX::XMFLOAT2 TexC;
		};

		struct MeshData
		{
			MeshData() {}

			std::vector<Vertex> Vertices;
			std::vector<uint32_t> Indices32;

			std::vector<uint16_t>& GetIndices16()
			{
				// TODO: 메모리 등 최적화를 위한 작업은 알겠다.
				// 하지만 만약 static_cast<uint16_t> 부분에서 예외가 나오는 경우는 어떻게 처리할지 고민해보자.
				if (mIndices16.empty())
				{
					mIndices16.resize(Indices32.size());
					for (size_t i = 0; i < Indices32.size(); ++i)
						mIndices16[i] = static_cast<uint16_t>(Indices32[i]);
				}

				return mIndices16;
			}

		private:
			std::vector<uint16_t> mIndices16;
		};
		

		MeshData CreateBox(float width, float height, float depth);

		MeshData CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

		MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);

		MeshData CreateGrid(float width, float depth, uint32_t m, uint32_t n);

		MeshData CreateQuad(float x, float y, float w, float h, float depth);

	private:
		void Subdivide(MeshData& meshData);
		Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);

	};
}