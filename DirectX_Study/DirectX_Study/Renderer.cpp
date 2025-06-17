#include "Renderer.h"

#include "GraphicUtils.h"
#include "WindowsUtils.h"

namespace DK
{
	Renderer::~Renderer()
	{
	}

	void Renderer::Init(HWND hWnd)
	{
		if (hWnd == INVALID_HANDLE_VALUE)
			return;

		_hWnd = hWnd;

		WindowsUtils::GetScreenSize(_hWnd, &_vScreenSize.x, &_vScreenSize.y);
		_hdc = GetDC(_hWnd);
		_memDC = CreateCompatibleDC(_hdc);
		_hBitmap = CreateCompatibleBitmap(_hdc, (int)_vScreenSize.x, (int)_vScreenSize.y);
		_hOldBitmap = (HBITMAP)SelectObject(_memDC, _hBitmap);
	}

	void Renderer::PreUpdate()
	{
		Clear();
	}

	void Renderer::LastUpdate()
	{
		BitBlt(_hdc, 0, 0, _vScreenSize.x, _vScreenSize.y, _memDC, 0, 0, SRCCOPY);
	}

	void Renderer::DrawPixel(Vector3 point, COLORREF color)
	{
		point = ToScreenCoordinate(point);
		Vector2 screenPoint = ToScreenPoint(point.x, point.y);

		SetPixel(_memDC, screenPoint.x, screenPoint.y, color);
	}

	void Renderer::DrawLine(Vector3 p1, Vector3 p2, COLORREF color)
	{
		p1 = ToScreenCoordinate(p1);
		p2 = ToScreenCoordinate(p2);

		Vector2 sp1 = ToScreenPoint(p1.x, p1.y);
		Vector2 sp2 = ToScreenPoint(p2.x, p2.y);

		DrawLine(_memDC, sp1, sp2, color);
	}

	void Renderer::DrawAxisX(int y)
	{
		Vector2 coordinate = ToScreenCoordinate(Vector2(0, y));
		Vector2 screen = ToScreenPoint(coordinate.x, coordinate.y);

		DrawLine(_memDC, Vector3(0, screen.y, 0), Vector3(_vScreenSize.x, screen.y, 0), RGB(255, 0, 0));
	}

	void Renderer::DrawAxisY(int x)
	{
		Vector2 coordinate = ToScreenCoordinate(Vector2(x, 0));
		Vector2 screen = ToScreenPoint(coordinate.x, coordinate.y);

		DrawLine(_memDC, Vector3(screen.x, 0, 0), Vector3(screen.x, _vScreenSize.y, 0), RGB(255, 0, 0));
	}

	void Renderer::Clear()
	{
		RECT rtScreen;
		rtScreen.right = _vScreenSize.x;
		rtScreen.bottom = _vScreenSize.y;
		FillRect(_memDC, &rtScreen, GetSysColorBrush(COLOR_WINDOW));
	}

	Vector2 Renderer::ToScreenPoint(float x, float y)
	{
		return Vector2(floorf(x), floorf(y));
	}

	Vector2 Renderer::ToScreenCoordinate(const Vector2& vPos)
	{
		return Vector2(vPos.x + _vScreenSize.x * 0.5f, -vPos.y + _vScreenSize.y * 0.5f);
	}

	Vector3 Renderer::ToScreenCoordinate(const Vector3& vPos)
	{
		return Vector3(vPos.x + _vScreenSize.x * 0.5f, -vPos.y + _vScreenSize.y * 0.5f, vPos.z);
	}

	void Renderer::DrawLine(HDC hdc, Vector2 p1, Vector2 p2, COLORREF color)
	{
		float dx = p2.x - p1.x;
		float dy = p2.y - p1.y;

		if (dx == 0.0f)
		{
			int sy = dy > 0 ? 1 : -1;
			int step = fabs(dy);
			for (int i = 0; i <= step; ++i)
				SetPixel(hdc, p1.x, p1.y + i * sy, color);
		}
		else
		{
			float a = dy / dx;				// 기울기
			float b = p1.y - a * p1.x;	// y절편

			bool bUseAxisX = fabs(a) <= 1;

			float step = bUseAxisX ? fabs(dx) : fabs(dy);
			float start = bUseAxisX ? min(p1.x, p2.x) : min(p1.y, p2.y);

			for (int i = 0; i <= step; ++i)
			{
				float x = bUseAxisX ? start + i : floorf(MathUtils::GetLinearX(a, b, start + i));
				float y = bUseAxisX ? floorf(MathUtils::GetLinearY(a, b, start + i)) : start + i;

				SetPixel(hdc, x, y, color);
			}
		}
	}

	void Renderer::DrawLine(HDC hdc, Vector3 p1, Vector3 p2, COLORREF color)
	{
		DrawLine(hdc, Vector2(p1.x, p1.y), Vector2(p2.x, p2.y), color);
	}


}