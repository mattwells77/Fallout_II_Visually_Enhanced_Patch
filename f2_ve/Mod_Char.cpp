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

//To-Do update char window
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"

#include "Fall_General.h"
#include "Fall_Windows.h"
#include "Fall_Text.h"


//_________________________________________
void GetCharMouse(LONG* pxPos, LONG* pyPos) {

    WinStruct* charWin = fall_Win_Get(*pWinRef_Char);
    fall_Mouse_GetPos(pxPos, pyPos);
    *pxPos -= charWin->rect.left;
    *pyPos -= charWin->rect.top;
}


//__________________________________
void __declspec(naked) h_mouse_fix() {
    __asm {
        push ebx
        push ecx
        push esi

        push edx//char mouse y pos address
        push eax//char mouse x pos address
        call GetCharMouse
        add esp, 0x8

        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//____________________________________________________________________________
int CharSubWin(int x, int y, int width, int height, int colour, int winFlags) {
    WinStruct* charWin = fall_Win_Get(*pWinRef_Char);
    return fall_Win_Create(x + charWin->rect.left, y + charWin->rect.top, width, height, colour, winFlags);
}


//_______________________________________
void __declspec(naked) h_char_sub_panel() {
    __asm {
        push dword ptr ss : [esp + 0x8]
        push dword ptr ss : [esp + 0x8]

        push ecx
        push ebx
        push edx
        push eax
        call CharSubWin
        add esp, 0x18
        ret 0x8
    }

}


//________________________________________
void __declspec(naked) char_sub_menu_fix() {
    //00436C8E  |. E8 A5F50900    CALL fallout2.004D6238

    __asm {
        mov eax, pWinRef_Char
        push dword ptr ds : [eax]
        call fall_Win_Get
        add esp, 0x4
        test eax, eax
        je noCharWin
        mov ecx, dword ptr ds : [eax + 0x8]//charWin->rect.left
        add dword ptr ss : [ebp + 0x10] , ecx//subWin xPos
        mov ecx, dword ptr ds : [eax + 0x0C]//charWin->rect.top
        add dword ptr ss : [ebp + 0x14] , ecx//subWin yPos
        noCharWin :
        call fall_GetFont
            ret
    }

}


//_______________________________
void Modifications_Character_CH() {

    FuncWrite32(0x43A448, 0x091B86, (DWORD)&h_mouse_fix);
    FuncWrite32(0x43A50E, 0x091AC0, (DWORD)&h_mouse_fix);
    FuncWrite32(0x43C091, 0x08FF3D, (DWORD)&h_mouse_fix);

    //CREATE NEW CHAR OPTIONS POSITION
    FuncWrite32(0x4375B7, 0x0A4B02, (DWORD)&h_char_sub_panel);
    //PERK SCRN POSITION
    FuncWrite32(0x43BB29, 0x0A0590, (DWORD)&h_char_sub_panel);
    //name sub window
    FuncWrite32(0x436343, 0x0A5D76, (DWORD)&h_char_sub_panel);
    //age sub window
    FuncWrite32(0x43669E, 0x0A5A1B, (DWORD)&h_char_sub_panel);
    //sex sub window
    FuncWrite32(0x436C98, 0x0A5421, (DWORD)&h_char_sub_panel);
    //load char sub menu
    FuncWrite32(0x41DDCB, 0x0BD636, (DWORD)&char_sub_menu_fix);
    //save char, print new char, print ingame char sub menues
    FuncWrite32(0x41E8DF, 0x0BCB22, (DWORD)&char_sub_menu_fix);

}


//__________________________________
void Modifications_Character_MULTI() {

    FuncReplace32(0x43AE94, 0x08FB44, (DWORD)&h_mouse_fix);
    FuncReplace32(0x43AF5A, 0x08FA7E, (DWORD)&h_mouse_fix);
    FuncReplace32(0x43CB5D, 0x08DE7B, (DWORD)&h_mouse_fix);

    //CREATE NEW CHAR OPTIONS POSITION
    FuncReplace32(0x43800B, 0x09E229, (DWORD)&h_char_sub_panel);
    //PERK SCRN POSITION
    FuncReplace32(0x43C581, 0x099CB3, (DWORD)&h_char_sub_panel);
    //name sub window
    FuncReplace32(0x436C8F, 0x09F5A5, (DWORD)&h_char_sub_panel);
    //age sub window
    FuncReplace32(0x437046, 0x09F1EE, (DWORD)&h_char_sub_panel);
    //sex sub window
    FuncReplace32(0x43769C, 0x09EB98, (DWORD)&h_char_sub_panel);
    //load char sub menu
    FuncReplace32(0x41DEBB, 0x0B7A15, (DWORD)&char_sub_menu_fix);
    //save char, print new char, print ingame char sub menues
    FuncReplace32(0x41EAA3, 0x0B6E2D, (DWORD)&char_sub_menu_fix);

}


//____________________________
void Modifications_Character() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Character_CH();
    else
        Modifications_Character_MULTI();

}






