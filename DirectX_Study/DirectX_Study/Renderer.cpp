#include "Renderer.h"

#include <math.h>
#include "GraphicUtils.h"
#include "WindowsUtils.h"
#include "HSVColor.h"
#include "RGBColor.h"

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

	void Renderer::DrawSquare(Square square, BITMAP* bmp, float S, float V)
	{
		std::vector<Triangle> triangles = square.GetTriangles();
		int count = triangles.size();

		for (int i = 0; i < count; ++i)
		{
			DrawTriangle(triangles[i], bmp, S, V);
		}
	}

	void Renderer::DrawTriangle(Triangle triangle, BITMAP* bmp, float S, float V)
	{
		Dot dot0 = triangle.GetDot(0);
		Dot dot1 = triangle.GetDot(1);
		Dot dot2 = triangle.GetDot(2);

		if (bmp != nullptr)
		{
			Vector2 minPos;
			minPos.x = fminf(fminf(dot0.GetPos().x, dot1.GetPos().x), dot2.GetPos().x);
			minPos.y = fminf(fminf(dot0.GetPos().y, dot1.GetPos().y), dot2.GetPos().y);

			Vector2 maxPos;
			maxPos.x = fmaxf(fmaxf(dot0.GetPos().x, dot1.GetPos().x), dot2.GetPos().x);
			maxPos.y = fmaxf(fmaxf(dot0.GetPos().y, dot1.GetPos().y), dot2.GetPos().y);

			Vector2 u = Vector2((dot1.GetPos() - dot0.GetPos()).x, (dot1.GetPos() - dot0.GetPos()).y);
			Vector2 v = Vector2((dot2.GetPos() - dot0.GetPos()).x, (dot2.GetPos() - dot0.GetPos()).y);

			float vv = v.Dot(v);
			float uu = u.Dot(u);
			float uv = u.Dot(v);
			float denominator = uv * uv - uu * vv;

			// 직선인 경우
			if (denominator == 0)
				return;

			Vector2 minSP = ToScreenPoint(minPos.x, minPos.y);
			Vector2 maxSP = ToScreenPoint(maxPos.x, maxPos.y);
			Vector2 sp1 = ToScreenPoint(dot0.GetPos().x, dot0.GetPos().y);

			for (int x = minSP.x; x <= maxSP.x; ++x)
			{
				for (int y = minSP.y; y <= maxSP.y; ++y)
				{
					Vector2 w = Vector2(x, y) - sp1;
					float wu = w.Dot(u);
					float wv = w.Dot(v);

					float s = (wv * uv - wu * vv) / denominator;
					float t = (wu * uv - wv * uu) / denominator;
					float oneMinusST = 1.f - s - t;

					if (((s >= 0.f) && (s <= 1.f)) && ((t >= 0.f) && (t <= 1.f)) && ((oneMinusST >= 0.f) && (oneMinusST <= 1.f)))
					{
						Vector2 UV = dot0.GetUV() * oneMinusST + dot1.GetUV() * s + dot2.GetUV() * t;

						RGBColor rgbColor = GetColorLerp(*bmp, UV);
						RGBColor grayColor = GetColor(*bmp, UV);

						BYTE gray = static_cast<BYTE>((299 * grayColor.R + 587 * grayColor.G + 114 * grayColor.B) / 1000);

						if (rgbColor.A == 0)
						{
							rgbColor.R = 255;
							rgbColor.G = 255;
							rgbColor.B = 255;
							gray = 255;
						}

						HSVColor hsvColor = rgbColor.ConvertToHSV();

						hsvColor.S *= S;
						hsvColor.V *= V;
						rgbColor = hsvColor.ConvertToRGB();

						DrawPixel(Vector3(x - 300, y, 0), RGB(rgbColor.R, rgbColor.G, rgbColor.B));
						DrawPixel(Vector3(x + 300, y, 0), RGB(gray, gray, gray));
					}
				}
			}
		}
		else
		{
			DrawLine(dot0.GetPos(), dot1.GetPos());
			DrawLine(dot1.GetPos(), dot2.GetPos());
			DrawLine(dot2.GetPos(), dot0.GetPos());
		}
	}

	void Renderer::DrawTextIn(LPCTSTR str, int x, int y)
	{
		TextOut(_memDC, x, y, str, lstrlen(str));
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

	RGBColor Renderer::GetColor(BITMAP& bitmap, Vector2 uv)
	{
		int bytePixel = bitmap.bmBitsPixel / 8;
		int width = bitmap.bmWidthBytes / bytePixel;

		int x = static_cast<int>(floorf(uv.x * (width - 1)));
		int y = static_cast<int>(floorf(uv.y * (bitmap.bmHeight - 1)));

		return GetColor(bitmap, x, y);
	}

	RGBColor Renderer::GetColorLerp(BITMAP& bitmap, Vector2 uv)
	{
		int bytePixel = bitmap.bmBitsPixel / 8;
		int width = bitmap.bmWidthBytes / bytePixel;

		float fx = uv.x * (width - 1);
		float fy = uv.y * (bitmap.bmHeight - 1);

		int x0 = static_cast<int>(floorf(uv.x * (width - 1)));
		int y0 = static_cast<int>(floorf(uv.y * (bitmap.bmHeight - 1)));

		int x1 = min(x0 + 1, width - 1);
		int y1 = min(y0 + 1, bitmap.bmHeight - 1);

		RGBColor c00 = GetColor(bitmap, x0, y0);
		RGBColor c10 = GetColor(bitmap, x1, y0);
		RGBColor c01 = GetColor(bitmap, x0, y1);
		RGBColor c11 = GetColor(bitmap, x1, y1);

		RGBColor cx0 = RGBColor::LerpColor(c00, c10, fx - x0);
		RGBColor cx1 = RGBColor::LerpColor(c01, c11, fx - x0);
		RGBColor c = RGBColor::LerpColor(cx0, cx1, fy - y0);

		return c;
	}

	RGBColor Renderer::GetColor(BITMAP& bitmap, int x, int y)
	{
		int bytePixel = bitmap.bmBitsPixel / 8;
		int index = bitmap.bmWidthBytes * y + bytePixel * x;

		BYTE* bits = (BYTE*)bitmap.bmBits;

		BYTE b = bits[index];
		BYTE g = bits[index + 1];
		BYTE r = bits[index + 2];
		BYTE a = 255;
		if (bytePixel >= 4)
			a = bits[index + 3];

		return RGBColor(r, g, b, a);
	}

}
