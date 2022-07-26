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

//To-Do update credits window
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"
#include "Fall_General.h"
#include "win_fall.h"

#include "Dx_Windows.h"

BYTE *creditWinBuff=0;
BYTE *creditBuff1=0;
BYTE *creditBuff2=0;
BYTE *creditBuff2_top=0;
BYTE *creditBuff2_bottom=0;
int winRef_Credits=-1;

//______________________________________________________
DWORD _stdcall CreditsWinSetup(int colour, int winFlags) {

    winRef_Credits = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
    WinStruct* win = fall_Win_Get(winRef_Credits);
    creditWinBuff = win->buff;

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef_Credits);
    if (pWin && pWin->winDx) {
        pWin->winDx->ClearRenderTarget(nullptr);
    }
    return winRef_Credits;

}


//___________________________________________________________________________________________________________
void CreditsBltWinBuff(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {
    WinStruct* creditWin = fall_Win_Get(winRef_Credits);
    fromBuff = creditBuff2;
    toBuff = creditWinBuff;
    toBuff += (SCR_WIDTH >> 1) - 320;
    MemBlt8(fromBuff, subWidth, SCR_HEIGHT, fromWidth, toBuff, SCR_WIDTH);
}


//_________________________________________________________________________________________________________
void CreditsBltBuff1(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {
    fromBuff = creditBuff1;
    toBuff = creditWinBuff;
    toBuff += (SCR_WIDTH >> 1) - 320;
    MemBlt8(fromBuff, subWidth, SCR_HEIGHT, fromWidth, toBuff, SCR_WIDTH);
}



//____________________________________________________
void __declspec(naked)allocate_credit_scrn_buff1(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        mov eax, 640
        imul eax, SCR_HEIGHT
        push eax
        call fall_Mem_Allocate
        add esp, 0x04
        //lea eax, eax
        mov creditBuff1, eax
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________
BYTE* AllocateCreditScrnBuff2() {
    creditBuff2 = (BYTE*)fall_Mem_Allocate(640 * SCR_HEIGHT);
    creditBuff2_top = &creditBuff2[640];
    creditBuff2_bottom = &creditBuff2[640 * (SCR_HEIGHT - 1)];
    return creditBuff2;
}


//____________________________________________________
void __declspec(naked)allocate_credit_scrn_buff2(void) {

   __asm {
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp
      call AllocateCreditScrnBuff2
      pop ebp
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
      ret
   }
}


//________________________________________________
void* F_memset(void* buffer, int ch, size_t count) {
    return memset(buffer, ch, count);
}


//________________________________________________
void __declspec(naked)clear_credit_scrn_buff(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp


        mov ebx, 640
        imul ebx, SCR_HEIGHT
        push ebx
        push edx
        push eax
        call F_memset
        add esp, 0x0C

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//______________________________________________________
void* F_memcpy(void* to, const void* from, size_t count) {
    count = 640 * (SCR_HEIGHT - 1);
    return memcpy(to, from, count);
}


//_________________________________________________
void __declspec(naked)memcpy_credit_scrn_buff(void) {

    __asm {
        push ecx
        push esi
        push edi


        push ebx
        push edx
        push eax
        call F_memcpy
        add esp, 0x0C

        pop edi
        pop esi
        pop ecx
        ret
    }
}


//___________________________________________
void __declspec(naked)copy_textbuff_fix(void) {

    __asm {
        mov edi, 640
        sub edi, ecx
        shr edi, 1
        add edi, creditBuff2_bottom
        ret
    }
}


//_____________________________________________
void __declspec(naked)destroy_win_credits(void) {

    __asm {
        cmp winRef_Credits, -1
        je exitFunc
        push winRef_Credits
        call fall_Win_Destroy
        add esp, 0x4
        mov winRef_Credits, -1
        exitFunc:
        ret
    }
}

//__________________
void ReSizeCredits() {

    if (winRef_Credits == -1)
        return;

    WinStruct* creditWin = fall_Win_Get(winRef_Credits);
    if (!creditWin)
        return;

    DWORD oldHeight = creditWin->height;
    BYTE* oldBuff = new BYTE[640 * oldHeight];
    memcpy(oldBuff, creditBuff2, 640 * oldHeight);

    creditWin->width = SCR_WIDTH;
    creditWin->height = SCR_HEIGHT;
    creditWin->rect.left = 0;
    creditWin->rect.top = 0;
    creditWin->rect.right = SCR_WIDTH - 1;
    creditWin->rect.bottom = SCR_HEIGHT - 1;

    if (creditWin->buff != 0) {
        creditWin->buff = (BYTE*)fall_Mem_Reallocate(creditWin->buff, SCR_WIDTH * SCR_HEIGHT);
        creditWinBuff = creditWin->buff;
        memset(creditWin->buff, 0, SCR_WIDTH * SCR_HEIGHT);
    }

    if (creditBuff1 != 0) {
        creditBuff1 = (BYTE*)fall_Mem_Reallocate(creditBuff1, 640 * SCR_HEIGHT);
        memset(creditBuff1, 0, 640 * SCR_HEIGHT);
    }


    if (creditBuff2 != 0) {
        creditBuff2 = (BYTE*)fall_Mem_Reallocate(creditBuff2, 640 * SCR_HEIGHT);
        memset(creditBuff2, 0, 640 * SCR_HEIGHT);

        UINT offset = 0;
        if (SCR_HEIGHT > oldHeight) {
            offset = 640 * SCR_HEIGHT - 640 * oldHeight;
            memcpy(creditBuff2 + offset, oldBuff, 640 * oldHeight);
        }
        else {
            offset = 640 * oldHeight - 640 * SCR_HEIGHT;
            memcpy(creditBuff2, oldBuff + offset, 640 * SCR_HEIGHT);
        }
        creditBuff2_top = &creditBuff2[640];
        creditBuff2_bottom = &creditBuff2[640 * (SCR_HEIGHT - 1)];
    }

    if (oldBuff)
        delete[] oldBuff;
}


//_________________________________
void Modifications_Credits_CH(void) {

    FuncWrite32(0x42CA66, 0x0AF956, (DWORD)&destroy_win_credits);

    FuncWrite32(0x42C88A, 0x0D0558, (DWORD)&memcpy_credit_scrn_buff);
    FuncWrite32(0x42C990, 0x0D0452, (DWORD)&memcpy_credit_scrn_buff);

    MemWrite8(0x42C882, 0x8B, 0x90);
    MemWrite16(0x42C883, 0x24BC, 0xE890);
    FuncWrite32(0x42C885, 0x0238, (DWORD)&copy_textbuff_fix);

    MemWrite8(0x42C811, 0x8B, 0x90);
    MemWrite16(0x42C812, 0x2494, 0x158B);
    MemWrite32(0x42C814, 0x0220, (DWORD)&creditBuff2_bottom);

    MemWrite8(0x42C866, 0x8B, 0x90);
    MemWrite16(0x42C867, 0x2494, 0x158B);
    MemWrite32(0x42C869, 0x0230, (DWORD)&creditBuff2_top);

    MemWrite16(0x42CA3E, 0x848B, 0x9090);
    MemWrite8(0x42CA40, 0x24, 0xA1);
    MemWrite32(0x42CA41, 0x0240, (DWORD)&creditBuff1);

    MemWrite16(0x42C86D, 0x848B, 0x9090);
    MemWrite8(0x42C86F, 0x24, 0xA1);
    MemWrite32(0x42C870, 0x024C, (DWORD)&creditBuff2);

    MemWrite16(0x42C94E, 0x848B, 0x9090);
    MemWrite8(0x42C950, 0x24, 0xA1);
    MemWrite32(0x42C951, 0x024C, (DWORD)&creditBuff2);

    MemWrite8(0x42C955, 0x8B, 0x90);
    MemWrite16(0x42C956, 0x24BC, 0x3D8B);
    MemWrite32(0x42C958, 0x024C, (DWORD)&creditBuff2_bottom);
    MemWrite32(0x42C963, 306560, 0);

    MemWrite16(0x42C988, 0x848B, 0x9090);
    MemWrite8(0x42C98A, 0x24, 0xA1);
    MemWrite32(0x42C98B, 0x024C, (DWORD)&creditBuff2);

    MemWrite16(0x42CA32, 0x848B, 0x9090);
    MemWrite8(0x42CA34, 0x24, 0xA1);
    MemWrite32(0x42CA35, 0x024C, (DWORD)&creditBuff2);

    FuncWrite32(0x42C5D0, 0x0981C7, (DWORD)&allocate_credit_scrn_buff1);
    FuncWrite32(0x42C5FA, 0x0D0932, (DWORD)&clear_credit_scrn_buff);

    FuncWrite32(0x42C692, 0x098105, (DWORD)&allocate_credit_scrn_buff2);
    FuncWrite32(0x42C6AF, 0x0D087D, (DWORD)&clear_credit_scrn_buff);

    MemWrite32(0x42C63F, 480, SCR_HEIGHT);

    FuncWrite32(0x42C599, 0x0AFB20, (DWORD)&CreditsWinSetup);

    FuncWrite32(0x42C8C3, 0x0AC5F5, (DWORD)&CreditsBltBuff1);
    FuncWrite32(0x42C8E8, 0x0AC60B, (DWORD)&CreditsBltWinBuff);

    FuncWrite32(0x42C9C7, 0x0AC4F1, (DWORD)&CreditsBltBuff1);
    FuncWrite32(0x42C9F3, 0x0AC500, (DWORD)&CreditsBltWinBuff);
}


//____________________________________
void Modifications_Credits_MULTI(void) {

    FuncReplace32(0x42CE26, 0x0A963E, (DWORD)&destroy_win_credits);

    FuncReplace32(0x42CC4A, 0x0C3393, (DWORD)&memcpy_credit_scrn_buff);
    FuncReplace32(0x42CD50, 0x0C328D, (DWORD)&memcpy_credit_scrn_buff);

    MemWrite8(0x42CC42, 0x8B, 0x90);
    MemWrite16(0x42CC43, 0x24BC, 0xE890);
    FuncWrite32(0x42CC45, 0x0238, (DWORD)&copy_textbuff_fix);

    MemWrite8(0x42CBD1, 0x8B, 0x90);
    MemWrite16(0x42CBD2, 0x2494, 0x158B);
    MemWrite32(0x42CBD4, 0x0220, (DWORD)&creditBuff2_bottom);

    MemWrite8(0x42CC26, 0x8B, 0x90);
    MemWrite16(0x42CC27, 0x2494, 0x158B);
    MemWrite32(0x42CC29, 0x0230, (DWORD)&creditBuff2_top);

    MemWrite16(0x42CDFE, 0x848B, 0x9090);
    MemWrite8(0x42CE00, 0x24, 0xA1);
    MemWrite32(0x42CE01, 0x0240, (DWORD)&creditBuff1);

    MemWrite16(0x42CC2D, 0x848B, 0x9090);
    MemWrite8(0x42CC2F, 0x24, 0xA1);
    MemWrite32(0x42CC30, 0x024C, (DWORD)&creditBuff2);

    MemWrite16(0x42CD0E, 0x848B, 0x9090);
    MemWrite8(0x42CD10, 0x24, 0xA1);
    MemWrite32(0x42CD11, 0x024C, (DWORD)&creditBuff2);

    MemWrite8(0x42CD15, 0x8B, 0x90);
    MemWrite16(0x42CD16, 0x24BC, 0x3D8B);
    MemWrite32(0x42CD18, 0x024C, (DWORD)&creditBuff2_bottom);
    MemWrite32(0x42CD23, 306560, 0);

    MemWrite16(0x42CD48, 0x848B, 0x9090);
    MemWrite8(0x42CD4A, 0x24, 0xA1);
    MemWrite32(0x42CD4B, 0x024C, (DWORD)&creditBuff2);

    MemWrite16(0x42CDF2, 0x848B, 0x9090);
    MemWrite8(0x42CDF4, 0x24, 0xA1);
    MemWrite32(0x42CDF5, 0x024C, (DWORD)&creditBuff2);

    FuncReplace32(0x42C990, 0x09913C, (DWORD)&allocate_credit_scrn_buff1);
    FuncReplace32(0x42C9BA, 0x0C36C2, (DWORD)&clear_credit_scrn_buff);

    FuncReplace32(0x42CA52, 0x09907A, (DWORD)&allocate_credit_scrn_buff2);
    FuncReplace32(0x42CA6F, 0x0C360D, (DWORD)&clear_credit_scrn_buff);

    MemWrite32(0x42C9FF, 480, SCR_HEIGHT);

    FuncReplace32(0x42C959, 0x0A98DB, (DWORD)&CreditsWinSetup);


    FuncReplace32(0x42CC83, 0x0A6A4D, (DWORD)&CreditsBltBuff1);
    FuncReplace32(0x42CCA8, 0x0A6A58, (DWORD)&CreditsBltWinBuff);

    FuncReplace32(0x42CD87, 0x0A6949, (DWORD)&CreditsBltBuff1);
    FuncReplace32(0x42CDB3, 0x0A694D, (DWORD)&CreditsBltWinBuff);
}


//__________________________
void Modifications_Credits() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Credits_CH();
    else
        Modifications_Credits_MULTI();
}




