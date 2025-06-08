#pragma once

namespace DK
{
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
}