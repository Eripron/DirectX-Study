#pragma once

#include "../Engine/EngineBase.h"

namespace DK
{
	class ExTesselation : public EngineBase
	{
	public:
		ExTesselation(HWND hWnd);
		~ExTesselation();

	protected:
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual void LoadTextures() override;
		virtual void CreateMesh() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;

		virtual void BuildInputLayoutAndShader() override;
		virtual void BuildPSO() override;
	};

}