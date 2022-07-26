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
#include "Fall_Windows.h"
#include "memwrite.h"
#include "modifications.h"

#include "Dx_Game.h"
#include "Dx_Windows.h"


LONG* pWinRef_Char = nullptr;
LONG* pWinRef_Bio = nullptr;
LONG* pWinRef_DialogMain = nullptr;
LONG* pWinRef_DialogNpcText = nullptr;
LONG* pWinRef_DialogPcText = nullptr;
LONG* pWinRef_DialogBaseSub = nullptr;
LONG* pWinRef_Inventory = nullptr;
LONG* pWinRef_EndSlides = nullptr;
LONG* pWinRef_Iface = nullptr;
LONG* pWinRef_NotifyBar = nullptr;
LONG* pWinRef_Skills = nullptr;
LONG* pWinRef_LoadSave = nullptr;
LONG* pWinRef_MainMenu = nullptr;
LONG* pWinRef_Movies = nullptr;
LONG* pWinRef_Options = nullptr;
LONG* pWinRef_Pipboy = nullptr;
LONG* pWinRef_GameArea = nullptr;
LONG* pWinRef_WorldMap = nullptr;

WinStruct** pWin_Array = nullptr;
LONG* p_winRef_Index = nullptr;
LONG* p_num_windows = nullptr;
LONG* p_draw_window_flag = nullptr;

BYTE** pp_win_mainBuff = nullptr;


void* pfall_window_create = nullptr;
void* pfall_window_destroy = nullptr;
void* pfall_window_get = nullptr;
void* pfall_window_draw = nullptr;
void* pfall_window_show = nullptr;
void* pfall_window_hide = nullptr;
void* pfall_window_print_text = nullptr;
void* pfall_window_get_window_at_pos = nullptr;

void* pfall_windows_draw_rect = nullptr;

void* pfall_button_create = nullptr;
void* pfall_button_destroy = nullptr;
void* pfall_button_get = nullptr;
void* pfall_button_set_functions = nullptr;
void* pfall_button_set_sounds = nullptr;
void* pfall_button_disable = nullptr;
void* pfall_button_enable = nullptr;
void* pfall_button_set_toggle_array = nullptr;
void* pfall_button_set_toggle_state = nullptr;
void* pfall_button_draw = nullptr;



//____________________________________________________________________
LONG fall_Button_SetToggleState(LONG buttRef, LONG state, DWORD flags) {
    //state 0/1 = up/down, flags & 1 send key, flags & 2 = don't draw button.
    LONG retVal = 0;
    __asm {
        mov ebx, flags
        mov edx, state
        mov eax, buttRef
        call pfall_button_set_toggle_state
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________
LONG fall_Button_SetToggleArray(LONG numButtons, LONG* pButtonRefs) {
    LONG retVal = 0;
    __asm {
        mov edx, pButtonRefs
        mov eax, numButtons
        call pfall_button_set_toggle_array
        mov retVal, eax
    }
    return retVal;
}


//______________________________________
LONG fall_Button_Disable(LONG buttonRef) {
    LONG retVal = 0;
    __asm {
        mov eax, buttonRef
        call pfall_button_disable
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________
LONG fall_Button_Enable(LONG buttonRef) {
    LONG retVal = 0;
    __asm {
        mov eax, buttonRef
        call pfall_button_enable
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________
LONG fall_Win_GetWinAtPos(LONG x, LONG y) {
    int winRef = 0;
    __asm {
        mov edx, y
        mov eax, x
        call pfall_window_get_window_at_pos
        mov winRef, eax
    }
    return winRef;
}


//________________________________________________________________
void fall_Windows_Draw_Win(WinStruct* win, RECT* rect, BYTE* buff) {
    Draw_Window_OLD((WinStructDx*)win, rect, buff);
}


//_____________________________________
void fall_Windows_Draw_Rect(RECT* rect) {
    __asm {
        mov eax, rect
        CALL pfall_windows_draw_rect
    }
}


//__________________________________
WinStruct* fall_Win_Get(LONG winRef) {
    WinStruct* winStruct;
    __asm {
        mov eax, winRef
        CALL pfall_window_get
        mov winStruct, eax
    }
    return winStruct;
}


//_______________________________________________________________________________________________
LONG fall_Win_Create(LONG x, LONG y, DWORD width, DWORD height, DWORD BGColourIndex, DWORD flags) {
    LONG winRef;
    __asm {
        push flags
        push BGColourIndex
        mov ecx, height
        mov ebx, width
        mov eax, x
        mov edx, y
        call pfall_window_create
        mov winRef, eax
    }
    return winRef;
}


//________________________________
void fall_Win_Destroy(LONG winRef) {
    __asm {
        mov eax, winRef
        call pfall_window_destroy
    }
}


//__________________________________
BYTE* fall_Win_Get_Buff(LONG winRef) {
    WinStruct* win = fall_Win_Get(winRef);
    if (!win)return nullptr;
    return win->buff;
}


//_____________________________
void fall_Win_Show(LONG winRef) {
    __asm {
        mov eax, winRef
        call pfall_window_show
    }
}


//_____________________________
void fall_Win_Hide(LONG winRef) {
    __asm {
        mov eax, winRef
        call pfall_window_hide
    }
}


//_______________________________
void fall_Win_ReDraw(LONG winRef) {
    __asm {
        mov eax, winRef
        call pfall_window_draw
    }
}


//____________________________________________________________________________________________________________
void fall_Win_PrintText(LONG WinRef, const char* txtBuff, LONG txtWidth, LONG x, LONG y, DWORD palColourFlags) {
    __asm {
        push palColourFlags
        push y
        mov ecx, x
        mov ebx, txtWidth
        mov edx, txtBuff
        mov eax, WinRef
        call pfall_window_print_text
    }
}


//_______________________________________________________________________________________________________________________________________________________________________________________________________________
LONG fall_Button_Create(LONG winRef, LONG x, LONG y, LONG width, LONG height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, BYTE* upPicBuff, BYTE* downPicBuff, BYTE* hoverPicBuff, DWORD flags) {
    LONG buttonRef;
    __asm {
        push flags
        push hoverPicBuff
        push downPicBuff
        push upPicBuff
        push keyLift
        push keyPush
        push keyHoverOff
        push keyHoverOn
        push height
        mov ecx, width
        mov ebx, y
        mov edx, x
        mov eax, winRef
        call pfall_button_create
        mov buttonRef, eax
    }
    return buttonRef;
}


//__________________________________________________________________________________________________________________
LONG fall_Button_SetFunctions(LONG buttonRef, void* hoverOnfunc, void* hoverOffFunc, void* pushfunc, void* liftfunc) {
    int retVal;
    __asm {
        push liftfunc
        mov ecx, pushfunc
        mov ebx, hoverOffFunc
        mov edx, hoverOnfunc
        mov eax, buttonRef
        call pfall_button_set_functions
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________________
LONG fall_Button_SetSounds(LONG buttonRef, void* soundFunc1, void* soundFunc2) {
    int retVal;
    __asm {
        mov ebx, soundFunc2
        mov edx, soundFunc1
        mov eax, buttonRef
        call pfall_button_set_sounds
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________
ButtonStruct* fall_Button_Get(LONG buttRef, WinStruct** retWin) {
    ButtonStruct* button;
    __asm {
        mov edx, retWin
        mov eax, buttRef
        call pfall_button_get
        mov button, eax
    }
    return button;
}


//____________________________________
LONG fall_Button_Destroy(LONG buttRef) {
    LONG retVal = 0;
    __asm {
        mov eax, buttRef
        call pfall_button_destroy
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________________
void fall_Button_Draw(WinStruct* p_win, RECT* p_rect) {

    __asm {
        mov edx, p_rect
        mov eax, p_win
        call pfall_button_draw
    }

}


//____________________________________
void Fallout_Functions_Setup_Windows() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        pWinRef_Char = (LONG*)0x58060C;
        pWinRef_Bio = (LONG*)0x52C5E8;
        pWinRef_DialogMain = (LONG*)0x528530;
        pWinRef_DialogNpcText = (LONG*)0x5284D4;
        pWinRef_DialogPcText = (LONG*)0x5284D8;
        pWinRef_DialogBaseSub = (LONG*)0x528534;
        pWinRef_Inventory = (LONG*)0x5AEEE4;
        pWinRef_EndSlides = (LONG*)0x580BF4;
        pWinRef_Iface = (LONG*)0x528E14;
        pWinRef_NotifyBar = (LONG*)0x528E18;
        pWinRef_Skills = (LONG*)0x6786C0;
        pWinRef_LoadSave = (LONG*)0x624844;
        pWinRef_MainMenu = (LONG*)0x5292E0;
        pWinRef_Movies = (LONG*)0x5293A8;
        pWinRef_Options = (LONG*)0x673E84;
        pWinRef_Pipboy = (LONG*)0x674A50;
        pWinRef_GameArea = (LONG*)0x6423CC;
        pWinRef_WorldMap = (LONG*)0x52DC04;

        pWin_Array = (WinStruct**)0x6BE3B4;
        //To-Do p_winRef_Index
        p_num_windows = (LONG*)0x6BE480;
        pp_win_mainBuff = (BYTE**)0x52E1D8;
        p_draw_window_flag = (LONG*)0x6BE494;

        pfall_window_create = (void*)0x4DC0BD;
        pfall_window_destroy = (void*)0x4DC3C0;
        pfall_window_get = (void*)0x4DDF63;
        pfall_window_get_window_at_pos = (void*)0x4DE01C;
        pfall_window_draw = (void*)0x4DD278;
        pfall_window_show = (void*)0x4DCFCE;
        pfall_window_hide = (void*)0x4DD0FE;
        pfall_window_print_text = (void*)0x4DC8FE;

        pfall_windows_draw_rect = (void*)0x4DDB46;

        pfall_button_create = (void*)0x4DEC67;
        //To-Do pfall_button_destroy
        pfall_button_get = (void*)0x4DE25A;
        pfall_button_set_functions = (void*)0x4DF26D;
        pfall_button_set_sounds = (void*)0x4DF3A6;
        //To-Do pfall_button_disable
        //To-Do pfall_button_enable
        //To-Do pfall_button_set_toggle_array
        //To-Do pfall_button_set_toggle_state
        pfall_button_draw = (void*)FixAddress(0x4E0D55);
    }
    else {
        pWinRef_Char = (LONG*)FixAddress(0x57060C);
        pWinRef_Bio = (LONG*)FixAddress(0x51C7F8);
        pWinRef_DialogMain = (LONG*)FixAddress(0x518740);
        pWinRef_DialogNpcText = (LONG*)FixAddress(0x5186E4);
        pWinRef_DialogPcText = (LONG*)FixAddress(0x5186E8);
        pWinRef_DialogBaseSub = (LONG*)FixAddress(0x518744);
        pWinRef_Inventory = (LONG*)FixAddress(0x59E964);
        pWinRef_EndSlides = (LONG*)FixAddress(0x570BF4);
        pWinRef_Iface = (LONG*)FixAddress(0x519024);
        pWinRef_NotifyBar = (LONG*)FixAddress(0x519028);
        pWinRef_Skills = (LONG*)FixAddress(0x668140);
        pWinRef_LoadSave = (LONG*)FixAddress(0x6142C4);
        pWinRef_MainMenu = (LONG*)FixAddress(0x5194F0);
        pWinRef_Movies = (LONG*)FixAddress(0x5195B8);
        pWinRef_Options = (LONG*)FixAddress(0x663904);
        pWinRef_Pipboy = (LONG*)FixAddress(0x6644C4);
        pWinRef_GameArea = (LONG*)FixAddress(0x631E4C);
        pWinRef_WorldMap = (LONG*)FixAddress(0x51DE14);

        pWin_Array = (WinStruct**)FixAddress(0x6ADE58);
        p_winRef_Index = (LONG*)FixAddress(0x6ADD90);
        p_num_windows = (LONG*)FixAddress(0x6ADF24);
        pp_win_mainBuff = (BYTE**)FixAddress(0x51E3FC);
        p_draw_window_flag = (LONG*)FixAddress(0x6ADF38);

        pfall_window_create = (void*)FixAddress(0x4D6238);
        pfall_window_destroy = (void*)FixAddress(0x4D6468);
        pfall_window_get = (void*)FixAddress(0x4D7888);
        pfall_window_get_window_at_pos = (void*)FixAddress(0x4D78CC);
        pfall_window_draw = (void*)FixAddress(0x4D6F5C);
        pfall_window_show = (void*)FixAddress(0x4D6DAC);
        pfall_window_hide = (void*)FixAddress(0x4D6E64);
        pfall_window_print_text = (void*)FixAddress(0x4D684C);

        pfall_windows_draw_rect = (void*)FixAddress(0x4D759C);

        pfall_button_create = (void*)FixAddress(0x4D8260);
        pfall_button_destroy = (void*)FixAddress(0x4D92BC);
        pfall_button_get = (void*)FixAddress(0x4D79DC);
        pfall_button_set_functions = (void*)FixAddress(0x4D8758);
        pfall_button_set_sounds = (void*)FixAddress(0x4D87F8);
        pfall_button_disable = (void*)FixAddress(0x4D94D0);
        pfall_button_enable = (void*)FixAddress(0x4D9474);
        pfall_button_set_toggle_array = (void*)FixAddress(0x4D96EC);
        pfall_button_set_toggle_state = (void*)FixAddress(0x4D9554);
        pfall_button_draw = (void*)FixAddress(0x4D9A58);
    }

}