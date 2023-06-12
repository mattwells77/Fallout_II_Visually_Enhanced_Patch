#include "pch.h"


#include "win_Set_Ambient_Light.h"

#include "../resource.h"



//________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_SetAmbientLight(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {


    static DWORD* p_light_intensity_return = nullptr;
    static DWORD light_intensity_percentage = 100;
    static DWORD light_intensity = 65536;
    switch (Message) {

    case WM_INITDIALOG: {
        //set position to centre of parent window.
        RECT rc_Win{ 0,0,0,0 };
        GetWindowRect(hwnd, &rc_Win);
        HWND hwnd_parent = GetParent(hwnd);
        RECT rcParent{ 0,0,0,0 };
        if (hwnd_parent)
            GetWindowRect(hwnd_parent, &rcParent);
        SetWindowPos(hwnd, nullptr, ((rcParent.right - rcParent.left) - (rc_Win.right - rc_Win.left)) / 2, ((rcParent.bottom - rcParent.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);


        p_light_intensity_return = (DWORD*)lParam;
        //use the current light if present, convert from xbgr to rgbx. 
        if (p_light_intensity_return) {
            if (*p_light_intensity_return <= 65536) {
                light_intensity = *p_light_intensity_return;
                light_intensity_percentage = (DWORD)(light_intensity / 655.36f);
            }
        }

        HWND hwnd_sub = nullptr;

        //setup intensity edit box
        hwnd_sub = GetDlgItem(hwnd, IDC_EDIT1);
        SendMessage(hwnd_sub, EM_LIMITTEXT, (WPARAM)3, 0);

        SetDlgItemInt(hwnd, IDC_EDIT1, light_intensity_percentage, TRUE);

        //setup slider
        hwnd_sub = GetDlgItem(hwnd, IDC_SLIDER1);
        
        SendMessage(hwnd_sub, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hwnd_sub, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)65536);
        //SendMessage(hwnd_sub, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 65536));
        SendMessage(hwnd_sub, TBM_SETLINESIZE, 0, (LPARAM)1);
        SendMessage(hwnd_sub, TBM_SETPAGESIZE, 0, (LPARAM)655);
        SendMessage(hwnd_sub, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)light_intensity);
        
        //setup combo box
        wchar_t* str = new wchar_t[20];
        hwnd_sub = GetDlgItem(hwnd, IDC_COMBO1);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_BRIGHT, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_CAVERN, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_DUSK, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_ENCLAVE, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_INDOOR, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_BASEMENT, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_AMBIENT_LIGHT_MILITARY_BASE, str, 20);
        SendMessage(hwnd_sub, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        delete[] str;

        return TRUE;
    }

    case WM_NOTIFY:
        break;
    case WM_HSCROLL:
        //update associated edit box when slider is moved.
        if ((HWND)lParam != nullptr) {
            int idc_slider = GetDlgCtrlID((HWND)lParam);
            if (idc_slider == IDC_SLIDER1) {
                light_intensity = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
                light_intensity_percentage = (DWORD)(light_intensity / 655.36f);
                SetDlgItemInt(hwnd, IDC_EDIT1, light_intensity_percentage, TRUE);
            }
            return 0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_COMBO1: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                int num = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                DWORD preset = 0;
                switch (num) {
                case 0://BRIGHT_LIGHT
                    preset = 100;
                    break;
                case 1://CAVERN_LIGHT
                    preset = 50;
                    break;
                case 2://DUSK_LIGHT
                    preset = 40;
                    break;
                case 3://ENCLAVE_LIGHT
                    preset = 60;
                    break;
                case 4://INDOOR_LIGHT
                    preset = 60;
                    break;
                case 5://BASEMENT_LIGHT
                    preset = 50;
                    break;
                case 6://MILITARY_BASE_LIGHTING
                    preset = 75;
                    break;
                default:
                    return TRUE;
                    break;
                }
                light_intensity_percentage = preset;
                //update edit box
                SetDlgItemInt(hwnd, IDC_EDIT1, light_intensity_percentage, TRUE);
                //update slider
                light_intensity = (DWORD)(light_intensity_percentage * 655.36f);
                SendMessage(GetDlgItem(hwnd, IDC_SLIDER1), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)light_intensity);
            }
            return TRUE;
        }
        case IDC_EDIT1: {
            if (HIWORD(wParam) == EN_UPDATE) {
                BOOL translated = false;
                int idc_edit = LOWORD(wParam);
                if (idc_edit != IDC_EDIT1) 
                    break;
                DWORD light_intensity_percentage = GetDlgItemInt(hwnd, idc_edit, &translated, TRUE);
                if (translated) {
                    //keep component value under max 100%.
                    if (light_intensity_percentage > 100) {
                        light_intensity_percentage = 100;
                        SetDlgItemInt(hwnd, idc_edit, light_intensity_percentage, TRUE);
                    }
                    //update slider
                    light_intensity = (DWORD)(light_intensity_percentage * 655.36f);
                    SendMessage(GetDlgItem(hwnd, IDC_SLIDER1), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)light_intensity);
                }
            }
            break;
        }

        case IDOK: {
            if (p_light_intensity_return) {
                *p_light_intensity_return = light_intensity;

                EndDialog(hwnd, 1);
            }
            else
                EndDialog(hwnd, -1);
            return FALSE;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        //Fallout_Debug_Info("set ambient light box being destroyed");
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("set ambient light box box destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//______________________________________________________________________________________________
BOOL Set_Ambient_Light_Intensity(HWND hwndParent, HINSTANCE hinstance, DWORD* p_light_intensity) {

    if (!p_light_intensity)
        return FALSE;

    if (DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_SET_AMBIENT_INTENSITY), hwndParent, DlgProc_SetAmbientLight, (LPARAM)p_light_intensity) == 1)
        return TRUE;
    return FALSE;
}
