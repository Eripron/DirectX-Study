#pragma once

namespace DK
{
    class MainWindow
    {
    public:
        MainWindow() = default;
        ~MainWindow();

        bool Create(HINSTANCE hInstance, LPCTSTR strClassName, LPCTSTR strWindowTitle, int nCmdShow);
        bool Run();
        void Destroy();

        HWND GetHandle();

    private:
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

    private:
        HINSTANCE _hInst;
        HWND _hWnd;
    };
}