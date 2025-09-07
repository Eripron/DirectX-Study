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
		
		Material* GetMaterial();
		void SetMaterial(Material* pMat);

	public:
		int m_nCBIndex = -1;
		int m_nFrameDirty = -1;
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		XMFLOAT4X4 TexTransform = MathUtils::Identity4x4();

	private:
		Material* m_pMaterial = nullptr;


		// -------------------------
	public:
		void AddComponent(Component* pComponent);
		//void RemoveComponent(ComponentType eType);

		template <class T>
		T* GetComponent();

	private:
		std::vector<Component*> m_listComponent;

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