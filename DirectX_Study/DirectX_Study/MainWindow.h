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
        void Render();
        void Destroy();

        HWND GetHandle();

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

        void Create(HWND hWnd);

    private:
        HINSTANCE _hInst;
        HWND _hWnd;

        HWND _sValueBar;
        HWND _vValueBar;

    public:
        std::function<void(float, float)> BarValueChanged;
    };
}