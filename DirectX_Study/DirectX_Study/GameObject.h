#pragma once

#include <string>
#include "Transform.h"
#include "GeometryGenerator.h"
#include "D3DUtils.h"

namespace DK
{
	class GameObject
	{
	public:
		GameObject() = default;
		~GameObject();

		std::string mName;
		Transform mTransform;
		MeshDataDesc* mMeshData;

	};

}