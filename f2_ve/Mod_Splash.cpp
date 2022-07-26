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

#include "Fall_File.h"
#include "Fall_General.h"

#include "Dx_Windows.h"

int winRef_Splash = -1;


//_____________
bool IsSplash() {
    if (winRef_Splash != -1)
        return true;
    return false;
}

//___________
void Splash() {

    int SPLASH_SCRN_SIZE = ConfigReadInt(L"STATIC_SCREENS", L"SPLASH_SCRN_SIZE", 0);
    int SPLASH_DELAY = ConfigReadInt(L"OTHER_SETTINGS", L"SPLASH_SCRN_TIME", 0) * 1000;
    if (SPLASH_DELAY > 20000)
        SPLASH_DELAY = 20000;

    char splashPath[128];

    int splashNum = 0;
    FalloutCfg_Read_Int("system", "splash", &splashNum);
    char* language = nullptr;
    if (FalloutCfg_Read_String("system", "language", &language)) {
        int l = 0;
        while (language[l] != '\0') {
            language[l] = tolower(language[l]);
            l++;
        }
    }

    if (strncmp("english", language, 7) && strncmp("cht", language, 3))
        sprintf_s(splashPath, _countof(splashPath), "art\\%s\\splash\\splash", language);
    else sprintf_s(splashPath, _countof(splashPath), "art\\splash\\splash");
    char* pPathEnd = splashPath + strlen(splashPath);//pointer in path to write the splash number to.
    size_t pathEndSize = _countof(splashPath) - strlen(splashPath) - 1;//max number of chars to write splash number to.

    FRMdx* pSplashFrmDx = nullptr;
    FRMframeDx* pSplashFrame = nullptr;
    int splashMax = 20;
    for (int i = 0; i <= splashMax; i++) {
        sprintf_s(pPathEnd, pathEndSize, "%d", splashNum);
        pSplashFrmDx = new FRMdx(splashPath, -1, -1);
        pSplashFrame = pSplashFrmDx->GetFrame(0, 0);
        if(!pSplashFrame) {
            delete pSplashFrmDx;
            pSplashFrmDx = nullptr;
            splashNum++;
            if (splashNum > splashMax)
                splashNum = 0;
        }
        else
            i = splashMax + 1;
    }
    
    if (!pSplashFrame) {
        delete pSplashFrmDx;
        pSplashFrmDx = nullptr;
        return;
    }
    DWORD frame_width = pSplashFrame->GetWidth();
    DWORD frame_height = pSplashFrame->GetHeight();

    winRef_Splash = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, FLG_WinHidden | FLG_WinToFront);
    WinStructDx* win = (WinStructDx*)fall_Win_Get(winRef_Splash);
    if (!win || !win->winDx) {
        delete pSplashFrmDx;
        pSplashFrmDx = nullptr;
        pSplashFrame = nullptr;
        return;
    }
    win->winDx->SetDrawFlag(false);
    Window_DX* subwin = new Window_DX(0, 0, pSplashFrame->GetWidth(), pSplashFrame->GetHeight(), 0x00000000, win->winDx, nullptr);
    subwin->RenderTargetDrawFrame(0, 0, pSplashFrame, nullptr, nullptr);
    delete pSplashFrmDx;
    pSplashFrmDx = nullptr;
    pSplashFrame = nullptr;

    //size of a pixel on screen - for scaling
    float point_w = 1.0f;
    float point_h = 1.0f;

    DWORD bgWidth = subwin->GetWidth();
    DWORD bgHeight = subwin->GetHeight();

    RECT rect_bg_scaled = { 0,0,0,0 };
    //set window size and scaling - no longer bothering with "SPLASH_SCRN_SIZE == 2" - stretch fit
    if (SPLASH_SCRN_SIZE == 1 || SCR_WIDTH < bgWidth || SCR_HEIGHT < bgHeight) {//scale - maintaining aspect of backgroung image - forced if image is larger than screen size
        float bg_imageRO = (float)bgWidth / bgHeight;
        float screenRO = (float)SCR_WIDTH / SCR_HEIGHT;
        DWORD bgWidth_scaled = 0;
        DWORD bgHeight_scaled = 0;

        if (bg_imageRO >= screenRO) {
            rect_bg_scaled.left = 0;
            bgWidth_scaled = bgWidth;
            bgHeight_scaled = (DWORD)(bgWidth_scaled / screenRO);
            rect_bg_scaled.top = (bgHeight_scaled - bgHeight) / 2;
        }
        else {
            rect_bg_scaled.top = 0;
            bgHeight_scaled = bgHeight;
            bgWidth_scaled = (DWORD)(bgHeight_scaled * screenRO);
            rect_bg_scaled.left = (bgWidth_scaled - bgWidth) / 2;
        }
        rect_bg_scaled.bottom = bgHeight_scaled - rect_bg_scaled.top;
        rect_bg_scaled.right = bgWidth_scaled - rect_bg_scaled.left;

        point_h = (float)bgHeight_scaled / SCR_HEIGHT;
        point_w = (float)bgWidth_scaled / SCR_WIDTH;

        subwin->SetScale((float)SCR_WIDTH / bgWidth_scaled, (float)SCR_HEIGHT / bgHeight_scaled);
    }
    else {//original size
        rect_bg_scaled.left = ((LONG)SCR_WIDTH - bgWidth) / 2;
        rect_bg_scaled.top = ((LONG)SCR_HEIGHT - bgHeight) / 2;
        if (rect_bg_scaled.left < 0)
            rect_bg_scaled.left = 0;
        if (rect_bg_scaled.top < 0)
            rect_bg_scaled.top = 0;
        rect_bg_scaled.bottom = bgHeight + rect_bg_scaled.top;
        rect_bg_scaled.right = bgWidth + rect_bg_scaled.left;
    }
    subwin->SetPosition((float)(rect_bg_scaled.left / point_w), (float)(rect_bg_scaled.top / point_h));

    fall_Palette_Set(pBLACK_PAL);

    fall_Win_Show(winRef_Splash);//unhide the window
    fall_Palette_FadeTo(pLOADED_PAL);

    ULONGLONG oldTick = GetTickCount64();
    ULONGLONG newTick = oldTick;
    while (oldTick + SPLASH_DELAY > newTick) {
        newTick = GetTickCount64();
        if (newTick < oldTick)
            oldTick = newTick;
    }
    FalloutCfg_Write_Int("system", "splash", splashNum + 1);
}


//________________________________
void __declspec(naked)splash(void) {

   __asm {
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp

      call Splash

      pop ebp
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
      ret
   }
}


//____________________________
void Splash_Destroy(BYTE* pal) {
    fall_Palette_FadeTo(pal);

    if (winRef_Splash != -1)
        fall_Win_Destroy(winRef_Splash);
    winRef_Splash = -1;
}


//________________________________________
void __declspec(naked)splash_destroy(void) {

   __asm {
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp

      push eax
      call Splash_Destroy
      add esp, 0x4

      pop ebp
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
      ret
   }
}


//_________________________
void Modifications_Splash() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        MemWrite8(0x443BF4, 0x53, 0xE9);
        FuncWrite32(0x443BF5, 0x57565251, (DWORD)&splash);

        //movie fade in
        FuncWrite32(0x44DEBB, 0x0449C5, (DWORD)&splash_destroy);
        //main-menu fade in
        FuncWrite32(0x480EAC, 0x0119D4, (DWORD)&splash_destroy);
    }
    else {
        MemWrite8(0x444384, 0x53, 0xE9);
        FuncWrite32(0x444385, 0x57565251, (DWORD)&splash);

        //movie fade in
        FuncReplace32(0x44E7C3, 0x04530D, (DWORD)&splash_destroy);
        //main-menu fade in
        FuncReplace32(0x481A7C, 0x012054, (DWORD)&splash_destroy);
    }
}







