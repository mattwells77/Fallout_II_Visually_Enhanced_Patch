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
#include "Fall_General.h"
#include "memwrite.h"
#include "modifications.h"

BYTE* pWHITE_PAL = nullptr;
BYTE* pBLACK_PAL = nullptr;
BYTE* pLOADED_PAL = nullptr;
BYTE* pCURRENT_PAL = nullptr;
BYTE* pACTIVE_PAL = nullptr;


BOOL* pIS_MOUSE_HIDDEN = nullptr;
DWORD* pMOUSE_FLAGS = nullptr;
DWORD* pMOUSE_FLAGS_BACK = nullptr;

DWORD* pGAME_EXIT_FLAGS = nullptr;

LONG* p_PC_party_size = nullptr;



LONG(*fall_Get_Input)() = nullptr;

void(*fall_Mouse_Show)() = nullptr;
void(*fall_Mouse_Hide)() = nullptr;
void(*fall_Mouse_ToggleHex)() = nullptr;
void* pfall_mouse_set_buff = nullptr;
void* pfall_mouse_get_pos = nullptr;
void* pfall_mouse_is_in_area = nullptr;
void* pfall_mouse_get_rect = nullptr;
void* pfall_mouse_set_pos = nullptr;
void *pfall_mouse_set_frm=nullptr;
void* pfall_mouse_set_image = nullptr;

void* pfall_game_mouse_disable = nullptr;
void* pfall_mouse_set_image_from_list = nullptr;

void* pfall_play_acm = nullptr;

void* pfall_message_box = nullptr;

void* pfall_mem_allocate = nullptr;
void* pfall_mem_reallocate = nullptr;
void* pfall_mem_deallocate = nullptr;

void* pfall_palette_load = nullptr;
void* pfall_palette_set = nullptr;
void* pfall_palette_fade_to = nullptr;
void* pfall_palette_set_active = nullptr;

void* pfall_send_key = nullptr;

void *pfall_event_add = nullptr;
void *pfall_event_remove = nullptr;
LONG* pIsEventProcessingDisabled = nullptr;

void (*fall_EnableGameEvents)() = nullptr;
LONG(*fall_DisableGameEvents)() = nullptr;

//not sure what these functions do, but are called in original death screen function.
void(*fall_Input_Related_01)() = nullptr;// clear keyboard buffer ??
void(*fall_Input_Related_02)() = nullptr;// clear input buffer ??

LONG(*fall_FreeArtCache)() = nullptr;

void(*fall_Speech_Close)() = nullptr;
void(*fall_Speech_Play)() = nullptr;
void* pfall_Speech_Load = nullptr;
void* pfall_Speech_Set_Callback_Function = nullptr;

void* pfall_background_sound_set = nullptr;
void(*fall_Background_Sound_Stop)() = nullptr;

void(*fall_Sound_Continue_ALL)() = nullptr;

void* pfall_TEXT_Divide_By_Width = nullptr;

int (*fall_Debug_printf)(const char* format, ...);

LONG(*fall_ExitMessageBox)() = nullptr;

void* pfall_GameTime_Get_Date = nullptr;
LONG(*fall_GameTime_Get_Time)() = nullptr;

void(*fall_Process_Input)() = nullptr;
void(*fall_Process_Map_Mouse)() = nullptr;
void(*fall_Update_Mouse_State)() = nullptr;



//___________________________________________________________________
void fall_GameTime_Get_Date(LONG* p_month, LONG* p_day, LONG* p_year) {
    __asm {
        mov ebx, p_year
        mov edx, p_day
        mov eax, p_month
        call pfall_GameTime_Get_Date
    }
}


//________________________________________________________________________________________________
LONG fall_Background_Sound_Set(const char* pFileName_noEXT, DWORD flag1, DWORD flag2, DWORD flag3) {
    LONG retVal = 0;
    __asm {
        mov ecx, flag3
        mov ebx, flag2
        mov edx, flag1
        mov eax, pFileName_noEXT
        call pfall_background_sound_set
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________
LONG fall_Background_Sound_Set(const char* pFileName_noEXT, DWORD flag1) {
    LONG retVal = 0;
    __asm {
        mov ecx, 16
        mov ebx, 14
        mov edx, flag1
        mov eax, pFileName_noEXT
        call pfall_background_sound_set
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________
void fall_Mouse_SetImageFromList(LONG listNum) {

    __asm {
        mov eax, listNum
        call pfall_mouse_set_image_from_list
    }

}


//_____________________________________
void fall_GameMouse_Disable(DWORD flag) {

    __asm {
        mov eax, flag
        call pfall_game_mouse_disable
    }

}


//______________________
void button_sound_Dn_1() {
    fall_PlayAcm("ib1p1xx1");
}

//______________________
void button_sound_Up_1() {
    fall_PlayAcm("ib1lu1x1");
}


//______________________
void button_sound_Dn_2() {
    fall_PlayAcm("ib2p1xx1");
}


//______________________
void button_sound_Up_2() {
    fall_PlayAcm("ib2lu1x1");
}


//______________________
void button_sound_Dn_3() {
    fall_PlayAcm("ib3p1xx1");
}


//______________________
void button_sound_Up_3() {
    fall_PlayAcm("ib3lu1x1");
}


//________________________________________________________________________________________________________
LONG fall_TEXT_Divide_By_Width(const char* txt, LONG widthInPixels, SHORT* pLineOffsets, SHORT* pNumLines) {
    LONG retVal = 0;
    __asm {
        mov ecx, pNumLines
        mov ebx, pLineOffsets
        mov edx, widthInPixels
        mov eax, txt
        call pfall_TEXT_Divide_By_Width
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________________
LONG fall_Speech_Load(char* pFileName_noEXT, DWORD flag1, DWORD flag2, DWORD flag3) {
    LONG retVal = 0;
    __asm {
        mov ecx, flag3
        mov ebx, flag2
        mov edx, flag1
        mov eax, pFileName_noEXT
        call pfall_Speech_Load
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________
void fall_Speech_Set_Callback_Function(void(*pFunc)()) {
    __asm {
        mov eax, pFunc
        call pfall_Speech_Set_Callback_Function
    }
}


//_________________________________
void fall_Event_Add(void (*func)()) {
    __asm {
        mov eax, func
        call pfall_event_add
    }
}


//_________________________________
void fall_Event_Remove(void (*func)()) {
    __asm {
        mov eax, func
        call pfall_event_remove
    }
}


//________________________________
void fall_EventProcessing_Enable() {
    *pIsEventProcessingDisabled = 0;
}


//_________________________________
void fall_EventProcessing_Disable() {
    *pIsEventProcessingDisabled = 1;
}


//__________________________________________________________________________________________________________________
LONG fall_Mouse_SetBuff(BYTE* fBuff, LONG subWidth, LONG subHeight, LONG fWidth, LONG fX, LONG fY, DWORD maskColour) {
    LONG retVal = 0; //0=pass, -1=fail
    __asm {
        push maskColour
        push fY
        push fX
        mov ecx, fWidth
        mov ebx, subHeight
        mov edx, subWidth
        mov eax, fBuff
        call pfall_mouse_set_buff
        add esp, 0xC
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________
void fall_Mouse_SetPos(LONG xPos, LONG yPos) {
    __asm {
        mov edx, yPos
        mov eax, xPos
        call pfall_mouse_set_pos
    }
}


//____________________________________
void fall_Mouse_GetRect(RECT* rcMouse) {
    __asm {
        mov eax, rcMouse
        call pfall_mouse_get_rect
    }
}


//______________________________
void fall_SendKey(DWORD keyCode) {
    __asm {
        mov eax, keyCode
        call pfall_send_key
    }
}


//_______________
bool IsExitGame() {
    if (*pGAME_EXIT_FLAGS)
        return true;
    else
        return false;
}


//__________________________________________
LONG fall_Palette_Load(const char* FileName) {
    int retVal;
    __asm {
        mov eax, FileName
        CALL pfall_palette_load
        mov retVal, eax
    }
    return retVal;
}


//______________________________
void fall_Palette_Set(BYTE* pal) {
    __asm {
        mov eax, pal
        CALL pfall_palette_set
    }
}


//_________________________________
void fall_Palette_FadeTo(BYTE* pal) {
    __asm {
        mov eax, pal
        CALL pfall_palette_fade_to
    }
}


//____________________________________
void fall_Palette_SetActive(BYTE* pal) {

    __asm {
        pushad
        mov eax, pal
        call pfall_palette_set_active
        popad
    }

}

//__________________
BOOL IsMouseHidden() {
    return *pIS_MOUSE_HIDDEN;
}


//___________________
DWORD GetMouseFlags() {
   return *pMOUSE_FLAGS;
}


//________________________
DWORD GetMouseFlags_BACK() {
    return *pMOUSE_FLAGS_BACK;
}


//____________________________________________
void fall_Mouse_GetPos(LONG* xPtr, LONG* yPtr) {
    __asm {
        mov edx, yPtr
        mov eax, xPtr
        call pfall_mouse_get_pos
    }
}


//____________________________________________________________________
LONG fall_Mouse_IsInArea(long left, long top, long right, long bottom) {
    DWORD retVal;
    __asm {
        mov ecx, bottom
        mov ebx, right
        mov edx, top
        mov eax, left
        call pfall_mouse_is_in_area
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________
LONG fall_Mouse_SetImage(LONG imageNum) {
    LONG ret_val;
    __asm {
        mov eax, imageNum
        call pfall_mouse_set_image
        mov ret_val, eax
    }
    return ret_val;//0 = success, -1 = fail
}


//_________________________________
LONG fall_Mouse_SetFrm(DWORD frmID) {
    LONG ret_val;
    __asm {
        mov eax, frmID
        call pfall_mouse_set_frm
        mov ret_val, eax
    }
    return ret_val;//0 = success, -1 = fail
}


//_______________________________________
LONG fall_PlayAcm(const char* sound_name) {
    int ret_val;
    __asm {
        mov eax, sound_name
        call pfall_play_acm
        mov ret_val, eax
    }
    return ret_val;
}


//________________________________________________________________________________________________________________________________________________________________
LONG fall_MessageBox(const char* text1, const char* text2, LONG text2Flag, LONG xPos, LONG yPos, DWORD text1Colour, DWORD unknown, DWORD text2Colour, DWORD flags) {
    //flags 0x10 = yes/no box
    int winRef;
    __asm {
        push flags
        push text2Colour
        push unknown
        push text1Colour
        push yPos
        mov ecx, xPos
        mov ebx, text2Flag
        lea edx, text2//edx=char**
        mov eax, text1
        call pfall_message_box
        mov winRef, eax
    }
    return winRef;
}


//______________________________________
void* fall_Mem_Allocate(DWORD sizeBytes) {
    BYTE* mem = nullptr;
    __asm {
        mov eax, sizeBytes
        call pfall_mem_allocate
        mov mem, eax
    }
    return mem;
}


//___________________________________________________
void* fall_Mem_Reallocate(void* mem, DWORD sizeBytes) {
    __asm {
        mov edx, sizeBytes
        mov eax, mem
        call pfall_mem_reallocate
        mov mem, eax
    }
    return mem;
}


//_________________________________
void fall_Mem_Deallocate(void* mem) {
    __asm {
        mov eax, mem
        call pfall_mem_deallocate
    }
}


//____________________________________
void Fallout_Functions_Setup_General() {

    if (fallout_exe_region == EXE_Region::Chinese) {

        pfall_play_acm = (void*)0x4510A8;

        pfall_message_box = (void*)0x41CEB0;


        pIS_MOUSE_HIDDEN = (BOOL*)0x6BCCFC;
        pMOUSE_FLAGS = (DWORD*)0x6BCD1C;
        //To-Do pMOUSE_FLAGS_BACK = (DWORD*)0x;

        fall_Mouse_Show = (void(*)())0x4CB73D;
        fall_Mouse_Hide = (void(*)())0x4CB97B;
        pfall_mouse_get_pos = (void*)0x4CBFD2;
        pfall_mouse_is_in_area = (void*)0x4CBE94;
        pfall_mouse_get_rect = (void*)0x4CBF6F;
        pfall_mouse_set_image = (void*)0x44BF90;
        pfall_mouse_set_frm = (void*)0x44C34C;
        //To-Do pfall_mouse_set_image_from_list = (void*)0x;
        pfall_mouse_set_pos = (void*)0x4CC017;
        pfall_mouse_set_buff = (void*)0x4CB419;

        fall_Mouse_ToggleHex = (void(*)())0x44C2C4;

        pfall_mem_allocate = (void*)0x4C479B;
        pfall_mem_reallocate = (void*)0x4C4876;
        pfall_mem_deallocate = (void*)0x4C49A8;

        pfall_palette_load = (void*)0x4C7C16;
        pfall_palette_set = (void*)0x4928F8;
        pfall_palette_fade_to = (void*)0x492884;
        pfall_palette_set_active = (void*)0x4C7681;

        pWHITE_PAL = (BYTE*)0x674250;
        pBLACK_PAL = (BYTE*)0x674550;
        pCURRENT_PAL = (BYTE*)0x673F50;
        pLOADED_PAL = (BYTE*)0x52DD1C;
        pACTIVE_PAL = (BYTE*)0x683608;


        pGAME_EXIT_FLAGS = (DWORD*)0x5284BC;


        pfall_send_key = (void*)0x4C93DC;

        fall_ExitMessageBox = (LONG(*)())0x443928;

        fall_EnableGameEvents = (void (*)())0x4814F0;
        fall_DisableGameEvents = (LONG(*)())0x481534;

        fall_Get_Input = (LONG(*)())0x4C92C0;
/*To-Do
        pfall_event_add = (void*)0x;
        pfall_event_remove = (void*)0x;

        pIsEventProcessingDisabled = (LONG*)0x;

        fall_Sound_Continue_ALL = (void(*)())0x;

        fall_FreeArtCache = (LONG(*)())0x;

        pfall_Speech_Set_Callback_Function = (void*)0x;
        pfall_Speech_Load = (void*)0x;
        fall_Speech_Close = (void(*)())0x;
        fall_Speech_Play = (void(*)())0x;


        fall_Input_Related_01 = (void(*)())0x;// clear keyboard buffer ??
        fall_Input_Related_02 = (void(*)())0x;// clear input buffer ??

        pfall_TEXT_Divide_By_Width = (void*)0x;

        fall_Debug_printf = (int(*)(const char*, ...)0x);

        pfall_game_mouse_disable = (void*)0x;

        p_PC_party_size = (LONG*)0x;

        pfall_set_background_sound = (void*)0x;

        fall_GameTime_Get_Time = (LONG(*)())0x;
        pfall_GameTime_Get_Date = (void*)0x;

        fall_Update_Mouse_State = (void(*)())0x;
*/
        fall_Process_Input = (void(*)())0x4C9379;
        fall_Process_Map_Mouse = (void(*)())0x44ADD4;

    }
    else {
        pfall_play_acm = (void*)FixAddress(0x4519A8);

        pfall_message_box = (void*)FixAddress(0x41CF20);

        pIS_MOUSE_HIDDEN = (BOOL*)FixAddress(0x6AC790);
        pMOUSE_FLAGS = (DWORD*)FixAddress(0x6AC7B0);
        pMOUSE_FLAGS_BACK = (DWORD*)FixAddress(0x6AC7A0);

        fall_Mouse_Show = (void(*)())FixAddress(0x4CA34C);
        fall_Mouse_Hide = (void(*)())FixAddress(0x4CA534);
        pfall_mouse_get_pos = (void*)FixAddress(0x4CA9DC);
        pfall_mouse_is_in_area = (void*)FixAddress(0x4CA934);
        pfall_mouse_get_rect = (void*)FixAddress(0x4CA9A0);
        pfall_mouse_set_image = (void*)FixAddress(0x44C840);
        pfall_mouse_set_frm = (void*)FixAddress(0x44CBFC);
        pfall_mouse_set_image_from_list = (void*)FixAddress(0x470BCC);
        pfall_mouse_set_pos = (void*)FixAddress(0x4CAA04);
        pfall_mouse_set_buff = (void*)FixAddress(0x4CA0AC);

        fall_Mouse_ToggleHex = (void(*)())FixAddress(0x44CB74);


        pfall_mem_allocate = (void*)FixAddress(0x4C5AD0);
        pfall_mem_reallocate = (void*)FixAddress(0x4C5B50);
        pfall_mem_deallocate = (void*)FixAddress(0x4C5C24);

        pfall_palette_load = (void*)FixAddress(0x4C78E4);
        pfall_palette_set = (void*)FixAddress(0x493B48);
        pfall_palette_fade_to = (void*)FixAddress(0x493AD4);
        pfall_palette_set_active = (void*)FixAddress(0x4C73E4);

        pWHITE_PAL = (BYTE*)FixAddress(0x663CD0);
        pBLACK_PAL = (BYTE*)FixAddress(0x663FD0);
        pCURRENT_PAL = (BYTE*)FixAddress(0x6639D0);
        pLOADED_PAL = (BYTE*)FixAddress(0x51DF34);
        pACTIVE_PAL = (BYTE*)FixAddress(0x673090);

        pGAME_EXIT_FLAGS = (DWORD*)FixAddress(0x5186CC);

        pfall_send_key = (void*)FixAddress(0x4C8C04);
        
        fall_ExitMessageBox = (LONG(*)())FixAddress(0x4440B8);
        
        fall_EnableGameEvents = (void (*)())FixAddress(0x4820C0);
        fall_DisableGameEvents = (LONG(*)())FixAddress(0x482104);
        
        fall_Get_Input = (LONG(*)())FixAddress(0x4C8B78);

        pfall_event_add = (void*)FixAddress(0x4C8D74);
        pfall_event_remove = (void*)FixAddress(0x4C8DC4);

        pIsEventProcessingDisabled = (LONG*)FixAddress(0x6AC780);

        fall_Sound_Continue_ALL = (void(*)())FixAddress(0x4AEBE0);

        fall_FreeArtCache = (LONG(*)())FixAddress(0x41927C);

        pfall_Speech_Set_Callback_Function = (void*)FixAddress(0x450C74);
        pfall_Speech_Load = (void*)FixAddress(0x450CA0);
        fall_Speech_Close = (void(*)())FixAddress(0x451024);
        fall_Speech_Play = (void(*)())FixAddress(0x450F8C);

        fall_Input_Related_01 = (void(*)())FixAddress(0x4CBDA8);// clear keyboard buffer ??
        fall_Input_Related_02 = (void(*)())FixAddress(0x4C8D04);// clear input buffer ??

        pfall_TEXT_Divide_By_Width = (void*)FixAddress(0x4BC6F0);

        fall_Debug_printf = (int(*)(const char*, ...))FixAddress(0x4C6F48);

        pfall_game_mouse_disable = (void*)FixAddress(0x44B48C);

        p_PC_party_size = (LONG*)FixAddress(0x51884C);

        pfall_background_sound_set = (void*)FixAddress(0x45067C);

        fall_Background_Sound_Stop = (void(*)())FixAddress(0x450AB4);

        fall_GameTime_Get_Time = (LONG(*)())FixAddress(0x4A33C8);
        pfall_GameTime_Get_Date = (void*)FixAddress(0x4A3338);


        fall_Process_Input = (void(*)())FixAddress(0x4C8BDC);
        fall_Process_Map_Mouse = (void(*)())FixAddress(0x44B684);
        fall_Update_Mouse_State = (void(*)())FixAddress(0x4CA59C);
    }
}



