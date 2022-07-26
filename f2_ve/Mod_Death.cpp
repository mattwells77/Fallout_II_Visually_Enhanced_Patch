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
#include "modifications.h"
#include "memwrite.h"
#include "configTools.h"
#include "win_fall.h"

#include "Fall_Graphics.h"
#include "Fall_Text.h"
#include "Fall_General.h"
#include "Fall_File.h"

#include "Dx_Windows.h"
#include "Dx_Graphics.h"

int winRef_Death = -1;
int winRef_DeathImage = -1;

BYTE* pDeathSubtitle_PalOffset = nullptr;

char*(*fall_GetDeathTitle)() = nullptr;


void* pfall_Get_Death_Subtitles = nullptr;

void* pfall_Cut_Death_Subtitles = nullptr;

bool deathNarration_Ended = false;


//___________________________
void DeathNarrationCallBack() {
    deathNarration_Ended = true;
}


//_________________________________________________________________
LONG fall_Get_Death_Subtitles(char* pFileName_noEXT, char* pRetMsg) {
    LONG retVal = 0;
    __asm {
        mov edx, pRetMsg
        mov eax, pFileName_noEXT
        call pfall_Get_Death_Subtitles
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________________________________________
LONG fall_Cut_Death_Subtitles(char* pMsg, LONG width, WORD* pLineOffsets, WORD* pNumLines) {
    LONG retVal = 0;
    __asm {
        mov ecx, pNumLines
        mov ebx, pLineOffsets
        mov edx, width
        mov eax, pMsg
        call pfall_Cut_Death_Subtitles
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________
void OnScreenResize_DeathX(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    pWin_This->ResizeWindow(SCR_WIDTH, SCR_HEIGHT);
    pWin_This->ClearRenderTarget(nullptr);

    Window_DX* subwin = pWin_This->GetSubWin(winRef_DeathImage);
    if (!subwin)
        return;
    RECT rect_bg_scrn = { 0,0,0,0 };
    ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"DEATH_SCRN_SIZE", 0), nullptr, nullptr, &rect_bg_scrn);
}


//_______________
void DeathScrnX() {

    fall_FreeArtCache();
    fall_Mouse_SetImage(0);
    bool was_mouse_hidden = IsMouseHidden();
    if (was_mouse_hidden)
        fall_Mouse_Show();

    winRef_Death = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, FLG_WinToFront);
    if (winRef_Death == -1) {
        if (was_mouse_hidden)
            fall_Mouse_Hide();
        return;
    }
    WinStructDx* pWin = nullptr;
    pWin = (WinStructDx*)fall_Win_Get(winRef_Death);

    Window_DX* winDx = pWin->winDx;
    if (!pWin || !winDx) {
        fall_Win_Destroy(winRef_Death);
        winRef_Death = -1;
        winRef_DeathImage = -1;
        if (was_mouse_hidden)
            fall_Mouse_Hide();
        return;
    }
    winDx->SetDrawFlag(false);
    winDx->Set_OnScreenResizeFunction(&OnScreenResize_DeathX);
    winDx->ClearRenderTarget(nullptr);

    DWORD frmID_DeathBG = (ART_INTRFACE << 24 | 0x135);
    FRMCached* frmDeathBG = new FRMCached(frmID_DeathBG);
    if (!frmDeathBG) {
        fall_Win_Destroy(winRef_Death);
        winRef_Death = -1;
        winRef_DeathImage = -1;
        if (was_mouse_hidden)
            fall_Mouse_Hide();
        return;
    }
    FRMframeDx* pFrame = frmDeathBG->GetFrame(0, 0);
    if (!pFrame) {
        delete frmDeathBG;
        frmDeathBG = nullptr;
        fall_Win_Destroy(winRef_Death);
        winRef_Death = -1;
        winRef_DeathImage = -1;
        if (was_mouse_hidden)
            fall_Mouse_Hide();
        return;
    }
    while (*pMOUSE_FLAGS != 0) {
        fall_Get_Input();
    }
    fall_Input_Related_01();
    fall_Input_Related_02();

    Window_DX* subwin = nullptr;
    subwin = new Window_DX(0, 0, pFrame->GetWidth(), pFrame->GetHeight(), 0x000000FF, winDx, &winRef_DeathImage);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    delete frmDeathBG;
    frmDeathBG = nullptr;
    pFrame = nullptr;


    float point_w = 1.0f;
    float point_h = 1.0f;

    ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"DEATH_SCRN_SIZE", 0), &point_w, &point_h, nullptr);

    int show_subtitles = 0;
    FalloutCfg_Read_Int("preferences", "subtitles", &show_subtitles);

    char* deatNarrationName = fall_GetDeathTitle();

    if (show_subtitles) {
        char txt[512];
        if (fall_Get_Death_Subtitles(deatNarrationName, txt) == 0) {
            //remove the number and ':' before each line of text. The original just turned them into spaces leaving large gaps in the text.
            char txt2[512] = { 0 };
            int c1 = 0;
            int c2 = 0;
            while (c1 < 512 - 1 && txt[c1] != '\0') {
                if (txt[c1] == ':') {
                    if (c2 - 1 >= 0) {
                        c2 -= 1;
                        c1 += 1;
                    }
                }
                if (c1 < 512 - 1)
                    txt2[c2] = txt[c1];
                c1++;
                c2++;
            }
            Subtitles_Set(txt2, 560);
        }

    }

    TransitionalFader_Start(true);
    TransitionalFader_Complete();

    deathNarration_Ended = false;
    fall_Speech_Set_Callback_Function(&DeathNarrationCallBack);
    ULONGLONG waitTime = -1;//set wait time to max

    if (fall_Speech_Load(deatNarrationName, 0xA, 0xE, 0xF) == -1)
        waitTime = 3000;//set a 3 second delay if there is no speech 
    fall_Speech_Play();

    LONG input = -1;
    ULONGLONG oldTick = GetTickCount64();
    ULONGLONG newTick = oldTick;
    while (newTick - oldTick < waitTime && !deathNarration_Ended && input == -1) {
        newTick = GetTickCount64();
        input = fall_Get_Input();
    }
    fall_Speech_Set_Callback_Function(nullptr);
    fall_Speech_Close();
    while (*pMOUSE_FLAGS != 0) {
        fall_Get_Input();
    }
    if (input == -1) {
        waitTime = 500;
        oldTick = GetTickCount64();
        while (newTick - oldTick < waitTime) {
            newTick = GetTickCount64();
            fall_Process_Input();
        }
    }

    TransitionalFader_Start(false);
    TransitionalFader_Complete();

    fall_Win_Destroy(winRef_Death);
    winRef_Death = -1;
    winRef_DeathImage = -1;
    winDx = nullptr;
    subwin = nullptr;
    Subtitles_Destroy();

    if (was_mouse_hidden)
        fall_Mouse_Hide();
    fall_Mouse_SetImage(1);
}





//______________________________________
void __declspec(naked)death_scrn_x(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call DeathScrnX

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//__________________________
void Modifications_Death_CH(void) {
    //To-Do all Modifications_Death_CH

   ///done///004807EE  |.  E8 3F890500       CALL 004D9132                            ; \Fallout2.004D9132, colour_fill(EAX *buff, EDX subWidth, EBX subHeight, ECX Width, Arg1 colour)
//   FuncWrite32(0x4807EF, 0x05893F,  (DWORD)&death_create_text_box);

   ///done///00480832  |.  FF15 94E15200     |CALL DWORD PTR DS:[52E194]              ; void print_text(EAX *tBuff, EDX *DisplayText, ECX tWidth, EBX TxtWidth, Arg1 ColourIndex)
//   MemWrite16(0x480832, 0x15FF, 0xE890);
//   FuncWrite32(0x480834, 0x52E194,  (DWORD)&death_print_text_ch);


   //DEATH DRAW FIX
   //004806ED  |. E8 CA870500    CALL Fallout2.004D8EBC
//   FuncWrite32(0x4806EE, 0x0587CA,  (DWORD)&DrawDeathBack);

   //004806A9  |.  E8 728AF9FF   CALL 00419120                            ; [Fallout2.00419120
//   FuncWrite32(0x4806AA, 0xFFF98A72,  (DWORD)&get_death_frm_vars);


   ///done///00480665  |.  E8 53BA0500   CALL 004DC0BD                            ; \Fallout2.004DC0BD
//   FuncWrite32(0x480666, 0x05BA53,  (DWORD)&CreateWin_Death);
   ///done///0048093C  |.  E8 7FBA0500   CALL 004DC3C0                            ; [Fallout2.004DC3C0, destroy_win(EAX winRef)
//   FuncWrite32(0x48093D, 0x05BA7F,  (DWORD)&destroy_win_death);

}


//______________________________
void Modifications_Death_MULTI() {

    pDeathSubtitle_PalOffset = (BYTE*)FixAddress(0x6AB8CF);

    fall_GetDeathTitle = (char* (*)())FixAddress(0x440D8C);

    pfall_Get_Death_Subtitles = (void*)FixAddress(0x4814B4);
    pfall_Cut_Death_Subtitles = (void*)FixAddress(0x481598);

    MemWrite8(0x48118C, 0x53, 0xE9);
    FuncWrite32(0x48118D, 0x57565251, (DWORD)&death_scrn_x);
}


//________________________
void Modifications_Death() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Death_CH();
    else
        Modifications_Death_MULTI();
}





