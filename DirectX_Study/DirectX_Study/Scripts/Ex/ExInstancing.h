#pragma once

#include "../Engine/EngineBase.h"

namespace DK
{
	class ExInstancing : public EngineBase
	{
	public:
		ExInstancing(HWND hWnd);
		~ExInstancing();

	protected:
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual void LoadTextures() override;
		virtual void CreateMesh() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;

	private:
		int m_instanceCount = 0;
	};
}