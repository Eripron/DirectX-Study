#pragma once

#include <vector>
//#include <cstdint>
#include <DirectXMath.h>

#include "../Data/DataTypes.h"

namespace DK
{
	class GeometryGenerator
	{
	public:
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