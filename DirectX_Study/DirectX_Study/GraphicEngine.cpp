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

	BITMAP bmp;
	void GraphicEngine::Start()
	{
		LPCTSTR path = L"Image/mario.bmp";
		if (LoadBitmapData(path, bmp) == false)
			return;
	}

	void GraphicEngine::Run()
	{
		_render.PreUpdate();

		Render(&_render);

		_render.LastUpdate();
	}

	Triangle triangle = GraphicUtils::CreateTriangle(Vector3(100, 100, 0), 40);

	void GraphicEngine::Render(Renderer* pRender)
	{
		pRender->DrawAxisX(0);
		pRender->DrawAxisY(0);

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
			transform *= Matrix44::MoveMatrix44(0, yInput, 0);

		/*int x = 0, y = 0;
		for (int i = 0; i < bmp.bmHeight; ++i)
		{
			BYTE* ptr = (BYTE*)bmp.bmBits + bmp.bmWidthBytes * (bmp.bmHeight - 1 - i);
			int bitPixel = bmp.bmBitsPixel / 8;

			for (int j = 0; j < bmp.bmWidth; ++j)
			{
				BYTE b = ptr[j * bitPixel];
				BYTE g = ptr[j * bitPixel + 1];
				BYTE r = ptr[j * bitPixel + 2];
				BYTE a = ptr[j * bitPixel + 3];
				if (a == 0)
				{
					r = 255;
					g = 255;
					b = 255;
				}
				BYTE gray = static_cast<BYTE>((299 * r + 587 * g + 114 * b) / 1000);

				COLORREF color = RGB(r, g, b);
				COLORREF grayColor = RGB(gray, gray, gray);

				pRender->DrawPixel(Vector3(x + j, y + i, 0), color);
				pRender->DrawPixel(Vector3(x + j + bmp.bmWidth, y + i, 0), grayColor);
			}
		}*/
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

	bool GraphicEngine::LoadBitmapData(LPCTSTR path, BITMAP& bitmap)
	{
		// open file
		HANDLE hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		// read bmp file header
		DWORD byteRead;
		BITMAPFILEHEADER bmpFileHeader;
		if (ReadFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &byteRead, NULL) == 0 ||
			bmpFileHeader.bfType != 0x4d42)
		{
			CloseHandle(hFile);
			return false;
		}

		// read bmp info header
		BITMAPINFOHEADER bmpInfoHeader;
		if (ReadFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &byteRead, NULL) == 0)
		{
			CloseHandle(hFile);
			return false;
		}

		SetFilePointer(hFile, bmpFileHeader.bfOffBits, NULL, FILE_BEGIN);

		DWORD pixelDataSize = bmpFileHeader.bfSize - bmpFileHeader.bfOffBits;
		BYTE* data = (BYTE*)malloc(pixelDataSize);
		if (ReadFile(hFile, data, pixelDataSize, &byteRead, NULL) == 0)
		{
			free(data);
			CloseHandle(hFile);
			return false;
		}

		bitmap.bmType = 0;
		bitmap.bmWidth = bmpInfoHeader.biWidth;
		bitmap.bmHeight = bmpInfoHeader.biHeight;
		bitmap.bmWidthBytes = (((bmpInfoHeader.biWidth * bmpInfoHeader.biBitCount) + 31) / 32) * 4;
		bitmap.bmPlanes = bmpInfoHeader.biPlanes;
		bitmap.bmBitsPixel = bmpInfoHeader.biBitCount;
		bitmap.bmBits = data;

		CloseHandle(hFile);

		return true;
	}

}