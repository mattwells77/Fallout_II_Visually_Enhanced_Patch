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
#include "modifications.h"
#include "Fall_General.h"
#include "Fall_Windows.h"
#include "Fall_File.h"
#include "Fall_Graphics.h"
#include "Fall_Text.h"
#include "Fall_Msg.h"
#include "Fall_Objects.h"
#include "Fall_GameMap.h"
#include "Fall_Scripts.h"

#include "text.h"
#include "win_fall.h"

HINSTANCE phinstDLL;


void Initialize() {

    if (!Set_Fallout_EXE_Region())
        return;
    Fallout_Functions_Setup_Graphics();
    Fallout_Functions_Setup_File();
    Fallout_Functions_Setup_Text();
    Fallout_Functions_Setup_Msg();
    Fallout_Functions_Setup_General();
    Fallout_Functions_Setup_Windows();
    Fallout_Functions_Setup_Objects();
    Fallout_Functions_Setup_Scripts();
    Fallout_Functions_Setup_GameMap();

    Modifications_Text();
    Modifications_Win();

    Modifications_Dx_General();
    Modifications_Dx_Game();
    Modifications_Dx_Window();
    Modifications_Dx_Movies();
    Modifications_Dx_Mouse();

    Modifications_Game_Map();
    Modifications_Splash();
    Modifications_Character();
    Modifications_Credits();
    Modifications_Dialogue();
    Modifications_Inventory();
    Modifications_Options();
    Modifications_EndSlides();
    Modifications_WorldMap();
    Modifications_Death();
    Modifications_Movie();
    Modifications_LoadSave();
    Modifications_MainMenu();
    Modifications_Interface_Bar();
    Modifications_Pipboy();
    Modifications_Help();
    Modifications_Pause();
    Modifications_Other();
}


//__________________________________________________________________________________
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Initialize();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Fallout_On_Exit();
        break;
    }
    return TRUE;
}

