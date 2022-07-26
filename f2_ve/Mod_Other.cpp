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

//To-Do Lots to do here
//To-Do Update Called Shot window - text not highlighting
#include "pch.h"
#include "modifications.h"
#include "memwrite.h"
#include "configTools.h"
#include "win_fall.h"

#include "Fall_Graphics.h"
#include "Fall_General.h"

#include "Dx_Windows.h"


//_____________________________________________________
void OnScreenResize_BackGroundWin(Window_DX* pWin_This) {
    pWin_This->SetScale((float)SCR_WIDTH / (float)pWin_This->GetWidth(), (float)SCR_HEIGHT / (float)pWin_This->GetHeight());
}


//____________________________________________________
LONG BackGroundWin_Setup(DWORD colour, DWORD winFlags) {

    LONG winRef = fall_Win_Create(0, 0, 1, 1, colour, winFlags);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (pWin && pWin->winDx) {
        pWin->winDx->SetScale((float)SCR_WIDTH / 1.0f, (float)SCR_HEIGHT / 1.0f);
        pWin->winDx->ClearRenderTarget(nullptr);
        pWin->winDx->SetDrawFlag(false);
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_BackGroundWin);
    }
    return winRef;
}


//_____________________________________
void __declspec(naked) h_bg_win_setup() {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        call BackGroundWin_Setup
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}


//_____________________________________________________
void __declspec(naked) gen_win_create_centred_on_game() {
    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        push ecx
        push ebx
        call Win_Create_CenteredOnGame
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}


//______________________________
void __declspec(naked) get_480() {
    __asm {
        mov eax, 480
        ret
    }
}


//______________________________
void __declspec(naked) get_640() {
    __asm {
        mov eax, 640
        ret
    }
}

//___________________________
void Modifications_Other_CH() {

    //Say Window Fixes---
    FuncReplace32(0x462228, 0x055AE8, (DWORD)&get_480);
    FuncReplace32(0x462243, 0x055AC5, (DWORD)&get_640);
    //createwin
    FuncReplace32(0x4626F7, 0x055611, (DWORD)&get_640);
    FuncReplace32(0x46270F, 0x0555F9, (DWORD)&get_640);
    FuncReplace32(0x462722, 0x0555EE, (DWORD)&get_480);
    FuncReplace32(0x46273E, 0x0555D2, (DWORD)&get_480);
    //resizewin
    FuncReplace32(0x46287B, 0x05548D, (DWORD)&get_640);
    FuncReplace32(0x462893, 0x055475, (DWORD)&get_640);
    FuncReplace32(0x4628A6, 0x05546A, (DWORD)&get_480);
    FuncReplace32(0x4628C2, 0x05544E, (DWORD)&get_480);
    //scalewin
    FuncReplace32(0x4629F3, 0x055315, (DWORD)&get_640);
    FuncReplace32(0x462A0B, 0x0552FD, (DWORD)&get_640);
    FuncReplace32(0x462A1E, 0x0552F2, (DWORD)&get_480);
    FuncReplace32(0x462A3A, 0x0552D6, (DWORD)&get_480);
    //addbutton
    FuncReplace32(0x46346A, 0x0548A6, (DWORD)&get_480);
    FuncReplace32(0x463481, 0x054887, (DWORD)&get_640);
    FuncReplace32(0x463499, 0x054877, (DWORD)&get_480);
    FuncReplace32(0x4634B5, 0x054853, (DWORD)&get_640);

    //centre windows---
    //Target
    FuncWrite32(0x425EB0, 0x0B6209, (DWORD)&gen_win_create_centred_on_game);
    //ELEVATOR
    FuncWrite32(0x43EA91, 0x09D628, (DWORD)&gen_win_create_centred_on_game);
    //IN GAME MENU
    FuncWrite32(0x48F406, 0x04CCB3, (DWORD)&gen_win_create_centred_on_game);
    //MESSAGE_BOX
    FuncWrite32(0x41D095, 0x0BF024, (DWORD)&gen_win_create_centred_on_game);
    //LOCAL_MAP
    FuncWrite32(0x41B90A, 0x0C07AF, (DWORD)&gen_win_create_centred_on_game);
    //options window
    FuncWrite32(0x48FD06, 0x04C3B3, (DWORD)&gen_win_create_centred_on_game);
    //bio scrn window
    FuncWrite32(0x4A6197, 0x035F22, (DWORD)&gen_win_create_centred_on_game);
    //char scrn window
    FuncWrite32(0x432A6C, 0x0A964D, (DWORD)&gen_win_create_centred_on_game);
    //Pipboy
    FuncWrite32(0x4961A5, 0x045F14, (DWORD)&gen_win_create_centred_on_game);

    //black background windows---
    //QUICK LOAD BACKGROUND WIN
    FuncWrite32(0x47BCF3, 0x0603C6, (DWORD)&h_bg_win_setup);
    //To-Do skip over QUICK LOAD BACKGROUND FILL BLACK
    //MemWrite8(0x, 0x74, 0xEB);

    //Load from main-menu background window
    //To-Do set bg behind main-menu
    //MemWrite8(0x, 0x14, FLG_WinExclusive | FLG_WinToBack);
    FuncWrite32(0x47FF8E, 0x05C12B, (DWORD)&h_bg_win_setup);
    //Game start background window - to hide loading map
    FuncWrite32(0x480212, 0x05BEA7, (DWORD)&h_bg_win_setup);
}


//______________________________
void Modifications_Other_MULTI() {

    //Say Window Fixes---
    FuncReplace32(0x462B28, 0x056524, (DWORD)&get_480);
    FuncReplace32(0x462B43, 0x056501, (DWORD)&get_640);
    //createwin
    FuncReplace32(0x462FF7, 0x05604D, (DWORD)&get_640);
    FuncReplace32(0x46300F, 0x056035, (DWORD)&get_640);
    FuncReplace32(0x463022, 0x05602A, (DWORD)&get_480);
    FuncReplace32(0x46303E, 0x05600E, (DWORD)&get_480);
    //resizewin
    FuncReplace32(0x46317B, 0x055EC9, (DWORD)&get_640);
    FuncReplace32(0x463193, 0x055EB1, (DWORD)&get_640);
    FuncReplace32(0x4631A6, 0x055EA6, (DWORD)&get_480);
    FuncReplace32(0x4631C2, 0x055E8A, (DWORD)&get_480);
    //scalewin
    FuncReplace32(0x4632F3, 0x055D51, (DWORD)&get_640);
    FuncReplace32(0x46330B, 0x055D39, (DWORD)&get_640);
    FuncReplace32(0x46331E, 0x055D2E, (DWORD)&get_480);
    FuncReplace32(0x46333A, 0x055D12, (DWORD)&get_480);
    //addbutton
    FuncReplace32(0x463D6A, 0x0552E2, (DWORD)&get_480);
    FuncReplace32(0x463D81, 0x0552C3, (DWORD)&get_640);
    FuncReplace32(0x463D99, 0x0552B3, (DWORD)&get_480);
    FuncReplace32(0x463DB5, 0x05528F, (DWORD)&get_640);

    //centre windows---
    //Target
    FuncReplace32(0x42626C, 0x0AFFC8, (DWORD)&gen_win_create_centred_on_game);
    //ELEVATOR
    FuncReplace32(0x43F561, 0x096CD3, (DWORD)&gen_win_create_centred_on_game);
    //IN GAME MENU
    FuncReplace32(0x490006, 0x04622E, (DWORD)&gen_win_create_centred_on_game);
    // MESSAGE_BOX
    FuncReplace32(0x41D105, 0x0B912F, (DWORD)&gen_win_create_centred_on_game);
    //LOCAL_MAP
    FuncReplace32(0x41B97A, 0x0BA8BA, (DWORD)&gen_win_create_centred_on_game);
    //options window
    FuncReplace32(0x490962, 0x0458D2, (DWORD)&gen_win_create_centred_on_game);
    //bio scrn panel
    FuncReplace32(0x4A7497, 0x02ED9D, (DWORD)&gen_win_create_centred_on_game);
    //char scrn panel
    FuncReplace32(0x432DE9, 0x0A344B, (DWORD)&gen_win_create_centred_on_game);
    //Pipboy
    FuncReplace32(0x497406, 0x03EE2E, (DWORD)&gen_win_create_centred_on_game);

    //black background windows---
     //Quick load background window - to hide loading map
    FuncReplace32(0x47C6F3, 0x059B41, (DWORD)&h_bg_win_setup);
    //skip over QUICK LOAD BACKGROUND FILL BLACK
    MemWrite8(0x47C6FE, 0x74, 0xEB);

    //Load from main-menu background window
    //set bg behind main-menu
    MemWrite8(0x480AF6, 0x14, FLG_WinExclusive | FLG_WinToBack);
    FuncReplace32(0x480B0E, 0x055726, (DWORD)&h_bg_win_setup);
    //Game start background window - to hide loading map
    FuncReplace32(0x480D8E, 0x0554A6, (DWORD)&h_bg_win_setup);
}


//___________________________
void cd_checker(int CD_CHECK) {
    if (CD_CHECK == 0) {
        if (*(DWORD*)0x4426A4 == 1)
            MemWrite8(0x4426A4, 0x1, 0x0);
    }
}


//________________________
void Modifications_Other() {

   switch (fallout_exe_region) {
   case EXE_Region::USA:
      case EXE_Region::Russian_Lev_Corp:
         Modifications_Other_MULTI();
         break;
      case EXE_Region::UK:
      case EXE_Region::French_German:
         Modifications_Other_MULTI();
         cd_checker(ConfigReadInt(L"OTHER_SETTINGS", L"CD_CHECK", 0));
         break;
      case EXE_Region::Chinese:
         Modifications_Other_CH();
         break;
      default:
         break;
   }
}


