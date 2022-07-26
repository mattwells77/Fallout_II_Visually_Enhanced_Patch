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
#include "Fall_File.h"

#include "Dx_General.h"
#include "Dx_Windows.h"
#include "Dx_Graphics.h"
using namespace DirectX;
using namespace DirectX::PackedVector;


BYTE **winBuff_EndSlides=nullptr;

int winRef_EndSlidesImage = -1;


LONG* pEndSlides_Speech_Loaded = nullptr;
LONG* pEndSlides_Subtitles_Loaded = nullptr;

LONG* pEndSlides_Subs_Total = nullptr;

DWORD** ppEndSlides_Subs_Time = nullptr;

CHAR*** pppEndSlides_Subs_Text = nullptr;

BYTE* pEndSlides_Subtitle_PalOffset = nullptr;

ULONGLONG ull_endSlides_Subs_Start_Time = 0;

bool end_slides_speech_done = false;
bool is_slide_scrolling = false;

bool can_slide_scroll = true;
float slide_scroll_xMax = 0;
float slide_scroll_xStep = 0;
float slide_scroll_xPos = 0;

void* pfall_EndSlides_Speech_Load = nullptr;
void* (*fall_EndSlides_Subtitles_Destroy)() = nullptr;

DWORD (*fall_Get_Current_Sound_Length)() = nullptr;


//_____________________________
void EndSlides_SpeechCallBack() {
    end_slides_speech_done = true;
}


//____________________________________________________________
void fall_LoadSpeech_EndingSlides(const char* pFileName_noEXT) {
    __asm {
        mov eax, pFileName_noEXT
        call pfall_EndSlides_Speech_Load
    }
}


//______________________________________
void ProcessInputFor(ULONGLONG waitTime) {
    ULONGLONG oldTick = GetTickCount64();
    ULONGLONG newTick = oldTick;
    oldTick = GetTickCount64();
    while (newTick - oldTick < waitTime) {
        newTick = GetTickCount64();
        fall_Process_Input();
    }
}


//________________________________________________________
bool EndingSlides_Subtitles_Print_Next_Section(bool reset) {
    static bool sub_displayed = false;
    static LONG sub_num = 0;
    if (reset) {
        sub_num = 0;
        sub_displayed = false;
    }

    if (sub_num >= *pEndSlides_Subs_Total) {
        if (*pEndSlides_Subtitles_Loaded != 0)
            return true;
        return false;
    }
    ULONGLONG tickDiff = GetTickCount64() - ull_endSlides_Subs_Start_Time;
    DWORD* pSubsTime = *ppEndSlides_Subs_Time;

    if (tickDiff > pSubsTime[sub_num]) {
        sub_num++;
        sub_displayed = false;
        return false;
    }

    if (sub_displayed)
        return false;
    sub_displayed = true;

    char** pSubsText = *pppEndSlides_Subs_Text;
    char* txt = pSubsText[sub_num];

    Subtitles_Set(txt, 560);
    return false;
}


//__________________________________________________
void OnScreenResize_EndSlidesX(Window_DX* pWin_This) {

    if (!pWin_This)
        return;

    pWin_This->ResizeWindow(SCR_WIDTH, SCR_HEIGHT);
    pWin_This->ClearRenderTarget(nullptr);

    Window_DX* subwin = pWin_This->GetSubWin(winRef_EndSlidesImage);
    if (!subwin)
        return;

    if (is_slide_scrolling) {
        float point_h = (float)subwin->GetHeight() / SCR_HEIGHT;// scale the scrolling image to fit the screen height.
        float point_w = point_h;
        subwin->SetScale(1 / point_w, 1 / point_h);

        float imageWidthScaled = (float)subwin->GetWidth() / point_w;
        DWORD speechTime = (fall_Get_Current_Sound_Length() - 1) * 1000; // minus one second to allow scrolling image to fade out completely before speech ends.
        slide_scroll_xMax = imageWidthScaled - (float)SCR_WIDTH;
        slide_scroll_xStep = slide_scroll_xMax / speechTime;
        can_slide_scroll = true;
        if (imageWidthScaled < (float)SCR_WIDTH) {
            can_slide_scroll = false;
            subwin->SetPosition(((float)SCR_WIDTH - imageWidthScaled) / 2, 0);
        }
        else
            slide_scroll_xPos = 0;

    }
    else
        ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"END_SLIDE_SIZE", 0), nullptr, nullptr, nullptr);
}




//_______________________________________________________________
void EndingSlides_Still(DWORD frmID, const char* pFileName_noEXT) {

    WinStructDx* pWin = nullptr;
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_EndSlides);

    if (!pWin || !pWin->winDx) {
        fall_Win_Destroy(*pWinRef_EndSlides);
        *pWinRef_EndSlides = -1;
        return;
    }

    FRMCached* pfrm = new FRMCached(frmID, -1);
    if (!pfrm)
        return;

    FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
    if (!pFrame) {
        delete pfrm;
        pfrm = nullptr;
        return;
    }
    is_slide_scrolling = false;

    Window_DX* subwin = nullptr;
    subwin = new Window_DX(0, 0, pFrame->GetWidth(), pFrame->GetHeight(), 0x000000FF, pWin->winDx, &winRef_EndSlidesImage);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    float point_w = 1.0f;
    float point_h = 1.0f;

    ScaleWindowToScreen(subwin, ConfigReadInt(L"STATIC_SCREENS", L"END_SLIDE_SIZE", 0), &point_w, &point_h, nullptr);

    fall_LoadSpeech_EndingSlides(pFileName_noEXT);

    ULONGLONG waitTime = -1;//set wait time to max
    if (*pEndSlides_Subtitles_Loaded == 0 && *pEndSlides_Speech_Loaded == 0) {
        waitTime = 3000;
    }

    TransitionalFader_Start(true);
    TransitionalFader_Complete();

    ProcessInputFor(500);

    end_slides_speech_done = false;
    if (*pEndSlides_Speech_Loaded)
        fall_Speech_Play();

    ULONGLONG startTime = GetTickCount64();
    ULONGLONG runningTime = 0;
    if (*pEndSlides_Subtitles_Loaded) {
        ull_endSlides_Subs_Start_Time = startTime;
    }

    fall_EventProcessing_Disable();

    LONG input = -1;
    bool subs_start = true;
    bool subs_done = false;

    while (runningTime < waitTime && !end_slides_speech_done && !subs_done && input == -1) {
        runningTime = GetTickCount64() - startTime;
        input = fall_Get_Input();


        subs_done = EndingSlides_Subtitles_Print_Next_Section(subs_start);
        subs_start = false;
        fall_Sound_Continue_ALL();
        Dx_Present_Main();
    }
    fall_EventProcessing_Enable();
    fall_Speech_Close();
    fall_EndSlides_Subtitles_Destroy();

    *pEndSlides_Speech_Loaded = 0;
    *pEndSlides_Subtitles_Loaded = 0;

    if (input == -1) {
        ProcessInputFor(500);
    }

    TransitionalFader_Start(false);
    TransitionalFader_Complete();

    while (*pMOUSE_FLAGS != 0) {
        fall_Get_Input();
    }

    if (winRef_EndSlidesImage != -1)
        pWin->winDx->DeleteSubWin(winRef_EndSlidesImage);
    winRef_EndSlidesImage = -1;

    Subtitles_Destroy();
}


//______________________________________________
void __declspec(naked) ending_slides_still(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call EndingSlides_Still
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//____________________________________________________________________
void EndingSlides_Scroll(LONG xDirection, const char* pFileName_noEXT) {

    //fade 1/4 of scroll in then out

    WinStructDx* pWin = nullptr;
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_EndSlides);

    if (!pWin || !pWin->winDx) {
        fall_Win_Destroy(*pWinRef_EndSlides);
        *pWinRef_EndSlides = -1;
        return;
    }
    DWORD frmID = fall_GetFrmID(ART_INTRFACE, 0x147, 0, 0, 0);
    FRMCached* pfrm = new FRMCached(frmID, -1);
    if (!pfrm)
        return;

    FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
    if (!pFrame) {
        delete pfrm;
        pfrm = nullptr;
        return;
    }
    is_slide_scrolling = true;


    fall_LoadSpeech_EndingSlides(pFileName_noEXT);

    ULONGLONG waitTime = -1;//set wait time to max
    if (*pEndSlides_Subtitles_Loaded == 0 && *pEndSlides_Speech_Loaded == 0) {
        waitTime = 3000;
    }

    ProcessInputFor(500);

    end_slides_speech_done = false;
    if (*pEndSlides_Speech_Loaded)
        fall_Speech_Play();

    ULONGLONG startTime = GetTickCount64();
    ULONGLONG runningTime = 0;
    ULONGLONG runningTime_Last = 0;

    if (*pEndSlides_Subtitles_Loaded) {
        ull_endSlides_Subs_Start_Time = startTime;
    }

    fall_EventProcessing_Disable();

    LONG input = -1;
    bool subs_start = true;
    bool subs_done = false;

    float xPos = 0;

    DWORD speechTime = (fall_Get_Current_Sound_Length() - 1) * 1000; // minus one second to allow scrolling image to fade out completely before speech ends.


    Window_DX* subwin = nullptr;
    subwin = new Window_DX(0, 0, pFrame->GetWidth() - 1, pFrame->GetHeight(), 0x000000FF, pWin->winDx, &winRef_EndSlidesImage);// original DP.frm has a blank column of pixels down the right side, minus 1 to width to remove.

    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    float point_h = (float)subwin->GetHeight() / SCR_HEIGHT;// scale the scrolling image to fit the screen height.
    float point_w = point_h;
    subwin->SetScale(1 / point_w, 1 / point_h);

    float imageWidthScaled = (float)subwin->GetWidth() / point_w;

    slide_scroll_xMax = imageWidthScaled - (float)SCR_WIDTH;
    slide_scroll_xStep = slide_scroll_xMax / speechTime;

    DWORD fadeLength = speechTime / 4;
    XMFLOAT4 fadeColour = { 0.0f, 0.0f, 0.0f, 1.0f };
    TransitionalFader_SetColour(fadeColour);

    if (xDirection == 1) {
        xPos = 0;
    }
    else {
        xPos = -slide_scroll_xMax;
    }
    can_slide_scroll = true;
    if (imageWidthScaled < (float)SCR_WIDTH) {
        can_slide_scroll = false;
        subwin->SetPosition(((float)SCR_WIDTH - imageWidthScaled) / 2, 0);
    }
    else
        subwin->SetPosition(xPos, 0);

    float newX = 0;
    slide_scroll_xPos = 0;

    while (runningTime < waitTime && !end_slides_speech_done && !subs_done && input == -1) {

        runningTime = GetTickCount64() - startTime;
        input = fall_Get_Input();
        if (can_slide_scroll && runningTime > runningTime_Last) {
            runningTime_Last = runningTime;
            newX = runningTime * slide_scroll_xStep;
            if (newX >= slide_scroll_xPos + slide_scroll_xStep && newX <= slide_scroll_xMax) {
                slide_scroll_xPos = newX;
                if (xDirection == 1)
                    xPos = -newX;
                else
                    xPos = -slide_scroll_xMax + newX;
                subwin->SetPosition(xPos, 0);
            }

            if (runningTime <= fadeLength) {
                fadeColour.w = 1.0f - ((float)runningTime / fadeLength);
                TransitionalFader_SetColour(fadeColour);
            }
            else if (runningTime >= speechTime - fadeLength) {
                if (runningTime <= speechTime) {
                    fadeColour.w = 1.0f - ((float)(speechTime - runningTime) / fadeLength);
                    TransitionalFader_SetColour(fadeColour);
                }
            }
            else if (fadeColour.w != 0.0f) {
                fadeColour.w = 0.0f;
                TransitionalFader_SetColour(fadeColour);
            }
        }

        subs_done = EndingSlides_Subtitles_Print_Next_Section(subs_start);
        subs_start = false;
        fall_Sound_Continue_ALL();
        Dx_Present_Main();
    }

    fall_EventProcessing_Enable();
    fall_Speech_Close();
    fall_EndSlides_Subtitles_Destroy();

    *pEndSlides_Speech_Loaded = 0;
    *pEndSlides_Subtitles_Loaded = 0;

    if (input == -1) {
        ProcessInputFor(500);
    }

    TransitionalFader_Start(false);
    TransitionalFader_Complete();

    while (*pMOUSE_FLAGS != 0) {
        fall_Get_Input();
    }

    if (winRef_EndSlidesImage != -1)
        pWin->winDx->DeleteSubWin(winRef_EndSlidesImage);
    winRef_EndSlidesImage = -1;

    Subtitles_Destroy();
}


//_______________________________________________
void __declspec(naked) ending_slides_scroll(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call EndingSlides_Scroll
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________________________________________
LONG _stdcall CreateWin_EndSlides(DWORD colour, DWORD winFlags) {

    LONG winRef = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, colour, winFlags);
    if (winRef == -1)
        return winRef;

    WinStructDx* pWin = nullptr;
    pWin = (WinStructDx*)fall_Win_Get(winRef);

    if (!pWin || !pWin->winDx) {
        fall_Win_Destroy(winRef);
        return -1;
    }

    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_EndSlidesX);
    return winRef;
}


//_______________________________
void Modifications_EndSlides_CH() {
    //To-Do All Modifications_EndSlides_CH


    winBuff_EndSlides = (BYTE**)FixAddress(0x580BF0);

    FuncWrite32(0x43EF4E, 0x09D16B, (DWORD)&CreateWin_EndSlides);
}


//__________________________________
void Modifications_EndSlides_MULTI() {


    winBuff_EndSlides = (BYTE**)FixAddress(0x570BF0);

    pfall_EndSlides_Speech_Load = (void*)FixAddress(0x4401A0);

    pEndSlides_Speech_Loaded = (LONG*)FixAddress(0x570AB8);
    pEndSlides_Subtitles_Loaded = (LONG*)FixAddress(0x570BE0);

    pEndSlides_Subs_Total = (LONG*)FixAddress(0x518668);
    ppEndSlides_Subs_Time = (DWORD**)FixAddress(0x570BE8);
    pppEndSlides_Subs_Text = (char***)FixAddress(0x570BC8);

    fall_EndSlides_Subtitles_Destroy = (void* (*)())FixAddress(0x4406CC);

    pEndSlides_Subtitle_PalOffset = (BYTE*)FixAddress(0x6AB8CF);

    fall_Get_Current_Sound_Length = (DWORD(*)())FixAddress(0x450C94);

    FuncReplace32(0x43FA1E, 0x096816, (DWORD)&CreateWin_EndSlides);

    MemWrite8(0x440004, 0x53, 0xE9);
    FuncWrite32(0x440005, 0x55575651, (DWORD)&ending_slides_still);

    MemWrite8(0x43FBDC, 0x53, 0xE9);
    FuncWrite32(0x43FBDD, 0x55575651, (DWORD)&ending_slides_scroll);

    MemWrite32(0x43FA48, FixAddress(0x4403F0), (DWORD)&EndSlides_SpeechCallBack);

    //extended the string length of ending slide subtitles from 256 to 512, as some were being clipped.
    MemWrite32(0x440404, 256, 512);
    MemWrite32(0x4404CE, 256, 512);
    MemWrite32(0x4404E0, 256, 512);
    MemWrite32(0x440426, 256, 512);
}


//____________________________
void Modifications_EndSlides() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_EndSlides_CH();
    else
        Modifications_EndSlides_MULTI();

}





