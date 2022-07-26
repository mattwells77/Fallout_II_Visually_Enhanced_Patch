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

//To-Do Update pause window, add a grey scale effect to game view
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"
#include "win_fall.h"

#include "Fall_General.h"
#include "Fall_Windows.h"

#include "Dx_Windows.h"


int winRef_Pause = -1;
void *SET_PAUSE_BG = nullptr;


//______________________________________
void __declspec(naked) pause_win_setup() {

    __asm {
        push dword ptr ss : [esp + 0x8]
        push dword ptr ss : [esp + 0x8]
        push ecx
        push ebx
        call Win_Create_CenteredOnGame
        add esp, 0x10
        mov winRef_Pause, eax
        ret 0x8
    }

}


//_________________________________________
void __declspec(naked) pause_win_kill(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call fall_Win_Destroy
        add esp, 0x4
        mov winRef_Pause, -1

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//___________________________________
void __declspec(naked) fix_pause_bg() {

    __asm {
        push eax
        push esi
        push edi
        push eax
        call fall_Win_Get
        add esp, 0x4

        mov ecx, dword ptr ds : [eax + 0x18]
        pop edi
        pop esi
        pop eax
        ret
    }

}


//__________________________________
void SetPauseBackGround(int isWorld) {
    __asm {
        mov eax, isWorld
        call SET_PAUSE_BG
    }
}


//___________________
void ResizePauseWin() {
    if (winRef_Pause != -1)
        SetPauseBackGround(0);

}



//___________________________
void Modifications_Pause_CH() {
    FuncWrite32(0x48F875, 0x04C844, (DWORD)&pause_win_setup);
    FuncWrite32(0x48FA7B, 0x04C941, (DWORD)&pause_win_kill);

    MemWrite8(0x48FB06, 0xB9, 0xE8);
    FuncWrite32(0x48FB07, 640, (DWORD)&fix_pause_bg);

    SET_PAUSE_BG = (void*)0x48FAF0;
}


//______________________________
void Modifications_Pause_MULTI() {

    FuncReplace32(0x490475, 0x045DBF, (DWORD)&pause_win_setup);
    FuncReplace32(0x4906D3, 0x045D91, (DWORD)&pause_win_kill);

    MemWrite8(0x49075E, 0xB9, 0xE8);
    FuncWrite32(0x49075F, 640, (DWORD)&fix_pause_bg);

    SET_PAUSE_BG = (void*)FixAddress(0x490748);
}


//________________________
void Modifications_Pause() {

   if(fallout_exe_region == EXE_Region::Chinese)
      Modifications_Pause_CH();
   else
      Modifications_Pause_MULTI();
}

