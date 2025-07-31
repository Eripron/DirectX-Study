#include "Dialog.h"

Dialog::Dialog() : m_hParent(NULL), m_hDialog(NULL)
{
}

Dialog::~Dialog()
{
}

void Dialog::CreateModle(HINSTANCE hInstance, HWND hWnd, int nId)
{
    m_hParent = hWnd;
	DialogBoxParam(hInstance, MAKEINTRESOURCE(nId), hWnd, StaticDialogProc, (LPARAM)this);
}

bool Dialog::CreateModleless(HINSTANCE hInstance, HWND hWnd, int nId)
{
	if (IsWindow(m_hDialog))
		return FALSE;
    
    m_hParent = hWnd;
	m_hDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(nId), hWnd, StaticDialogProc, (LPARAM)this);

	return m_hDialog == NULL ? FALSE : TRUE;
}

INT_PTR Dialog::StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    Dialog* pThis = nullptr;

    if (message == WM_INITDIALOG) 
    {
        pThis = reinterpret_cast<Dialog*>(lParam);
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else 
    {
        pThis = reinterpret_cast<Dialog*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
    }

    return pThis ? pThis->DialogProc(hDlg, message, wParam, lParam) : FALSE;
}
