#pragma once

#include "GraphicEngine.h"

namespace DK
{
	class ExLandAndWave : public GraphicEngine
	{
	public:
		ExLandAndWave(HWND hWnd);
		~ExLandAndWave();

		virtual bool Init() override;

	protected:
		virtual void OnResize(int width, int height) override;
		virtual void Update() override;
		virtual void Render() override;

	protected:
		void BuildMeshData();

	};
}