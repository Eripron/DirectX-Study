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

	private:
		Renderer _render;

	};
}