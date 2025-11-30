#pragma once

#include "../Engine/EngineBase.h"

namespace DK
{
	class ExRaycast : public EngineBase
	{
	public:
		ExRaycast(HWND hWnd);
		~ExRaycast();

	protected:

		virtual bool OnResize(int width, int height, bool force) override;

		virtual void LoadTextures() override;
		virtual void CreateMesh() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;
		virtual void CreateRenderObjectInfo() override;

	private:

	};
}
