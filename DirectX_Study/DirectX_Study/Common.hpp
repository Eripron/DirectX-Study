#pragma once

#include <Windows.h>

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC = CreateCompatibleDC(hdc);
	HBITMAP OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	BITMAP bit;
	GetObject(hBit, sizeof(BITMAP), &bit);

	BitBlt(hdc, x, y, bit.bmWidth, bit.bmHeight, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}
