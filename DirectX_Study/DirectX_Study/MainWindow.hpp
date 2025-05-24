#pragma once

#include <Windows.h>
#include <functional>
#include "resource.h"

#include "DrawOffset.hpp"
#include "DrawFunc.hpp"
#include "LineContainer.h"

#include "DialogLineAdd.h"

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    bool Create(HINSTANCE, LPCTSTR, LPCTSTR, int);
    int Run();

    void Draw();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

    HINSTANCE m_hInst;
    HWND m_hWnd;
    HBITMAP m_hBackBuffer;

    // member
    LineContainer m_LineContainer;
    DrawOffset m_DrawOffset;

    void DrawLine(HDC);

    // message function
    UINT Create(HWND);
    void Destroy();
    void Paint(HWND, HDC);

    void RButtonDown(HWND, WPARAM, int, int);
    void RButtonUp(HWND, WPARAM, int, int);

    void MouseMove(HWND, WPARAM, int, int);

    void Command(HWND, WPARAM, LPARAM);
};