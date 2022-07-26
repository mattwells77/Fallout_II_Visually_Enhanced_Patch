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
#include "Fall_Text.h"
#include "Fall_General.h"
#include "memwrite.h"


void* pfall_SET_FONT=0;
LONG* p_current_font_ref = nullptr;
void* pfall_PRINT_TEXT = nullptr;
void* pfall_GET_TEXT_HEIGHT = nullptr;
void* pfall_GET_TEXT_WIDTH = nullptr;
void* pfall_GET_CHAR_WIDTH = nullptr;
void* pfall_GET_MAX_TEXT_WIDTH = nullptr;
void* pfall_GET_CHAR_GAP_WIDTH = nullptr;
void* pfall_GET_TEXT_SIZE = nullptr;
void* pfall_GET_MAX_CHAR_WIDTH = nullptr;
void* pfall_DRAW_TEXT_OUTLINE = nullptr;

FONT_AFF** lpFONT_AFF = nullptr;
FONT_FON** lpFONT_FON = nullptr;


//____________________________
FONT_FON* GetCurrentFont_Fon() {
    return *lpFONT_FON;
}

//____________________________
FONT_AFF* GetCurrentFont_Aff() {
    return *lpFONT_AFF;
}



//_________________________
void fall_SetFont(LONG ref) {

    __asm {
        mov eax, ref
        call pfall_SET_FONT
    }
}


//_________________
LONG fall_GetFont() {
    return *p_current_font_ref;
}


//___________________________________________________________________________________________________
void fall_PrintText(BYTE* toBuff, const char* txtBuff, DWORD txtWidth, DWORD toWidth, BYTE palColour) {

    __asm {
        push esi
        xor eax, eax
        MOV AL, palColour
        push eax
        mov ecx, toWidth
        mov ebx, txtWidth
        mov edx, txtBuff
        mov eax, toBuff
        mov esi, pfall_PRINT_TEXT
        call dword ptr ds : [esi]
        pop esi
    }
}


//________________________
DWORD fall_GetTextHeight() {
    DWORD TxtHeight;
    __asm {
        mov eax, pfall_GET_TEXT_HEIGHT
        call dword ptr ds : [eax]//get text height
        mov TxtHeight, eax
    }
    return TxtHeight;
}


//__________________________________________
DWORD fall_GetTextWidth(const char* TextMsg) {
    DWORD TxtWidth;
    __asm {
        mov eax, TextMsg
        push esi
        mov esi, pfall_GET_TEXT_WIDTH
        call dword ptr ds : [esi]//get text width
        pop esi
        mov TxtWidth, eax
    }
    return TxtWidth;
}


//_________________________________________
DWORD fall_GetCharWidth(const char CharVal) {
    DWORD charWidth;
    __asm {
        mov al, CharVal
        push esi
        mov esi, pfall_GET_CHAR_WIDTH
        call dword ptr ds : [esi]
        pop esi
        mov charWidth, eax
    }
    return charWidth;
}


//_____________________________________________
DWORD fall_GetMaxTextWidth(const char* TextMsg) {
    DWORD msgWidth;
    __asm {
        mov eax, TextMsg
        push esi
        mov esi, pfall_GET_MAX_TEXT_WIDTH
        call dword ptr ds : [esi]
        pop esi
        mov msgWidth, eax
    }
    return msgWidth;
}


//__________________________
DWORD fall_GetCharGapWidth() {
    DWORD gapWidth;
    __asm {
        mov eax, pfall_GET_CHAR_GAP_WIDTH
        call dword ptr ds : [eax]
        mov gapWidth, eax
    }
    return gapWidth;
}



//____________________________________________
DWORD fall_GetTextBoxSize(const char* TextMsg) {
    DWORD msgSize;
    __asm {
        mov eax, TextMsg
        push esi
        mov esi, pfall_GET_TEXT_SIZE
        call dword ptr ds : [esi]
        pop esi
        mov msgSize, eax
    }
    return msgSize;
}


//__________________________
DWORD fall_GetMaxCharWidth() {
    DWORD charWidth = 0;
    __asm {
        mov eax, pfall_GET_MAX_CHAR_WIDTH
        call dword ptr ds : [eax]
        mov charWidth, eax
    }
    return charWidth;
}


//_______________________________________________________________________________________________
void fall_DrawTextOutline(BYTE* txtBackBuff, LONG width1, LONG height, LONG width2, DWORD Colour) {

    __asm {
        pushad
        push Colour
        mov ecx, width2
        mov ebx, height
        mov edx, width1
        mov eax, txtBackBuff
        call pfall_DRAW_TEXT_OUTLINE
        popad
    }

}


//_________________________________
void Fallout_Functions_Setup_Text() {

    if (fallout_exe_region == EXE_Region::Chinese) {

        pfall_SET_FONT = (void*)0x4DB431;
        p_current_font_ref = (LONG*)0x52E18C;

        pfall_PRINT_TEXT = (void*)0x52E194;
        pfall_GET_TEXT_HEIGHT = (void*)0x52E198;
        pfall_GET_TEXT_WIDTH = (void*)0x52E19C;
        pfall_GET_CHAR_WIDTH = (void*)0x52E1A0;
        pfall_GET_MAX_TEXT_WIDTH = (void*)0x52E1A4;
        pfall_GET_CHAR_GAP_WIDTH = (void*)0x52E1A8;
        pfall_GET_TEXT_SIZE = (void*)0x52E1AC;
        pfall_GET_MAX_CHAR_WIDTH = (void*)0x52E1B0;

        //To_Do pfall_DRAW_TEXT_OUTLINE = (void*)0x;
    }
    else {

        pfall_SET_FONT = (void*)FixAddress(0x4D58DC);
        p_current_font_ref = (LONG*)FixAddress(0x51E3B0);

        pfall_PRINT_TEXT = (void*)FixAddress(0x51E3B8);
        pfall_GET_TEXT_HEIGHT = (void*)FixAddress(0x51E3BC);
        pfall_GET_TEXT_WIDTH = (void*)FixAddress(0x51E3C0);
        pfall_GET_CHAR_WIDTH = (void*)FixAddress(0x51E3C4);
        pfall_GET_MAX_TEXT_WIDTH = (void*)FixAddress(0x51E3C8);
        pfall_GET_CHAR_GAP_WIDTH = (void*)FixAddress(0x51E3CC);
        pfall_GET_TEXT_SIZE = (void*)FixAddress(0x51E3D0);
        pfall_GET_MAX_CHAR_WIDTH = (void*)FixAddress(0x51E3D4);

        pfall_DRAW_TEXT_OUTLINE = (void*)FixAddress(0x4D3AE0);

        lpFONT_AFF = (FONT_AFF**)FixAddress(0x58E93C);
        lpFONT_FON = (FONT_FON**)FixAddress(0x6ADD88);
    }

}









