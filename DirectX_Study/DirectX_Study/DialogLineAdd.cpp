#include "DialogLineAdd.h"

DialogLineAdd::DialogLineAdd(LineContainer* pLineContainer)
    : Dialog(), m_pLineContainer(pLineContainer)
{
}

DialogLineAdd::~DialogLineAdd()
{
}

void DialogLineAdd::SetEventAddLine(std::function<void(void)> callback)
{
    m_callbackAddLine = callback;
}

INT_PTR DialogLineAdd::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (AddLine(hDlg))
            {
                if (m_callbackAddLine)
                    m_callbackAddLine();

                EndDialog(hDlg, IDOK);
            }
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return FALSE;

        }
        break;
    }

    return FALSE;
}

bool DialogLineAdd::AddLine(HWND hDlg)
{
    bool bAllDecimal = true;

    Numeric::Vector3 p1;
    Numeric::Vector3 p2;

    bool bResult = true;
    p1.x = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT1), bResult);
    bAllDecimal &= bResult;

    p1.y = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT2), bResult);
    bAllDecimal &= bResult;

    p1.z = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT3), bResult);
    bAllDecimal &= bResult;

    p2.x = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT4), bResult);
    bAllDecimal &= bResult;

    p2.y = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT5), bResult);
    bAllDecimal &= bResult;

    p2.z = GetFloatValue(GetDlgItem(hDlg, IDC_EDIT6), bResult);
    bAllDecimal &= bResult;

    if (bAllDecimal == false)
    {
        MessageBox(hDlg, L"소수가 아닌 문자열이 포함되어 있습니다.", L"선 추가 실패", MB_OK);
        return false;
    }

    if (m_pLineContainer != nullptr)
    {
        m_pLineContainer->AddLine(p1, p2);
        return true;
    }

    return false;
}

float DialogLineAdd::GetFloatValue(HWND hEdit, bool& bResult)
{
    int len = GetWindowTextLength(hEdit);

    TCHAR* buffer = new TCHAR[len + 1];
    GetWindowText(hEdit, buffer, len + 1);
    float value = _tstof(buffer);

    bResult = true;
    for (int i = 0; i < len; ++i)
    {
        if (isdigit(buffer[i]) == false && buffer[i] != '.')
        {
            bResult = false;
            break;
        }
    }

    delete[] buffer;

    return value;
}
