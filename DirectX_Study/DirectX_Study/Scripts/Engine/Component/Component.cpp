#include "Component.h"

DK::Component::Component(ComponentType eType) : m_eComponentType(eType)
{
}
DK::Component::~Component()	{}

DK::ComponentType DK::Component::GetType()
{
	return m_eComponentType;
}
