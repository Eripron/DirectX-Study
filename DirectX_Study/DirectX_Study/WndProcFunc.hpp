//#pragma once
//
//#include <Windows.h>
//#include <math.h>
//
//#include "DrawFunc.hpp"
//#include "DrawOffset.h"
//
//class WndProcFunc
//{
//private:
//	HINSTANCE m_hInst;
//	HBITMAP m_hDisplayBitmap;
//
//	DrawOffset m_DrawOffset;
//
//public:
//	WndProcFunc(HINSTANCE hInst) 
//		: m_hInst(hInst), m_DrawOffset(Vector2(0, 0))
//	{
//	}
//
//	~WndProcFunc()
//	{
//	}
//
//public:
//
//	UINT OnCreate(HWND hWnd)
//	{
//		// 출력용 비트맵 생성
//		HDC hdc = GetDC(hWnd);
//
//		RECT rt;
//		GetClientRect(hWnd, &rt);
//		m_hDisplayBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
//
//		ReleaseDC(hWnd, hdc);
//
//		return 0;
//	}
//
//	UINT OnDestroy(HWND hWnd)
//	{
//		PostQuitMessage(0);
//		return 0;
//	}
//
//	void OnPaint(HWND hWnd, HDC hdc)
//	{
//		RECT rt;
//		GetClientRect(hWnd, &rt);
//
//		HDC hMemDC = CreateCompatibleDC(hdc);
//		HBITMAP hOldBit = (HBITMAP)SelectObject(hMemDC, m_hDisplayBitmap);
//		FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_WINDOW));
//
//		// draw line
//		Vector3 vPoint1(100, 50, 0);
//		Vector3 vPoint2(600, 300, 0);
//
//		Vector2 offset = m_DrawOffset.GetOffset();
//
//		vPoint1.x = (offset.x + vPoint1.x);
//		vPoint1.y = (offset.y + vPoint1.y);
//
//		vPoint2.x = (offset.x + vPoint2.x);
//		vPoint2.y = (offset.y + vPoint2.y);
//
//		DrawLine(hMemDC, vPoint1, vPoint2);
//
//		SelectObject(hMemDC, hOldBit);
//
//		DrawBitmap(hdc, 0, 0, m_hDisplayBitmap);
//	}
//
//	void OnRButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
//	{
//		m_DrawOffset.StartMove(Vector2(LOWORD(lParam), HIWORD(lParam)));
//	}
//
//	void OnRButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
//	{
//		m_DrawOffset.EndMove();
//
//		InvalidateRect(hWnd, NULL, FALSE);
//	}
//
//	void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
//	{
//		m_DrawOffset.Moving(Vector2(LOWORD(lParam), HIWORD(lParam)));
//
//		InvalidateRect(hWnd, NULL, FALSE);
//	}
//
//	void OnCammand(HWND hWnd, WPARAM wParam, LPARAM lParam)
//	{
//
//	}
//
//
//};
