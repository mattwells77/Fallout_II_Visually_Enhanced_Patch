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

#include "Fall_General.h"
#include "Fall_Graphics.h"
#include "Fall_Text.h"
#include "Fall_Msg.h"
#include "Fall_GameMap.h"

#include "Dx_Windows.h"
#include "Dx_Graphics.h"
#include "Dx_Game.h"
#include "Dx_General.h"
#include "Dx_Mouse.h"

using namespace DirectX;
using namespace DirectX::PackedVector;



struct PC_OPTION {
    LONG unknown1;
    LONG unknown2;
    LONG reaction_empathy;
    LONG unknown3;
    LONG buttRef;
    LONG yStart;
    char text[900];
    LONG yEnd;
};

struct REVIEW_EXCHANGE {
    LONG num1_npc;
    LONG num2_npc;
    char* text_npc;
    LONG num1_pc;
    LONG num2_pc;
    char* text_pc;
};

struct HISTORY_BUTTON_DETAILS {
    LONG width[3];
    LONG height[3];
    DWORD fid_lst_num[6];
};


struct DISPOSITION_DETAILS {
    LONG msg_num;
    LONG value;

};

struct BUTTON_DETAILS {
    LONG x;
    LONG y;
    DWORD lst_num_up;
    DWORD lst_num_down;
    DWORD lst_num_disabled;
    DWORD f_obj_1;
    DWORD f_obj_2;
    DWORD f_obj_3;
    LONG key;
    LONG disposition_type_num;
};


BUTTON_DETAILS* p_contolButtons = nullptr;
BUTTON_DETAILS* p_customButtons = nullptr;


WinStructDx* pWin_Dialog = nullptr;

Window_DX* pWin_Dialog_NPC_View = nullptr;

int winRef_Dialog_Screen_Background = -1;
int winRef_NPC_View_HighLight = -1;
int winRef_NPC_View = -1;
int winRef_NPC_Reply = -1;
int winRef_PC_Reply = -1;

int winRef_dialog_base_background = -1;
int winRef_dialog_base_overlay = -1;
int winRef_dialog_history_overlay = -1;

int buttRef_NPC_Reply_Up = -1;
int buttRef_NPC_Reply_Dn = -1;

LONG* p_winRef_Dialog_Base = nullptr;
LONG* p_winRef_barter_for_inv = nullptr;

LONG* p_talkin_head_BG_lst_num = nullptr;

LONG* p_dialog_win_state_num = nullptr;
LONG* p_dialog_draw_NPC_on_map = nullptr;

LONG* p_is_dialog_talk_setup = nullptr;

RECT* pRect_DialogPcText = nullptr;
RECT* pRect_DialogNpcText = nullptr;

LONG* p_buttRef_Dialog_Base = nullptr;//size = 9

OBJStruct** ppObj_Barter1 = nullptr;
OBJStruct** ppObj_Barter2 = nullptr;
OBJStruct** ppObj_Barter3 = nullptr;

LONG* p_contol_Active_Button = nullptr;

DWORD* pPalColour_ConsoleGreen = nullptr;
DWORD* pPalColour_ConsoleDarkGreen = nullptr;
DWORD* pPalColour_ConsoleRed = nullptr;
DWORD* pPalColour_ConsoleGrey = nullptr;
DWORD* pPalColour_ConsoleDarkGrey = nullptr;
DWORD* pPalColour_ConsoleYellow = nullptr;
DWORD* pPalColour_DarkYellow = nullptr;
DWORD* pPalColour_LightGrey = nullptr;
DWORD* pPalColour_Mustard = nullptr;

DWORD* pPalColour_Dialog_HighLight_1 = nullptr;
DWORD* pPalColour_Dialog_HighLight_2 = nullptr;


MSGList* pMsgList_Custom = nullptr;
LONG* p_customValues = nullptr;

DISPOSITION_DETAILS* p_custom_dp_details[6] = { nullptr, nullptr ,nullptr ,nullptr ,nullptr ,nullptr };

int pre_dialog_history_font = 0;
HISTORY_BUTTON_DETAILS* p_history_button_details;

LONG* p_review_num_exchanges = nullptr;
REVIEW_EXCHANGE* p_review_exchange = nullptr;

LONG* p_is_lips_sound_playing = nullptr;
void** pp_lips_sound_struct = nullptr;


LONG(*fall_Dialog_Talk_setup)() = nullptr;

void(*fall_Dialog_Main_Destructor)() = nullptr;


void(*fall_Dialog_Disable_Text_Windows)() = nullptr;
void(*fall_Dialog_Hide_Text_Windows)() = nullptr;

void(*fall_Dialog_History)() = nullptr;

void* pfall_Does_Party_Member_Have_Disposition = nullptr;

void* pfall_Does_Party_Member_Have_DP_Burst_Val = nullptr;
void* pfall_Does_Party_Member_Have_DP_Run_Away_Val = nullptr;
void* pfall_Does_Party_Member_Have_DP_Weapon_Pref_Val = nullptr;
void* pfall_Does_Party_Member_Have_DP_Distance_Val = nullptr;
void* pfall_Does_Party_Member_Have_DP_Attack_Who_Val = nullptr;
void* pfall_Does_Party_Member_Have_DP_Chem_Use_Val = nullptr;


void(*fall_dialog_talking_head_play)() = nullptr;
void* pfall_is_sound_playing = nullptr;
void(*fall_lips_end)() = nullptr;

void* pfall_script_get_msg_string = nullptr;


//____________________________________________________
char* fall_script_get_msg_string(LONG num1, LONG num2) {
    char* retVal = 0;
    __asm {
        mov edx, num2
        mov eax, num1
        call pfall_script_get_msg_string
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________
LONG fall_is_sound_playing(void* p_sound_struct) {
    LONG retVal = 0;
    __asm {
        mov eax, p_sound_struct
        call pfall_is_sound_playing
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________________
LONG fall_Does_Party_Member_Have_Disposition(OBJStruct* pObj, LONG dpNum) {
    LONG retVal = 0;
    __asm {
        mov edx, dpNum
        mov eax, pObj
        call pfall_Does_Party_Member_Have_Disposition
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Burst_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Burst_Val
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Run_Away_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Run_Away_Val
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Weapon_Pref_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Weapon_Pref_Val
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Distance_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Distance_Val
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Attack_Who_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Attack_Who_Val
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________
LONG fall_Does_Party_Member_Have_DP_Chem_Use_Val(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_Does_Party_Member_Have_DP_Chem_Use_Val
        mov retVal, eax
    }
    return retVal;
}


//_________________________
void  SetMouseImageDefault() {
    fall_Mouse_SetImage(1);
}


//____________________
void  SetMouseImageUp() {
    fall_Mouse_SetImage(2);
}


//______________________
void SetMouseImageDown() {
    fall_Mouse_SetImage(3);
}


//_________________________________________________________________________________
void Dialog_Barter_Draw_Critters(DWORD frmID, LONG ori, LONG frameNum, RECT* pRect) {

    if (*p_winRef_barter_for_inv == -1)
        return;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_barter_for_inv);
    if (!pWin || !pWin->winDx)
        return;

    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return;
    pFrame = pfrm->GetFrame(ori, frameNum);
    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return;
    }

    LONG xPos = ((pRect->right - pRect->left) - pFrame->GetWidth()) / 2 + pRect->left;
    LONG yPos = ((pRect->bottom - pRect->top) - pFrame->GetHeight()) / 2 + pRect->top;

    Window_DX* subwin = nullptr;

    if (winRef_dialog_base_overlay == -1) {
        subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_overlay);
        subwin->ClearRenderTarget(nullptr);
    }
    else
        subwin = pWin->winDx->GetSubWin(winRef_dialog_base_overlay);
    if (!subwin)
        return;

    subwin->RenderTargetDrawFrame((float)xPos, (float)yPos, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;
}


//________________________________________________________
void __declspec(naked) h_dialog_barter_draw_critters(void) {
    // esi framenum
    // ebp count critt
    // edi critt offset
    __asm {
        push esi
        push edi
        push ebp

        lea eax, dword ptr ss : [esp + 0x10]//rect
        push eax
        push esi// frameNum
        push dword ptr ss : [edi + esp + 0x10 + 0x18]//ori
        push dword ptr ss : [edi + esp + 0x18 + 0x1C]//frmID
        call Dialog_Barter_Draw_Critters
        add esp, 0x10

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//__________________________________________________
void Dialog_Base_Animated_Transition(bool direction) {
    if (*p_winRef_Dialog_Base == -1)
        return;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_dialog_base_background);
    if (!subwin)
        return;

    float yPos_Relative = 0;
    float yPos_End_Relative = (float)subwin->GetHeight();

    float y_unit = yPos_End_Relative / 800.0f / refreshTime_ms;

    float yPos = yPos_Relative;
    if (direction)
        yPos = yPos_End_Relative;

    subwin->SetPosition(0, yPos);

    LARGE_INTEGER lastTime = { 0LL };
    LARGE_INTEGER thisTime = { 0LL };
    lastTime.QuadPart = thisTime.QuadPart;
    LARGE_INTEGER ElapsedMicroseconds = { 0LL };

    while (yPos_Relative < yPos_End_Relative) {
        QueryPerformanceCounter(&thisTime);
        ElapsedMicroseconds.QuadPart = thisTime.QuadPart - lastTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000LL;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        if (ElapsedMicroseconds.QuadPart < 0) {
            lastTime.QuadPart = thisTime.QuadPart;
        }
        else if (ElapsedMicroseconds.QuadPart > refreshTime.QuadPart) {
            yPos_Relative += y_unit;

            fall_Update_Mouse_State();
            fall_Sound_Continue_ALL();
            Dx_Present_Main();

            if (direction)
                yPos = yPos_End_Relative - yPos_Relative;
            else
                yPos = yPos_Relative;
            subwin->SetPosition(0, yPos);

            lastTime.QuadPart = thisTime.QuadPart;
        }
        CheckMessagesNoWait();

        if (direction)
            yPos = yPos_End_Relative - yPos_Relative;
        else
            yPos = yPos_Relative;
        subwin->SetPosition(0, yPos);

    }
    if (direction)
        yPos = 0;
    else
        yPos = yPos_Relative;
    subwin->SetPosition(0, yPos);
    Dx_Present_Main();
}


//___________________________
void Dialog_Base_Destructor() {

    for (int i = 0; i < 9; i++) {
        fall_Button_Destroy(p_buttRef_Dialog_Base[i]);
        p_buttRef_Dialog_Base[i] = -1;
    }

    if (winRef_dialog_base_overlay != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
        if (pWin && pWin->winDx)
            pWin->winDx->DeleteSubWin(winRef_dialog_base_overlay);
    }
    winRef_dialog_base_overlay = -1;

    Dialog_Base_Animated_Transition(false);

    fall_Win_Destroy(*p_winRef_Dialog_Base);
    *p_winRef_Dialog_Base = -1;
    winRef_dialog_base_background = -1;

}


//___________________________________________________
void OnScreenResize_Dialog_Base(Window_DX* pWin_This) {
    //Dialogue base windows are moved in "OnScreenResize_Dialog_Main".
}



//_______________________________
void Dialog_Talk_Update_Display() {

    if (*p_winRef_Dialog_Base == -1)
        return;

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_dialog_base_overlay == -1) {
        subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_overlay);
    }
    else {
        subwin = pWin->winDx->GetSubWin(winRef_dialog_base_overlay);
        if (!subwin) {
            Fallout_Debug_Error("Dialog_Control_Update_Display - subwin fail");
        }
    }
    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    int oldFont = fall_GetFont();
    fall_SetFont(101);

    FONT_FUNC_STRUCT* font = GetCurrentFont();

    LONG total_caps = fall_Obj_GetTotalCaps(*ppObj_PC);
    char text[64];

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    sprintf_s(text, "%d", total_caps);

    LONG textWidth = (LONG)font->GetTextWidth(text);

    subwin->Draw_Text(text, 38 - textWidth / 2, 36, colour, 0, TextEffects::none);

    fall_SetFont(oldFont);
}


//_______________________________________________________
void __declspec(naked) h_dialog_talk_update_display(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Talk_Update_Display

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//___________________________
void Dialog_Talk_Destructor() {
    Dialog_Base_Destructor();
    *p_is_dialog_talk_setup = 0;
}


//___________________________________________________
void __declspec(naked) h_dialog_talk_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Talk_Destructor

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//______________________
LONG Dialog_Talk_Setup() {
    if (*p_is_dialog_talk_setup)
        return -1;
    if (!pWin_Dialog)
        return -1;

    for (int i = 0; i < 9; i++)
        p_buttRef_Dialog_Base[i] = -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    if (*p_PC_party_size)
        frmID = fall_GetFrmID(ART_INTRFACE, 389, 0, 0, 0);
    else
        frmID = fall_GetFrmID(ART_INTRFACE, 99, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winHeight = pFrame->GetHeight();

    if (*p_winRef_Dialog_Base != -1)
        Dialog_Base_Destructor();

    *p_winRef_Dialog_Base = fall_Win_Create(pWin_Dialog->rect.left, pWin_Dialog->rect.bottom + 1 - winHeight, pFrame->GetWidth(), winHeight, 0x100, FLG_WinToBack);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Base);

    Window_DX* subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_background);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    //subwin->ClipAtParent(true);
    subwin->SetClippingRect(nullptr);
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    Dialog_Base_Animated_Transition(true);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[0] = CreateButtonX(*p_winRef_Dialog_Base, 593, 41, 14, 14, -1, -1, -1, -1, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[0] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    fall_Button_SetFunctions(p_buttRef_Dialog_Base[0], nullptr, nullptr, nullptr, fall_Dialog_Disable_Text_Windows);
    SetButtonSoundsX(p_buttRef_Dialog_Base[0], &button_sound_Dn_2, &button_sound_Up_2);

    //0x61 di_rest1.frm; dialog rest button <UP>
    //0x62 di_rest2.frm; dialog rest button <DOWN>
    p_buttRef_Dialog_Base[1] = CreateButtonX(*p_winRef_Dialog_Base, 13, 154, 51, 29, -1, -1, -1, -1, 0x06000061, 0x06000062, 0, FLG_ButtTrans);
    if (p_buttRef_Dialog_Base[1] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    fall_Button_SetFunctions(p_buttRef_Dialog_Base[1], nullptr, nullptr, nullptr, fall_Dialog_History);
    SetButtonSoundsX(p_buttRef_Dialog_Base[1], &button_sound_Dn_1, &button_sound_Up_1);

    if (*p_PC_party_size) {
        //0x60 di_rdbt2.frm; dialog red button <DOWN>
        //0x5F di_rdbt1.frm; dialog red button <UP>
        p_buttRef_Dialog_Base[2] = CreateButtonX(*p_winRef_Dialog_Base, 593, 116, 14, 14, -1, -1, -1, -1, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

        if (p_buttRef_Dialog_Base[2] == -1) {
            Dialog_Base_Destructor();
            return -1;
        }
        fall_Button_SetFunctions(p_buttRef_Dialog_Base[2], nullptr, nullptr, nullptr, fall_Dialog_Hide_Text_Windows);
        SetButtonSoundsX(p_buttRef_Dialog_Base[2], &button_sound_Dn_2, &button_sound_Up_2);
    }

    *p_is_dialog_talk_setup = 1;

    Dialog_Talk_Update_Display();
    return 0;
}


//______________________________________________
void __declspec(naked) h_dialog_talk_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Talk_Setup

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
void Dialog_Barter_Destructor() {
    if (*ppObj_Barter1)
        fall_Obj_Destroy(*ppObj_Barter1, nullptr);
    *ppObj_Barter1 = nullptr;
    if (*ppObj_Barter2)
        fall_Obj_Destroy(*ppObj_Barter2, nullptr);
    *ppObj_Barter2 = nullptr;
    if (*ppObj_Barter3)
        fall_Obj_Destroy(*ppObj_Barter3, nullptr);
    *ppObj_Barter3 = nullptr;

    Dialog_Base_Destructor();
    fall_Obj_AttemptWeaponReload(*ppObj_DialogFocus, 0);
}


//_____________________________________________________
void __declspec(naked) h_dialog_barter_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Barter_Destructor

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//________________________
LONG Dialog_Barter_Setup() {

    if (!pWin_Dialog)
        return -1;

    *p_dialog_win_state_num = 4;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //0x1A4 420 - trade.frm; party member barter / trade interface
    //0x6F 111  - barter.frm     ; barter window
    if (*p_PC_party_size)
        frmID = fall_GetFrmID(ART_INTRFACE, 420, 0, 0, 0);
    else
        frmID = fall_GetFrmID(ART_INTRFACE, 111, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winHeight = pFrame->GetHeight();

    if (*p_winRef_Dialog_Base != -1)
        Dialog_Base_Destructor();

    *p_winRef_Dialog_Base = fall_Win_Create(pWin_Dialog->rect.left, pWin_Dialog->rect.bottom + 1 - winHeight, pFrame->GetWidth(), winHeight, 0x100, FLG_WinToBack);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Base);

    Window_DX* subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_background);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    //subwin->ClipAtParent(true);
    subwin->SetClippingRect(nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    Dialog_Base_Animated_Transition(true);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[0] = CreateButtonX(*p_winRef_Dialog_Base, 41, 163, 14, 14, -1, -1, -1, 109, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[0] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[0], &button_sound_Dn_2, &button_sound_Up_2);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[1] = CreateButtonX(*p_winRef_Dialog_Base, 584, 162, 14, 14, -1, -1, -1, 116, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[1] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[1], &button_sound_Dn_2, &button_sound_Up_2);


    if (fall_Obj_Create(ppObj_Barter1, -1, -1) == -1) {
        Dialog_Base_Destructor();
        return -1;

    }

    OBJStruct* pObj = *ppObj_Barter1;
    pObj->flags |= 0x1;

    if (fall_Obj_Create(ppObj_Barter2, -1, -1) == -1) {
        fall_Obj_Destroy(*ppObj_Barter1, nullptr);
        Dialog_Base_Destructor();
        return -1;
    }
    pObj = *ppObj_Barter2;
    pObj->flags |= 0x1;

    pObj = *ppObj_DialogFocus;
    if (fall_Obj_Create(ppObj_Barter3, pObj->frmID, -1) == -1) {
        fall_Obj_Destroy(*ppObj_Barter1, nullptr);
        fall_Obj_Destroy(*ppObj_Barter2, nullptr);
        Dialog_Base_Destructor();
        return -1;
    }
    pObj = *ppObj_Barter3;
    pObj->flags |= 0x5;
    pObj->scriptID1 = -1;

    return 0;
}


//________________________________________________
void __declspec(naked) h_dialog_barter_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Barter_Setup

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//__________________________________
void Dialog_Control_Update_Display() {
    if (*p_winRef_Dialog_Base == -1)
        return;

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_dialog_base_overlay == -1) {
        subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_overlay);
    }
    else {
        subwin = pWin->winDx->GetSubWin(winRef_dialog_base_overlay);
        if (!subwin) {
            Fallout_Debug_Error("Dialog_Control_Update_Display - subwin fail");
        }
    }
    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    int oldFont = fall_GetFont();
    fall_SetFont(101);


    char* pText = nullptr;
    OBJStruct* pObj = nullptr;
    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);


    //draw weapon used text
    pObj = fall_obj_getInvItem_HeldInSlot_2(*ppObj_DialogFocus);
    if (pObj)
        pText = fall_obj_GetProtoName(pObj);
    else {
        pText = GetMsg(pMsgList_Proto_Main, 10, 2);
    }

    subwin->Draw_Text(pText, 112, 20, colour, 0, TextEffects::none);

    //draw armor used text
    pObj = fall_obj_getInvItem_Wearing(*ppObj_DialogFocus);
    if (pObj)
        pText = fall_obj_GetProtoName(pObj);
    else {
        pText = GetMsg(pMsgList_Proto_Main, 10, 2);
    }
    subwin->Draw_Text(pText, 112, 49, colour, 0, TextEffects::none);

    //draw critter frm
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    pfrm = new FRMCached((*ppObj_DialogFocus)->frmID);
    if (!pfrm) {
        fall_SetFont(oldFont);
        return;
    }
    pFrame = pfrm->GetFrame(3, 0);
    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        fall_SetFont(oldFont);
        return;
    }

    LONG xPos = 39 - pFrame->GetWidth() / 2;
    LONG yPos = 132 - pFrame->GetHeight() / 2;

    subwin->RenderTargetDrawFrame((float)xPos, (float)yPos, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    char msg[12];

    //draw hitpoints text
    LONG hitPoints = fall_Obj_GetStat(*ppObj_DialogFocus, STAT::max_hit_points);
    LONG hitPoints_max = fall_Obj_GetStat(*ppObj_DialogFocus, STAT::current_hp);
    sprintf_s(msg, 11, " %d/%d", hitPoints, hitPoints_max);
    subwin->Draw_Text(msg, 240, 96, colour, 0, TextEffects::none);

    //draw skill text
    LONG skill_best = Obj_GetSkillWithHighestPoints(*ppObj_DialogFocus);
    char* skill_txt = fall_obj_GetSkillName(skill_best);
    sprintf_s(msg, 11, " %s", skill_txt);
    subwin->Draw_Text(msg, 240, 113, colour, 0, TextEffects::none);

    //draw carrying text
    LONG carryWeight = fall_Obj_Inventory_GetTotalWeight(*ppObj_DialogFocus);
    LONG carryWeight_max = fall_Obj_GetStat(*ppObj_DialogFocus, STAT::carry_amt);

    sprintf_s(msg, 11, " %d/%d", carryWeight, carryWeight_max);
    if (carryWeight > carryWeight_max && color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleRed & 0x000000FF);
    subwin->Draw_Text(msg, 240, 131, colour, 0, TextEffects::none);
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    //draw melee damage text
    LONG meleeDamage = fall_Obj_GetStat(*ppObj_DialogFocus, STAT::melee_dmg);
    sprintf_s(msg, 11, " %d", meleeDamage);
    subwin->Draw_Text(msg, 240, 148, colour, 0, TextEffects::none);

    //draw aps text
    LONG apPoints = 0;
    LONG apPoints_max = fall_Obj_GetStat(*ppObj_DialogFocus, STAT::max_move_points);
    if (*p_combat_state_flags & 0x1) {
        apPoints = (*ppObj_DialogFocus)->pud.critter.combat_data.currentAP;
    }
    else
        apPoints = apPoints_max;
    sprintf_s(msg, 11, " %d/%d", apPoints, apPoints_max);
    subwin->Draw_Text(msg, 240, 167, colour, 0, TextEffects::none);

    fall_SetFont(oldFont);
}



//__________________________________________________________
void __declspec(naked) h_dialog_control_update_display(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Control_Update_Display

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
void __declspec(naked) h_dialog_control_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Base_Destructor

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
LONG Dialog_Control_Setup() {

    if (!pWin_Dialog)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //0x186 390 - control.frm    ; party member control interface
    frmID = fall_GetFrmID(ART_INTRFACE, 390, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winHeight = pFrame->GetHeight();

    if (*p_winRef_Dialog_Base != -1)
        Dialog_Base_Destructor();

    *p_winRef_Dialog_Base = fall_Win_Create(pWin_Dialog->rect.left, pWin_Dialog->rect.bottom + 1 - winHeight, pFrame->GetWidth(), winHeight, 0x100, FLG_WinToBack);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Base);

    Window_DX* subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_background);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    subwin->SetClippingRect(nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    Dialog_Base_Animated_Transition(true);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[0] = CreateButtonX(*p_winRef_Dialog_Base, 593, 41, 14, 14, -1, -1, -1, 27, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[0] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[0], &button_sound_Dn_2, &button_sound_Up_2);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[1] = CreateButtonX(*p_winRef_Dialog_Base, 593, 97, 14, 14, -1, -1, -1, 100, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[1] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[1], &button_sound_Dn_2, &button_sound_Up_2);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[2] = CreateButtonX(*p_winRef_Dialog_Base, 236, 15, 14, 14, -1, -1, -1, 119, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[2] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[2], &button_sound_Dn_2, &button_sound_Up_2);

    //0x60 di_rdbt2.frm; dialog red button <DOWN>
    //0x5F di_rdbt1.frm; dialog red button <UP>
    p_buttRef_Dialog_Base[3] = CreateButtonX(*p_winRef_Dialog_Base, 235, 46, 14, 14, -1, -1, -1, 97, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[3] == -1) {
        Dialog_Base_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[3], &button_sound_Dn_2, &button_sound_Up_2);


    *p_contol_Active_Button = 4;
    int numButtons = 4;
    DWORD frmID_up = 0;
    DWORD frmID_down = 0;
    DWORD frmID_disabled = 0;
    BUTTON_DETAILS* pControl = nullptr;
    for (int i = 0; i < 5; i++) {
        pControl = &p_contolButtons[i];

        frmID_up = fall_GetFrmID(ART_INTRFACE, pControl->lst_num_up, 0, 0, 0);
        frmID_down = fall_GetFrmID(ART_INTRFACE, pControl->lst_num_down, 0, 0, 0);
        frmID_disabled = fall_GetFrmID(ART_INTRFACE, pControl->lst_num_disabled, 0, 0, 0);
        if (frmID_up == -1 || frmID_down == -1 || frmID_disabled == -1) {
            Dialog_Base_Destructor();
            return -1;
        }
        pfrm = new FRMCached(frmID_up);
        if (!pfrm) {
            Dialog_Base_Destructor();
            return -1;
        }
        pFrame = pfrm->GetFrame(0, 0);

        if (!pFrame) {
            if (pfrm)
                delete pfrm;
            pfrm = nullptr;
            Dialog_Base_Destructor();
            return -1;
        }

        p_buttRef_Dialog_Base[numButtons] = CreateButtonX(*p_winRef_Dialog_Base, pControl->x, pControl->y, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, pControl->key, -1, frmID_up, frmID_down, 0, FLG_ButtTrans | FLG_ButtTglOnce | FLG_ButtToggle);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;

        SetButtonDisabledFrmsX(p_buttRef_Dialog_Base[numButtons], frmID_disabled, frmID_disabled, frmID_disabled);
        SetButtonSoundsX(p_buttRef_Dialog_Base[numButtons], &button_sound_Dn_2, &button_sound_Up_2);

        if (!fall_Does_Party_Member_Have_Disposition(*ppObj_DialogFocus, pControl->disposition_type_num))
            SetButtonDisabledX(p_buttRef_Dialog_Base[numButtons]);

        numButtons++;

    }

    fall_Button_SetToggleArray(5, &p_buttRef_Dialog_Base[*p_contol_Active_Button]);
    LONG disposition = fall_AI_GetDisposition(*ppObj_DialogFocus);
    LONG buttRef = p_buttRef_Dialog_Base[*p_contol_Active_Button + 4 - disposition];

    fall_Button_SetToggleState(buttRef, 1, 0);

    Dialog_Control_Update_Display();
    *p_dialog_win_state_num = 10;
    return 0;
}


//_________________________________________________
void __declspec(naked) h_dialog_control_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Control_Setup

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_________________________________
void Dialog_Custom_Update_Display() {
    if (*p_winRef_Dialog_Base == -1)
        return;

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_dialog_base_overlay == -1) {
        subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_overlay);
    }
    else {
        subwin = pWin->winDx->GetSubWin(winRef_dialog_base_overlay);
        if (!subwin) {
            Fallout_Debug_Error("Dialog_Custom_Update_Display. subwin fail");
        }
    }
    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    int oldFont = fall_GetFont();
    fall_SetFont(101);


    char* pText = nullptr;
    OBJStruct* pObj = nullptr;
    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    LONG msgNum = 0;
    //draw burst text
    if (p_customValues[0] != -1) {
        //Fallout_Debug_Info("burst: %d", p_customValues[0]);
        msgNum = p_custom_dp_details[0][p_customValues[0]].msg_num;
    }
    else
        msgNum = 99;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 20, colour, 0, TextEffects::none);

    //draw run away text
    msgNum = p_custom_dp_details[1][p_customValues[1]].msg_num;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 48, colour, 0, TextEffects::none);

    //draw weapon pref text
    msgNum = p_custom_dp_details[2][p_customValues[2]].msg_num;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 78, colour, 0, TextEffects::none);


    //draw distance text
    msgNum = p_custom_dp_details[3][p_customValues[3]].msg_num;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 108, colour, 0, TextEffects::none);

    //draw attack who text
    msgNum = p_custom_dp_details[4][p_customValues[4]].msg_num;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 137, colour, 0, TextEffects::none);

    //draw chem use text
    msgNum = p_custom_dp_details[5][p_customValues[5]].msg_num;
    pText = GetMsg(pMsgList_Custom, msgNum, 2);
    subwin->Draw_Text(pText, 232, 166, colour, 0, TextEffects::none);

    fall_SetFont(oldFont);
}


//_________________________________________________________
void __declspec(naked) h_dialog_custom_update_display(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Custom_Update_Display

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
void Dialog_Custom_Destructor() {
    fall_MsgList_Destroy(pMsgList_Custom);
    Dialog_Base_Destructor();
}


//_____________________________________________________
void __declspec(naked) h_dialog_custom_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Custom_Destructor

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//________________________
LONG Dialog_Custom_Setup() {

    if (!pWin_Dialog)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //0x186 391 - custom.frm     ; party member control interface
    frmID = fall_GetFrmID(ART_INTRFACE, 391, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winHeight = pFrame->GetHeight();

    if (*p_winRef_Dialog_Base != -1)
        Dialog_Custom_Destructor();


    *p_winRef_Dialog_Base = fall_Win_Create(pWin_Dialog->rect.left, pWin_Dialog->rect.bottom + 1 - winHeight, pFrame->GetWidth(), winHeight, 0x100, FLG_WinToBack);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Base);

    Window_DX* subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_base_background);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    //subwin->ClipAtParent(true);
    subwin->SetClippingRect(nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    Dialog_Base_Animated_Transition(true);

    pMsgList_Custom->MsgNodes = 0;
    pMsgList_Custom->numMsgs = 0;
    if (fall_LoadMsgList(pMsgList_Custom, "game\\custom.msg") != TRUE) {
        Dialog_Custom_Destructor();
        return -1;
    }

    p_buttRef_Dialog_Base[0] = CreateButtonX(*p_winRef_Dialog_Base, 593, 101, 14, 14, -1, -1, -1, 13, 0x06000060, 0x0600005F, 0, FLG_ButtTrans);

    if (p_buttRef_Dialog_Base[0] == -1) {
        Dialog_Custom_Destructor();
        return -1;
    }
    SetButtonSoundsX(p_buttRef_Dialog_Base[0], &button_sound_Dn_2, &button_sound_Up_2);

    int numButtons = 1;
    DWORD frmID_up = 0;
    DWORD frmID_down = 0;

    BUTTON_DETAILS* pControl = nullptr;
    for (int i = 0; i < 6; i++) {
        pControl = &p_customButtons[i];

        frmID_up = fall_GetFrmID(ART_INTRFACE, pControl->lst_num_up, 0, 0, 0);
        frmID_down = fall_GetFrmID(ART_INTRFACE, pControl->lst_num_down, 0, 0, 0);

        if (frmID_up == -1 || frmID_down == -1) {
            Dialog_Custom_Destructor();
            return -1;
        }
        pfrm = new FRMCached(frmID_up);
        if (!pfrm) {
            Dialog_Custom_Destructor();
            return -1;
        }
        pFrame = pfrm->GetFrame(0, 0);

        if (!pFrame) {
            if (pfrm)
                delete pfrm;
            pfrm = nullptr;
            Dialog_Custom_Destructor();
            return -1;
        }

        p_buttRef_Dialog_Base[numButtons] = CreateButtonX(*p_winRef_Dialog_Base, pControl->x, pControl->y, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, pControl->key, frmID_up, frmID_down, 0, FLG_ButtTrans);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;

        SetButtonSoundsX(p_buttRef_Dialog_Base[numButtons], &button_sound_Dn_2, &button_sound_Up_2);

        numButtons++;
    }

    p_customValues[0] = fall_AI_GetBurstValue(*ppObj_DialogFocus);
    p_customValues[1] = fall_AI_GetRunAwayValue(*ppObj_DialogFocus);
    p_customValues[2] = fall_AI_GetWeaponPrefValue(*ppObj_DialogFocus);
    p_customValues[3] = fall_AI_GetDistanceValue(*ppObj_DialogFocus);
    p_customValues[4] = fall_AI_GetAttackWhoValue(*ppObj_DialogFocus);
    p_customValues[5] = fall_AI_GetChemUseValue(*ppObj_DialogFocus);

    *p_dialog_win_state_num = 13;

    Dialog_Custom_Update_Display();

    return 0;
}


//________________________________________________
void __declspec(naked) h_dialog_custom_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Custom_Setup

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_________________________________________________________
void OnScreenResize_Dialog_Custom_Box(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    if (!pWin_Dialog)
        return;

    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    DWORD winWidth = pWin_This->GetWidth();
    DWORD winHeight = pWin_This->GetHeight();
    LONG winX = pWin_Dialog->rect.left + (pWin_Dialog->width - winWidth) / 2;
    LONG winY = pWin_Dialog->rect.top + (pWin_Dialog->height - winHeight) / 2;
    MoveWindowX(pWin, winX, winY);
}


//________________________________________________________________________________________
void Dialog_Custom_Box_Update_Display(Window_DX* pWin, LONG disposition, LONG selectedVal) {
    if (!pWin)
        return;
    pWin->ClearRenderTarget(nullptr);
    //msgNum = p_custom_dp_msg[0][p_customValues[0]].num_msg_list;
    fall_SetFont(101);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    DWORD fontHeight = font->GetFontHeight();

    LONG isValViable = 0;
    DWORD colour = 0xFFFFFFFF;
    char* pText = nullptr;
    LONG text_y = 42;
    for (int i = 0; i < 6; i++) {

        if (p_custom_dp_details[disposition][i].msg_num != -1) {
            switch (disposition) {
            case 0:
                isValViable = fall_Does_Party_Member_Have_DP_Burst_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            case 1:
                isValViable = fall_Does_Party_Member_Have_DP_Run_Away_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            case 2:
                isValViable = fall_Does_Party_Member_Have_DP_Weapon_Pref_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            case 3:
                isValViable = fall_Does_Party_Member_Have_DP_Distance_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            case 4:
                isValViable = fall_Does_Party_Member_Have_DP_Attack_Who_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            case 5:
                isValViable = fall_Does_Party_Member_Have_DP_Chem_Use_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][i].value);
                break;
            default:
                isValViable = 0;
                break;

            }

            if (color_pal) {
                if (isValViable == 0)
                    colour = color_pal->GetColour(*pPalColour_ConsoleDarkGrey & 0x000000FF);
                else if (selectedVal != i)
                    colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
                else
                    colour = color_pal->GetColour(*pPalColour_ConsoleYellow & 0x000000FF);
            }
            pText = GetMsg(pMsgList_Custom, p_custom_dp_details[disposition][i].msg_num, 2);
            pWin->Draw_Text(pText, 42, text_y, colour, 0, TextEffects::none);
        }
        text_y += fontHeight;
    }
}


//________________________________________________________________
LONG Dialog_Custom_Box_Set_Disposition(LONG disposition, LONG val) {
    LONG retVal = -1;
    switch (disposition) {
    case 0:
        retVal = fall_AI_SetBurstValue(*ppObj_DialogFocus, val);
        break;
    case 1:
        retVal = fall_AI_SetRunAwayValue(*ppObj_DialogFocus, val);
        break;
    case 2:
        retVal = fall_AI_SetWeaponPrefValue(*ppObj_DialogFocus, val);
        break;
    case 3:
        retVal = fall_AI_SetDistanceValue(*ppObj_DialogFocus, val);
        break;
    case 4:
        retVal = fall_AI_SetAttackWhoValue(*ppObj_DialogFocus, val);
        break;
    case 5:
        retVal = fall_AI_SetChemUseValue(*ppObj_DialogFocus, val);
        break;
    default:
        retVal = -1;
        break;

    }

    if (retVal == 0)
        p_customValues[disposition] = val;

    return retVal;
}


//______________________________________
LONG Dialog_Custom_Box(LONG disposition) {
    if (!pWin_Dialog)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //0x1A3 419 - cussel.frm     ; party member custom interface
    frmID = fall_GetFrmID(ART_INTRFACE, 419, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winWidth = pFrame->GetWidth();
    DWORD winHeight = pFrame->GetHeight();
    LONG winX = pWin_Dialog->rect.left + (pWin_Dialog->width - winWidth) / 2;
    LONG winY = pWin_Dialog->rect.top + (pWin_Dialog->height - winHeight) / 2;

    int winRef_main = fall_Win_Create(winX, winY, winWidth, winHeight, 0x100, FLG_WinToFront | FLG_WinExclusive);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef_main);

    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Custom_Box);

    int subWin_BG = -1;
    Window_DX* subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &subWin_BG);
    subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    if (CreateButtonX(winRef_main, 70, 164, 14, 14, -1, -1, -1, 13, 0x06000060, 0x0600005F, 0, FLG_ButtTrans) == -1) {
        fall_Win_Destroy(winRef_main);
        return -1;
    }

    if (CreateButtonX(winRef_main, 176, 163, 14, 14, -1, -1, -1, 27, 0x06000060, 0x0600005F, 0, FLG_ButtTrans) == -1) {
        fall_Win_Destroy(winRef_main);
        return -1;
    }

    int oldFont = fall_GetFont();
    fall_SetFont(103);

    char* pText = nullptr;
    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_DarkYellow & 0x000000FF);

    //print box title.
    pText = GetMsg(pMsgList_Custom, disposition, 2);//custom_type_num is also the msg num for custom type text.
    subwin->Draw_Text(pText, 40, 15, colour, 0, TextEffects::none);

    //print done button text.
    pText = GetMsg(pMsgList_Custom, 10, 2);
    subwin->Draw_Text(pText, 88, 163, colour, 0, TextEffects::none);
    //print cancel button text.
    pText = GetMsg(pMsgList_Custom, 11, 2);
    subwin->Draw_Text(pText, 193, 162, colour, 0, TextEffects::none);

    LONG selectedVal = p_customValues[disposition];


    int subWin_Overlay = -1;
    subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &subWin_Overlay);

    Dialog_Custom_Box_Update_Display(subwin, disposition, selectedVal);

    LONG input = -1;
    bool exitFlag = false;

    LONG mouseX = 0;
    LONG mouseY = 0;
    float f_winX = 0;
    float f_winY = 0;
    RECT rectMouse = { 42, 42, (LONG)pWin->winDx->GetWidth() - 42, (LONG)pWin->winDx->GetHeight() - 42 };
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    DWORD fontHeight = font->GetFontHeight();

    while (!exitFlag && !*pGAME_EXIT_FLAGS) {
        input = fall_Get_Input();
        if (input != -1) {
            if (input == 0x11 || input == 0x18 || input == 0x144)
                fall_ExitMessageBox();
            else if (input == 0x0D) {
                Dialog_Custom_Box_Set_Disposition(disposition, selectedVal);
                exitFlag = true;
            }
            else if (input == 0x1B) {
                exitFlag = true;
            }
            else if (input == -2) {
                DWORD mouseFlags = *pMOUSE_FLAGS;
                if (mouseFlags & 0x10) {
                    pWin->winDx->GetPosition(&f_winX, &f_winY);
                    winX = (LONG)f_winX;
                    winY = (LONG)f_winY;
                    if (fall_Mouse_IsInArea(winX + rectMouse.left, winY + rectMouse.top, winX + rectMouse.right, winY + rectMouse.bottom)) {
                        LONG isValViable = 0;
                        fall_Mouse_GetPos(&mouseX, &mouseY);
                        LONG selectedVal_Mouse = (mouseY - (winY + rectMouse.top)) / fontHeight;
                        if (selectedVal_Mouse != selectedVal && selectedVal_Mouse >= 0 && selectedVal_Mouse < 6) {
                            if (selectedVal_Mouse != selectedVal) {

                                if (p_custom_dp_details[disposition][selectedVal_Mouse].msg_num != -1) {
                                    switch (disposition) {
                                    case 0:
                                        isValViable = fall_Does_Party_Member_Have_DP_Burst_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    case 1:
                                        isValViable = fall_Does_Party_Member_Have_DP_Run_Away_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    case 2:
                                        isValViable = fall_Does_Party_Member_Have_DP_Weapon_Pref_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    case 3:
                                        isValViable = fall_Does_Party_Member_Have_DP_Distance_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    case 4:
                                        isValViable = fall_Does_Party_Member_Have_DP_Attack_Who_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    case 5:
                                        isValViable = fall_Does_Party_Member_Have_DP_Chem_Use_Val(*ppObj_DialogFocus, p_custom_dp_details[disposition][selectedVal_Mouse].value);
                                        break;
                                    default:
                                        isValViable = 0;
                                        break;

                                    }
                                    if (isValViable) {
                                        selectedVal = selectedVal_Mouse;
                                        Dialog_Custom_Box_Update_Display(subwin, disposition, selectedVal);
                                    }
                                }
                            }
                            else {
                                ULONGLONG oldTick = GetTickCount64();
                                ULONGLONG newTick = oldTick;
                                while (newTick - oldTick < 250) {
                                    newTick = GetTickCount64();
                                    fall_Process_Input();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fall_SetFont(oldFont);
    fall_Win_Destroy(winRef_main);
    return 0;
}



//______________________________________________
void __declspec(naked) h_dialog_custom_box(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Dialog_Custom_Box
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


//__________________________________________________________________________________________________________________________________________________________
int DRAW_FORMATED_TEXT_TO_BUFF(LONG winRef, RECT* pRect, char* pTxt, LONG* retCount, LONG lineHeight, LONG buffWidth, DWORD colourPal, LONG insertLineSpace) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);

    if (!pWin || !pWin->winDx)
        return 0;

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(colourPal & 0x000000FF);

    if (retCount)
        pTxt += *retCount;

    LONG count = pWin->winDx->Draw_Text_Formated(pTxt, pRect, colour, 0xFF000000, FLG_TEXT_INDENT, TextEffects::none);
    //Fallout_Debug_Info("count %d", count);
    if (retCount) {
        if (pTxt[count] == '\0')
            *retCount = 0;
        else
            *retCount += count;
        //Fallout_Debug_Info("retCount %d %c", *retCount, pTxt[count]);
    }

    return pRect->top;
}


//______________________________________________________
void __declspec(naked) h_dialog_draw_formated_text(void) {

    __asm {

        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push ecx
        push ebx
        push edx
        push eax
        call DRAW_FORMATED_TEXT_TO_BUFF
        add esp, 0x20

        pop ebp
        pop edi
        pop esi
        ret 0x10
    }
}


//____________________________________________________________________________________________________________________________________________________________
int Dialog_PC_Set_Colour(PC_OPTION* pcOption, RECT* pRect, char* pTxt, LONG* retCount, LONG lineHeight, LONG buffWidth, DWORD colourPal, LONG insertLineSpace) {

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(colourPal & 0x000000FF);

    XMFLOAT4 fcolour = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f,  ((colour & 0x000000FF)) / 256.0f,  ((colour & 0xFF000000) >> 24) / 256.0f };

    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(pcOption->buttRef, nullptr);
    if (p_butt && p_butt->buttDx)
        p_butt->buttDx->OverLay_SetColours(&fcolour, nullptr, nullptr, nullptr);

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    return font->PrintText_Formated(nullptr, pTxt, buffWidth, pRect->bottom - pRect->top, pRect, FLG_TEXT_INDENT);
}



//_________________________________________________
void __declspec(naked) h_dialog_pc_set_colour(void) {

    __asm {

        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push ecx
        push ebx
        push edx
        push esi
        call Dialog_PC_Set_Colour
        add esp, 0x20

        pop ebp
        pop edi
        pop esi
        ret 0x10
    }
}


//___________________________________________________________________________________________________________________________________________________________
int Dialog_PC_GetTextDimensions(LONG winRef, RECT* pRect, char* pTxt, LONG* retCount, LONG lineHeight, LONG buffWidth, DWORD colourPal, LONG insertLineSpace) {
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    return font->PrintText_Formated(nullptr, pTxt, buffWidth, pRect->bottom - pRect->top, pRect, FLG_TEXT_INDENT);
}


//__________________________________________________________
void __declspec(naked) h_dialog_pc_get_text_dimensions(void) {

    __asm {

        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push dword ptr ss : [esp + 0x1C]
        push ecx
        push ebx
        push edx
        push eax
        call Dialog_PC_GetTextDimensions
        add esp, 0x20

        pop ebp
        pop edi
        pop esi
        ret 0x10
    }
}


//_____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
LONG Dialog_PC_CreateTextButton(PC_OPTION* pcOption, DWORD text_colour, char* pTxt, LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, DWORD flags) {

    height += 4;
    LONG buttRef = CreateButtonX_Overlay(winRef, x, y, width, height, keyHoverOn, keyHoverOff, keyPush, keyLift, frmID_UP, frmID_Dn, frmID_Hv, flags);

    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (p_butt == nullptr || p_butt->buttDx == nullptr)
        return buttRef;

    p_butt->buttDx->Overlay_CreateTexture(false, false, true);
    p_butt->buttDx->Clear_Staging();

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(text_colour & 0x000000FF);
    XMFLOAT4 fcolour = { ((colour & 0x000000FF)) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f,  ((colour & 0x00FF0000) >> 16) / 256.0f,  ((colour & 0xFF000000) >> 24) / 256.0f };

    p_butt->buttDx->OverLay_SetAsColouredButton(FLG_BUTT_UP);
    p_butt->buttDx->OverLay_SetColours(&fcolour, nullptr, nullptr, nullptr);


    BYTE* tBuff = nullptr;
    UINT pitchBytes = 0;
    RECT rect = { pRect_DialogPcText->left - x, pcOption->yStart - y, pRect_DialogPcText->right - x, pcOption->yStart - y + (LONG)height };

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG char_count = 0;
    if (SUCCEEDED(p_butt->buttDx->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
        memset(tBuff, 0, pitchBytes * height);
        char_count = font->PrintText_Formated(tBuff, pTxt, pitchBytes, height, &rect, FLG_TEXT_INDENT);
        p_butt->buttDx->Unlock(nullptr);
    }

    return buttRef;
}

//_________________________________________________________
void __declspec(naked) h_dialog_pc_create_text_button(void) {

    __asm {

        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push dword ptr ss : [esp + 0x30]
        push ecx
        push ebx
        push edx
        push eax
        lea eax, [ebp + 0x18]
        push eax
        push dword ptr ss : [esp + 0x6C + 0x2C]//colour
        push ebp
        call Dialog_PC_CreateTextButton
        add esp, 0x40

        pop ebp
        pop edi
        pop esi
        ret 0x24
    }
}


//_____________________________________
void Dialog_Text_Win_Clear(LONG winRef) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (!pWin || !pWin->winDx)
        return;
    pWin->winDx->ClearRenderTarget(nullptr);
}


//______________________________________________________
void __declspec(naked) h_dialog_npc_text_win_clear(void) {

    __asm {
        push eax
        call Dialog_Text_Win_Clear
        add esp, 0x4

        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________________________________
void __declspec(naked) h_dialog_pc_text_win_clear(void) {

    __asm {
        push eax
        call Dialog_Text_Win_Clear
        add esp, 0x4

        add esp, 0x10
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//___________________________________________________
void OnScreenResize_Dialog_Text(Window_DX* pWin_This) {
    //conversation windows "pWinRef_DialogNpcText" & "pWinRef_DialogPcText" are moved in "OnScreenResize_Dialog_Main".
    return;
}


//_______________________________________________________________________________________
LONG Dialog_Text_Win_Setup(int x, int y, int width, int height, int colour, int winFlags) {

    WinStruct* pWinDialog = fall_Win_Get(*pWinRef_DialogMain);
    if (!pWinDialog)
        return -1;
    LONG winRef = fall_Win_Create(x + pWinDialog->rect.left, y + pWinDialog->rect.top, width, height, colour, winFlags);

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (pWin && pWin->winDx) {
        pWin->winDx->SetBackGroungColour(0x00000000);
        pWin->winDx->SetDrawFlag(false);
        pWin->winDx->ClearRenderTarget(nullptr);
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Text);
    }
    return winRef;
}


//______________________________________________
void __declspec(naked) h_dialog_text_win_setup() {

    __asm {
        push dword ptr ss : [esp + 0x8]
        push dword ptr ss : [esp + 0x8]
        push ecx
        push ebx
        push edx
        push eax
        call Dialog_Text_Win_Setup
        add esp, 0x18

        ret 0x8
    }
}


//___________________________________________________
void OnScreenResize_Dialog_Main(Window_DX* pWin_This) {
    if (!pWin_This)
        return;

    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    LONG oldX = pWin->rect.left;
    LONG oldY = pWin->rect.top;

    LONG winX = 0;
    LONG winY = 0;

    LONG centring_height = 0;
    if (winRef_Dialog_Screen_Background != -1)
        centring_height = SCR_HEIGHT;
    else
        centring_height = Get_GameWin_Height();

    winX = ((LONG)SCR_WIDTH - (pWin->rect.right - pWin->rect.left)) / 2;
    winY = (centring_height - (pWin->rect.bottom - pWin->rect.top)) / 2;

    if (winY < 0)
        winY = 0;

    MoveWindowX(pWin, winX, winY);

    if (winRef_Dialog_Screen_Background != -1) {
        Window_DX* subwin = pWin_This->GetSubWin(winRef_Dialog_Screen_Background);
        float xPos = 0;
        float yPos = 0;
        pWin_This->GetPosition(&xPos, &yPos);
        if (subwin) {
            subwin->SetPosition(-xPos, -yPos);
            subwin->SetScale((float)SCR_WIDTH, (float)SCR_HEIGHT);
        }
    }

    LONG newX = pWin->rect.left;
    LONG newY = pWin->rect.top;
    LONG newY_bottom = pWin->rect.bottom;

    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_DialogNpcText);
    if (pWin) {
        winX = pWin->rect.left - oldX;
        winY = pWin->rect.top - oldY;
        MoveWindowX(pWin, winX + newX, winY + newY);
    }
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_DialogPcText);
    if (pWin) {
        winX = pWin->rect.left - oldX;
        winY = pWin->rect.top - oldY;
        MoveWindowX(pWin, winX + newX, winY + newY);
    }

    pWin = (WinStructDx*)fall_Win_Get(*p_winRef_Dialog_Base);

    if (pWin)
        MoveWindowX(pWin, newX, newY_bottom + 1 - pWin->winDx->GetHeight());

    //barter
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (pWin) {
        winX = pWin->rect.left - oldX;
        winY = pWin->rect.top - oldY;
        MoveWindowX(pWin, winX + newX, winY + newY);
    }
}


//________________
LONG Dialog_Main() {

    *p_dialog_win_state_num = 1;

    int DIALOG_SCRN_BACKGROUND = ConfigReadInt(L"OTHER_SETTINGS", L"DIALOG_SCRN_BACKGROUND", 0);

    LONG xPos = (LONG)SCR_WIDTH / 2 - 320;
    LONG centring_height = 0;
    if (DIALOG_SCRN_BACKGROUND)
        centring_height = SCR_HEIGHT;
    else
        centring_height = Get_GameWin_Height();

    LONG yPos = centring_height / 2 - 240;
    if (yPos < 0)
        yPos = 0;

    *pWinRef_DialogMain = fall_Win_Create(xPos, yPos, 640, 480, 0x100, FLG_WinToBack);
    pWin_Dialog = (WinStructDx*)fall_Win_Get(*pWinRef_DialogMain);
    if (!pWin_Dialog || !pWin_Dialog->winDx)
        return -1;
    pWin_Dialog->winDx->SetBackGroungColour(0x00000000);
    pWin_Dialog->winDx->SetDrawFlag(false);
    pWin_Dialog->winDx->ClearRenderTarget(nullptr);
    pWin_Dialog->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Dialog_Main);

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    
    float yPos_NpcView = 14;

    int is_alternate_dialog_bG = ConfigReadInt(L"OTHER_SETTINGS", L"DIALOG_SCRN_ART_FIX", 1);
    if (is_alternate_dialog_bG) {
        FRMdx* altBG_frm = new FRMdx("HR_ALLTLK", ART_INTRFACE, -1);
        pFrame = altBG_frm->GetFrame(0, 0);
        if (pFrame) {
            pWin_Dialog->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
            yPos_NpcView += 5;
        }
        else {
            Fallout_Debug_Error("Dialog_Main - DIALOG_SCRN_ART_FIX, load HR_ALLTLK backGround image failed.");
            is_alternate_dialog_bG = 0;
        }
        delete altBG_frm;
        altBG_frm = nullptr;
        pFrame = nullptr;
    }

    if (is_alternate_dialog_bG == 0) {
        frmID = fall_GetFrmID(ART_INTRFACE, 0x67, 0, 0, 0);//alltlk.frm     ; dialog screen background
        pfrm = new FRMCached(frmID);
        pFrame = pfrm->GetFrame(0, 0);
        if(pFrame)
            pWin_Dialog->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }


    //NPC view screen highlights window.

    Window_DX* subwin = nullptr;

    if (winRef_NPC_View_HighLight != -1)
        subwin = pWin_Dialog->winDx->GetSubWin(winRef_NPC_View_HighLight);
    if (!subwin)
        subwin = new Window_DX(126, yPos_NpcView, 388, 200, 0x00000000, pWin_Dialog->winDx, &winRef_NPC_View_HighLight);

    subwin->ClearRenderTarget(nullptr);
    subwin->ArrangeWin(TRUE);

    //Setup and draw the upper right highlight.------------------

    //This frm is expected to be 8Bit.
    //frame values range from 0-254, values as alpha are reversed with 1 being the most opaque and 254 being the most transparent, 0 is masked.
    frmID = fall_GetFrmID(ART_INTRFACE, 115, 0, 0, 0);//hilight1.frm   ; dialogue upper hilight
    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);

    GEN_SURFACE_BUFF_DATA genSurfaceData;

    DWORD colour = 0xFFFFFFFF;
    if (color_pal)
        colour = color_pal->GetColour((*pPalColour_Dialog_HighLight_1) & 0x000000FF);

    genSurfaceData.genData4_1 = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

    //Two passes here to get a result similar to original.
    if (pFrame)
        subwin->RenderTargetDrawFrame(300, 1, pFrame, pd3d_PS_Colour_32_RevAlpha_ZeroMasked, nullptr);
    if (pFrame)
        subwin->RenderTargetDrawFrame(300, 1, pFrame, pd3d_PS_Colour_32_RevAlpha_ZeroMasked, nullptr);
    pfrm = nullptr;
    pFrame = nullptr;

    //Setup and draw the lower left highlight.------------------

    //This frm is expected to be 8Bit.
    //frame values range from 0-254, values as alpha are reversed with 1 being the most opaque and 254 being the most transparent, 0 is masked.
    frmID = fall_GetFrmID(ART_INTRFACE, 116, 0, 0, 0);//hilight2.frm   ; dialogue lower hilight
    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);

    //Three passes with two colours to get a result similar to original.
    if (color_pal)
        colour = color_pal->GetColour((*pPalColour_Dialog_HighLight_2) & 0x000000FF);
    genSurfaceData.genData4_1 = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, 1.0f };
    //genSurfaceData.genData4_1 = { 252.0f / 256, 156.0f / 256, 72.0f / 256, 1.0f };//Approximate colour
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

    if (pFrame)
        subwin->RenderTargetDrawFrame(3, 200 - (float)pFrame->GetHeight(), pFrame, pd3d_PS_Colour_32_RevAlpha_ZeroMasked, nullptr);
    if (pFrame)
        subwin->RenderTargetDrawFrame(3, 200 - (float)pFrame->GetHeight(), pFrame, pd3d_PS_Colour_32_RevAlpha_ZeroMasked, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    //NPC view screen window.
    if (!pWin_Dialog_NPC_View)
        pWin_Dialog_NPC_View = new Window_DX(126, yPos_NpcView, 388, 200, 0x000000FF, pWin_Dialog->winDx, &winRef_NPC_View);
    pWin_Dialog_NPC_View->ClearRenderTarget(nullptr);
    pWin_Dialog_NPC_View->ArrangeWin(TRUE);

    if (DIALOG_SCRN_BACKGROUND) {
        subwin = new Window_DX((float)-xPos, (float)-yPos, 1, 1, 0x000000FF, pWin_Dialog->winDx, &winRef_Dialog_Screen_Background);
        if (subwin) {
            subwin->ClearRenderTarget(nullptr);
            subwin->SetScale((float)SCR_WIDTH, (float)SCR_HEIGHT);
            subwin->ArrangeWin(TRUE);
        }
    }

    if (fall_Dialog_Talk_setup() == -1) {
        fall_Dialog_Main_Destructor();
        return -1;
    }

    return 0;
}


//________________________________________
void __declspec(naked) h_dialog_main(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Dialog_Main

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_______________________________________
void Dialog_Main_Destroy_Win(LONG winRef) {
    fall_Win_Destroy(winRef);

    winRef_NPC_View = -1;
    pWin_Dialog_NPC_View = nullptr;

    winRef_NPC_View_HighLight = -1;
    winRef_Dialog_Screen_Background = -1;
}


//____________________________________________________
void __declspec(naked) h_dialog_main_destroy_win(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        push eax
        call Dialog_Main_Destroy_Win
        add esp, 0x4

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________________________________________________________
void Dialog_Draw_NPC_View(FRMdx* pFrmPtr_Talking_head, INT16 frameNum) {

    static LONG frm_x_shift = 0;

    if (*p_winRef_Dialog_Base == -1)
        return;
    if (!pWin_Dialog_NPC_View)
        return;
    if (pFrmPtr_Talking_head != nullptr) {
        if (frameNum == 0)
            frm_x_shift = 0;
        DWORD frmID = fall_GetFrmID(ART_BACKGRND, *p_talkin_head_BG_lst_num, 0, 0, 0);
        FRMCached* pfrm = new FRMCached(frmID);
        FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
        if (!pFrame)
            Fallout_Debug_Error("Talk Head - no background frame");
        else
            pWin_Dialog_NPC_View->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;

        pFrame = pFrmPtr_Talking_head->GetFrame(0, frameNum);
        if (!pFrame) {
            Fallout_Debug_Error("Talk Head - no head frame\n\0");
        }
        else {
            //frm_x_shift += pFrame->x2;
            LONG xOffset = pFrmPtr_Talking_head->GetShift_X(0) + pFrame->GetOffset_FromFirstFrame_X();// +frm_x_shift;
            xOffset += (pWin_Dialog_NPC_View->GetWidth() - pFrame->GetWidth()) / 2;
            pWin_Dialog_NPC_View->RenderTargetDrawFrame((float)xOffset, 0, pFrame, nullptr, nullptr);
            pFrame = nullptr;
        }

    }
    else {
        if (*p_dialog_draw_NPC_on_map) {
            OBJStruct* pObj_DialogFocus = *ppObj_DialogFocus;
            if (GameAreas_DrawToWindow(pWin_Dialog_NPC_View, pObj_DialogFocus->hexNum, pWin_Dialog_NPC_View->GetWidth(), pWin_Dialog_NPC_View->GetHeight()))
                *p_dialog_draw_NPC_on_map = 0;
        }
    }
}


//_________________________________________________
void __declspec(naked) h_dialog_Draw_NPC_View(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        and edx, 0x0000FFFF
        push edx
        push eax
        call Dialog_Draw_NPC_View
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//__________________________________________________________________
void Dialog_History_Update_Display(LONG winRef, LONG exchange_start) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_dialog_history_overlay == -1) {
        subwin = new Window_DX(0, 0, pWin->winDx->GetWidth(), pWin->winDx->GetHeight(), 0x00000000, pWin->winDx, &winRef_dialog_history_overlay);
    }
    else
        subwin = pWin->winDx->GetSubWin(winRef_dialog_history_overlay);

    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    if (exchange_start >= *p_review_num_exchanges)
        return;

    LONG exchange_num = exchange_start;

    REVIEW_EXCHANGE* p_review_current = nullptr;
    char* p_text = nullptr;
    char text[128];
    DWORD colour_Green = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    DWORD colour_DarkGreen = 0xFFFFFFFF;
    DWORD colour_Grey = 0xFFFFFFFF;
    DWORD colour_DarkGrey = 0xFFFFFFFF;
    if (color_pal) {
        colour_Green = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
        colour_DarkGreen = color_pal->GetColour(*pPalColour_ConsoleDarkGreen & 0x000000FF);
        colour_Grey = color_pal->GetColour(*pPalColour_ConsoleGrey & 0x000000FF);
        colour_DarkGrey = color_pal->GetColour(*pPalColour_ConsoleDarkGrey & 0x000000FF);
    }
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG textHeight = (LONG)font->GetFontHeight();

    RECT pMargins = { 113, 76, 422, 418 };

    while (exchange_num < *p_review_num_exchanges) {
        p_review_current = &p_review_exchange[exchange_num];

        p_text = fall_obj_GetName(*ppObj_DialogFocus);
        sprintf_s(text, "%s:", p_text);
        subwin->Draw_Text(text, 88, pMargins.top, colour_Green, 0, TextEffects::none);
        pMargins.top += textHeight;

        if (pMargins.top >= pMargins.bottom)
            return;

        if (p_review_current->num1_npc > -3)
            p_text = fall_script_get_msg_string(p_review_current->num1_npc, p_review_current->num2_npc);
        else
            p_text = p_review_current->text_npc;
        if (p_text == nullptr)
            Fallout_Debug_Error("Dialog_History_Update_Display - Failed NPC GET TEXT");
        subwin->Draw_Text_Formated(p_text, &pMargins, colour_DarkGreen, 0xFF000000, FLG_TEXT_INDENT, TextEffects::none);

        if (pMargins.top >= pMargins.bottom)
            return;

        if (p_review_current->num1_pc != -3) {
            p_text = fall_obj_GetName(*ppObj_PC);
            sprintf_s(text, "%s:", p_text);
            subwin->Draw_Text(text, 88, pMargins.top, colour_Grey, 0, TextEffects::none);
            pMargins.top += textHeight;

            if (pMargins.top >= pMargins.bottom)
                return;

            if (p_review_current->num1_pc > -3)
                p_text = fall_script_get_msg_string(p_review_current->num1_pc, p_review_current->num2_pc);
            else
                p_text = p_review_current->text_pc;
            if (p_text == nullptr)
                Fallout_Debug_Error("Dialog_History_Update_Display - Failed PC GET TEXT");
            subwin->Draw_Text_Formated(p_text, &pMargins, colour_DarkGrey, 0xFF000000, FLG_TEXT_INDENT, TextEffects::none);

            if (pMargins.top >= pMargins.bottom)
                return;
        }
        exchange_num++;
    }
}


//__________________________________________________________
void __declspec(naked) h_dialog_history_update_display(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Dialog_History_Update_Display
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//____________________________________________
LONG Dialog_History_Destructor(LONG* p_winRef) {

    fall_SetFont(pre_dialog_history_font);
    fall_Event_Add(fall_dialog_talking_head_play);

    if (!p_winRef)
        return -1;

    fall_Win_Destroy(*p_winRef);
    *p_winRef = -1;
    winRef_dialog_history_overlay = -1;
    return 0;
}


//______________________________________________________
void __declspec(naked) h_dialog_history_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Dialog_History_Destructor
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


//________________________________________
LONG Dialog_History_Setup(LONG* p_winRef) {
    if (*p_is_lips_sound_playing == 1) {
        if (fall_is_sound_playing(*pp_lips_sound_struct))
            fall_lips_end();
    }
    pre_dialog_history_font = fall_GetFont();

    if (!p_winRef)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //0x66 102 - review.frm     ; review screen
    frmID = fall_GetFrmID(ART_INTRFACE, 102, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return -1;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return -1;
    }

    DWORD winWidth = pFrame->GetWidth();
    DWORD winHeight = pFrame->GetHeight();

    LONG winX = ((LONG)SCR_WIDTH - (LONG)winWidth) / 2;
    LONG winY = (Get_GameWin_Height() - winHeight) / 2;//sub 100 to centre above interface bar.
    if (winY < 0)
        winY = 0;

    *p_winRef = fall_Win_Create(winX, winY, winWidth, winHeight, 0x100, FLG_WinToFront | FLG_WinExclusive);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Centred_On_GameWin);

    pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    LONG buttRef = -1;
    DWORD frmID_up = 0;
    DWORD frmID_dn = 0;

    frmID_up = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[0], 0, 0, 0);
    frmID_dn = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[1], 0, 0, 0);
    buttRef = CreateButtonX(*p_winRef, 475, 152, p_history_button_details->width[0], p_history_button_details->height[0], -1, -1, -1, 328, frmID_up, frmID_dn, 0, FLG_ButtTrans);
    if (buttRef == -1) {
        fall_Win_Destroy(*p_winRef);
        *p_winRef = -1;
        return -1;
    }
    SetButtonSoundsX(buttRef, &button_sound_Dn_2, &button_sound_Up_2);


    frmID_up = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[2], 0, 0, 0);
    frmID_dn = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[3], 0, 0, 0);
    buttRef = CreateButtonX(*p_winRef, 475, 191, p_history_button_details->width[1], p_history_button_details->height[1], -1, -1, -1, 336, frmID_up, frmID_dn, 0, FLG_ButtTrans);
    if (buttRef == -1) {
        fall_Win_Destroy(*p_winRef);
        *p_winRef = -1;
        return -1;
    }
    SetButtonSoundsX(buttRef, &button_sound_Dn_2, &button_sound_Up_2);

    frmID_up = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[4], 0, 0, 0);
    frmID_dn = fall_GetFrmID(ART_INTRFACE, p_history_button_details->fid_lst_num[5], 0, 0, 0);
    buttRef = CreateButtonX(*p_winRef, 499, 398, p_history_button_details->width[2], p_history_button_details->height[2], -1, -1, -1, 27, frmID_up, frmID_dn, 0, FLG_ButtTrans);
    if (buttRef == -1) {
        fall_Win_Destroy(*p_winRef);
        *p_winRef = -1;
        return -1;
    }
    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    fall_SetFont(101);
    fall_Event_Remove(fall_dialog_talking_head_play);

    return 0;
}


//_________________________________________________
void __declspec(naked) h_dialog_history_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Dialog_History_Setup
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


int count = 0;
//__________________________________________________
FRMdx* GET_FRM_PTR(DWORD frmID, FRMCached** pp_cfrm) {

    if (!pp_cfrm) {
        //Fallout_Debug_Info("LOADING FRM failed no obj ptr");
        return nullptr;
    }
    if (frmID == -1) {
        //Fallout_Debug_Info("LOADING FRM failed fid -1 ");
        *pp_cfrm = nullptr;
        return nullptr;
    }

    FRMCached* p_cfrm = new FRMCached(frmID);


    *pp_cfrm = p_cfrm;
    if (!p_cfrm) {
        //Fallout_Debug_Info("LOADING FRM failed ");
        return nullptr;
    }
    //Fallout_Debug_Info("LOADING FRM mem%d ", p_cfrm);
    FRMdx* pFrm = p_cfrm->GetFrm();

    if (!pFrm) {
        delete p_cfrm;
        p_cfrm = nullptr;
        *pp_cfrm = nullptr;
        //Fallout_Debug_Info("LOADING FRM failed no frm ");
        return nullptr;
    }
    FRMframeDx* pFrame = pFrm->GetFrame(0, 0);
    if (!pFrame) {
        delete p_cfrm;
        p_cfrm = nullptr;
        *pp_cfrm = nullptr;
        pFrm = nullptr;
        //Fallout_Debug_Info("LOADING FRM failed no frame ");
        return nullptr;
    }
    //Fallout_Debug_Info("LOADED FRM %d", count);
    count++;
    return pFrm;
}


//_____________________________________________________
void __declspec(naked) h_talking_head_get_frm_ptr(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GET_FRM_PTR
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//___________________________________
DWORD GET_FRAMES_PER_SEC(FRMdx* pFrm) {
    DWORD FPS = 0;
    if (pFrm)
        FPS = pFrm->GetFPS() & 0x0000FFFF;
    if (!FPS)
        FPS = 10;
    //Fallout_Debug_Info("GET_FRAMES_PER_SEC %d", FPS);
    return FPS;
}


//____________________________________________________________
void __declspec(naked) h_talking_head_get_frames_per_sec(void) {

    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp


        push eax
        call GET_FRAMES_PER_SEC
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//__________________________________
DWORD GET_NUM_OF_FRAMES(FRMdx* pFrm) {
    DWORD numFrames = -1;
    if (pFrm)
        numFrames = pFrm->GetNumFrames();// &0x0000FFFF;
    return numFrames;
}


//________________________________________________________
void __declspec(naked) h_talking_head_get_num_frames(void) {

    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call GET_NUM_OF_FRAMES
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//________________________________
void UNLOAD_FRM(FRMCached* p_cfrm) {
    if (!p_cfrm) {
        //Fallout_Debug_Info("UNLOADING FRM failed invalid ptr %d ", count);
        return;
    }
    //FRMCached* p_cfrm = pp_cfrm;
    if (p_cfrm == (void*)0xFFFFFFFF) {
        //Fallout_Debug_Info("UNLOAD FRM == -1 %d ", count);
    }
    else if (p_cfrm) {
        //Fallout_Debug_Info("UNLOADING FRM mem%d ", p_cfrm);
        FRMdx* frm = p_cfrm->GetFrm();
        if (frm) {
            //Fallout_Debug_Info("UNLOADING FRM  num frames%d ", frm->GetNumFrames());
            frm = nullptr;
        }
        //else
        //    Fallout_Debug_Info("UNLOADING FRM  no frm%d ", frm->GetNumFrames());
        delete p_cfrm;
        count--;
        //Fallout_Debug_Info("UNLOADED FRM %d ", count);
    }
    p_cfrm = nullptr;
}


//____________________________________________________
void __declspec(naked) h_talking_head_unload_frm(void) {

    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call UNLOAD_FRM
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//______________________________________________________________
void __declspec(naked) h_dialog_control_mouse_on_custom_button() {

    __asm {
        push esi
        push edi
        push ebp

        mov eax, p_buttRef_Dialog_Base
        mov eax, dword ptr ds : [eax + 32]//custom button is p_buttRef_Dialog_Base[8] 4x8=32
        push eax
        call IsMouseInButtonRect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//________________________________
void Modifications_Dialogue_CH(void) {

    //To-Do All Modifications_Dialogue_CH

   //DIALOG SCRN STUFF-------------------------------------------

   //CUSTOM COMBAT BUTTON MOUSE SELECTION RECTANGLE
   //00448E25  |.  E8 6A300800   |CALL 004CBE94
   FuncWrite32(0x448E26, 0x08306A, (DWORD)&h_dialog_control_mouse_on_custom_button);


   //DIALOG_NPC_RESPONSE TEXT SCRN POSITION
   //00445ABF  |.  E8 F9650900   CALL 004DC0BD                            ; \Fallout2.004DC0BD
   //FuncWrite32(0x445AC0, 0x0965F9, (DWORD)&h_dialog_sub_win);


   //DIALOG_CHARACTOR_RESPONSE TEXT SCRN POSITION
   //00445B9F  |.  E8 19650900   CALL 004DC0BD                            ; \Fallout2.004DC0BD
   //FuncWrite32(0x445BA0, 0x096519, (DWORD)&h_dialog_sub_win);


   //CUSTOM COMBAT CHOICE PANEL POSITION
   //00449809  |.  BA E0010000   MOV EDX,1E0
 //  MemWrite8(0x449809, 0xBA, 0xE8);
 //  FuncWrite32(0x44980A, 480, (DWORD)&custom_combat_choice);


   //BAKGROUNG CAPTURE YPOS FOR DIALOG_CHARACTOR_RESPONSE TEXT SCRN
   //00447124  |.  E8 931D0900   CALL 004D8EBC                            ; \Fallout2.004D8EBC
///   FuncWrite32(0x447125, 0x091D93, (DWORD)&h_dialog_pc_txt_bg);
   //0044726A  |.  E8 4D1C0900   CALL 004D8EBC                            ; \Fallout2.004D8EBC
///   FuncWrite32(0x44726B, 0x091C4D, (DWORD)&h_dialog_pc_txt_bg);

   /*
   //BARTER STUFF PANEL------
   //mouseMaxInvX & mouseMaxInvY handled in h_dialog_sub_win
   MemWrite8(0x46E4CE, 0xBB, 0x90);
   MemWrite32(0x46E4CF, 560, 0x90909090);
   MemWrite8(0x46E4D3, 0xB9, 0x90);
   MemWrite32(0x46E4D4, 470, 0x90909090);
   */
   /*
   //barter PC trade
   //00474572  |.  E8 1D790500   CALL 004CBE94
   FuncWrite32(0x474573, 0x05791D, (DWORD)&h_dialog_mouse_rec);
   //barter NPC trade
   //004745F5  |.  E8 9A780500   CALL 004CBE94
   FuncWrite32(0x4745F6, 0x05789A, (DWORD)&h_dialog_mouse_rec);
   //barter PC inv
   //0047483D  |.  E8 52760500   CALL 004CBE94
   FuncWrite32(0x47483E, 0x057652, (DWORD)&h_dialog_mouse_rec);
   //barter NPC inv
   //004748BE  |.  E8 D1750500   CALL 004CBE94
   FuncWrite32(0x4748BF, 0x0575D1, (DWORD)&h_dialog_mouse_rec);
   */


   /*
if(ConfigReadInt(L"OTHER_SETTINGS", L"BARTER_PC_INV_DROP_FIX", 1)) {
   ///0047482C  |.  BB 90000000   MOV EBX,90
   MemWrite32(0x47482D, 144, 164);
   ///00474838  |.  B8 50000000   MOV EAX,50
   MemWrite32(0x474839, 80, 100);
}*/






   //DIALOG PANEL WIDTH FIXES--ORIGINALY SET TO GLOBAL SCRN SIZE-----------------------
   //7F02 = 639
/*
   DWORD width_fix[51];

   width_fix[0]=0x445204;
   width_fix[1]=0x445220;
   width_fix[2]=0x4455BB;
   width_fix[3]=0x4455E5;
   width_fix[4]=0x446A25;
   width_fix[5]=0x446A6C;
   width_fix[6]=0x447667;
   width_fix[7]=0x447699;
   width_fix[8]=0x447706;
   width_fix[9]=0x447722;
   width_fix[10]=0x447B14;
   width_fix[11]=0x447D6C;
   width_fix[12]=0x447FBF;
   width_fix[13]=0x448460;
   width_fix[14]=0x448F64;
   width_fix[15]=0x449264;
   width_fix[16]=0x449E51;
   width_fix[17]=0x44A174;
   width_fix[18]=0x44A22A;
   width_fix[19]=0x44A298;
   width_fix[20]=0x44A2B6;
   width_fix[21]=0x44A36F;
   width_fix[22]=0x44A599;
   width_fix[23]=0x44A6E0;
   width_fix[24]=0x44A721;
   width_fix[25]=0x44A761;
   width_fix[26]=0x46E504;
   width_fix[27]=0x46F75A;
   width_fix[28]=0x46F78F;
   width_fix[29]=0x46FB01;
   width_fix[30]=0x46FB3E;
   width_fix[31]=0x46FF0E;
   width_fix[32]=0x4729DB;
   width_fix[33]=0x472A36;
   width_fix[34]=0x474431;
   width_fix[35]=0x474472;
   width_fix[36]=0x4746FA;
   width_fix[37]=0x474735;
   width_fix[38]=0x4749BD;
   width_fix[39]=0x4749FC;
   width_fix[40]=0x474B9B;
   width_fix[41]=0x474BF1;
   width_fix[42]=0x4475D9;
   width_fix[43]=0x4493F0;
   width_fix[44]=0x44A4C7;
   width_fix[45]=0x44A5D4;
   width_fix[46]=0x44518E;
   width_fix[47]=0x447AD2;
   width_fix[48]=0x447F7D;
   width_fix[49]=0x448F22;
   width_fix[50]=0x449E0F;

   for(int i=0; i<51; i++)
       MemWrite32(width_fix[i], 0x6BCF64, (DWORD)&dialogWidthFix);
   */


   //004469B3  |. E8 05570900    CALL Fallout2.004DC0BD
//   FuncWrite32(0x4469B4, 0x095705, (DWORD)&h_dialog_main_win);

   //00446AF5  |. 89F8                MOV EAX,EDI
   //00446AF7  |. BD FFFFFFFF         MOV EBP,-1
   //00446AFC  |. E8 BF580900         CALL Fallout2.004DC3C0
   ///FuncWrite32(0x446AFD, 0x0958BF, (DWORD)&h_dialog_bg);


   //DIALOG_HISTORY
   //004451AB  |. E8 0D6F0900    CALL Fallout2.004DC0BD
   //FuncWrite32(0x4451AC, 0x096F0D, (DWORD)&h_dialog_sub_win);

   //DIALOG_BARTER
   //00447AF3  |. E8 C5450900    CALL Fallout2.004DC0BD
  // FuncWrite32(0x447AF4, 0x0945C5, (DWORD)&h_dialog_sub_win);

   //DIALOG_COMBAT
   //00447F8E  |. E8 2A410900    CALL Fallout2.004DC0BD
  // FuncWrite32(0x447F8F, 0x09412A, (DWORD)&h_dialog_sub_win);

   //DIALOG_CUSTOM
   //00448F33  |. E8 85310900    CALL Fallout2.004DC0BD
  // FuncWrite32(0x448F34, 0x093185, (DWORD)&h_dialog_sub_win);

   //DIALOG_TALK
   //00449E30  |. E8 88220900    CALL Fallout2.004DC0BD
  // FuncWrite32(0x449E31, 0x092288, (DWORD)&h_dialog_sub_win);



   /*
if(ConfigReadInt(L"OTHER_SETTINGS", L"DIALOG_SCRN_ART_FIX", 1)) {

///0044A2C5  |.  E8 F2EB0800   CALL 004D8EBC                            ; \Fallout2.004D8EBC
   FuncReplace32(0x44A2C6, 0x08EBF2, (DWORD)&DrawFixedDialogArt);

//00447255      69D2 00100000        IMUL EDX,EDX,1000
//0044725B      90                   NOP
//0044725C  |.  29D0                 SUB EAX,EDX

///00446A79  |.  8D04D5 00000000    LEA EAX,[EDX*8]
///00446A80  |.  29D0               SUB EAX,EDX
 MemWrite16(0x446A79, 0x048D, 0xD269);
 MemWrite32(0x446A7B, 0xD5, 14+5);
 MemWrite8(0x446A7F, 0x00, 0x90);
 MemWrite16(0x446A80, 0xD029, 0x03EB);


//set xpos here
//0044726D  |.  83C0 7E               ADD EAX,7E

//mask top right y and x
//0044AF9C  |.  6A 0F                 PUSH 0F                                  ; |Arg3 = 0F
///0044A6E8  |.  6A 0F              PUSH 0F                                  ; |Arg3 = 0F
 MemWrite8(0x44A6E9, 0x0F, 0x0F+5);
//0044AF9E  |.  68 AA010000           PUSH 1AA                                 ; |Arg2 = 1AA

//mask bottom left ybottom and x
//0044AFEA  |.  B8 D6000000           MOV EAX,0D6                              ; |
///0044A736  |.  B8 D6000000        MOV EAX,0D6                              ; |
 MemWrite32(0x44A737, 0xD6, 0xD6+5);
//0044AFEF  |.  29D0                  SUB EAX,EDX                              ; |
//0044AFF1  |.  83E8 02               SUB EAX,2                                ; |
//0044AFF4  |.  50                    PUSH EAX                                 ; |Arg3
//0044AFF5  |.  68 81000000           PUSH 81                                  ; |Arg2 = 81



//draw_win_area rect
//0044AF41  |> \BB 7E000000           MOV EBX,7E
//0044AF46  |.  B9 0E000000           MOV ECX,0E
///0044A692  |.  B9 0E000000        MOV ECX,0E
 MemWrite32(0x44A693, 0x0E, 0x0E+5);
//0044AF4B  |.  BE 02020000           MOV ESI,202
//0044AF50  |.  BF D6000000           MOV EDI,0D6
///0044A69C  |.  BF D6000000        MOV EDI,0D6
 MemWrite32(0x44A69D, 0xD6, 0xD6+5);




//00446D3E  |.  BE 0A000000          MOV ESI,0A
 //MemWrite32(0x446D3F, 0x0A, 100);

//004471B7  |.  8B8B 4C875100        |MOV ECX,DWORD PTR DS:[EBX+51874C]
///004469DB  |.  8B8B 3C855200      |MOV ECX,DWORD PTR DS:[EBX+52853C]
 MemWrite32(0x52853C, 0x0E, 0x0E+5);
 MemWrite32(0x528544, 0x28, 0x28+5);

 MemWrite32(0x52854C, 0x0E, 0x0E+5);
 MemWrite32(0x528554, 0x28, 0x28+5);

 MemWrite32(0x52855C, 0xBC, 0xBC+5);
 MemWrite32(0x528564, 0xD6, 0xD6+5);

 MemWrite32(0x52856C, 0xBC, 0xBC+5);
 MemWrite32(0x528574, 0xD6, 0xD6+5);

 MemWrite32(0x52857C, 0x0E, 0x0E+5);
 MemWrite32(0x528584, 0x18, 0x18+5);

 MemWrite32(0x52858C, 0xCC, 0xCC+5);
 MemWrite32(0x528594, 0xD6, 0xD6+5);

 MemWrite32(0x52859C, 0x28, 0x28+5);
 MemWrite32(0x5285A4, 0xBC, 0xBC+5);

 MemWrite32(0x5285AC, 0x28, 0x28+5);
 MemWrite32(0x5285B4, 0xBC, 0xBC+5);
   }
*/








}


//__________________________
void Modifications_Dialogue_MULTI(void) {

    //CUSTOM COMBAT BUTTON MOUSE SELECTION RECTANGLE
    FuncReplace32(0x449662, 0x0812CE, (DWORD)&h_dialog_control_mouse_on_custom_button);

    MemWrite8(0x44715C, 0x53, 0xE9);
    FuncWrite32(0x44715D, 0x57565251, (DWORD)&h_dialog_main);


    //temp fixes skip monitor edge draw;
    MemWrite16(0x4472E3, 0xD231, 0x13EB);
    MemWrite16(0x44B012, 0xDB31, 0x54EB);

    fall_Dialog_Talk_setup = (LONG(*)())FixAddress(0x44A62C);

    fall_Dialog_Main_Destructor = (void(*)())FixAddress(0x447294);

    MemWrite8(0x44ACFC, 0x53, 0xE9);
    FuncWrite32(0x44ACFD, 0x55575651, (DWORD)&h_dialog_Draw_NPC_View);

    p_winRef_Dialog_Base = (LONG*)FixAddress(0x518744);

    p_talkin_head_BG_lst_num = (LONG*)FixAddress(0x518700);

    p_dialog_win_state_num = (LONG*)FixAddress(0x518714);

    p_dialog_draw_NPC_on_map = (LONG*)FixAddress(0x5187C8);


    FuncReplace32(0x4472D9, 0x08F18B, (DWORD)&h_dialog_main_destroy_win);


    p_is_dialog_talk_setup = (LONG*)FixAddress(0x5186EC);

    //text is destroyed to so it isn't drawn in dialog view, this is no longer necessary with the way I'm doing things.
    //00445208 | .E8 0FB00600   CALL int DESTROY_ALL_FLOATING_TEXT(); [fallout2.int DESTROY_ALL_FLOATING_TEXT()
    MemWrite8(0x445208, 0xE8, 0x90);
    MemWrite32(0x445209, 0x06B00F, 0x90909090);
    
    //skip scroll to focus - not needed
    //MemWrite8(0x445224, 0xE8, 0x90);//sfall hooks in here to prevent scrolling qhen talking head enabled
    //0044521A | . / 74 0D         JE SHORT 00445229
    MemWrite8(0x44521A, 0x74, 0xEB);//do this instead to avoid conflict

    // skip scroll back to original pos - not needed
    MemWrite8(0x4452EE, 0x74, 0xEB);

    //skip scroll to focus - not needed
    MemWrite8(0x44502B, 0x74, 0xEB);

    MemWrite8(0x444FB5, 0x74, 0xEB);



    //talking heads
    //frm1
    FuncReplace32(0x447529, 0xFFFD1C33, (DWORD)&h_talking_head_get_frm_ptr);
    FuncReplace32(0x447565, 0xFFFD21F7, (DWORD)&h_talking_head_get_frames_per_sec);
    FuncReplace32(0x4475C0, 0xFFFD21C8, (DWORD)&h_talking_head_get_num_frames);
    FuncReplace32(0x447C4F, 0xFFFD1B39, (DWORD)&h_talking_head_get_num_frames);
    //obj1
    FuncReplace32(0x445335, 0xFFFD3F27, (DWORD)&h_talking_head_unload_frm);
    FuncReplace32(0x4474F0, 0xFFFD1D6C, (DWORD)&h_talking_head_unload_frm);
    FuncReplace32(0x44765E, 0xFFFD1BFE, (DWORD)&h_talking_head_unload_frm);

    //frm2
    FuncReplace32(0x4473F2, 0xFFFD1D6A, (DWORD)&h_talking_head_get_frm_ptr);
    //obj2
    FuncReplace32(0x44534D, 0xFFFD3F0F, (DWORD)&h_talking_head_unload_frm);
    FuncReplace32(0x4473A1, 0xFFFD1EBB, (DWORD)&h_talking_head_unload_frm);
    
    //other
    FuncReplace32(0x447693, 0xFFFD1AC9, (DWORD)&h_talking_head_get_frm_ptr);
    FuncReplace32(0x4476AD, 0xFFFD20AF, (DWORD)&h_talking_head_get_frames_per_sec);
    FuncReplace32(0x4476C8, 0xFFFD20C0, (DWORD)&h_talking_head_get_num_frames);

    FuncReplace32(0x4476F2, 0xFFFD1B6A, (DWORD)&h_talking_head_unload_frm);

    
    //DIALOG NPC RESPONSE TEXT SCRN POSITION
    FuncReplace32(0x4462A8, 0x08FF8C, (DWORD)&h_dialog_text_win_setup);


    //DIALOG PC RESPONSE TEXT SCRN POSITION
    FuncReplace32(0x446388, 0x08FEAC, (DWORD)&h_dialog_text_win_setup);


    //clear npc reply backgroung
    MemWrite8(0x44777A, 0xE8, 0xE9);
    FuncReplace32(0x44777B, 0x090199, (DWORD)&h_dialog_npc_text_win_clear);


    //clear pc reply backgroung
    MemWrite8(0x44783E, 0xE8, 0xE9);
    FuncReplace32(0x44783F, 0x0900D5, (DWORD)&h_dialog_pc_text_win_clear);

    //these cause winRef num to be passed to DRAW_FORMATED_TEXT_TO_BUFF instead of buffer, which is than picked up by dialog_draw_formated_text below.
    MemWrite8(0x4454B2, 0xE8, 0x90);
    MemWrite32(0x4454B3, 0x0923F9, 0x90909090);


    //these cause winRef num to be passed to DRAW_FORMATED_TEXT_TO_BUFF instead of buffer, which is than picked up by dialog_draw_formated_text below.
    //history
    MemWrite16(0x445EBE, 0x448B, 0xE889);
    MemWrite16(0x445EC0, 0x6024, 0x9090);

    MemWrite16(0x445F73, 0x448B, 0xE889);
    MemWrite16(0x445F75, 0x6024, 0x9090);


    //these cause winRef num to be passed to DRAW_FORMATED_TEXT_TO_BUFF instead of buffer, which is than picked up by dialog_draw_formated_text below.
    MemWrite8(0x446B02, 0xE8, 0x90);
    MemWrite32(0x446B03, 0x090DA9, 0x90909090);

    MemWrite8(0x446C3C, 0xE8, 0x90);
    MemWrite32(0x446C3D, 0x090C6F, 0x90909090);

    MemWrite8(0x446CF2, 0xE8, 0x90);
    MemWrite32(0x446CF3, 0x090BB9, 0x90909090);

    MemWrite8(0x44704B, 0xE8, 0x90);
    MemWrite32(0x44704C, 0x090860, 0x90909090);


    MemWrite16(0x447FA0, 0x5756, 0xE990);
    FuncReplace32(0x447FA2, 0x14EC8355, (DWORD)&h_dialog_draw_formated_text);

    //replace pc text option drawing functions
    FuncReplace32(0x447072, 0x0F2A, (DWORD)&h_dialog_pc_get_text_dimensions);
    FuncReplace32(0x4470B7, 0x0911A5, (DWORD)&h_dialog_pc_create_text_button);

    //set the colour of pc text options when hovering
    FuncReplace32(0x446B25, 0x1477, (DWORD)&h_dialog_pc_set_colour);
    FuncReplace32(0x446C5F, 0x133D, (DWORD)&h_dialog_pc_set_colour);


    //prevent clearing the pc text background, no longer needed.
    MemWrite8(0x447914, 0x53, 0xC3);


    pRect_DialogPcText = (RECT*)FixAddress(0x58ECC0);
    pRect_DialogNpcText = (RECT*)FixAddress(0x58ECD0);

    p_buttRef_Dialog_Base = (LONG*)FixAddress(0x58F470);

    fall_Dialog_Disable_Text_Windows = (void(*)())FixAddress(0x44A52C);
    fall_Dialog_Hide_Text_Windows = (void(*)())FixAddress(0x44928C);

    fall_Dialog_History = (void(*)())FixAddress(0x445CA0);


    MemWrite8(0x44A62C, 0x53, 0xE9);
    FuncWrite32(0x44A62D, 0x57565251, (DWORD)&h_dialog_talk_setup);

    MemWrite8(0x44A9D8, 0x53, 0xE9);
    FuncWrite32(0x44A9D9, 0x57565251, (DWORD)&h_dialog_talk_destructor);


    //00447D98 / $  56            PUSH ESI; fallout2.int DIOLOG_SUBWIN_TRANS_SCROLL(EAX winRef, EDX 1 - up 0 down, EBX* buff1, ECX* buff2, Arg1* buff3, Arg2 height, Arg3 dont animate - 1)(guessed Arg1, Arg2, Arg3)
    //00447D99 | .  57            PUSH EDI
    //00447D9A | .  55            PUSH EBP
    //C2 0C00       RETN 0C
    MemWrite8(0x447D98, 0x56, 0xC2);
    MemWrite16(0x447D99, 0x5557, 0x000C);


    ppObj_Barter1 = (OBJStruct**)FixAddress(0x518730);
    ppObj_Barter2 = (OBJStruct**)FixAddress(0x518734);
    ppObj_Barter3 = (OBJStruct**)FixAddress(0x518738);


    MemWrite8(0x448290, 0x53, 0xE9);
    FuncWrite32(0x448291, 0x57565251, (DWORD)&h_dialog_barter_setup);

    MemWrite8(0x44854C, 0x53, 0xE9);
    FuncWrite32(0x44854D, 0x57565251, (DWORD)&h_dialog_barter_destructor);


    MemWrite16(0x4707B8, 0x3D83, 0xE890);
    FuncWrite32(0x4707BA, FixAddress(0x51884C), (DWORD)&h_dialog_barter_draw_critters);
    MemWrite8(0x4707BE, 0x00, 0x90);
    //004707BF | .  74 07 | JE SHORT 004707C8//  EB 71
    MemWrite16(0x4707BF, 0x0774, 0x71EB);

    //skip barter and inv draw bg
    MemWrite16(0x4733DC, 0x7D75, 0x78EB);

    p_winRef_barter_for_inv = (LONG*)FixAddress(0x59E97C);

    p_contolButtons = (BUTTON_DETAILS*)FixAddress(0x51891C);

    p_contol_Active_Button = (LONG*)FixAddress(0x58F464);



    pfall_Does_Party_Member_Have_Disposition = (void*)FixAddress(0x4958B0);
    pfall_Does_Party_Member_Have_DP_Burst_Val = (void*)FixAddress(0x495920);
    pfall_Does_Party_Member_Have_DP_Run_Away_Val = (void*)FixAddress(0x495980);
    pfall_Does_Party_Member_Have_DP_Weapon_Pref_Val = (void*)FixAddress(0x4959E0);
    pfall_Does_Party_Member_Have_DP_Distance_Val = (void*)FixAddress(0x495A40);
    pfall_Does_Party_Member_Have_DP_Attack_Who_Val = (void*)FixAddress(0x495AA0);
    pfall_Does_Party_Member_Have_DP_Chem_Use_Val = (void*)FixAddress(0x495B00);


    MemWrite8(0x448740, 0x53, 0xE9);
    FuncWrite32(0x448741, 0x57565251, (DWORD)&h_dialog_control_setup);

    MemWrite8(0x448C10, 0x53, 0xE9);
    FuncWrite32(0x448C11, 0x57565251, (DWORD)&h_dialog_control_destructor);

    MemWrite8(0x448D30, 0x53, 0xE9);
    FuncWrite32(0x448D31, 0x57565251, (DWORD)&h_dialog_control_update_display);


    pPalColour_ConsoleGreen = (DWORD*)FixAddress(0x6A3CB0);
    pPalColour_ConsoleDarkGreen = (DWORD*)FixAddress(0x6A3BD0);
    pPalColour_ConsoleRed = (DWORD*)FixAddress(0x6AB4D0);
    pPalColour_DarkYellow = (DWORD*)FixAddress(0x6A82F3);
    pPalColour_LightGrey = (DWORD*)FixAddress(0x6AB8CF);
    pPalColour_ConsoleGrey = (DWORD*)FixAddress(0x6A8B64);
    pPalColour_ConsoleDarkGrey = (DWORD*)FixAddress(0x6A76BF);
    pPalColour_ConsoleYellow = (DWORD*)FixAddress(0x6AB8BB);
    pPalColour_Mustard = (DWORD*)FixAddress(0x6A8B33);

    pPalColour_Dialog_HighLight_1 = (DWORD*)FixAddress(0x6A7F01);
    pPalColour_Dialog_HighLight_2 = (DWORD*)FixAddress(0x6A8F7B);



    pMsgList_Custom = (MSGList*)FixAddress(0x58EA98);

    p_customButtons = (BUTTON_DETAILS*)FixAddress(0x518B04);

    p_customValues = (LONG*)FixAddress(0x58EA80);

    p_custom_dp_details[0] = (DISPOSITION_DETAILS*)FixAddress(0x5189E4);
    p_custom_dp_details[1] = (p_custom_dp_details[0] + 6);
    p_custom_dp_details[2] = (p_custom_dp_details[1] + 6);
    p_custom_dp_details[3] = (p_custom_dp_details[2] + 6);
    p_custom_dp_details[4] = (p_custom_dp_details[3] + 6);
    p_custom_dp_details[5] = (p_custom_dp_details[4] + 6);


    MemWrite8(0x4496A0, 0x53, 0xE9);
    FuncWrite32(0x4496A1, 0x57565251, (DWORD)&h_dialog_custom_setup);

    MemWrite8(0x449A10, 0x53, 0xE9);
    FuncWrite32(0x449A11, 0x57565251, (DWORD)&h_dialog_custom_destructor);

    MemWrite8(0x449BB4, 0x53, 0xE9);
    FuncWrite32(0x449BB5, 0x57565251, (DWORD)&h_dialog_custom_update_display);

    MemWrite8(0x449FC0, 0x53, 0xE9);
    FuncWrite32(0x449FC1, 0x57565251, (DWORD)&h_dialog_custom_box);

    p_is_lips_sound_playing = (LONG*)FixAddress(0x518710);
    
    pp_lips_sound_struct = (void**)FixAddress(0x519258);

    pfall_is_sound_playing = (void*)FixAddress(0x4ADA84);

    fall_lips_end = (void(*)())FixAddress(0x4450C4);

    p_history_button_details = (HISTORY_BUTTON_DETAILS*)FixAddress(0x518818);


    p_review_num_exchanges = (LONG*)FixAddress(0x5186DC);
    p_review_exchange = (REVIEW_EXCHANGE*)FixAddress(0x58ECE0);

    fall_dialog_talking_head_play = (void(*)())FixAddress(0x447A58);

    pfall_script_get_msg_string = (void*)FixAddress(0x4A6C50);


    MemWrite8(0x445938, 0x53, 0xE9);
    FuncWrite32(0x445939, 0x57565251, (DWORD)&h_dialog_history_setup);

    MemWrite8(0x445C18, 0x53, 0xE9);
    FuncWrite32(0x445C19, 0x57565251, (DWORD)&h_dialog_history_destructor);

    MemWrite8(0x445D44, 0x53, 0xE9);
    FuncWrite32(0x445D45, 0x55575651, (DWORD)&h_dialog_history_update_display);

    MemWrite8(0x446504, 0x53, 0xE9);
    FuncWrite32(0x446505, 0x83565251, (DWORD)&h_dialog_talk_update_display);
    MemWrite16(0x446509, 0x24EC, 0x9090);

}


//___________________________
void Modifications_Dialogue() {
    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Dialogue_CH();
    else
        Modifications_Dialogue_MULTI();
}






