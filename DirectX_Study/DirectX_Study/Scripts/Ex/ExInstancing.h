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
		virtual bool Update() override;
		virtual void Render(ID3D12GraphicsCommandList* cmdList) override;

		virtual bool OnResize(int width, int height, bool force) override;

		virtual void LoadTextures() override;
		virtual void CreateMesh() override;
		virtual void CreateMaterial() override;
		virtual void CreateGameObject() override;
		virtual void CreateRenderObjectInfo() override;

		DirectX::XMFLOAT3 GetPositionByIndex(int x, int y, int z, float width, float depth, float height, int index);

	private:
		Camera m_cameraCullingTest;
		DirectX::BoundingFrustum m_frustomCulTest;

	};
}