#include "MainWindow.hpp"

MainWindow::MainWindow() 
	: m_hInst(NULL), m_hWnd(NULL), m_hBackBuffer(NULL), m_LineContainer(), m_DrawOffset(Numeric::Vector2())
{
}

MainWindow::~MainWindow()
{
}

bool MainWindow::Create(HINSTANCE hInstance, LPCTSTR strClassName, LPCTSTR strWindowTitle, int nCmdShow)
{
	m_hInst = hInstance;

	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(RGB(0, 0, 0));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = strClassName;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	HWND hWnd = CreateWindow(strClassName, strWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, this);
	if (hWnd == NULL)
		return false;

	ShowWindow(hWnd, nCmdShow);

	return true;
}

int MainWindow::Run()
{
	MSG Message;
	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

LRESULT MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MainWindow* pThis;

	if (message == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		pThis = (MainWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	else 
	{
		pThis = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	if (pThis) 
	{
		return pThis->HandleMessage(hWnd, message, wParam, lParam);
	}
	else 
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT MainWindow::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_CREATE:
		return Create(hWnd);

	case WM_DESTROY:
		Destroy();
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		Paint(hWnd, hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_RBUTTONDOWN:
		RButtonDown(hWnd, wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_RBUTTONUP:
		RButtonUp(hWnd, wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
	
	case WM_MOUSEMOVE:
		MouseMove(hWnd, wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_COMMAND:
		Command(hWnd, wParam, lParam);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

UINT MainWindow::Create(HWND hWnd)
{
	m_hWnd = hWnd;

	// 출력용 비트맵 생성
	HDC hdc = GetDC(hWnd);

	RECT rt;
	GetClientRect(hWnd, &rt);
	m_hBackBuffer = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);

	ReleaseDC(hWnd, hdc);

	Draw();

	return 0;
}

void MainWindow::Destroy()
{
	PostQuitMessage(0);
}

void MainWindow::Paint(HWND hWnd, HDC hdc)
{
	Graphic::DrawBitmap(hdc, 0, 0, m_hBackBuffer);
}

void MainWindow::Draw()
{
	RECT rt;
	GetClientRect(m_hWnd, &rt);

	HDC hdc = GetDC(m_hWnd);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldBit = (HBITMAP)SelectObject(hMemDC, m_hBackBuffer);
	FillRect(hMemDC, &rt, GetSysColorBrush(COLOR_WINDOW));

	// draw logic
	{
		DrawLine(hMemDC);
	}

	SelectObject(hMemDC, hOldBit);
	ReleaseDC(m_hWnd, hdc);

	InvalidateRect(m_hWnd, NULL, FALSE);
}

void MainWindow::DrawLine(HDC hdc)
{
	for (int i = 0; i < m_LineContainer.GetCount(); ++i)
	{
		Line line = m_LineContainer.GetLine(i);

		Numeric::Vector3 p1 = line.GetPoint1();
		p1.x += m_DrawOffset.GetOffset().x;
		p1.y += m_DrawOffset.GetOffset().y;

		Numeric::Vector3 p2 = line.GetPoint2();
		p2.x += m_DrawOffset.GetOffset().x;
		p2.y += m_DrawOffset.GetOffset().y;

		Graphic::DrawLine(hdc, p1, p2);
	}
}

void MainWindow::RButtonDown(HWND hWnd, WPARAM wParam, int cursorX, int cursorY)
{
	m_DrawOffset.StartMove(Numeric::Vector2(cursorX, cursorY));
}

void MainWindow::RButtonUp(HWND hWnd, WPARAM wParam, int cursorX, int cursorY)
{
	m_DrawOffset.EndMove();

	Draw();
}

void MainWindow::MouseMove(HWND hWnd, WPARAM wParam, int cursorX, int cursorY)
{
	m_DrawOffset.Moving(Numeric::Vector2(cursorX, cursorY));

	Draw();
}

void MainWindow::Command(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case ID_40001:
	{
		MainWindow* mainWindow = this;
		DialogLineAdd dialogLineAdd(&m_LineContainer);
		dialogLineAdd.SetEventAddLine(std::bind(&MainWindow::Draw, this));
		dialogLineAdd.CreateModle(m_hInst, hWnd, IDD_DIALOG1);
		break;
	}

	}
}
