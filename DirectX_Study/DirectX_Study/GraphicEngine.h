#pragma once

#include <Windows.h>
#include "Renderer.h"

namespace DK
{
	class GraphicEngine
	{
	public:
		GraphicEngine(HWND hWnd);
		~GraphicEngine();

		void Run();

	private:
		void Render(Renderer* pRender);

		float GetXAxisInput();
		float GetYAxisInput();
		float GetRotateInput();
		float GetScaleInput();


	private:
		Renderer _render;

	};
}