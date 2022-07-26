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

#include "Fall_Graphics.h"
#include "Fall_General.h"
#include "Fall_GameMap.h"

#include "Dx_Windows.h"
#include "Dx_Graphics.h"


int winRefHelpScrn = -1;
int winRefHelpScrnImage = -1;


//_________________________________________________
void OnScreenResize_HelpScrnX(Window_DX* pWin_This) {
    if (!pWin_This)
        return;

    pWin_This->ResizeWindow(SCR_WIDTH, SCR_HEIGHT);
    pWin_This->ClearRenderTarget(nullptr);

    Window_DX* subwin = pWin_This->GetSubWin(winRefHelpScrnImage);
    if (!subwin)
        return;
    ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"HELP_SCRN_SIZE", 0), nullptr, nullptr, nullptr);
}


//______________
void HelpScrnX() {
    LONG were_game_events_disabled = fall_DisableGameEvents();
    fall_Mouse_SetImage(1);//needed to help clear mouse image - when mouse is arrow head image already = 0
    fall_Mouse_SetImage(0);

    winRefHelpScrn = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, FLG_WinHidden);
    if (winRefHelpScrn == -1) {
        if (were_game_events_disabled)
            fall_EnableGameEvents();
        return;
    }
    WinStructDx* win = nullptr;
    win = (WinStructDx*)fall_Win_Get(winRefHelpScrn);

    Window_DX* winDx = win->winDx;
    if (!win || !winDx) {
        fall_Win_Destroy(winRefHelpScrn);
        winRefHelpScrn = -1;
        winRefHelpScrnImage = -1;
        if (were_game_events_disabled)
            fall_EnableGameEvents();
        return;

    }
    winDx->SetDrawFlag(false);
    winDx->Set_OnScreenResizeFunction(&OnScreenResize_HelpScrnX);
    winDx->ClearRenderTarget(nullptr);

    FRMdx* frmHelpBG = new FRMdx("helpscrn", ART_INTRFACE, -1);
    if (!frmHelpBG) {
        if (were_game_events_disabled)
            fall_EnableGameEvents();
        return;
    }
    FRMframeDx* pFrame = frmHelpBG->GetFrame(0, 0);
    if (!pFrame) {
        delete frmHelpBG;
        frmHelpBG = nullptr;
        fall_Win_Destroy(winRefHelpScrn);
        winRefHelpScrn = -1;
        winRefHelpScrnImage = -1;
        if (were_game_events_disabled)
            fall_EnableGameEvents();
        return;
    }

    Window_DX* subwin = nullptr;
    subwin = new Window_DX(0, 0, pFrame->GetWidth(), pFrame->GetHeight(), 0x000000FF, win->winDx, &winRefHelpScrnImage);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    delete frmHelpBG;
    frmHelpBG = nullptr;
    pFrame = nullptr;
    ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"HELP_SCRN_SIZE", 0), nullptr, nullptr, nullptr);

    fall_Win_Show(winRefHelpScrn);

    while (fall_Get_Input() == -1 && *pGAME_EXIT_FLAGS == 0) {
    }
    while (*pMOUSE_FLAGS != 0) {
        fall_Get_Input();
    }

    fall_Win_Destroy(winRefHelpScrn);
    winRefHelpScrn = -1;
    winRefHelpScrnImage = -1;
    winDx = nullptr;
    subwin = nullptr;

    if (were_game_events_disabled)
        fall_EnableGameEvents();
}


//_____________________________________
void __declspec(naked) helpscrn_x(void) {

    __asm {
        pushad
        call HelpScrnX
        popad
        ret
    }
}


//_______________________
void Modifications_Help() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        MemWrite8(0x4437E4, 0x53, 0xE9);
        FuncWrite32(0x4437E5, 0x57565251, (DWORD)&helpscrn_x);
    }
    else {
        MemWrite8(0x443F74, 0x53, 0xE9);
        FuncWrite32(0x443F75, 0x57565251, (DWORD)&helpscrn_x);
    }
}
