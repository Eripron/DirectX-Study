#include <Windows.h>

#include "Window/MainWindow.h"
#include "Ex/ExShape.h"

using namespace DK;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpszCmdParam, int nCmdShow)
{
	LPCTSTR strClassName = TEXT("Game Engine");
	MainWindow mainWindow;
	if (mainWindow.Create(hInstance, strClassName, strClassName, nCmdShow) == false)
	{
		return -1;
	}

	HWND hWnd = mainWindow.GetHandle();

	ExShape engine(hWnd);
	engine.Init();

	while (mainWindow.Run())
	{
		engine.Run();
	}

	mainWindow.Destroy();
	return 0;
}
