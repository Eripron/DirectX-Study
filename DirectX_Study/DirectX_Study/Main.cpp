#include <Windows.h>

#include "MainWindow.hpp"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpszCmdParam, int nCmdShow)
{
	LPCTSTR strClassName = TEXT("Graphic");

	MainWindow mainWindow;
	if (mainWindow.Create(hInstance, strClassName, strClassName, nCmdShow) == false)
		return 0;

	return mainWindow.Run();
}
