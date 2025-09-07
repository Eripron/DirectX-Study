#pragma once

#include <string>

namespace DK
{
	enum ComponentType
	{
		CT_None = 0,

		CT_Transform,
		CT_MeshFilter,
	};

	class Component
	{
	public:
		Component(ComponentType eType);
		virtual ~Component();

		ComponentType GetType();

	private:
		ComponentType m_eComponentType;
	};

}