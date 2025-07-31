#include "GameObject.h"

using namespace DK;

GameObject::~GameObject()
{
}

Transform& GameObject::GetTransform()
{
	return m_transform;
}

MeshBuffer* GameObject::GetMeshBuffer()
{
	return m_pMeshBuffer;
}

MeshSection GameObject::GetMeshSection()
{
	return m_meshSection;
}

Material* GameObject::GetMaterial()
{
	return m_pMaterial;
}

void GameObject::SetMeshData(MeshBuffer* pMeshBuffer, MeshSection meshSection)
{
	m_pMeshBuffer = pMeshBuffer;
	m_meshSection = meshSection;
}

void GameObject::SetMaterial(Material* pMat)
{
	m_pMaterial = pMat;
}
