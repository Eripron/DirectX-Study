#pragma once

#include <Windows.h>

class Dialog
{
public:
	Dialog();
	virtual ~Dialog();

	void CreateModle(HINSTANCE hInstance, HWND hWnd, int nId);
	bool CreateModleless(HINSTANCE hInstance, HWND hWnd, int nId);

protected:
	HWND m_hParent;
	HWND m_hDialog;
	virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;

private:
	static INT_PTR CALLBACK StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};
