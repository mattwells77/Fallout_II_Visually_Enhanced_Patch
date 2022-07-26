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

//To-Do Update options window.
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"

#include "Fall_General.h"
#include "Fall_Msg.h"
#include "Fall_Windows.h"

MSGList *pMsgList_Options=nullptr;

void(*fall_OptionsWindow)() = nullptr;


//_________________________________________________
void Options_GetMousePosition(LONG* p_x, LONG* p_y) {
    fall_Mouse_GetPos(p_x, p_y);
    WinStruct* pWin = fall_Win_Get(*pWinRef_Options);
    if (pWin) {
        *p_x -= pWin->rect.left;
        *p_y -= pWin->rect.top;
    }
}


//_____________________________________________________
void __declspec(naked) options_get_mouse_position(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Options_GetMousePosition
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________
void Modifications_Options_CH() {

    pMsgList_Options = (MSGList*)0x673D68;

    fall_OptionsWindow = (void(*)())0x48FB40;

    //Dials Pos Fix
    FuncWrite32(0x490053, 0x03BF7B, (DWORD)&options_get_mouse_position);
    //Sliders Pos Fix
    FuncWrite32(0x4906C7, 0x03B907, (DWORD)&options_get_mouse_position);
}


//________________________________
void Modifications_Options_MULTI() {

    pMsgList_Options = (MSGList*)FixAddress(0x6637E8);

    fall_OptionsWindow = (void(*)())FixAddress(0x490798);

    //Dials Pos Fix
    FuncReplace32(0x490EAD, 0x039B2B, (DWORD)&options_get_mouse_position);
    //Sliders Pos Fix
    FuncReplace32(0x491547, 0x039491, (DWORD)&options_get_mouse_position);
}


//__________________________
void Modifications_Options() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Options_CH();
    else
        Modifications_Options_MULTI();

}






