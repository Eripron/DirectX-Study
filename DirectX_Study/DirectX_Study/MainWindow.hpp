#pragma once

#include <Windows.h>
#include "resource.h"

#include "DrawOffset.hpp"
#include "DrawFunc.hpp"

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    bool Create(HINSTANCE, LPCTSTR, LPCTSTR, int);
    int Run();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

    HINSTANCE m_hInst;
    HBITMAP m_hBackBuffer;

    // member
    DrawOffset m_DrawOffset;

    UINT Create(HWND);
    void Destroy();
    void Paint(HWND, HDC);

    void RButtonDown(HWND, WPARAM, int, int);
    void RButtonUp(HWND, WPARAM, int, int);

    void MouseMove(HWND, WPARAM, int, int);
};