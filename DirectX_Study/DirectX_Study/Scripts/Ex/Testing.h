#pragma once

#include "../Engine/EngineBase.h"

namespace DK
{
	class Testing : public EngineBase
	{
	public:
		Testing(HWND hWnd);
		~Testing();

	protected:
		virtual bool Update() override;
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual bool OnResize(int width, int height, bool force) override;

		virtual void CreateMesh() override;
		virtual void LoadTextures() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;
		virtual void CreateRenderObjectInfo() override;

		virtual void BuildRootSignature() override;
		virtual void BuildPSO() override;

	protected:
		void RenderCubeMap(ID3D12GraphicsCommandList* cmdList, int i);

	};

}