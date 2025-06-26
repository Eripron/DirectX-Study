#include <Windows.h>

#include "MainWindow.h"
#include "GraphicEngine.h"

using namespace DK;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpszCmdParam, int nCmdShow)
{
	LPCTSTR strClassName = TEXT("Graphic");
	MainWindow mainWindow;
	if (mainWindow.Create(hInstance, strClassName, strClassName, nCmdShow) == false)
	{
		return -1;
	}

	GraphicEngine engine(mainWindow.GetHandle());

	mainWindow.BarValueChanged = [&engine](float s, float v) {
			engine.BarValueChanged(s, v);
		};


	engine.Init();
	while (mainWindow.Run())
	{
		engine.Run();
	}

	mainWindow.Destroy();
	return 0;
}
