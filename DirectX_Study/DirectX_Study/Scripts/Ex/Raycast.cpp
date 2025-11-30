#include "Raycast.h"

DK::ExRaycast::ExRaycast(HWND hWnd) : EngineBase(hWnd)
{
}

DK::ExRaycast::~ExRaycast()
{
}

bool DK::ExRaycast::OnResize(int width, int height, bool force)
{
	if (EngineBase::OnResize(width, height, force) == false)
		return false;

	return true;
}

void DK::ExRaycast::LoadTextures()
{
}

void DK::ExRaycast::CreateMesh()
{
	LoadTexture(L"Textures/bricks.dds");
	LoadTexture(L"Textures/checkboard.dds");
	LoadTexture(L"Textures/ice.dds");
	LoadTexture(L"Textures/white1x1.dds");
	LoadTexture(L"Textures/water1.dds");
	LoadTexture(L"Textures/stone.dds");
	LoadTexture(L"Textures/grass.dds");
}

void DK::ExRaycast::CreateMaterial()
{
}

void DK::ExRaycast::CreateGameObject()
{
}

void DK::ExRaycast::CreateRenderObjectInfo()
{
}
