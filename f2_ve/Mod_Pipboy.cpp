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

//To-Do update pipboy
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"

#include "Fall_General.h"
#include "Fall_Windows.h"


//________________________________________________
void Pipboy_GetMousePosition(LONG* p_x, LONG* p_y) {
    fall_Mouse_GetPos(p_x, p_y);
    WinStruct* pWin = fall_Win_Get(*pWinRef_Pipboy);
    if (pWin) {
        *p_x -= pWin->rect.left;
        *p_y -= pWin->rect.top;
    }
}


//____________________________________________________
void __declspec(naked) pipboy_get_mouse_position(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Pipboy_GetMousePosition
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//____________________________
void Modifications_Pipboy_CH() {

    FuncWrite32(0x495E43, 0x03618B, (DWORD)&pipboy_get_mouse_position);
    FuncWrite32(0x498EAB, 0x033123, (DWORD)&pipboy_get_mouse_position);
}


//_______________________________
void Modifications_Pipboy_MULTI() {

    FuncReplace32(0x497093, 0x033945, (DWORD)&pipboy_get_mouse_position);
    FuncReplace32(0x49A1B7, 0x030821, (DWORD)&pipboy_get_mouse_position);
}


//_________________________
void Modifications_Pipboy() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Pipboy_CH();
    else
        Modifications_Pipboy_MULTI();
}




