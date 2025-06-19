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

		void Start();
		void Run();

		void ScreenChanged(Vector2 screenSize);
		void BarValueChanged(float s, float v);

	private:
		void Render(Renderer* pRender);

		float GetXAxisInput();
		float GetYAxisInput();
		float GetRotateInput();
		float GetScaleInput();

		bool LoadBitmapData(LPCTSTR path, BITMAP& bitmap);

	private:
		Renderer _render;
		Vector2 _vScreenSize;

		float S = 1.f;
		float V = 1.f;

	};
}