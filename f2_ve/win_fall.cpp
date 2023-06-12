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

#include "resource.h"
#include "win_fall.h"

#include "modifications.h"
#include "memwrite.h"
#include "configTools.h"

#include "Fall_General.h"
#include "Fall_GameMap.h"
#include "Fall_File.h"

#include "Dx_Mouse.h"
#include "Dx_General.h"
#include "Dx_Graphics.h"
#include "Dx_Game.h"
#include "Dx_Windows.h"

#include "text.h"

#include "mapper\win_Mapper.h"


POINTERstate pointerState{0,0,0};

LONG SFALL_UseScrollWheel = 0;
LONG SFALL_MiddleMouse = 'B';


bool isGameMode = true;
bool isMapperExiting = false;

DWORD SCR_WIDTH = 640;
DWORD SCR_HEIGHT = 480;
LONG scaleLevel_GUI = 1;

//fallout screen rect
RECT* pFALL_RC = nullptr;

//fallout screen width
LONG* pFALL_WIDTH = nullptr;
//fallout screen height
LONG* pFALL_HEIGHT = nullptr;

HWND* phWinMain = nullptr;
HWND hGameWnd = nullptr;
HINSTANCE* phInstance = nullptr;
HHOOK* keyboardHook = nullptr;

LPSTR* ppCmdLineArgs = nullptr;

//mapper
HACCEL hAccelerators = nullptr;      // handle to accelerator table

const wchar_t windowName_Game[]= L"Fallout II";
const wchar_t* pWindowName = windowName_Game;

BOOL* p_is_winActive = nullptr;

void* pfall_win_exit = nullptr;
void* pfall_hook_input = nullptr;

bool isMapperSizing = false;

bool isWindowed = true;

bool isAltMouseInput = false;

bool isGrayScale = false;

wchar_t winClassName_Game[] = L"GNW95 Class";

//wchar_t* winClassName = winClassName_Game;


DWORD winStyle = WIN_MODE_STYLE;

int* pPointerX = nullptr;
int* pPointerY = nullptr;
int* pPointerXOffset = nullptr;
int* pPointerYOffset = nullptr;

double* pPointerSensitivity = nullptr;
double PointerSensitivityOne = 1.0f;

LONG* pPointerXTemp = nullptr;
LONG* pPointerYTemp = nullptr;

LONG* pPointerWidth = nullptr;
LONG* pPointerHeight = nullptr;


//____________________
BOOL Is_WindowActive() {
    if (!p_is_winActive) {
        Fallout_Debug_Error("!p_is_winActive");
        p_is_winActive = (BOOL*)FixAddress(0x51E444);
    }
    return *p_is_winActive;
}

//________________________________________
void Set_WindowActive_State(BOOL isActive) {
    if (!p_is_winActive) {
        Fallout_Debug_Error("!p_is_winActive");
        p_is_winActive = (BOOL*)FixAddress(0x51E444);
    }
    *p_is_winActive = isActive;
    SetWindowActivation(isActive);
}
/*
struct Fake_IDirectInputDeviceA {//just enough to make things work
    // IUnknown methods
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    //IDirectInputDeviceA methods 
    STDMETHOD(GetCapabilities)(THIS_ LPVOID) PURE;
    STDMETHOD(EnumObjects)(THIS_ LPVOID, LPVOID, DWORD) PURE;
    STDMETHOD(GetProperty)(THIS_ REFGUID, LPVOID) PURE;
    STDMETHOD(SetProperty)(THIS_ REFGUID, LPVOID) PURE;
    STDMETHOD(Acquire)(THIS) PURE;
    STDMETHOD(Unacquire)(THIS) PURE;
    STDMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID) PURE;
    STDMETHOD(GetDeviceData)(THIS_ DWORD, LPVOID, LPDWORD, DWORD) PURE;
    STDMETHOD(SetDataFormat)(THIS_ LPVOID) PURE;
    STDMETHOD(SetEventNotification)(THIS_ HANDLE) PURE;
    STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(GetObjectInfo)(THIS_ LPVOID, DWORD, DWORD) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPVOID) PURE;
    STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD) PURE;
    STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID) PURE;
};
Fake_IDirectInputDeviceA*** pppDinputMouseDevice = nullptr;


//__________________
BOOL Mouse_Acquire() {
    if (!*pppDinputMouseDevice || !**pppDinputMouseDevice)
        return FALSE;
    HRESULT hr = (**pppDinputMouseDevice)->Acquire();
    if (hr == S_FALSE)
        return FALSE;
    return TRUE;
}


//____________________
BOOL Mouse_Unacquire() {
    if (!*pppDinputMouseDevice || !**pppDinputMouseDevice)
        return FALSE;
    HRESULT hr = (**pppDinputMouseDevice)->Unacquire();
    if (hr == S_FALSE)
        return FALSE;
    return TRUE;
}


//_________________________________________
BOOL Mouse_SetCooperativeLevel(DWORD flags) {
    if (isAltMouseInput)
        return TRUE;

    if (!*pppDinputMouseDevice || !**pppDinputMouseDevice)
        return FALSE;
    
    (**pppDinputMouseDevice)->Unacquire();
    HRESULT hr = (**pppDinputMouseDevice)->SetCooperativeLevel(hGameWnd, flags);
    (**pppDinputMouseDevice)->Acquire();

    if (hr == S_FALSE)
        return FALSE;
    return TRUE;
}
*/

//_______________________
void ClipAltMouseCursor() {
    if (!isAltMouseInput)
        return;
    POINT p{ 0,0 };
    ClientToScreen(hGameWnd, &p);
    RECT rcClient;
    GetClientRect(hGameWnd, &rcClient);
    rcClient.left += p.x;
    rcClient.top += p.y;
    rcClient.right += p.x;
    rcClient.bottom += p.y;

    ClipCursor(&rcClient);
}


//_____________________________
LONG fall_HookInput(BOOL flag) {

    __asm {
        mov eax, flag
        call pfall_hook_input
    }
}


//_____________________________________
void SetWindowActivation(BOOL isActive) {

    //When fallout window loses focus, fullscreen mode needs to temporarily be put into windowed mode in order to appear on the taskbar and alt-tab display.
    if (!isWindowed) {
        if (isActive == FALSE && winStyle == WS_POPUP) {//Convert to windowed mode when app loses focus.
            winStyle = WIN_MODE_STYLE;
            SetWindowLongPtr(*phWinMain, GWL_EXSTYLE, 0);
            SetWindowLongPtr(*phWinMain, GWL_STYLE, winStyle | WS_VISIBLE);
            SetWindowPos(*phWinMain, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            ShowWindow(*phWinMain, SW_RESTORE);
        }
        else if (isActive && winStyle != WS_POPUP) {//Return to fullscreen mode when app regains focus.
            winStyle = WS_POPUP;
            SetWindowLongPtr(*phWinMain, GWL_EXSTYLE, 0);
            SetWindowLongPtr(*phWinMain, GWL_STYLE, winStyle | WS_VISIBLE);
            SetWindowPos(*phWinMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            ShowWindow(*phWinMain, SW_MAXIMIZE);
        }
    }

    fall_HookInput(isActive);
}


//_____________________________________________
void SetWindowTitle(HWND hwnd, const wchar_t* msg) {
    wchar_t winText[64];
    swprintf_s(winText, 64, L"%s  @%ix%ix%i   %s", pWindowName, SCR_WIDTH, SCR_HEIGHT, 1 * scaleLevel_GUI, msg);
    SendMessage(hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)winText);

}


//____________________
void WindowVars_Save() {

    if (ConfigReadInt(L"MAIN", L"WINDOWED", 0)) {
        WINDOWPLACEMENT winPlacement{0};
        winPlacement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hGameWnd, &winPlacement);
        ConfigWriteWinData(L"MAIN", L"WIN_DATA", &winPlacement);
    }
    ConfigWriteInt(L"MAPS", L"ZOOM_LEVEL", scaleLevel_Game);
}


//____________________
void Fallout_On_Exit() {
    Subtitles_Destroy();
    GameAreas_Destroy();
    WindowVars_Save();
    Destroy_FRM_DX_CACHE();
    MemWrite_DestroyMem();
    //Errors_DestroyMem();
}


//________________________________________
 void __declspec(naked) exit_fallout(void) {
     __asm {
         pushad
         call Fallout_On_Exit
         popad
         ret
     }
 }


//____________________
void WindowVars_Load() {

    scaleLevel_GUI = ConfigReadInt(L"MAIN", L"SCALE_LEVEL", 0);
    if (scaleLevel_GUI == 0)
        scaleLevel_GUI = 1;

    if (ConfigReadInt(L"EFFECTS", L"IS_GRAY_SCALE", 0))
        isGrayScale = true;
    else
        isGrayScale = false;
}


//_________________________
void fall_WinExit(LONG val) {

    __asm {
        mov eax, val
        call pfall_win_exit
    }
}


//___________________________________________________________
void Set_Fallout_Screen_Dimensions(DWORD width, DWORD height) {
    //Fallout_Debug_Info("Set_Fallout_Screen_Dimensions w%d h%d", width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    pFALL_RC->left = 0;
    pFALL_RC->top = 0;
    pFALL_RC->right = SCR_WIDTH - 1;
    pFALL_RC->bottom = SCR_HEIGHT - 1;
    *pFALL_WIDTH = SCR_WIDTH;
    *pFALL_HEIGHT = SCR_HEIGHT;
}


//________________________________________
//direction - true equals up, false equals down
bool GUI_ScaleLevel_Adjust(bool direction) {
    LONG new_scaleLevel_GUI = scaleLevel_GUI;
    if (direction)
        new_scaleLevel_GUI++;
    else {
        if (scaleLevel_GUI == 1)
            return false;
        new_scaleLevel_GUI--;
    }

    
    WINDOWINFO windowInfo {0};
    windowInfo.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(hGameWnd, &windowInfo);

    RECT winRect{ 0,0,0,0 };

    int CLIENT_WIDTH = windowInfo.rcClient.right - windowInfo.rcClient.left;
    int CLIENT_HEIGHT = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
    if ((CLIENT_WIDTH / new_scaleLevel_GUI) < 640 || (CLIENT_HEIGHT / new_scaleLevel_GUI) < 480)
        return false;

    scaleLevel_GUI = new_scaleLevel_GUI;
    ConfigWriteInt(L"MAIN", L"SCALE_LEVEL", scaleLevel_GUI);

    DWORD old_SCR_WIDTH = SCR_WIDTH;
    DWORD old_SCR_HEIGHT = SCR_HEIGHT;
    Set_Fallout_Screen_Dimensions(CLIENT_WIDTH / scaleLevel_GUI, CLIENT_HEIGHT / scaleLevel_GUI);

    bool isMapperSizing_temp = isMapperSizing;

    isMapperSizing = true;
    Set_ViewPort(SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI);
    Display_Release_RenderTargets();
    SetScreenProjectionMatrix_XM(SCR_WIDTH, SCR_HEIGHT);
    
    scaleSubUnit = 1.0f / scaleLevel_GUI;
    if (scaleLevel_Game < 1)
        scaleLevel_Game = 1;
    if (scaleLevel_Game < scaleLevel_GUI)
        scaleGame_RO = scaleSubUnit * (float)scaleLevel_Game;
    else
        scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
    ResizeGameWin();

    OnScreenResize_Windows(old_SCR_WIDTH, old_SCR_HEIGHT);
    isMapperSizing = isMapperSizing_temp;

    return true;
}


//____________________________
BOOL GUI_ScaleLevel_Increase() {

    if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
        return FALSE;
    GUI_ScaleLevel_Adjust(true);
    return TRUE;
}


//_____________________________________________________
void __declspec(naked) h_gui_scale_level_increase(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call GUI_ScaleLevel_Increase

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        
        cmp eax, FALSE
        je set_brightness_instead

        pop ebp //ditch return for this function

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret

        set_brightness_instead:
        mov ebx, 0x3FF00000
        ret
    }

}


//____________________________
BOOL GUI_ScaleLevel_Decrease() {

    if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
        return FALSE;
    GUI_ScaleLevel_Adjust(false);
    return TRUE;
}


//_____________________________________________________
void __declspec(naked) h_gui_scale_level_decrease(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call GUI_ScaleLevel_Decrease

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        cmp eax, FALSE
        je set_brightness_instead

        pop ebp //ditch return for this function

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret

        set_brightness_instead :
        mov ebx, 0x3FF00000
            ret
    }

}


//___________________________________________________________________________________________
void Get_Client_Rect_From_Window_Rect(RECT* p_rc, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    //get the dimensions of the window frame style.
    RECT rc_style{ 0,0,0,0 };
    if (AdjustWindowRectEx(&rc_style, dwStyle, bMenu, dwExStyle)) {
        //subtract the window style rectangle leaving the client rectangle.
        p_rc->left -= rc_style.left;
        p_rc->top -= rc_style.top;
        p_rc->right -= rc_style.right;
        p_rc->bottom -= rc_style.bottom;
    }
}


//__________________________________________________________________________________________________________
bool Check_Window_GUI_Scaling_Limits(RECT* p_rc_win, LONG* p_scaleLevel_GUI, DWORD dwStyle, DWORD dwExStyle) {
    if (!p_rc_win || !p_scaleLevel_GUI)
        return false;
    bool resized = false;

    //get the preferred scale level from the ini .
    LONG scaleLevel_GUI_Temp = ConfigReadInt(L"MAIN", L"SCALE_LEVEL", 0);
    if (scaleLevel_GUI_Temp == 0)
        scaleLevel_GUI_Temp = 1;

    //get the dimensions of the window frame style.
    RECT rc_style{ 0,0,0,0 };
    AdjustWindowRectEx(&rc_style, dwStyle, false, dwExStyle);
    RECT rc_client;
    CopyRect(&rc_client, p_rc_win);
    //subtract the window style rectangle leaving the client rectangle.
    rc_client.left -= rc_style.left;
    rc_client.top -= rc_style.top;
    rc_client.right -= rc_style.right;
    rc_client.bottom -= rc_style.bottom;


    LONG client_width = rc_client.right - rc_client.left;
    LONG client_height = rc_client.bottom - rc_client.top;

    //prevent window dimensions going beyond what is supported by your graphics card.
    if (client_width > (LONG)max_texDim || client_height > (LONG)max_texDim) {
        if (client_width > (LONG)max_texDim)
            client_width = (LONG)max_texDim;
        if (client_height > (LONG)max_texDim)
            client_height = (LONG)max_texDim;
        rc_client.right = rc_client.left + client_width;
        rc_client.bottom = rc_client.top + client_height;
        //add the client and style rects to get the window rect.
        p_rc_win->left = rc_client.left + rc_style.left;
        p_rc_win->top = rc_client.top + rc_style.top;
        p_rc_win->right = rc_client.right + rc_style.right;
        p_rc_win->bottom = rc_client.bottom + rc_style.bottom;
        resized = true;
    }

    LONG game_width = client_width / scaleLevel_GUI_Temp;
    LONG game_height = client_height / scaleLevel_GUI_Temp;

    //prevent window dimensions going under the minumum values of 640x480.
    if (game_width < 640 || game_height < 480) {
        //decrement the scale level until dimensions are equal to or greater than minumum values.
        while (scaleLevel_GUI_Temp >= 1 && (game_width < 640 || game_height < 480)) {

            if (scaleLevel_GUI_Temp == 1) {//if level 1 is still too low, set dimensions to minumum values.
                client_width = 640;
                client_height = 480;
                rc_client.right = rc_client.left + client_width;
                rc_client.bottom = rc_client.top + client_height;
                //add the client and style rects to get the window rect.
                p_rc_win->left = rc_client.left + rc_style.left;
                p_rc_win->top = rc_client.top + rc_style.top;
                p_rc_win->right = rc_client.right + rc_style.right;
                p_rc_win->bottom = rc_client.bottom + rc_style.bottom;
                resized = true;
            }
            else
                scaleLevel_GUI_Temp--;
            game_width = client_width / scaleLevel_GUI_Temp;
            game_height = client_height / scaleLevel_GUI_Temp;
        }

    }
    *p_scaleLevel_GUI = scaleLevel_GUI_Temp;
    return resized;
}


//___________________________________________
bool Set_Window_Size_On_First_Show(HWND hwnd) {

    //run this function once on first WM_SHOWWINDOW.
    static bool win_first_show = true;
    if (!win_first_show)
        return false;
    win_first_show = false;

    if (hwnd == nullptr)
        return false;

    hGameWnd = hwnd;

    if (ConfigReadInt(L"MAIN", L"WINDOWED", 0)) {
        isWindowed = true;
        WINDOWPLACEMENT winPlace{0};
        winPlace.length = sizeof(WINDOWPLACEMENT);

        if (ConfigReadWinData(L"MAIN", L"WIN_DATA", &winPlace)) {
            if (winPlace.showCmd != SW_MAXIMIZE)
                winPlace.showCmd = SW_SHOWNORMAL;
        }
        else {
            GetWindowPlacement(hwnd, &winPlace);
            winPlace.showCmd = SW_SHOWNORMAL;
        }
        if (winPlace.showCmd == SW_SHOWNORMAL) {//if the window isn't maximized
            Check_Window_GUI_Scaling_Limits(&winPlace.rcNormalPosition, &scaleLevel_GUI, winStyle, 0);
            SetWindowPlacement(hwnd, &winPlace);
            return true;
        }
        // if the window is maximized.
        SetWindowPlacement(hwnd, &winPlace);
    }
    else {//create a fullscreen borderless window.
        isWindowed = false;
        winStyle = WS_POPUP;
        SetWindowLongPtr(hwnd, GWL_STYLE, winStyle);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, 0);
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    return true;
}


//_____________________
bool CreateMainWindow() {
    //Create a window with a border, buttons and a 640 x 480 client.
    //The window position, size and style are configured on first show.

    HINSTANCE hinstance = *phInstance;
    HMENU hmenu = nullptr;
    wchar_t* winClassName = winClassName_Game;


    if (isRunAsMapper) {
        hinstance = phinstDLL;
        hmenu = (HMENU)IDR_MENU1;
        hAccelerators = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
        winClassName = winClassName_MapperMain;
    }
    winStyle = WIN_MODE_STYLE;
    RECT rc_win{ 0,0,640,480 };
    AdjustWindowRectEx(&rc_win, winStyle, false, 0);

    *phWinMain = CreateWindowEx(0, winClassName, pWindowName,
        winStyle,
        0, 0, rc_win.right - rc_win.left, rc_win.bottom - rc_win.top,
        HWND_DESKTOP, nullptr, hinstance, nullptr);

    if (*phWinMain == nullptr)
        return false;

    ShowWindow(*phWinMain, SW_SHOW);

    if (pGvx)
        pGvx->LoadFontLib(*phWinMain);

    return true;
}


//_________________
int CheckMessages() {
    MSG msg;            // application messages
    MsgWaitForMultipleObjectsEx(0, nullptr, 1, QS_ALLINPUT, 0);
    while (PeekMessage(&msg, nullptr, 0, 0, 0)) {
        if ((GetMessage(&msg, nullptr, 0, 0)) != 0) {
            if (isGameMode) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else if (!hDlgCurrent || !IsDialogMessage(hDlgCurrent, &msg)) {
                if (!TranslateAccelerator(*phWinMain, hAccelerators, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    return 0;
}


//_______________________
int CheckMessagesNoWait() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, 0)) {
        if ((GetMessage(&msg, nullptr, 0, 0)) != 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

        }
    }
    return 0;
}


//__________________________
void Wait_ms(DWORD delay_ms) {
    ULONGLONG startTick = GetTickCount64();
    ULONGLONG currentTick = 0;
    while (startTick + delay_ms > currentTick) {
        CheckMessages();
        currentTick = GetTickCount64();
        if (currentTick < startTick)
            return;
    }
    return;
}


//_________________________________________
void __declspec(naked) check_messages(void) {

    __asm {
        call CheckMessages
        add esp, 0x20
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//____________________________________________
void __declspec(naked) check_messages_CH(void) {

    __asm {
        call CheckMessages
        mov esp, ebp
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//____________________________________________
void __declspec(naked) check_combat_msgs(void) {

    __asm {
        pushad
        call CheckMessages
        popad
        call fall_Process_Input
        ret
    }

}


//_________________
LONG DisplaySetup() {
    if (!CreateMainWindow())
        return -1;
    if (DxSetup())
        return -1;

    UpdateWindow(*phWinMain);
    SetFocus(*phWinMain);
    return 0;
}


//________________________________________
void __declspec(naked) display_setup(void) {
    __asm {
        push ecx
        push esi
        push edi
        push ebp

        call DisplaySetup

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//___________________
int CheckClientRect() {
    //check if mouse within client rect.
    RECT rcClient;
    POINT p{ 0,0 }, m{ 0,0 };

    GetCursorPos(&m);

    ClientToScreen(hGameWnd, &p);
    GetClientRect(hGameWnd, &rcClient);

    rcClient.left += p.x;
    rcClient.top += p.y;
    rcClient.right += p.x;
    rcClient.bottom += p.y;


    if (m.x < rcClient.left || m.x > rcClient.right)
        return 1;
    if (m.y < rcClient.top || m.y > rcClient.bottom)
        return 1;
    return 0;
}


//________________________
void SetRelativeMousePos() {

    int xPos = *pPointerX + *pPointerXOffset;
    int yPos = *pPointerY + *pPointerYOffset;

    if (!isAltMouseInput) {
        RECT rcClient;
        POINT p{ 0,0 }, m{ 0,0 };

        GetCursorPos(&m);
        ClientToScreen(hGameWnd, &p);
        GetClientRect(hGameWnd, &rcClient);

        xPos = (m.x - p.x) / scaleLevel_GUI;
        yPos = (m.y - p.y) / scaleLevel_GUI;
    }

    if (xPos < pFALL_RC->left)
        xPos = pFALL_RC->left;
    else if (xPos > pFALL_RC->right)
        xPos = pFALL_RC->right;
    *pPointerX = xPos - *pPointerXOffset;

    if (yPos < pFALL_RC->top)
        yPos = pFALL_RC->top;
    else if (yPos > pFALL_RC->bottom)
        yPos = pFALL_RC->bottom;
    *pPointerY = yPos - *pPointerYOffset;

    return;
}


//_____________________________________________
void __declspec(naked) set_relative_mouse(void) {
    __asm {
        push ebx
        push ecx
        push edx
        push ebp
        call SetRelativeMousePos
        pop ebp
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________________________
LONG GetWinAtPos(LONG xPos, LONG yPos) {
    long num = *p_num_windows - 1;
    if (num) {
        int cflags = FLG_WinHidden | FLG_WinTrans;
        do {
            WinStruct* win = pWin_Array[num];
            if (xPos >= win->rect.left && xPos <= win->rect.right && yPos >= win->rect.top && yPos <= win->rect.bottom) {
                if (!(win->flags & cflags)) {
                    return win->ref;
                }
            }
        } while (--num);
    }
    return pWin_Array[0]->ref;
}


//___________________________________
BOOL IsMouseInButtonRect(int buttRef) {
    WinStruct* pWin = nullptr;
    ButtonStruct* pButt = fall_Button_Get(buttRef, &pWin);
    if (pWin && pButt) {
        LONG mouseX = *pPointerX + *pPointerXOffset - pWin->rect.left;
        LONG mouseY = *pPointerY + *pPointerYOffset - pWin->rect.top;
        if (mouseX >= pButt->rect.left && mouseX <= pButt->rect.right && mouseY >= pButt->rect.top && mouseY <= pButt->rect.bottom)
            return TRUE;
    }
    return FALSE;
}


//_________________________________________________________________________________
BOOL IsMouseInWindowRect(LONG winRef, LONG left, LONG top, LONG right, LONG bottom) {
    WinStruct* pWin = fall_Win_Get(winRef);
    if (!pWin)
        return FALSE;

    LONG mouseX = *pPointerX + *pPointerXOffset - pWin->rect.left;
    LONG mouseY = *pPointerY + *pPointerYOffset - pWin->rect.top;
    if (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom)
        return TRUE;

    return FALSE;
}


//_______________________________________________
BOOL IsMouseInWindowRect2(LONG winRef, RECT* p_rc) {
    WinStruct* pWin = fall_Win_Get(winRef);
    if (!pWin)
        return FALSE;

    LONG mouseX = *pPointerX + *pPointerXOffset - pWin->rect.left;
    LONG mouseY = *pPointerY + *pPointerYOffset - pWin->rect.top;
    if (mouseX >= p_rc->left && mouseX <= p_rc->right && mouseY >= p_rc->top && mouseY <= p_rc->bottom)
        return TRUE;

    return FALSE;
}


//______________________________________________________________
BOOL IsMouseInRect(LONG left, LONG top, LONG right, LONG bottom) {

    LONG mouseX = *pPointerX + *pPointerXOffset;
    LONG mouseY = *pPointerY + *pPointerYOffset;
    if (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom)
        return TRUE;

    return FALSE;
}


//____________________________
BOOL IsMouseInRect(RECT* p_rc) {
    if (!p_rc)
        return TRUE;
    LONG mouseX = *pPointerX + *pPointerXOffset;
    LONG mouseY = *pPointerY + *pPointerYOffset;
    if (mouseX >= p_rc->left && mouseX <= p_rc->right && mouseY >= p_rc->top && mouseY <= p_rc->bottom)
        return true;

    return FALSE;
}


//________________________________________
void GetMousePos(LONG* pXPos, LONG* pYPos) {
    *pXPos = *pPointerX + *pPointerXOffset;
    *pYPos = *pPointerY + *pPointerYOffset;
}


//_____________________
LONG GetMouseWinAtPos() {
    LONG xPos = *pPointerX + *pPointerXOffset;
    LONG yPos = *pPointerY + *pPointerYOffset;
    return GetWinAtPos(xPos, yPos);
}


//_______________________________________________
void __declspec(naked) get_mouse_win_at_pos(void) {
    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp
        call GetMouseWinAtPos
        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_______________________________________
void LockMouseInWin(LONG xPos, LONG yPos) {

    if (xPos < pFALL_RC->left)
        xPos = pFALL_RC->left;
    else if (xPos > pFALL_RC->right)
        xPos = pFALL_RC->right;

    if (yPos < pFALL_RC->top)
        yPos = pFALL_RC->top;
    else if (yPos > pFALL_RC->bottom)
        yPos = pFALL_RC->bottom;

    *pPointerX = xPos - *pPointerXOffset;
    *pPointerY = yPos - *pPointerYOffset;
    *pPointerXTemp = *pPointerX;
    *pPointerYTemp = *pPointerY;
}


//____________________________________
void SetMousePos(LONG xPos, LONG yPos) {

    LockMouseInWin(xPos, yPos);

    POINT pClient{ 0,0 };
    ClientToScreen(hGameWnd, &pClient);

    xPos = (*pPointerX * scaleLevel_GUI) + pClient.x;
    yPos = (*pPointerY * scaleLevel_GUI) + pClient.y;

    SetCursorPos(xPos, yPos);
    MouseDX_Show();
}


//________________________________________
void __declspec(naked) set_mouse_pos(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call SetMousePos
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//________________________________________
void SetMousePosGame(LONG xPos, LONG yPos) {
    SetMousePos((LONG)((float)xPos * (float)scaleGame_RO), (LONG)((float)yPos * (float)scaleGame_RO));
    return;
}


//_____________________________________________
void __declspec(naked) set_mouse_pos_game(void) {
    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call SetMousePosGame
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//________________________________________
LONG GetPointerState(POINTERstate* pState) {
    POINT p_mouse;
    GetCursorPos(&p_mouse);

    POINT p_win{ 0,0 };
    ClientToScreen(hGameWnd, &p_win);
    p_mouse.x -= p_win.x;
    p_mouse.y -= p_win.y;

    pState->x = (p_mouse.x / scaleLevel_GUI) - *pPointerX - *pPointerXOffset;
    pState->y = (p_mouse.y / scaleLevel_GUI) - *pPointerY - *pPointerYOffset;
    pState->flags = pointerState.flags;

    return 1;
}


//____________________________________________
void __declspec(naked) get_pointer_state(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push ebp

        push eax
        call GetPointerState
        add esp, 0x4

        pop ebp
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//______________________________________
void SetWindowPointerPos(POINT* pointer) {

    POINT p{ 0,0 }, m{ 0,0 };

    ClientToScreen(hGameWnd, &p);
    m.x = pointer->x;
    m.x -= p.x;
    m.y = pointer->y;
    m.y -= p.y;

    pointerState.x = (m.x / scaleLevel_GUI) - *pPointerX - *pPointerXOffset;
    pointerState.y = (m.y / scaleLevel_GUI) - *pPointerY - *pPointerYOffset;
}


//__________________________________________________________________________________
LRESULT CALLBACK WinProc_Game(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static bool is_cursor_hidden = true;
    switch (Message) {
    case WM_LBUTTONDOWN:
        pointerState.flags = pointerState.flags | 0x1;
        return 0;
    case WM_RBUTTONDOWN:
        pointerState.flags = pointerState.flags | 0x100;
        return 0;
    case WM_LBUTTONUP:
        pointerState.flags = pointerState.flags & 0xFFFFFFFE;
        return 0;
    case WM_RBUTTONUP:
        pointerState.flags = pointerState.flags & 0xFFFFFEFF;
        return 0;
    case WM_MBUTTONDOWN: {
        if (!isAltMouseInput)
            break;
        if (Mouse_Wheel_Imonitor(0, false))//check if mouse over imonitor and return if so.
            return 0;
        if (SFALL_MiddleMouse != 0) {
            fall_SendKey(SFALL_MiddleMouse);//otherwise send sfalls middle mouse key.
            return 0;
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        int fwKeys = GET_KEYSTATE_WPARAM(wParam);
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        bool setTrans = false;
        if (fwKeys & MK_CONTROL)
            setTrans = true;
        if (Mouse_Wheel_GameWindow(zDelta, setTrans))
            return 0;
        if (Mouse_Wheel_WorldMap(zDelta))
            return 0;

        if (!isAltMouseInput)//the functions below will conflict with sfall.
            break;
        bool scrollPage = false;
        if (fwKeys & MK_MBUTTON)
            scrollPage = true;
        if (Mouse_Wheel_Imonitor(zDelta, scrollPage))
            return 0;
        int keyCode = -1;
        int pageSize = 0;
        if (Mouse_Wheel_Inventory(zDelta, &keyCode, &pageSize)) {
            if (!(fwKeys & MK_MBUTTON))//if middle button NOT down, scroll one item only
                pageSize = 1;
            for (int i = 0; i < pageSize; i++)
                fall_SendKey(keyCode);
            return 0;
        }
        break;
    }
    case WM_SHOWWINDOW:
        if (Set_Window_Size_On_First_Show(hwnd)) {
            //paint the window client black, otherwise it appears white during the rest of the fallout setup. 
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
    case WM_WINDOWPOSCHANGING: {
        WINDOWPOS* winpos = (WINDOWPOS*)lParam;
        //Fallout_Debug_Info("WM_WINDOWPOSCHANGING flags=%X", winpos->flags);
        if (winpos->flags & (SWP_NOACTIVATE | SWP_NOSIZE))
            return 0;
        RECT rcWindow = { winpos->x, winpos->y, winpos->x + winpos->cx, winpos->y + winpos->cy };
        Check_Window_GUI_Scaling_Limits(&rcWindow, &scaleLevel_GUI, winStyle, 0);
        winpos->x = rcWindow.left;
        winpos->y = rcWindow.top;
        winpos->cx = rcWindow.right - rcWindow.left;
        winpos->cy = rcWindow.bottom - rcWindow.top;

        //Fallout_Debug_Info("WM_WINDOWPOSCHANGING scaleLevel_GUI %d", scaleLevel_GUI);

        return 0;
    }
    case WM_SIZING: {
        //Fallout_Debug_Info("WM_SIZING");
        RECT* rcWindow = (RECT*)lParam;
        Check_Window_GUI_Scaling_Limits(rcWindow, &scaleLevel_GUI, winStyle, 0);
        return TRUE;
    }
    case WM_WINDOWPOSCHANGED: {
        if (IsIconic(hwnd))
            break;
        WINDOWPOS* winpos = (WINDOWPOS*)lParam;
        if (winpos->flags & (SWP_NOSIZE)) {
            SetWindowTitle(hwnd, L"");
            return 0;
        }
        //Fallout_Debug_Info("WM_WINDOWPOSCHANGED");
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        DWORD old_SCR_WIDTH = SCR_WIDTH;
        DWORD old_SCR_HEIGHT = SCR_HEIGHT;
        Set_Fallout_Screen_Dimensions((rcClient.right - rcClient.left) / scaleLevel_GUI, (rcClient.bottom - rcClient.top) / scaleLevel_GUI);

        bool isMapperSizing_temp = isMapperSizing;
        isMapperSizing = true;
        ReSizeDisplayEx();
        OnScreenResize_Windows(old_SCR_WIDTH, old_SCR_HEIGHT);
        isMapperSizing = isMapperSizing_temp;

        SetWindowTitle(hwnd, L"");
        return 0;
    }
    case WM_SIZE: {
        switch ((wParam)) {
        case SIZE_MINIMIZED:
            break;
        case SIZE_RESTORED:
            break;
        case SIZE_MAXIMIZED:
            break;
        default:
            break;
        }
        if (IsIconic(hwnd))
            break;
        //Fallout_Debug_Info("WM_SIZE");

        DWORD old_SCR_WIDTH = SCR_WIDTH;
        DWORD old_SCR_HEIGHT = SCR_HEIGHT;

        Set_Fallout_Screen_Dimensions((LOWORD(lParam)) / scaleLevel_GUI, (HIWORD(lParam)) / scaleLevel_GUI);

        bool isMapperSizing_temp = isMapperSizing;
        isMapperSizing = true;

        ReSizeDisplayEx();
        OnScreenResize_Windows(old_SCR_WIDTH, old_SCR_HEIGHT);
        isMapperSizing = isMapperSizing_temp;

        SetWindowTitle(hwnd, L"");
        return 0;
    }
    case WM_CLOSE: {
        if (IsIconic(hwnd)) {//restore window first - game needs focus to exit
            if (SetForegroundWindow(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
        }
        if (IsSplash() || IsMainMenu()) {//if on the splash screen or main menu - just exit.
            *pGAME_EXIT_FLAGS = 3;//this will quit main-menu or skip intro movies if splash screen is running.
            MainMenu_SetToExit();//this will cause main-menu to abort loading and quit after splash screen has finished.
        }
        else {//if in the game - ask before exiting.
            if (fall_ExitMessageBox() == 1)
                MainMenu_SetToExit();//set main-menu to quit before loading.
        }
        return 0;
    }
    case WM_ENTERMENULOOP://allows system menu keys to fuction
        //*p_is_winActive = FALSE;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(FALSE);
        break;
    case WM_EXITMENULOOP:
        //*p_is_winActive = TRUE;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(TRUE);
    break;    case WM_DISPLAYCHANGE:
        break;
    case WM_COMMAND:
        break;
    case WM_SYSCOMMAND:
        switch ((wParam & 0xFFF0)) {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return 0;
        default:
            break;
        }
        break;
    case WM_SETCURSOR: {
        DWORD currentWinStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        if (!(currentWinStyle & (WIN_MODE_STYLE | WS_OVERLAPPED | WS_BORDER))) {
            ClipAltMouseCursor();
            break;
        }

        WORD ht = LOWORD(lParam);
        if (HTCLIENT == ht) {
            ClipCursor(nullptr);
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            if (!isGameMode)
                break;
            if (!CheckClientRect()) {
                if (!is_cursor_hidden) {
                    is_cursor_hidden = true;
                    ShowCursor(false);
                }
            }
            else {
                if (is_cursor_hidden) {
                    is_cursor_hidden = false;
                    ShowCursor(true);
                }
            }
        }
        else {
            if (is_cursor_hidden) {
                is_cursor_hidden = false;
                ShowCursor(true);
            }
        }
        break;
    }
    case WM_ACTIVATEAPP:
        //*p_is_winActive = wParam;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(wParam);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        fall_WinExit(0);
        return 1;
    case WM_PAINT:
        /*Fallout_Debug_Info("WM_PAINT");
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.
            FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

            EndPaint(hwnd, &ps);
        }
        return 0;*/
        //if (isMapperSizing)
        //    break;
        //if (GetUpdateRect(hwnd, nullptr, 0))
        //    fall_Windows_Draw_Rect(pFALL_RC);
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, Message, wParam, lParam);
}


//_______________________________________________________________________
LRESULT CALLBACK KeyboardProcMain(int code, WPARAM wParam, LPARAM lParam) {
    if (code < 0)
        return CallNextHookEx(*keyboardHook, code, wParam, lParam);

    switch (wParam) {
    case VK_DELETE:
        if (!(lParam & KF_MENUMODE))
            break;
    case VK_ESCAPE:
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            return 0;
        break;
    case VK_TAB:
        if ((lParam & KF_MENUMODE))
            return 0;
        break;
    case VK_F4:
        if ((lParam & KF_MENUMODE))
            return 0;
        break;
    case VK_SPACE:
        if ((lParam & KF_MENUMODE)) {
            return 0;
        }
        break;

    case VK_NUMLOCK:
    case VK_CAPITAL:
    case VK_SCROLL:
        break;
    default: {
        if (isGameMode)
            return 1;
        return 0;
        break;
    }
    }

    return CallNextHookEx(*keyboardHook, code, wParam, lParam);
}

/*
//__________________________________________________________________________
int SetHiResDataPath(char* path1, int isFolder1, char* path2, int isFolder2) {

    if (fall_SetDataPath(path1, isFolder1, path2, isFolder2))
        return -1;

    wchar_t datPathw[MAX_PATH];
    wchar_t patchesPathw[MAX_PATH];
    ConfigReadString(L"MAIN", L"f2_res_dat", L"f2_res.dat", datPathw, MAX_PATH);
    ConfigReadString(L"MAIN", L"f2_res_patches", L"data\\", patchesPathw, MAX_PATH);

    size_t retSize;
    char datPath[MAX_PATH];
    char patchesPath[MAX_PATH];
    if (!wcstombs_s(&retSize, datPath, MAX_PATH, datPathw, MAX_PATH) && !wcstombs_s(&retSize, patchesPath, MAX_PATH, patchesPathw, MAX_PATH))
        fall_SetDataPath(datPath, 0, patchesPath, 1);

    return 0;
}


//________________________________________
void __declspec(naked) set_data_path(void) {
    __asm {
        push esi
        push edi
        push ebp

        push ecx
        push ebx
        push edx
        push eax
        call SetHiResDataPath
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret
    }
}
*/

#define DOUBLE_CLICK_TIME   400
bool isDoubleClick = false;


//_______________________
bool IsMouseDoubleClick() {
    return isDoubleClick;
}


//__________________________________
void Check_Double_Click(DWORD click) {
    isDoubleClick = false;
    if (IsMouseHidden())
        return;

    ULONGLONG newTick = GetTickCount64();
    static ULONGLONG startTick = newTick;

    static LONG xPos = 0;
    static LONG yPos = 0;

    static int dc_sequence = 0;

    if (newTick - startTick > DOUBLE_CLICK_TIME)
        dc_sequence = 0;

    if ((click & 1)) {
        if (dc_sequence == 0) {//first click down
            startTick = newTick;
            dc_sequence = 1;
            isDoubleClick = false;
            xPos = *pPointerX + *pPointerXOffset;
            yPos = *pPointerY + *pPointerYOffset;
        }
        else if (dc_sequence == 2) {//second click down
            dc_sequence++;
            if (xPos != *pPointerX + *pPointerXOffset || yPos != *pPointerY + *pPointerYOffset)//check if the mouse pos matches the first click
                dc_sequence = 0;
        }
    }
    else {
        if (dc_sequence == 1)//first clip up
            dc_sequence++;
        else if (dc_sequence == 3) {//second click up
            dc_sequence = 0;
            isDoubleClick = true;
        }
    }
}


//_______________________________________________
void __declspec(naked) h_check_double_click(void) {

    __asm {
        pushad
        push ebx
        call Check_Double_Click
        add esp, 0x4
        popad

        and eax, 0xFF
        ret
    }
}


//___________________________________________________________________________
ATOM Check_RunAsMapper_RegisterClass(HINSTANCE hinstance, LPSTR pCmdLineArgs) {
    //Fallout_Debug_Info("pCmdLine arguments: %s", pCmdLineArgs);
    if (strstr(pCmdLineArgs, "/mapper")) {
        Modifications_Mapper();
        ShowCursor(true);
        isRunAsMapper = true;
        isGameMode = false;
        pWindowName = windowName_Mapper;
        MapperMain_RegisterClass();
    }


    WNDCLASSEX WndClass{ 0 };
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.lpfnWndProc = &WinProc_Game;
    WndClass.hInstance = hinstance;
    WndClass.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(99));
    WndClass.hCursor = nullptr;
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = nullptr;
    WndClass.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(99));
    WndClass.lpszClassName = winClassName_Game;

    return RegisterClassEx(&WndClass);
}


//_____________________________________________________________
void __declspec(naked) check_run_as_mapper_register_class(void) {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push esi
        push eax
        call Check_RunAsMapper_RegisterClass
        add esp, 0x8
        and eax, 0x0000FFFF

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        ret
    }
}


//_________________________
void Modifications_Win_CH() {
    //To-Do Modifications_Win_CH

    ///FuncWrite32(0x443AC5, 0x0810F0, (DWORD)&set_data_path);

    pFALL_RC = (RECT*)0x6BCF5C;

    phWinMain = (HWND*)0x52E210;
    phInstance = (HINSTANCE*)0x52E214;

    p_is_winActive = (BOOL*)0x52E220;

    pfall_win_exit = (void*)0x4F1EFE;

    pfall_hook_input = (void*)0x4CAC7B;

    //default display mode dimensions---------------------------------------
    MemWrite32(0x4CC630, 640, SCR_WIDTH);
    MemWrite32(0x4CC62B, 480, SCR_HEIGHT);

    MemWrite32(0x4E6FE6, 0x4E730E, (DWORD)&WinProc_Game);


    if (ConfigReadInt(L"OTHER_SETTINGS", L"CPU_USAGE_FIX", 0)) {
        MemWrite8(0x4CAF7B, 0x6A, 0xE9);
        FuncWrite32(0x4CAF7C, 0x6A006A00, (DWORD)&check_messages_CH);
    }




    ///done///004CC9DC  |.  E8 FD000000       CALL 004CCADE                            ; window_setup()
    ///FuncWrite32(0x4CC9DD, 0xFD,  (DWORD)&window_setup);


     //fix mouse window lock
    MemWrite8(0x4EA01A, 0x05, 0x06);

    ///pppDinputMouseDevice = (Fake_IDirectInputDeviceA***)0x52E24C;

    pPointerSensitivity = (double*)FixAddress(0x52E080);

    pPointerX = (int*)0x6BCD14;
    pPointerY = (int*)0x6BCD10;
    pPointerXOffset = (int*)0x6BCD3C;
    pPointerYOffset = (int*)0x6BCD38;

    FuncWrite32(0x4CBD99, 0x02D8, (DWORD)&set_relative_mouse);


    pFALL_WIDTH = (LONG*)0x6832FC;
    pFALL_HEIGHT = (LONG*)0x683308;

    MemWrite16(0x4B7E9E, 0x048B, 0x9090);
    MemWrite8(0x4B7EA0, 0xDD, 0xA1);
    MemWrite32(0x4B7EA1, 0x52DB10, (DWORD)&SCR_HEIGHT);

    MemWrite8(0x4B7EB2, 0x8B, 0x90);
    MemWrite16(0x4B7EB3, 0xDD0C, 0x0D8B);
    MemWrite32(0x4B7EB5, 0x52DB0C, (DWORD)&SCR_WIDTH);


    keyboardHook = (HHOOK*)0x6BCCC8;

    MemWrite32(0x4CAD02, 0x4CADA4, (DWORD)&KeyboardProcMain);


    MemWrite8(0x4CC021, 0x53, 0xE9);
    FuncWrite32(0x4CC022, 0x55575651, (DWORD)&set_mouse_pos);


    pPointerXTemp = (LONG*)0x6BCD00;
    pPointerYTemp = (LONG*)0x6BCD08;

    pPointerWidth = (LONG*)0x6BCD30;
    pPointerHeight = (LONG*)0x6BCD04;


    if (!SfallReadInt(L"Graphics", L"Mode", 0) && ConfigReadInt(L"INPUT", L"ALT_MOUSE_INPUT", 0)) {

        isAltMouseInput = true;
        ///create_mouse_DI_device()
        MemWrite8(0x4E9BB8, 0xE8, 0xB8);
        MemWrite32(0x4E9BB9, 0x041B, 0x1);

        //aquire mouse func
        MemWrite16(0x4E9C6B, 0x2874, 0x1FEB);

        //unaquire mouse
        MemWrite16(0x4E9CC7, 0x1D74, 0x14EB);

        MemWrite8(0x4E9CF9, 0x68, 0xE9);
        FuncWrite32(0x4E9CFA, 0x48, (DWORD)&get_pointer_state);

        MemWrite32(0x4CBA4C, 0x52E080, (DWORD)&PointerSensitivityOne);
        MemWrite32(0x4CBA5D, 0x52E080, (DWORD)&PointerSensitivityOne);
        MemWrite32(0x4CC422, 0x52E080, (DWORD)&PointerSensitivityOne);
        MemWrite32(0x4CC42A, 0x52E084, (DWORD)&PointerSensitivityOne + 0x4);
    }


    FuncReplace32(0x44256E, 0x0707AE, (DWORD)&exit_fallout);



    //to prevent windows "NOT RESPONDING" error. add messege checking to loops
    FuncReplace32(0x4225A6, 0x0A6DCF, (DWORD)&check_combat_msgs);

    if (ConfigReadInt(L"MAIN", L"GRAPHICS_MODE", 0) == 0)
        return;
    if (SfallReadInt(L"Graphics", L"Mode", 0))
        return;

    MemWrite8(0x4CC9C1, 0x51, 0xE9);
    FuncWrite32(0x4CC9C2, 0x89555756, (DWORD)&DisplaySetup);

}


//____________________________
void Modifications_Win_MULTI() {

    ///FuncReplace32(0x444255, 0x081AD7, (DWORD)&set_data_path);

    pFALL_RC = (RECT*)FixAddress(0x6AC9F0);


    phWinMain = (HWND*)FixAddress(0x51E434);
    phInstance = (HINSTANCE*)FixAddress(0x51E438);

    ppCmdLineArgs = (LPSTR*)FixAddress(0x51E43C);

    p_is_winActive = (BOOL*)FixAddress(0x51E444);

    pfall_win_exit = (void*)FixAddress(0x4E660F);
    pfall_hook_input = (void*)FixAddress(0x4C9BB4);

    ///default display mode dimensions---------------------------------------
    MemWrite32(0x4CAD6B, 640, SCR_WIDTH);
    MemWrite32(0x4CAD66, 480, SCR_HEIGHT);

    MemWrite32(0x4DE802, FixAddress(0x4DE9FC), (DWORD)&WinProc_Game);

    if (ConfigReadInt(L"OTHER_SETTINGS", L"CPU_USAGE_FIX", 0)) {
        MemWrite8(0x4C9DA9, 0x53, 0xE9);
        FuncWrite32(0x4C9DAA, 0x8D535353, (DWORD)&check_messages);
    }

    //fix mouse window lock
    MemWrite8(0x4E072C, 0x05, 0x06);

    ///pppDinputMouseDevice = (Fake_IDirectInputDeviceA***)FixAddress(0x51E45C);

    pPointerSensitivity = (double*)FixAddress(0x51E2A0);


    pPointerX = (int*)FixAddress(0x6AC7A8);
    pPointerY = (int*)FixAddress(0x6AC7A4);
    pPointerXOffset = (int*)FixAddress(0x6AC7D0);
    pPointerYOffset = (int*)FixAddress(0x6AC7CC);

    FuncReplace32(0x4CA89B, 0x0199, (DWORD)&set_relative_mouse);

    pFALL_WIDTH = (LONG*)FixAddress(0x672D7C);
    pFALL_HEIGHT = (LONG*)FixAddress(0x672D88);

    ///scrDimArray[].height
    MemWrite16(0x4B91DA, 0x048B, 0x9090);
    MemWrite8(0x4B91DC, 0xDD, 0xA1);
    MemWrite32(0x4B91DD, FixAddress(0x51DD20), (DWORD)&SCR_HEIGHT);

    ///scrDimArray[].width
    MemWrite8(0x4B91EE, 0x8B, 0x90);
    MemWrite16(0x4B91EF, 0xDD0C, 0x0D8B);
    MemWrite32(0x4B91F1, FixAddress(0x51DD1C), (DWORD)&SCR_WIDTH);

    keyboardHook = (HHOOK*)FixAddress(0x6AC758);
    MemWrite32(0x4C9BD9, FixAddress(0x4C9C4C), (DWORD)&KeyboardProcMain);


    MemWrite8(0x4CAA04, 0x53, 0x90);
    MemWrite16(0x4CAA05, 0x1D8B, 0xE990);
    FuncWrite32(0x4CAA07, FixAddress(0x6AC7D0), (DWORD)&set_mouse_pos);

    pPointerXTemp = (LONG*)FixAddress(0x6AC794);
    pPointerYTemp = (LONG*)FixAddress(0x6AC79C);
    pPointerWidth = (LONG*)FixAddress(0x6AC7C4);
    pPointerHeight = (LONG*)FixAddress(0x6AC798);


    if (ConfigReadInt(L"INPUT", L"ALT_MOUSE_INPUT", 0)) {
        isAltMouseInput = true;

        MemWrite8(0x4E042F, 0xE8, 0xB8);
        MemWrite32(0x4E0430, 0x02D8, 0x1);

        //aquire mouse func
        MemWrite16(0x4E04F2, 0x1974, 0x11EB);

        //unaquire mouse
        MemWrite16(0x4E051E, 0x1474, 0x0CEB);

        MemWrite8(0x4E053C, 0x53, 0xE9);
        FuncWrite32(0x4E053D, 0xEC835251, (DWORD)&get_pointer_state);

        MemWrite32(0x4CA60C, FixAddress(0x51E2A0), (DWORD)&PointerSensitivityOne);
        MemWrite32(0x4CAC6E, FixAddress(0x51E2A0), (DWORD)&PointerSensitivityOne);
    }

    FuncReplace32(0x442CFE, 0x07148E, (DWORD)&exit_fallout);



    //to prevent windows "NOT RESPONDING" error. add messege checking to loops
    FuncReplace32(0x4227E6, 0x0A63F2, (DWORD)&check_combat_msgs);

    MemWrite8(0x4CAE1C, 0x51, 0xE9);
    FuncWrite32(0x4CAE1D, 0x89555756, (DWORD)&display_setup);

    FuncReplace32(0x44C6C6, 0x07E33A, (DWORD)&set_mouse_pos_game);



    MemWrite8(0x4CA63C, 0x25, 0xE8);
    FuncWrite32(0x4CA63D, 0xFF, (DWORD)&h_check_double_click);



    //004928EA | .BB 0000F03F   MOV EBX, 3FF00000
    MemWrite8(0x4928EA, 0xBB, 0xE8);
    FuncWrite32(0x4928EB, 0x3FF00000, (DWORD)&h_gui_scale_level_increase);

    //004929CE | .BB 0000F03F   MOV EBX, 3FF00000
    MemWrite8(0x4929CE, 0xBB, 0xE8);
    FuncWrite32(0x4929CF, 0x3FF00000, (DWORD)&h_gui_scale_level_decrease);


    //004DE7B4 | .BB 01000000   MOV EBX, 1
    //MemWrite8(0x4DE7B4, 0xBB, 0xE8);
    //FuncWrite32(0x4DE7B5, 0x00000001, (DWORD)&check_run_as_mapper);

    //004DE73B | .E8 B4000000   CALL REGISTER_CLASS(EAX hInstance); [fallout2.REGISTER_CLASS(EAX hInstance), register_class(EAX hInstance)
    FuncReplace32(0x4DE73C, 0x000000B4, (DWORD)&check_run_as_mapper_register_class);
}


//______________________
void Modifications_Win() {
    SFALL_UseScrollWheel = SfallReadInt(L"Input", L"UseScrollWheel", 0);

    //convert the sfall middle button direct input key code into an ascii code, hopefully.
   UINT scancode = SfallReadInt(L"Input", L"MiddleMouse", 0x30);
    if (scancode != 0) {
        HKL layout = GetKeyboardLayout(0);
        BYTE state[256] = { 0 };
        UINT vk = MapVirtualKeyExA(scancode, MAPVK_VSC_TO_VK, layout);
        char Char[2] = { 0,0 };
        if (ToAsciiEx(vk, scancode, state, (WORD*)Char, 0, layout) == 1)
            SFALL_MiddleMouse = Char[0];
    }
    else
        SFALL_MiddleMouse = 0;

    WindowVars_Load();

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Win_CH();
    else
        Modifications_Win_MULTI();
}
