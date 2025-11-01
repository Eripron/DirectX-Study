#pragma once

#include <string>
#include <vector>

#include "Component.h"
#include "Transform.h"
#include "../Data/DataTypes.h"

namespace DK
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();
		
		void AddComponent(Component* pComponent);

		template <class T>
		T* GetComponent();

		Material* GetMaterial();
		void SetMaterial(Material* pMat);

	private:
		std::vector<Component*> m_listComponent;

		Material* m_pMaterial = nullptr;
	};

	template<class T>
	inline T* GameObject::GetComponent()
	{
		for (int i = 0; i < m_listComponent.size(); ++i)
		{
			if (typeid(*m_listComponent[i]) == typeid(T))
				return static_cast<T*>(m_listComponent[i]);
		}

		return nullptr;
	}

}