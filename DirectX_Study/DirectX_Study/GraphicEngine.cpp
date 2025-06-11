#include "GraphicEngine.h"

#include "GraphicUtils.h"

namespace DK
{
	GraphicEngine::GraphicEngine(HWND hWnd)
	{
		_render.Init(hWnd);
	}

	GraphicEngine::~GraphicEngine()
	{
	}

	void GraphicEngine::Run()
	{
		Triangle triangle = GraphicUtils::CreateTriangle(Vector3(), 200);
		triangle.ApplyTransform(Matrix44::MoveMatrix44(500, 500, 0));

		_render.PreUpdate();

		triangle.Draw(&_render);

		_render.LastUpdate();
	}

}