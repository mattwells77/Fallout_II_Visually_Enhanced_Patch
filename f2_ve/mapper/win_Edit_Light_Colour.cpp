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

#include "win_Edit_Light_Colour.h"

#include "../resource.h"

#include "../Dx_General.h"
#include "../Dx_Graphics.h"
#include "../Dx_Game.h"


//________________________________________
COLORREF LightColour_Get(HWND hwnd_parent) {
    DWORD val = 0;
    BOOL translated = FALSE;
    BYTE r = GetDlgItemInt(hwnd_parent, IDC_EDIT_RED, &translated, TRUE);
    if (!translated)
        return 0;
    BYTE g = GetDlgItemInt(hwnd_parent, IDC_EDIT_GREEN, &translated, TRUE);
    if (!translated)
        return 0;
    BYTE b = GetDlgItemInt(hwnd_parent, IDC_EDIT_BLUE, &translated, TRUE);
    if (!translated)
        return 0;

    return RGB(r, g, b);
}


//_______________________________________
//Make sure at least one colour component is at max of 255 and adjust the others to maintain component ratios.
//This is done to avoid affecting the brightness of the light to much.
COLORREF LightColour_Fix(COLORREF colour) {

    DWORD r = GetRValue(colour);
    DWORD g = GetGValue(colour);
    DWORD b = GetBValue(colour);

    if (r < 255 && g < 255 && b < 255) {
        DWORD max_component_intensity = r;
        if (g > max_component_intensity)
            max_component_intensity = g;
        if (b > max_component_intensity)
            max_component_intensity = b;

        float multiplyer = 255.0f / max_component_intensity;

        if (max_component_intensity == 0) {
            r = 255;
            g = 255;
            b = 255;
        }
        else {
            r = (DWORD)round(r * multiplyer);
            g = (DWORD)round(g * multiplyer);
            b = (DWORD)round(b * multiplyer);

            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
        }
    }

    return RGB((BYTE)r, (BYTE)g, (BYTE)b);
}


//________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_EditColour(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static HBRUSH hbrush_colour_box = nullptr;
    static DWORD* p_light_colour_xbgr_return = nullptr;

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


        //set start colour white
        COLORREF colour = RGB(255, 255, 255);
        p_light_colour_xbgr_return = (DWORD*)lParam;

        //use the current light if present, convert from xbgr to rgbx. 
        if (p_light_colour_xbgr_return && *p_light_colour_xbgr_return != 0) {
            //       |                        R                         |                        G                          |                        B                         |
            colour = ((*p_light_colour_xbgr_return & 0xFF000000) >> 24) | ((*p_light_colour_xbgr_return & 0x00FF0000) >> 8) | ((*p_light_colour_xbgr_return & 0x0000FF00) << 8);// | ((colour & 0xFF000000) >> 24);//RGB((*p_light_colour_return >> 24) & 0xFF, (*p_light_colour_return >> 16) & 0xFF, (*p_light_colour_return >> 8) & 0xFF);
        }

        HWND hwnd_sub = nullptr;

        //setup component edit boxes
        hwnd_sub = GetDlgItem(hwnd, IDC_EDIT_RED);
        SendMessage(hwnd_sub, EM_LIMITTEXT, (WPARAM)3, 0);
        GetBValue(colour);
        SetDlgItemInt(hwnd, IDC_EDIT_RED, GetRValue(colour), TRUE);

        hwnd_sub = GetDlgItem(hwnd, IDC_EDIT_GREEN);
        SendMessage(hwnd_sub, EM_LIMITTEXT, (WPARAM)3, 0);
        SetDlgItemInt(hwnd, IDC_EDIT_GREEN, GetGValue(colour), TRUE);
        hwnd_sub = GetDlgItem(hwnd, IDC_EDIT_BLUE);
        SendMessage(hwnd_sub, EM_LIMITTEXT, (WPARAM)3, 0);
        SetDlgItemInt(hwnd, IDC_EDIT_BLUE, GetBValue(colour), TRUE);


        //setup component sliders
        //largest number at the top looks best, make these negative for easy conversion.
        hwnd_sub = GetDlgItem(hwnd, IDC_SLIDER_RED);
        SendMessage(hwnd_sub, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-255, 0));
        SendMessage(hwnd_sub, TBM_SETLINESIZE, 0, (LPARAM)1);
        SendMessage(hwnd_sub, TBM_SETPAGESIZE, 0, (LPARAM)5);
        SendMessage(hwnd_sub, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)-GetRValue(colour));

        hwnd_sub = GetDlgItem(hwnd, IDC_SLIDER_GREEN);
        SendMessage(hwnd_sub, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-255, 0));
        SendMessage(hwnd_sub, TBM_SETLINESIZE, 0, (LPARAM)1);
        SendMessage(hwnd_sub, TBM_SETPAGESIZE, 0, (LPARAM)5);
        SendMessage(hwnd_sub, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)-GetGValue(colour));

        hwnd_sub = GetDlgItem(hwnd, IDC_SLIDER_BLUE);
        SendMessage(hwnd_sub, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-255, 0));
        SendMessage(hwnd_sub, TBM_SETLINESIZE, 0, (LPARAM)1);
        SendMessage(hwnd_sub, TBM_SETPAGESIZE, 0, (LPARAM)5);
        SendMessage(hwnd_sub, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)-GetBValue(colour));

        return TRUE;
    }
    case WM_CTLCOLORSTATIC: {
        //draw the colour box
        if ((HWND)lParam == GetDlgItem(hwnd, IDC_STATIC_COLOUR_BOX)) {
            if (hbrush_colour_box)
                DeleteObject(hbrush_colour_box);
            hbrush_colour_box = CreateSolidBrush(LightColour_Get(hwnd));
            return (INT_PTR)hbrush_colour_box;
        }
        else
            return FALSE;
        break;
    }
    case WM_NOTIFY:
        break;
    case WM_VSCROLL:
        //update associated edit box when slider is moved.
        if ((HWND)lParam != nullptr) {
            int idc_slider = GetDlgCtrlID((HWND)lParam);
            int idc_edit = 0;
            if (idc_slider == IDC_SLIDER_RED)
                idc_edit = IDC_EDIT_RED;
            else if (idc_slider == IDC_SLIDER_GREEN)
                idc_edit = IDC_EDIT_GREEN;
            else if (idc_slider == IDC_SLIDER_BLUE)
                idc_edit = IDC_EDIT_BLUE;
            else
                break;
            //convert negative slider position to positive component value.
            int val = -SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
            SetDlgItemInt(hwnd, idc_edit, val, TRUE);
            //update the colour box.
            RedrawWindow(GetDlgItem(hwnd, IDC_STATIC_COLOUR_BOX), nullptr, nullptr, RDW_INVALIDATE);
            return 0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        //update associated slider when edit box value modified.
        case IDC_EDIT_RED:
        case IDC_EDIT_GREEN:
        case IDC_EDIT_BLUE: {
            if (HIWORD(wParam) == EN_UPDATE) {
                BOOL translated = false;
                int idc_edit = LOWORD(wParam);
                int idc_slider = 0;
                if (idc_edit == IDC_EDIT_RED)
                    idc_slider = IDC_SLIDER_RED;
                else if (idc_edit == IDC_EDIT_GREEN)
                    idc_slider = IDC_SLIDER_GREEN;
                else if (idc_edit == IDC_EDIT_BLUE)
                    idc_slider = IDC_SLIDER_BLUE;
                else
                    break;
                UINT val = GetDlgItemInt(hwnd, idc_edit, &translated, TRUE);
                if (translated) {
                    //keep component value under max 255.
                    if (val > 255) {
                        val = 255;
                        SetDlgItemInt(hwnd, idc_edit, val, TRUE);
                    }
                    //convert positive component value to negative when setting slider position.
                    SendMessage(GetDlgItem(hwnd, idc_slider), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)-(int)val);
                    //update the colour box.
                    RedrawWindow(GetDlgItem(hwnd, IDC_STATIC_COLOUR_BOX), nullptr, nullptr, RDW_INVALIDATE);
                }
            }
            break;
        }
        case IDOK: {
            if (p_light_colour_xbgr_return) {
                DWORD colour = LightColour_Get(hwnd);
                //don't fix the colour if it is equal to zero(no colour), as this is the original state and will allow an object's colour to be defined by it's proto. If a proto's colour is set to zero it will default to the original white colour.
                if(colour != 0)
                    colour = LightColour_Fix(LightColour_Get(hwnd));
                //convert from rgbx to xbgr. 
                //                            |          R            |            G             |             B             |
                *p_light_colour_xbgr_return = ((colour & 0xFF) << 24) | ((colour & 0xFF00) << 8) | ((colour & 0xFF0000) >> 8);
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
        //Fallout_Debug_Info("edit colour box being destroyed");
        if (hbrush_colour_box)
            DeleteObject(hbrush_colour_box);
        hbrush_colour_box = nullptr;
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("edit colour box destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//____________________________________________________________________________________
BOOL EditLightColour(HWND hwndParent, HINSTANCE hinstance, DWORD *p_light_colour_xbgr) {

    if (!p_light_colour_xbgr)
        return FALSE;

    if(DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_COLOUR_SELECTOR), hwndParent, DlgProc_EditColour, (LPARAM)p_light_colour_xbgr) == 1)
        return TRUE;
    return FALSE;
}
