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
#include "f2_ve_Config.h"

#include "configTools.h"
#include "hiResPatcher.h"

HINSTANCE hInst;
//LPWSTR lpCmdLineArgs;



//_____________________________________
bool CheckCmdLine(LPWSTR lpCmdLineArgs) {

    if (wcsstr(lpCmdLineArgs, L"/patch_install")) {
        HiRes_SetPatchData();
        HiRes_PatchExe();
        return true;
    }

    wchar_t* pCmd = wcsstr(lpCmdLineArgs, L"/config_delete");
    if (pCmd) {
        if (!ConfigDestroy())
            MessageBox(nullptr, L"Could not Delete config data!", L"Hi-Res patch Config", MB_ICONERROR);
        return true;
    }

    /*
    pCmd = wcsstr(lpCmdLineArgs, L"/config_create");
    if (pCmd) {
        pCmd += 14;
        wchar_t* tmpPath = new wchar_t[MAX_PATH];
        wmemset(tmpPath, L'\0', MAX_PATH);
        while (*pCmd == ' ')
            pCmd++;

        wchar_t* pCmdEnd = wcsstr(lpCmdLineArgs, L".ini ");
        if (!pCmdEnd)
            pCmdEnd = wcsstr(lpCmdLineArgs, L".ini\0");
        if (pCmdEnd) {
            pCmdEnd += 4;
            *pCmdEnd = '\0';

            wchar_t* ptmp = tmpPath;
            DWORD tmpPathSize = MAX_PATH;

            if (pCmd[1] != ':' && wcsncmp(pCmd, L".\\", 2) && wcsncmp(pCmd, L"..\\", 3)) {
                ptmp[0] = '.';
                ptmp[1] = '\\';
                ptmp += 2;
                tmpPathSize -= 2;
            }
            wcsncpy_s(ptmp, tmpPathSize, pCmd, wcslen(pCmd));
            ///MessageBox( nullptr, tmpPath, "Hi-Res patch Error", MB_ICONERROR);
            ConfigCreate(tmpPath);
        }
        else {
            MessageBox(nullptr, L"Could not find "".ini"" in Config Path!", L"Hi-Res patch Config", MB_ICONERROR);

        }
        delete[] tmpPath;
        tmpPath = nullptr;
        pCmd = nullptr;
        pCmdEnd = nullptr;
        return true;
    }*/
    pCmd = nullptr;

    return false;//retVal;
}


//________________________________________________________
void UpdatePatcher(HWND hButton, HWND hText1, HWND hText2) {
    wchar_t msg[256];
    if (exeData->isPatched) {
        LoadString(hInst, IDS_PATCH_DISABLE1, msg, 256);
        SendMessage(hButton, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)msg);
        LoadString(hInst, IDS_PATCH_DISABLE2, msg, 256);
    }
    else {
        LoadString(hInst, IDS_PATCH_ENABLE1, msg, 256);
        SendMessage(hButton, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)msg);
        LoadString(hInst, IDS_PATCH_ENABLE2, msg, 256);
    }
    SendMessage(hText2, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)msg);

    LoadString(hInst, IDS_VERSION0 + exeData->region, msg, 256);
    SendMessage(hText1, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)msg);
}

#define MSG_LENGTH 64
wchar_t general_msg[MSG_LENGTH];

//______________________________
int wm_gettext_to_int(HWND hwnd) {
    SendMessage(hwnd, WM_GETTEXT, (WPARAM)16, (LPARAM)general_msg);
    return _wtoi(general_msg);
}

//__________________________________________
void wm_settext_to_int(HWND hwnd, int value) {
    swprintf_s(general_msg, MSG_LENGTH, L"%d\0", value);
    SendMessage(hwnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);
}

//________________________________________________________________________________
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {


    switch (uMsg) {
    case WM_INITDIALOG: {
        SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1)));

        HWND hwnd = nullptr;

        //MAIN 
        hwnd = GetDlgItem(hwndDlg, IDC_CHECKBOX_WIN);
        if (ConfigReadInt(L"MAIN", L"WINDOWED", 0))
            SendMessage(hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);

        hwnd = GetDlgItem(hwndDlg, IDC_EDIT_SCALE_LEVEL);
        SendMessage(hwnd, (UINT)EM_LIMITTEXT, (WPARAM)3, (LPARAM)0);
        int scale_level = ConfigReadInt(L"MAIN", L"SCALE_LEVEL", 0);
        if (scale_level < 1)
            scale_level = 1;
        swprintf_s(general_msg, MSG_LENGTH, L"%d\0", scale_level);
        SendMessage(hwnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);

        //MAPS
        hwnd = GetDlgItem(hwndDlg, IDC_CHECK_MAP_FLOOR_LIGHT);
        if (ConfigReadInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_FLOOR", 0))
            SendMessage(hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);

        hwnd = GetDlgItem(hwndDlg, IDC_CHECK_MAP_OBJECT_LIGHT);
        if (ConfigReadInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_OBJECTS", 0))
            SendMessage(hwnd, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);


        int fog_of_war = ConfigReadInt(L"MAPS", L"FOG_OF_WAR", 0);
        hwnd = GetDlgItem(hwndDlg, IDC_COMBO_FOG_EFFECT);

        LoadString(hInst, IDS_FOG_EFFECT0, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        LoadString(hInst, IDS_FOG_EFFECT1, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        LoadString(hInst, IDS_FOG_EFFECT2, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)fog_of_war, (LPARAM)0);

        //IFACE
        int iface_location = ConfigReadInt(L"IFACE", L"IFACE_BAR_LOCATION", 0);
        hwnd = GetDlgItem(hwndDlg, IDC_COMBO_IFACE_POS);

        LoadString(hInst, IDS_IFACE_POS1, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        LoadString(hInst, IDS_IFACE_POS2, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        LoadString(hInst, IDS_IFACE_POS3, general_msg, MSG_LENGTH);
        SendMessage(hwnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)general_msg);
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iface_location, (LPARAM)0);

        int iface_width = ConfigReadInt(L"IFACE", L"IFACE_BAR_WIDTH", 640);
        hwnd = GetDlgItem(hwndDlg, IDC_EDIT_IFACE_WIDTH);
        SendMessage(hwnd, (UINT)EM_LIMITTEXT, (WPARAM)8, (LPARAM)0);
        swprintf_s(general_msg, MSG_LENGTH, L"%d\0", iface_width);
        SendMessage(hwnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);

        hwnd = GetDlgItem(hwndDlg, IDC_EDIT_IFACE_ART);
        SendMessage(hwnd, (UINT)EM_LIMITTEXT, (WPARAM)3, (LPARAM)0);
        int iface_art = ConfigReadInt(L"IFACE", L"IFACE_BAR_SIDE_ART", 0);
        if (iface_art < 0)
            iface_art = 0;
        swprintf_s(general_msg, MSG_LENGTH, L"%d\0", iface_art);
        SendMessage(hwnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);

        HWND hPatcherTxt1 = GetDlgItem(hwndDlg, IDC_PATCHER_TXT1);
        HWND hPatcherTxt2 = GetDlgItem(hwndDlg, IDC_PATCHER_TXT2);
        HWND hPatcherButton = GetDlgItem(hwndDlg, IDC_BUTTON_INSTALL);


        Button_SetElevationRequiredState(hPatcherButton, true);
        LONG error = HiRes_SetPatchData();
        if (error) {
            if (error == 1)
                LoadString(hInst, IDS_PATCH_FAIL1, general_msg, MSG_LENGTH);
            else if (error == 2)
                LoadString(hInst, IDS_PATCH_FAIL2, general_msg, MSG_LENGTH);
            SendMessage(hPatcherTxt2, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);
            LoadString(hInst, IDS_VERSION0, general_msg, MSG_LENGTH);
            SendMessage(hPatcherTxt1, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)general_msg);
            EnableWindow(hPatcherButton, false);
        }
        else
            UpdatePatcher(hPatcherButton, hPatcherTxt1, hPatcherTxt2);

        return TRUE;
    }
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    case WM_CTLCOLORSTATIC: {
        if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_PATCHER_TXT2)) {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode((HDC)wParam, TRANSPARENT);
            if (!exeData)
                SetTextColor(hdcStatic, RGB(255, 0, 0));
            else if (exeData->isPatched)
                SetTextColor(hdcStatic, RGB(0, 128, 255));
            else if (!exeData->isPatched)
                SetTextColor(hdcStatic, RGB(0, 0, 0));
            SetBkColor(hdcStatic, RGB(0, 0, 0));

            return (INT_PTR)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        }
        return FALSE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_INSTALL: {
            if (HIWORD(wParam) == BN_CLICKED) {
                HiRes_PatchInstall();
                HiRes_SetPatchData();
                HWND hPatcherTxt1 = GetDlgItem(hwndDlg, IDC_PATCHER_TXT1);
                HWND hPatcherTxt2 = GetDlgItem(hwndDlg, IDC_PATCHER_TXT2);
                UpdatePatcher((HWND)lParam, hPatcherTxt1, hPatcherTxt2);
            }
            return TRUE;
        }
        case IDC_EDIT_SCALE_LEVEL: {//make sure scale level if greater than or equal to 1 after edit.
            if (HIWORD(wParam) == EN_KILLFOCUS) {
                int scale_level = wm_gettext_to_int((HWND)lParam);
                if (scale_level < 1)
                    wm_settext_to_int((HWND)lParam, 1);
            }
            return TRUE;
        }
        case IDC_EDIT_IFACE_WIDTH: {//make sure interface width if greater than or equal to 640 after edit.
            if (HIWORD(wParam) == EN_KILLFOCUS) {
                int iface_width = wm_gettext_to_int((HWND)lParam);
                if (iface_width < 640)
                    wm_settext_to_int((HWND)lParam, 640);
            }
            return TRUE;
        }
        case IDC_EDIT_IFACE_ART: {//make sure interface art number if greater than 0 after edit.
            if (HIWORD(wParam) == EN_KILLFOCUS) {
                int iface_art = wm_gettext_to_int((HWND)lParam);
                if (iface_art < 0)
                    wm_settext_to_int((HWND)lParam, 0);
            }
            return TRUE;
        }
        case IDC_BUTTON_CONFIG_EDIT: {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (hr == S_OK && ShellExecute(nullptr, L"open", ConfigGetPath(), NULL, NULL, SW_SHOWNORMAL) > (HINSTANCE)32)
                EndDialog(hwndDlg, 0);
            else
                MessageBox(hwndDlg, ConfigGetPath(), L"Could not open file", MB_ICONERROR);
            return TRUE;
        }
        case IDC_BUTTON_EXIT: {

            HWND hwnd = nullptr;

            //MAIN
            hwnd = GetDlgItem(hwndDlg, IDC_CHECKBOX_WIN);
            if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
                ConfigWriteInt(L"MAIN", L"WINDOWED", 1);
            else
                ConfigWriteInt(L"MAIN", L"WINDOWED", 0);

            hwnd = GetDlgItem(hwndDlg, IDC_EDIT_SCALE_LEVEL);
            ConfigWriteInt(L"MAIN", L"SCALE_LEVEL", wm_gettext_to_int(hwnd));

            //MAPS
            hwnd = GetDlgItem(hwndDlg, IDC_CHECK_MAP_FLOOR_LIGHT);
            if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
                ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_FLOOR", 1);
            else
                ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_FLOOR", 0);

            hwnd = GetDlgItem(hwndDlg, IDC_CHECK_MAP_OBJECT_LIGHT);
            if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
                ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_OBJECTS", 1);
            else
                ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_OBJECTS", 0);

            hwnd = GetDlgItem(hwndDlg, IDC_COMBO_FOG_EFFECT);
            ConfigWriteInt(L"MAPS", L"FOG_OF_WAR", (int)SendMessage(hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));

            //IFACE
            hwnd = GetDlgItem(hwndDlg, IDC_COMBO_IFACE_POS);
            ConfigWriteInt(L"IFACE", L"IFACE_BAR_LOCATION", (int)SendMessage(hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));

            hwnd = GetDlgItem(hwndDlg, IDC_EDIT_IFACE_WIDTH);
            ConfigWriteInt(L"IFACE", L"IFACE_BAR_WIDTH", wm_gettext_to_int(hwnd));

            hwnd = GetDlgItem(hwndDlg, IDC_EDIT_IFACE_ART);
            ConfigWriteInt(L"IFACE", L"IFACE_BAR_SIDE_ART", wm_gettext_to_int(hwnd));

            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        case IDCANCEL: {
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        }
    }

    return FALSE;
}


//_________________________________________________________________________________________________________________________
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    hInst = hInstance;
    //lpCmdLineArgs = lpCmdLine;

    if (CheckCmdLine(lpCmdLine))
        return 0;

    // The user interface is a modal dialog box
    return (int)DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CONFIG), nullptr, (DLGPROC)DialogProc);
}


