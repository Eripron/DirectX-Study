#pragma once

#include <tchar.h>
#include <functional>

#include "Dialog.h"
#include "LineContainer.h"
#include "resource.h"

class DialogLineAdd : public Dialog
{
public:
	DialogLineAdd(LineContainer* pLineContainer);
	~DialogLineAdd();

	void SetEventAddLine(std::function<void(void)> callback);

protected:
	INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	LineContainer* m_pLineContainer;
	std::function<void(void)> m_callbackAddLine;

	bool AddLine(HWND hDlg);
	float GetFloatValue(HWND hEdit, bool& result);

};