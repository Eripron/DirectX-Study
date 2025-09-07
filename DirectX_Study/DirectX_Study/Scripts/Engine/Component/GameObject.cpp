#include "GameObject.h"

using namespace DK;

DK::GameObject::GameObject()
{
	m_listComponent.push_back(new Transform());
}

GameObject::~GameObject()
{
	for (int i = 0; i < m_listComponent.size(); ++i)
		delete m_listComponent[i];
}

Material* GameObject::GetMaterial()
{
	return m_pMaterial;
}

void GameObject::SetMaterial(Material* pMat)
{
	m_pMaterial = pMat;
}

void DK::GameObject::AddComponent(Component* pComponent)
{
	if (pComponent == nullptr)
		return;

	m_listComponent.push_back(pComponent);
}

//void DK::GameObject::RemoveComponent(ComponentType eType)
//{
//	for (int i = 0; m_listComponent.size(); ++i)
//	{
//		Component* component = m_listComponent[i];
//		if (component->GetType() == eType)
//		{
//			auto index = m_listComponent.begin() + i;
//			m_listComponent.erase(index);
//			break;
//		}
//	}
//}
