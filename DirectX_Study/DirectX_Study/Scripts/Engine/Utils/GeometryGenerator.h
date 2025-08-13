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
		MeshData<Vertex> CreateBox(float width, float height, float depth);
		MeshData<Vertex> CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);
		MeshData<Vertex> CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);
		MeshData<Vertex> CreateGrid(float width, float depth, uint32_t m, uint32_t n);
		MeshData<Vertex> CreateQuad(float x, float y, float w, float h, float depth);

	private:
		void Subdivide(MeshData<Vertex>& meshData);
		Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData<Vertex>& meshData);
		void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData<Vertex>& meshData);

	};
}