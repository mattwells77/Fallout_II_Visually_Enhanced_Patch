/*
The MIT License (MIT)
Copyright © 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pch.h"

#include "../resource.h"
#include "win_ProgressBar.h"


//_________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ProgressBar(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {


    static DWORD* p_light_colour_xbgr_return = nullptr;

    switch (Message) {

    case WM_INITDIALOG: {
        InitCommonControls();
        //set position to centre of parent window.
        RECT rc_Win{ 0,0,0,0 };
        GetWindowRect(hwnd, &rc_Win);
        HWND hwnd_parent = GetParent(hwnd);
        RECT rcParent{ 0,0,0,0 };
        if (hwnd_parent)
            GetWindowRect(hwnd_parent, &rcParent);
        SetWindowPos(hwnd, nullptr, ((rcParent.right - rcParent.left) - (rc_Win.right - rc_Win.left)) / 2, ((rcParent.bottom - rcParent.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        HWND hwnd_sub = nullptr;

        return TRUE;
    }

    case WM_DESTROY: {
        //Fallout_Debug_Info("progress bar being destroyed");
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("progress bar destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//_____________________________________________________________________________________________
HWND ProgressBar_Create(HWND hwndParent, HINSTANCE hinstance, const wchar_t* title, WORD range) {

    HWND hwnd = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_DIALOG_PROGRESS), hwndParent, (DLGPROC)DlgProc_ProgressBar);

    SendMessage(hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)title);

    HWND hwnd_sub = GetDlgItem(hwnd, IDC_PROGRESS1);
    SendMessage(hwnd_sub, PBM_SETRANGE, 0, MAKELPARAM(0, range));
    SendMessage(hwnd_sub, PBM_SETSTEP, (WPARAM)1, 0);

    ShowWindow(hwnd, SW_SHOW);
    return hwnd;
}


//_________________________________
BOOL ProgressBar_Update(HWND hwnd) {
    HWND hwnd_sub = GetDlgItem(hwnd, IDC_PROGRESS1);
    SendMessage(hwnd_sub, PBM_STEPIT, 0, 0);
    return TRUE;
}
