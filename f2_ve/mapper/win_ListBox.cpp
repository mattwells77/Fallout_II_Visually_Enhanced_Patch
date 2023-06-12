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

#include "win_ListBox.h"

#include "../resource.h"
#include "../win_fall.h"
#include "../errors.h"

class listDATA {
public:
    listDATA(int in_x, int in_y, const char* in_title, char** in_list, int in_numItems, int in_listPos, DWORD in_max_chars_edit_text, const char* in_excludeChars) {

        title = nullptr;
        list = nullptr;
        text_current_selection = nullptr;
        excludeChars = nullptr;
        num_chars_edit_text = in_max_chars_edit_text;
        numItems = in_numItems;
        x = in_x;
        y = in_y;
        listPos = in_listPos;

        size_t num_chars = 0;
        size_t num_chars_max = 0;

        if (in_title) {
            num_chars_max = strlen(in_title);
            title = new wchar_t[num_chars_max + 1];
            swprintf_s(title, num_chars_max + 1, L"%S", in_title);
        }

        if (in_list) {
            list = new wchar_t* [numItems] {0};
            for (int i = 0; i < numItems; i++) {
                num_chars = strlen(in_list[i]);
                if (num_chars > num_chars_max)
                    num_chars_max = num_chars;
            }
            for (int i = 0; i < numItems; i++) {
                list[i] = new wchar_t[num_chars_max + 1];
                swprintf_s(list[i], num_chars_max + 1, L"%S", in_list[i]);
            }
        }

        if (in_excludeChars) {
            num_chars_max = strlen(in_excludeChars);
            excludeChars = new wchar_t[num_chars_max + 1];
            swprintf_s(excludeChars, num_chars_max + 1, L"%S", in_excludeChars);
        }

        if (num_chars_edit_text)
            text_current_selection = new wchar_t[num_chars_edit_text + 1] {0};

    }
    listDATA(int in_x, int in_y, const wchar_t* in_title, wchar_t** in_list, int in_numItems, int in_listPos, DWORD in_max_chars_edit_text, const wchar_t* in_excludeChars) {

        title = nullptr;
        list = nullptr;
        text_current_selection = nullptr;
        excludeChars = nullptr;
        num_chars_edit_text = in_max_chars_edit_text;
        numItems = in_numItems;
        x = in_x;
        y = in_y;
        listPos = in_listPos;

        size_t num_chars = 0;
        size_t num_chars_max = 0;

        if (in_title) {
            num_chars_max = wcslen(in_title);
            title = new wchar_t[num_chars_max + 1];
            swprintf_s(title, num_chars_max + 1, L"%s", in_title);
        }

        if (in_list) {
            list = new wchar_t* [numItems] {0};
            for (int i = 0; i < numItems; i++) {
                num_chars = wcslen(in_list[i]);
                if (num_chars > num_chars_max)
                    num_chars_max = num_chars;
            }
            for (int i = 0; i < numItems; i++) {
                list[i] = new wchar_t[num_chars_max + 1];
                swprintf_s(list[i], num_chars_max + 1, L"%s", in_list[i]);
            }
        }

        if (num_chars_edit_text)
            text_current_selection = new wchar_t[num_chars_edit_text + 1] {0};
    }
    ~listDATA() {

        delete[] title;
        for (int i = 0; i < numItems; i++)
            delete[] list[i];
        delete[] list;

        delete[] text_current_selection;
    }
    wchar_t* title;
    wchar_t** list;
    wchar_t* text_current_selection;
    wchar_t* excludeChars;
    size_t num_chars_edit_text;
    int numItems;
    int x;
    int y;
    int listPos;
protected:
private:
};


//_________________________________________________________________________________
BOOL CALLBACK DlgProc_SaveAs(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    static wchar_t* text_current_selection = nullptr;
    static size_t num_chars_edit_text = 0;
    static const wchar_t* editExcludeChars = nullptr;

    switch (Message) {

    case WM_INITDIALOG: {

        HWND LIST = GetDlgItem(hwnd, IDC_LIST1);
        HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);

        listDATA* listData = (listDATA*)lParam;

        num_chars_edit_text = listData->num_chars_edit_text;
        text_current_selection = listData->text_current_selection;

        SendMessage(EDIT, EM_SETLIMITTEXT, (WPARAM)num_chars_edit_text, (LPARAM)0);

        editExcludeChars = listData->excludeChars;
        //set title text
        SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)(LPCWSTR)listData->title);

        LONG maxWidth = 0;

        if (listData->list) {
            SIZE listSIZE{ 0,0 };
            HDC listHDC = GetDC(LIST);
            //fill item list
            for (int i = 0; i < listData->numItems; i++) {
                SendMessage(LIST, LB_INSERTSTRING, i, (LPARAM)listData->list[i]);
                int charCount = SendMessage(LIST, LB_GETTEXTLEN, (WPARAM)i, 0);
                GetTextExtentPoint32(listHDC, listData->list[i], charCount, &listSIZE);
                if (listSIZE.cx > maxWidth)
                    maxWidth = listSIZE.cx;
            }
        }

        //set list horizotal scroll to max text width.
        SendMessage(LIST, LB_SETHORIZONTALEXTENT, (WPARAM)maxWidth, 0);
        //set list selected pos.
        SendMessage(LIST, LB_SETCURSEL, (WPARAM)listData->listPos, 0);

        //copy selected string to edit box.
        //SendMessage(EDIT, WM_SETTEXT, 0, (LPARAM)listData->list[listData->listPos]);

        //if list width is less than the maximum item text increase width of dialog & subs to match.
        RECT dialogRect;
        RECT listRect;
        RECT editRect;
        GetWindowRect(hwnd, &dialogRect);
        GetWindowRect(LIST, &listRect);
        GetWindowRect(EDIT, &editRect);
        LONG listWidth = listRect.right - listRect.left;
        LONG dialogWidth = dialogRect.right - dialogRect.left;

        if (listWidth < maxWidth) {
            //prevent the dialog from getting crazy wide.
            if (maxWidth > 320)
                maxWidth = 320;
            SetWindowPos(hwnd, nullptr, 0, 0, maxWidth + (dialogWidth - listWidth), dialogRect.bottom - dialogRect.top, SWP_NOZORDER | SWP_NOMOVE);
            SetWindowPos(LIST, nullptr, 0, 0, maxWidth, listRect.bottom - listRect.top, SWP_NOZORDER | SWP_NOMOVE);
            SetWindowPos(EDIT, nullptr, 0, 0, maxWidth, editRect.bottom - editRect.top, SWP_NOZORDER | SWP_NOMOVE);
        }

        //set the position of the dialog in relation to the parent win.
        HWND parent = GetParent(hwnd);
        RECT parentRect;
        if (parent)
            GetWindowRect(parent, &parentRect);
        SetWindowPos(hwnd, nullptr, parentRect.left + listData->x, parentRect.top + listData->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        return TRUE;
        break;
    }
    case WM_KEYDOWN: {
        if (wParam == VK_SPACE)
            //return 0;
            MessageBoxA(0, "hello", "New Map Name", MB_OK | MB_ICONINFORMATION);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_LIST1:
            switch (HIWORD(wParam)) {
            case LBN_DBLCLK:
                SendMessage(hwnd, WM_COMMAND, IDOK, 0);
                break;
            case LBN_SELCHANGE: {
                HWND LIST = GetDlgItem(hwnd, IDC_LIST1);
                HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);
                int listPos = (int)SendMessage(LIST, LB_GETCURSEL, 0, 0);//get selected pos from dialog list
                SendMessage(LIST, LB_GETTEXT, (WPARAM)listPos, (LPARAM)text_current_selection);//get selected string from dialog list
                SendMessage(EDIT, WM_SETTEXT, 0, (LPARAM)text_current_selection);//copy selected string to edit box
                break;
            }

            }
            break;
        case IDC_EDIT1:
            switch (HIWORD(wParam)) {
            case EN_UPDATE: {
                if (editExcludeChars) {
                    HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);

                    *(WORD*)text_current_selection = (WORD)num_chars_edit_text;
                    DWORD numCharsCopied = SendMessage(EDIT, EM_GETLINE, 0, (LPARAM)text_current_selection);
                    if (numCharsCopied > num_chars_edit_text)
                        numCharsCopied = num_chars_edit_text;
                    //Copied line does not contain null char, null end of string.
                    text_current_selection[numCharsCopied] = '\0';
                    size_t num = 0;
                    num = wcscspn(text_current_selection, editExcludeChars);
                    if (text_current_selection[num] != '\0') {
                        while (text_current_selection[num] != '\0') {
                            while (text_current_selection[num] != '\0') {
                                num++;
                                text_current_selection[num - 1] = text_current_selection[num];
                            }
                            num = wcscspn(text_current_selection, editExcludeChars);
                        }
                        SendMessage(EDIT, WM_SETTEXT, 0, (LPARAM)text_current_selection);
                        //MessageBoxA(0, txt, "New Map Name", MB_OK | MB_ICONINFORMATION);
                    }
                }

            }
                          break;
            case EN_CHANGE: {
                HWND LIST = GetDlgItem(hwnd, IDC_LIST1);
                HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);
                int listPos = (int)SendMessage(LIST, LB_GETCURSEL, 0, 0);//get selected pos from dialog list
                //Before sending the message, set the first word of this buffer to the size, in TCHARs, of the buffer. For ANSI text, this is the number of bytes; for Unicode text, this is the number of characters. 
                *(WORD*)text_current_selection = (WORD)num_chars_edit_text;
                //copy selected string from edit box
                DWORD numCharsCopied = SendMessage(EDIT, EM_GETLINE, 0, (LPARAM)text_current_selection);
                if (numCharsCopied > num_chars_edit_text)
                    numCharsCopied = num_chars_edit_text;
                //Copied line does not contain null char, null end of string.
                text_current_selection[numCharsCopied] = '\0';
                //editString[editStringLength]='\0';//null end of string
                listPos = (int)SendMessage(LIST, LB_FINDSTRING, -1, (LPARAM)text_current_selection);//find pos from dialog list of matching txt
                if (listPos != -1)
                    SendMessage(LIST, LB_SETCURSEL, (WPARAM)listPos, 0);//set selected pos in dialog list
            }
                          break;
            }
            break;
        case IDOK: {

            HWND LIST = GetDlgItem(hwnd, IDC_LIST1);
            HWND EDIT = GetDlgItem(hwnd, IDC_EDIT1);
            int listPos = (int)SendMessage(LIST, LB_GETCURSEL, 0, 0);
            //Before sending the message, set the first word of this buffer to the size, in TCHARs, of the buffer. For ANSI text, this is the number of bytes; for Unicode text, this is the number of characters. 
            *(WORD*)text_current_selection = (WORD)num_chars_edit_text;
            //copy selected string from edit box
            DWORD numCharsCopied = SendMessage(EDIT, EM_GETLINE, 0, (LPARAM)text_current_selection);
            if (numCharsCopied > num_chars_edit_text)
                numCharsCopied = num_chars_edit_text;
            //Copied line does not contain null char, null end of string.
            text_current_selection[numCharsCopied] = '\0';

            EndDialog(hwnd, listPos);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}


//_____________________________________________________________________________________
BOOL CALLBACK DlgProc_ListSelect(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    switch (Message) {
    case WM_INITDIALOG: {

        HWND LIST = GetDlgItem(hwnd, IDC_LIST1);

        listDATA* listData = (listDATA*)lParam;
        //set title text
        SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)(LPCWSTR)listData->title);

        LONG maxWidth = 0;

        if (listData->list) {
            SIZE listSIZE{ 0,0 };
            HDC listHDC = GetDC(LIST);
            //fill item list
            for (int i = 0; i < listData->numItems; i++) {
                SendMessage(LIST, LB_INSERTSTRING, i, (LPARAM)listData->list[i]);
                int charCount = SendMessage(LIST, LB_GETTEXTLEN, (WPARAM)i, 0);
                GetTextExtentPoint32(listHDC, listData->list[i], charCount, &listSIZE);
                if (listSIZE.cx > maxWidth)
                    maxWidth = listSIZE.cx;
            }
        }
        //set list horizotal scroll to max text width.
        SendMessage(LIST, LB_SETHORIZONTALEXTENT, (WPARAM)maxWidth, 0);
        //set list selected position.
        SendMessage(LIST, LB_SETCURSEL, (WPARAM)listData->listPos, 0);

        //if list width is less than the maximum item text increase width of dialog & subs to match.
        RECT dialogRect;
        RECT listRect;
        GetWindowRect(hwnd, &dialogRect);
        GetWindowRect(LIST, &listRect);

        LONG listWidth = listRect.right - listRect.left;
        LONG dialogWidth = dialogRect.right - dialogRect.left;

        if (listWidth < maxWidth) {
            //prevent the dialog from getting crazy wide.
            if (maxWidth > 320)
                maxWidth = 320;
            SetWindowPos(hwnd, nullptr, 0, 0, maxWidth + (dialogWidth - listWidth), dialogRect.bottom - dialogRect.top, SWP_NOZORDER | SWP_NOMOVE);
            SetWindowPos(LIST, nullptr, 0, 0, maxWidth, listRect.bottom - listRect.top, SWP_NOZORDER | SWP_NOMOVE);
        }

        //set the position of the dialog in relation to the parent win.
        HWND parent = GetParent(hwnd);
        RECT parentRect;
        if (parent)
            GetWindowRect(parent, &parentRect);
        
        //centre window on parent if both x and y == -1;
        if (listData->x == -1 && listData->y == -1) {
            RECT rc_Win{ 0,0,0,0 };
            GetWindowRect(hwnd, &rc_Win);
            SetWindowPos(hwnd, nullptr, ((parentRect.right - parentRect.left) - (rc_Win.right - rc_Win.left)) / 2, ((parentRect.bottom - parentRect.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
        else
            SetWindowPos(hwnd, nullptr, parentRect.left + listData->x, parentRect.top + listData->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        return TRUE;
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_LIST1:
            switch (HIWORD(wParam)) {
            case LBN_DBLCLK:
                SendMessage(hwnd, WM_COMMAND, IDOK, 0);
                break;
            }
            break;
        case IDOK: {
            HWND LIST = GetDlgItem(hwnd, IDC_LIST1);
            int listPos = (int)SendMessage(LIST, LB_GETCURSEL, 0, 0);
            EndDialog(hwnd, listPos);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}


//________________________________________________________________________________________________________________________________________________________________________________________
int Dialog_ListSelect(HWND hwndParent, HINSTANCE hinstance, int x, int y, const char* title, char** list, int numItems, int listPos, DWORD max_chars_selected_text, char* retSelectedText, DWORD retSelectedText_numChars) {

    listDATA* listData = new listDATA(x, y, title, list, numItems, listPos, max_chars_selected_text, nullptr);


    int ret = DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_LIST_SELECT), hwndParent, DlgProc_ListSelect, (LPARAM)listData);

    if (retSelectedText)
        sprintf_s(retSelectedText, retSelectedText_numChars, "%S", listData->text_current_selection);

    delete listData;
    return ret;
}


//____________________________________________________________________________________________________________________________________________________________________________________________________
int Dialog_ListSelect_WC(HWND hwndParent, HINSTANCE hinstance, int x, int y, const wchar_t* title, wchar_t** list, int numItems, int listPos, DWORD max_chars_selected_text, wchar_t* retSelectedText, DWORD retSelectedText_numChars) {

    listDATA* listData = new listDATA(x, y, title, list, numItems, listPos, max_chars_selected_text, nullptr);


    int ret = DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_LIST_SELECT), hwndParent, DlgProc_ListSelect, (LPARAM)listData);

    if (retSelectedText)
        swprintf_s(retSelectedText, retSelectedText_numChars, L"%s", listData->text_current_selection);

    delete listData;
    return ret;
}


//______________________________________________________________________________________________________________________________________________________________________________________________________________
int Dialog_SaveAs(HWND hwndParent, HINSTANCE hinstance, int x, int y, const char* title, char** list, int numItems, int listPos, DWORD max_chars_selected_text, const char* excludeChars, char* retSelectedText, DWORD retSelectedText_numChars) {

    listDATA* listData = new listDATA(x, y, title, list, numItems, listPos, max_chars_selected_text, excludeChars);


    int ret = DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_LIST_SELECT_EDIT), hwndParent, DlgProc_SaveAs, (LPARAM)listData);

    if (retSelectedText)
        sprintf_s(retSelectedText, retSelectedText_numChars, "%S", listData->text_current_selection);

    delete listData;
    return ret;
}


//_____________________________________________________________________________________________________________________________________________________________________________________________________________________________
int Dialog_SaveAs_WC(HWND hwndParent, HINSTANCE hinstance, int x, int y, const wchar_t* title, wchar_t** list, int numItems, int listPos, DWORD max_chars_selected_text, const wchar_t* excludeChars, wchar_t* retSelectedText, DWORD retSelectedText_numChars) {

    listDATA* listData = new listDATA(x, y, title, list, numItems, listPos, max_chars_selected_text, excludeChars);


    int ret = DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_LIST_SELECT_EDIT), hwndParent, DlgProc_SaveAs, (LPARAM)listData);

    if (retSelectedText)
        swprintf_s(retSelectedText, retSelectedText_numChars, L"%s", listData->text_current_selection);

    delete listData;
    return ret;
}