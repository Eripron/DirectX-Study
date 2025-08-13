#include "GeometryGenerator.h"
#include <algorithm>

using namespace DK;

MeshData<Vertex> GeometryGenerator::CreateBox(float width, float height, float depth)
{
	MeshData<Vertex> mesh;

	Vertex v[24];

	float w2 = width * 0.5f;
	float h2 = height * 0.5f;
	float d2 = depth * 0.5f;

	// front face
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// back face
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// top face
	v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// bottom face
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// left face
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// right face
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	mesh.Vertices.assign(&v[0], &v[24]);

	// index data
	uint32_t i[36];
	// front face
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// back face
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// top face
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// bottom face
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// left face
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// right face
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	mesh.Indices32.assign(&i[0], &i[36]);

	return mesh;
}

MeshData<Vertex> GeometryGenerator::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
	MeshData<Vertex> meshData;

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = DirectX::XM_PI * 2.0f / sliceCount;

	Vertex topVertex(0.0f, radius, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	for (int i = 0; i < stackCount; ++i)
	{
		float phi = (i + 1) * phiStep;

		for (int j = 0; j <= sliceCount; ++j)
		{
			float theta = thetaStep * j;

			Vertex vertex;
			vertex.Position.x = radius * sinf(phi) * cosf(theta);
			vertex.Position.y = radius * cosf(phi);
			vertex.Position.z = radius * sinf(phi) * sinf(theta);

			vertex.TangentU.x = -radius * sinf(phi) * sinf(theta);
			vertex.TangentU.y = 0.0f;
			vertex.TangentU.z = +radius * sinf(phi) * cosf(theta);

			DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&vertex.TangentU);
			DirectX::XMStoreFloat3(&vertex.TangentU, DirectX::XMVector3Normalize(T));

			DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&vertex.Position);
			DirectX::XMStoreFloat3(&vertex.Normal, DirectX::XMVector3Normalize(p));

			vertex.TexC.x = theta / DirectX::XM_2PI;
			vertex.TexC.y = phi / DirectX::XM_PI;

			meshData.Vertices.push_back(vertex);
		}
	}

	meshData.Vertices.push_back(bottomVertex);
	
	// top stack
	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back(i);
	}

	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			int curIndex = baseIndex + ringVertexCount * i + j;
			meshData.Indices32.push_back(curIndex);
			meshData.Indices32.push_back(curIndex + 1);
			meshData.Indices32.push_back(curIndex + ringVertexCount);

			meshData.Indices32.push_back(curIndex + ringVertexCount);
			meshData.Indices32.push_back(curIndex + 1);
			meshData.Indices32.push_back(curIndex + ringVertexCount + 1);
		}
	}

	// bottom stack
	uint32_t southPoleIndex = (uint32_t)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;
	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}

	return meshData;
}

MeshData<Vertex> GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount)
{
	MeshData<Vertex> meshData;

	float dh = height / stackCount;
	float dr = (topRadius - bottomRadius) / stackCount;

	for (int i = 0; i <= stackCount; ++i)
	{
		float y = (-height * 0.5f) + dh * i;
		float r = bottomRadius + dr * i;

		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
		for (int j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;

			float nx = cos(j * dTheta);
			float nz = sin(j * dTheta);
			float u = (float)j / sliceCount;
			float v = 1.0f - (float)i / stackCount;

			vertex.Position = DirectX::XMFLOAT3(r * nx, y, r * nz);

			vertex.TexC.x = u;
			vertex.TexC.y = v;

			vertex.TangentU = DirectX::XMFLOAT3(-nx, 0.0f, nz);

			float subR = bottomRadius - topRadius;
			DirectX::XMFLOAT3 bitangent(subR * nx, -height, subR * nz);	// 실린더의 경사면의 벡터

			DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&vertex.TangentU);
			DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&bitangent);
			DirectX::XMVECTOR N = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(T, B));
			DirectX::XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);
		}
	}

	int ringVertexCount = sliceCount + 1;

	for (int i = 0; i < stackCount; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			int curIndex = ringVertexCount * i + j;

			meshData.Indices32.push_back(curIndex);
			meshData.Indices32.push_back(curIndex + ringVertexCount);
			meshData.Indices32.push_back(curIndex + ringVertexCount + 1);

			meshData.Indices32.push_back(curIndex);
			meshData.Indices32.push_back(curIndex + ringVertexCount + 1);
			meshData.Indices32.push_back(curIndex + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

	return meshData;
}

MeshData<Vertex> GeometryGenerator::CreateGrid(float width, float depth, uint32_t m, uint32_t n)
{
	MeshData<Vertex> meshData;

	uint32_t vertexCount = m * n;

	float halfWidth = width * 0.5f;
	float halfDepth = depth * 0.5f;

	float deltaWidth = width / (n - 1);
	float deltaDepth = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	for (int i = 0; i < m; ++i)
	{
		float z = halfDepth - deltaDepth * i;

		for (int j = 0; j < n; ++j)
		{
			float x = -halfWidth + deltaWidth * j;

			Vertex vertex;
			vertex.Position = DirectX::XMFLOAT3(x, 0.0f, z);
			vertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
			vertex.TexC.x = j * du;	// u
			vertex.TexC.y = i * dv;	// v

			meshData.Vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			int curIndex = i * n + j;

			meshData.Indices32.push_back(curIndex);
			meshData.Indices32.push_back(curIndex + 1);
			meshData.Indices32.push_back(curIndex + n);

			meshData.Indices32.push_back(curIndex + n);
			meshData.Indices32.push_back(curIndex + 1);
			meshData.Indices32.push_back(curIndex + n + 1);
		}
	}

	return meshData;
}

MeshData<Vertex> GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
	MeshData<Vertex> meshData;

	// resize를 통해서 공간 확보 및 index 접근을 위해서 호출
	meshData.Vertices.resize(4);
	meshData.Indices32.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = Vertex(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = Vertex(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = Vertex(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = Vertex(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices32[0] = 0;
	meshData.Indices32[1] = 1;
	meshData.Indices32[2] = 2;

	meshData.Indices32[3] = 0;
	meshData.Indices32[4] = 2;
	meshData.Indices32[5] = 3;

	return meshData;
}

void GeometryGenerator::Subdivide(MeshData<Vertex>& meshData)
{
}

Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1)
{
	Vertex vertex;

	return vertex;
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData<Vertex>& meshData)
{
	uint32_t baseIndex = (uint32_t)meshData.Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;

	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// TODO: 왜 이런 uv 계산이 나오는지?
		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	uint32_t centerIndex = (uint32_t)meshData.Vertices.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i + 1);
		meshData.Indices32.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData<Vertex>& meshData)
{
	uint32_t baseIndex = (uint32_t)meshData.Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
	for (uint32_t i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32_t centerIndex = (uint32_t)meshData.Vertices.size() - 1;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}
}
