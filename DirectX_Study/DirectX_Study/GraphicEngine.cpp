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
		_render.PreUpdate();

		Render(&_render);

		_render.LastUpdate();
	}

	Triangle triangle = GraphicUtils::CreateTriangle(Vector3(300, 300, 0), 100);
	Triangle triangle2 = GraphicUtils::CreateTriangle(Vector3(500, 600, 0), 80);

	void GraphicEngine::Render(Renderer* pRender)
	{
		Matrix44 transform = Matrix44::Identity();

		// scale
		float scale = 1.0f + GetScaleInput();  // +0.01 또는 -0.01 같은 값
		transform *= Matrix44::ScaleMatrix44(scale, scale, scale);

		// rotate
		float rotate = GetRotateInput();
		transform *= Matrix44::RotateZMatrix44(rotate);

		// move
		float xInput = GetXAxisInput();
		if (xInput != 0.0f)
			transform *= Matrix44::MoveMatrix44(xInput, 0, 0);
		float yInput = GetYAxisInput();
		if (yInput != 0.0f)
			transform *= Matrix44::MoveMatrix44(0, -yInput, 0);

		triangle.ApplyTransform(transform);
		triangle.Draw(pRender);

		triangle2.ApplyTransform(transform);
		triangle2.Draw(pRender);
	}

	float GraphicEngine::GetXAxisInput()
	{
		bool bLeft = GetAsyncKeyState(VK_LEFT);
		bool bRight = GetAsyncKeyState(VK_RIGHT);
		if (bLeft ^ bRight)
		{
			return bLeft ? -1.0f : 1.0f;
		}
		return 0.f;
	}

	float GraphicEngine::GetYAxisInput()
	{
		bool bDown = GetAsyncKeyState(VK_DOWN);
		bool bUp = GetAsyncKeyState(VK_UP);
		if (bDown ^ bUp)
		{
			return bDown ? -1.0f : 1.0f;
		}
		return 0.0f;
	}

	float GraphicEngine::GetRotateInput()
	{
		bool bDown = GetAsyncKeyState(VK_END);
		bool bUp = GetAsyncKeyState(VK_HOME);
		if (bDown ^ bUp)
		{
			return bDown ? -1.0f : 1.0f;
		}
		return 0.0f;
	}

	float GraphicEngine::GetScaleInput()
	{
		bool bDown = GetAsyncKeyState(VK_NEXT);
		bool bUp = GetAsyncKeyState(VK_PRIOR);
		if (bDown ^ bUp)
		{
			return bDown ? -0.01f : 0.01f;
		}
		return 0.0f;
	}

}