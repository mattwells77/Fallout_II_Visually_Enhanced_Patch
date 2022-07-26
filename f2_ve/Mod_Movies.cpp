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
#include "memwrite.h"

#include "Fall_General.h"

#include "Dx_Windows.h"
#include "Dx_Movies.h"

//This is just for creating and destroying the movie background window now, movie rendering is done in Dx_Movies.cpp.


//______________________________________________
void OnScreenResize_Movie(Window_DX* pWin_This) {

    if (!pWin_This)
        return;

    pWin_This->ResizeWindow(SCR_WIDTH, SCR_HEIGHT);
    pWin_This->ClearRenderTarget(nullptr);

    MVE_ResetMatrices();
}


//______________________________________________
void OnDisplay_Post_Movie(Window_DX* pWin_This) {

    if (!pWin_This)
        return;

    MVE_Surface_Display();
}


//__________________________________________________________________________________
int MovieWinSetup(int x, int y, int width, int height, DWORD colour, DWORD winFlags) {
    if (*pWinRef_Movies != -1)
        return -1;

    *pWinRef_Movies = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Movies);
    if (pWin && pWin->winDx) {
        pWin->winDx->ClearRenderTarget(nullptr);
        pWin->winDx->SetDrawFlag(false);
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Movie);
        pWin->winDx->Set_OnDisplayFunctions(nullptr, &OnDisplay_Post_Movie, nullptr);
    }
    return *pWinRef_Movies;
}


//______________________________________
void __declspec(naked) movie_win_setup() {
    //00436C8E  |. E8 A5F50900    CALL fallout2.004D6238

    __asm {
        push dword ptr ss : [esp + 0x8]
        push dword ptr ss : [esp + 0x8]
        push ecx
        push ebx
        push edx
        push eax
        call MovieWinSetup
        add esp, 0x18
        ret 0x8
    }

}


//_________________________________________
void __declspec(naked) movie_win_kill(void) {
    //0044EA73  |> 89F8                  MOV EAX,EDI
    //0044EA75  |. E8 EE790800           CALL fallout2.004D6468
    __asm {
        mov eax, pWinRef_Movies
        mov eax, dword ptr ds : [eax]
        push eax
        call fall_Win_Destroy
        add esp, 0x4
        mov eax, pWinRef_Movies
        mov dword ptr ds : [eax] , -1
        ret
    }
}


//________________________
void Modifications_Movie() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        FuncWrite32(0x44DED8, 0x08E1E1, (DWORD)&movie_win_setup);
        FuncWrite32(0x44E16D, 0x08E24F, (DWORD)&movie_win_kill);

        //prevent movie winRef set to -1 as it has not been destroyed
        MemWrite16(0x4864FB, 0x2D89, 0x9090);
        MemWrite32(0x4864FD, 0x5293A8, 0x90909090);
    }
    else {
        FuncReplace32(0x44E7E4, 0x087A50, (DWORD)&movie_win_setup);
        FuncReplace32(0x44EA76, 0x0879EE, (DWORD)&movie_win_kill);

        //prevent movie winRef set to -1 as it has not been destroyed
        MemWrite16(0x48710B, 0x2D89, 0x9090);
        MemWrite32(0x48710D, FixAddress(0x5195B8), 0x90909090);
    }

}


