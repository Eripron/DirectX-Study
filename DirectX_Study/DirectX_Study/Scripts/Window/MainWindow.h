#pragma once

#include <iostream>
#include <functional>

namespace DK
{
    class MainWindow
    {
    public:
        MainWindow() = default;

        bool Create(HINSTANCE hInstance, LPCTSTR strClassName, LPCTSTR strWindowTitle, int nCmdShow);
        bool Run();
        void Destroy();

        HWND GetHandle();

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

    private:
        HINSTANCE m_hInst;
        HWND m_hWnd;

    };
}