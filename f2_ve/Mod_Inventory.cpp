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

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

LONG* p_winRef_MoveItems = nullptr;


struct INV_DETAILS {
    DWORD frm_list_num;
    DWORD width;
    DWORD height;
    LONG x;
    LONG y;
};

INV_DETAILS* inv_details;//inv background and dimensions of the various inventory windows 0 - 5.

enum class INV_TYPE : LONG {
    inv = 0,
    use_on = 1,
    loot = 2,
    barter = 3,
    move_items = 4,
    set_timer = 5
};


INV_TYPE invType_current = INV_TYPE::inv;

LONG* pInvListItemsVis = nullptr;

OBJStruct** ppObj_PC_for_INV = nullptr;

OBJStruct** ppObj_PC_Wearing = nullptr;
OBJStruct** ppObj_PC_LeftHand = nullptr;
OBJStruct** ppObj_PC_RightHand = nullptr;


DWORD* p_current_stack_PC = nullptr;
DWORD* p_stack_offset_PC = nullptr;
OBJStruct** pp_Obj_stack_PC = nullptr;
PUD** pp_pud_PC = nullptr;

DWORD* p_current_stack_Target = nullptr;
DWORD* p_stack_offset_Target = nullptr;
OBJStruct** pp_Obj_stack_Target = nullptr;
PUD** pp_pud_Target = nullptr;

LONG* p_inv_num_items_visible = nullptr;
LONG* p_inv_dropped_explosive = nullptr;


void** ppfall_INV_Display_Text = nullptr;
void* pfall_INV_Display_Text_Dialog_NPC = nullptr;
void* pfall_INV_Display_Text_IMonitor = nullptr;

void* pfall_inv_hover_on = nullptr;
void* pfall_inv_hover_off = nullptr;

DWORD(*fall_inv_adjust_pc_frm)() = nullptr;

LONG* p_inv_scroll_up_buttId_pc = nullptr;
LONG* p_inv_scroll_up_buttId_target = nullptr;
LONG* p_inv_scroll_dn_buttId_pc = nullptr;
LONG* p_inv_scroll_dn_buttId_target = nullptr;

LONG* p_inv_pc_table_listPos = nullptr;
LONG* p_inv_target_table_listPos = nullptr;


int winRef_Inv_PC_Two_Handed_Item_BG = -1;

int winRef_Inv_Console = -1;

MSGList* pMsgList_Inventory = nullptr;

DWORD* p_frmID_inv_pc = nullptr;

//LONG* p_inv_console_pc_stat_num_1 = nullptr;

//LONG* p_inv_console_pc_stat_num_2 = nullptr;

RECT rc_Items_PC = { 0,0,0,0 };
RECT rc_Items_Target = { 0,0,0,0 };
RECT rc_Items_Table_PC = { 0,0,0,0 };
RECT rc_Items_Table_Target = { 0,0,0,0 };

LONG* p_buttRef_inv_pc = nullptr;
LONG* p_buttRef_inv_target = nullptr;
LONG* p_buttRef_inv_pc_table = nullptr;
LONG* p_buttRef_inv_target_table = nullptr;
LONG buttRef_inv_pc_armor = -1;
LONG buttRef_inv_pc_leftHand = -1;
LONG buttRef_inv_pc_rightHand = -1;

LONG buttRef_inv_portrait_pc = -1;
LONG buttRef_inv_portrait_target = -1;


//_______________________________
void Inventory_DestroyPortraits() {//need to destroy these before the scroll down window animation on the dialog screen.
    if(buttRef_inv_portrait_pc != -1)
        fall_Button_Destroy(buttRef_inv_portrait_pc);
    buttRef_inv_portrait_pc = -1;
    if (buttRef_inv_portrait_target != -1)
        fall_Button_Destroy(buttRef_inv_portrait_target);
    buttRef_inv_portrait_target = -1;
}


//________________________________________________________
void OnScreenResize_Inventory_Barter(Window_DX* pWin_This) {
    //Inventory barter window is moved in "OnScreenResize_Dialog_Main" in win_dialog.cpp.
}


//__________________________________
LONG Inventory_Setup(LONG type_long) {
    //0 = inv, 1 = use-on, 2 = loot, 3 = barter, 4 = move-multi, 5 = move-multi
    INV_TYPE inv_type = static_cast<INV_TYPE>(type_long);
    *pp_Obj_stack_PC = *ppObj_PC_for_INV;
    *pp_pud_PC = &(*ppObj_PC_for_INV)->pud;

    *p_current_stack_PC = 0;
    *p_stack_offset_PC = 0;

    *p_inv_num_items_visible = 6;

    *p_inv_dropped_explosive = 0;

    WinStructDx* pWin = nullptr;
    ButtonStruct_DX* p_butt = nullptr;

    if (inv_type == INV_TYPE::barter) {
        if (*p_winRef_barter_for_inv == -1)
            return 0;

        WinStructDx* pWin_barter = (WinStructDx*)fall_Win_Get(*p_winRef_barter_for_inv);
        if (!pWin_barter || !pWin_barter->winDx) {
            *pWinRef_Inventory = -1;
            return 0;
        }
        else
            *pWinRef_Inventory = fall_Win_Create(pWin_barter->rect.left + 80, pWin_barter->rect.top + 290 - (480 - pWin_barter->height), 480, 180, 0x101, 0);
        pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
        if (pWin) {
            pWin->winDx->SetBackGroungColour(0x00000000);
            pWin->winDx->SetDrawFlag(false);
            pWin->winDx->ClearRenderTarget(nullptr);
        }

        WinStructDx* pWin_Dialog = (WinStructDx*)fall_Win_Get(*pWinRef_DialogMain);

        *ppfall_INV_Display_Text = pfall_INV_Display_Text_Dialog_NPC;
        *p_inv_num_items_visible = 3;
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Inventory_Barter);
    }
    else {
        DWORD frmID = 0;
        FRMCached* pfrm = nullptr;
        FRMframeDx* pFrame = nullptr;
        frmID = fall_GetFrmID(ART_INTRFACE, inv_details[static_cast<LONG>(inv_type)].frm_list_num, 0, 0, 0);

        pfrm = new FRMCached(frmID);
        if (!pfrm)
            return 0;
        pFrame = pfrm->GetFrame(0, 0);

        if (!pFrame) {
            if (pfrm)
                delete pfrm;
            pfrm = nullptr;
            return 0;
        }

        *pWinRef_Inventory = Win_Create_CenteredOnGame(inv_details[static_cast<LONG>(inv_type)].width, inv_details[static_cast<LONG>(inv_type)].height, 0x101, FLG_WinToFront | FLG_WinExclusive);

        pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
        if (!pWin || !pWin->winDx) {
            delete pfrm;
            pfrm = nullptr;
            pFrame = nullptr;
            return 0;
        }
        pWin->winDx->SetBackGroungColour(0x00000000);
        pWin->winDx->SetDrawFlag(false);
        pWin->winDx->ClearRenderTarget(nullptr);
        pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;

        *ppfall_INV_Display_Text = pfall_INV_Display_Text_IMonitor;
    }

    invType_current = inv_type;
    // *invMouseEdgeRight = pWin->rect.right;// +inv_details[inv_type]->width;
    // *invMouseEdgeBottom = pWin->rect.bottom;// +inv_details[inv_type]->height;
     ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (p_buttRef_inv_pc)
        delete[] p_buttRef_inv_pc;
    p_buttRef_inv_pc = new LONG[*p_inv_num_items_visible];

    if (p_buttRef_inv_target)
        delete[] p_buttRef_inv_target;
    p_buttRef_inv_target = new LONG[*p_inv_num_items_visible];


    LONG buttRef = -1;
    buttRef_inv_portrait_pc = -1;


    if (inv_type == INV_TYPE::barter) {
        if (p_buttRef_inv_pc_table)
            delete[] p_buttRef_inv_pc_table;
        p_buttRef_inv_pc_table = new LONG[*p_inv_num_items_visible];

        if (p_buttRef_inv_target_table)
            delete[] p_buttRef_inv_target_table;
        p_buttRef_inv_target_table = new LONG[*p_inv_num_items_visible];

        //set item list drop rects
        rc_Items_PC = { 29, 31, 29 + 64, 31 + 48 * *p_inv_num_items_visible };
        rc_Items_Target = { 387, 31, 387 + 64, 31 + 48 * *p_inv_num_items_visible };
        rc_Items_Table_PC = { 165, 20, 165 + 64, 20 + 48 * *p_inv_num_items_visible };
        rc_Items_Table_Target = { 250, 20, 250 + 64, 20 + 48 * *p_inv_num_items_visible };
        LONG butt_y = 0;
        for (LONG num = 0; num < *p_inv_num_items_visible; num++) {
            //pc inv list buttons
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 29, 31 + butt_y, 64, 48, 1000 + num, -1, 1000 + num, -1, 0, 0, 0, 0);//moved button a bit
            p_buttRef_inv_pc[num] = buttRef;
            if (buttRef != -1)
                fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
            //target inv list buttons
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 387, 31 + butt_y, 64, 48, 2000 + num, -1, 2000 + num, -1, 0, 0, 0, 0);//moved button a bit
            p_buttRef_inv_target[num] = buttRef;
            if (buttRef != -1)
                fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);

            //pc trade list buttons
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 165, 20 + butt_y, 64, 48, 2300 + num, -1, 2300 + num, -1, 0, 0, 0, 0);
            p_buttRef_inv_pc_table[num] = buttRef;
            if (buttRef != -1)
                fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
            //npc trade list buttons
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 250, 20 + butt_y, 64, 48, 2400 + num, -1, 2400 + num, -1, 0, 0, 0, 0);
            p_buttRef_inv_target_table[num] = buttRef;
            if (buttRef != -1)
                fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);

            butt_y += 48;
        }

        //scroll up button - table pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 109, 56, 23, 24, -1, -1, 328, -1, 0x06000064, 0x06000065, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        //scroll down button - table pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 109, 82, 24, 25, -1, -1, 336, -1, 0x0600005D, 0x0600005E, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

        //scroll up button - table target
        buttRef = CreateButtonX(*pWinRef_Inventory, 342, 56, 23, 24, -1, -1, 397, -1, 0x06000064, 0x06000065, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        //scroll down button - table target
        buttRef = CreateButtonX(*pWinRef_Inventory, 342, 82, 24, 25, -1, -1, 401, -1, 0x0600005D, 0x0600005E, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

        //portrait button - PC
        buttRef = CreateButtonX_Overlay(*p_winRef_barter_for_inv, 15, 25, 60, 100, -1, -1, 2500, -1, 0, 0, 0, 0);
        buttRef_inv_portrait_pc = buttRef;
        p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_pc, nullptr);
        if (p_butt)
            p_butt->buttDx->OverLay_SetClippingRect(true);

        //portrait button - Target
        buttRef = CreateButtonX_Overlay(*p_winRef_barter_for_inv, 560, 25, 60, 100, -1, -1, 2501, -1, 0, 0, 0, 0);
        buttRef_inv_portrait_target = buttRef;
        p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_target, nullptr);
        if (p_butt)
            p_butt->buttDx->OverLay_SetClippingRect(true);

        //scroll up button - pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 128, 113, 22, 23, -1, -1, 329, -1, 0x06000031, 0x06000032, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        //scroll down button - pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 128, 136, 22, 23, -1, -1, 337, -1, 0x06000033, 0x06000034, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

        //scroll up button - target
        buttRef = CreateButtonX(*pWinRef_Inventory, 333, 113, 22, 23, -1, -1, 388, -1, 0x06000031, 0x06000032, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        //scroll down button - target
        buttRef = CreateButtonX(*pWinRef_Inventory, 333, 136, 22, 23, -1, -1, 374, -1, 0x06000033, 0x06000034, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
    }
    else {

        //scroll up button - pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 128, 39, 22, 23, -1, -1, 328, -1, 0x06000031, 0x06000032, 0, 0);
        *p_inv_scroll_up_buttId_pc = buttRef;
        if (buttRef != -1) {
            SetButtonDisabledFrmsX(buttRef, 0x06000035, 0x06000035, 0x06000035);
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
            SetButtonDisabledX(buttRef);
        }

        //scroll down button - pc
        buttRef = CreateButtonX(*pWinRef_Inventory, 128, 62, 22, 23, -1, -1, 336, -1, 0x06000033, 0x06000034, 0, 0);
        *p_inv_scroll_dn_buttId_pc = buttRef;
        if (buttRef != -1) {
            SetButtonDisabledFrmsX(buttRef, 0x06000036, 0x06000036, 0x06000036);
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
            SetButtonDisabledX(buttRef);
        }


        if (inv_type == INV_TYPE::loot) {
            LONG butt_y = 37;
            //set item list drop rects
            rc_Items_PC = { 176, butt_y, 176 + 64, butt_y + 48 * *p_inv_num_items_visible };
            rc_Items_Target = { 297, butt_y, 297 + 64,  butt_y + 48 * *p_inv_num_items_visible };
            for (LONG num = 0; num < *p_inv_num_items_visible; num++) {
                //pc inv list buttons
                buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 176, butt_y, 64, 48, 1000 + num, -1, 1000 + num, -1, 0, 0, 0, 0);
                p_buttRef_inv_pc[num] = buttRef;
                if (buttRef != -1)
                    fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
                //loot-from inv list buttons
                buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 297, butt_y, 64, 48, 2000 + num, -1, 2000 + num, -1, 0, 0, 0, 0);
                p_buttRef_inv_target[num] = buttRef;
                if (buttRef != -1)
                    fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);

                butt_y += 48;
            }

            //done button
            buttRef = CreateButtonX(*pWinRef_Inventory, 476, 331, 15, 16, -1, -1, -1, 27, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
            if (buttRef != -1)
                SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

            //scroll up button - target
            buttRef = CreateButtonX(*pWinRef_Inventory, 379, 39, 22, 23, -1, -1, 397, -1, 0x06000031, 0x06000032, 0, 0);
            *p_inv_scroll_up_buttId_target = buttRef;
            if (buttRef != -1) {
                SetButtonDisabledFrmsX(buttRef, 0x06000035, 0x06000035, 0x06000035);
                SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
                SetButtonDisabledX(buttRef);
            }
            //scroll down button target
            buttRef = CreateButtonX(*pWinRef_Inventory, 379, 62, 22, 23, -1, -1, 401, -1, 0x06000033, 0x06000034, 0, 0);
            *p_inv_scroll_dn_buttId_target = buttRef;
            if (buttRef != -1) {
                SetButtonDisabledFrmsX(buttRef, 0x06000036, 0x06000036, 0x06000036);
                SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
                SetButtonDisabledX(buttRef);
            }
            //portrait button - PC
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 44, 35, 60, 100, -1, -1, 2500, -1, 0, 0, 0, 0);
            buttRef_inv_portrait_pc = buttRef;
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_pc, nullptr);
            if (p_butt)
                p_butt->buttDx->OverLay_SetClippingRect(true);
            //portrait button - Target
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 422, 25, 60, 100, -1, -1, 2501, -1, 0, 0, 0, 0);
            buttRef_inv_portrait_target = buttRef;
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_target, nullptr);
            if (p_butt)
                p_butt->buttDx->OverLay_SetClippingRect(true);

            //Loot All button
            buttRef = CreateButtonX(*pWinRef_Inventory, 432, 204, 39, 41, -1, -1, 65, -1, 0x060001B4, 0x060001B5, 0, 0);
            if (buttRef != -1)
                SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        }
        else if (inv_type == INV_TYPE::inv || inv_type == INV_TYPE::use_on) {
            LONG butt_y = 35;
            //set item list drop rect
            rc_Items_PC = { 44, butt_y, 44 + 64, butt_y + 48 * *p_inv_num_items_visible };
            //pc inv list buttons
            for (LONG num = 0; num < *p_inv_num_items_visible; num++) {
                buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 44, butt_y, 64, 48, 1000 + num, -1, 1000 + num, -1, 0, 0, 0, 0);
                p_buttRef_inv_pc[num] = buttRef;
                if (buttRef != -1)
                    fall_Button_SetFunctions(buttRef, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
                butt_y += 48;
            }

            //pc portrait button
            buttRef = CreateButtonX_Overlay(*pWinRef_Inventory, 176, 37, 60, 100, -1, -1, 2500, -1, 0, 0, 0, 0);
            buttRef_inv_portrait_pc = buttRef;
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_pc, nullptr);
            if (p_butt)
                p_butt->buttDx->OverLay_SetClippingRect(true);
            buttRef_inv_portrait_target = -1;

            if (inv_type == INV_TYPE::inv) {

                buttRef_inv_pc_armor = -1;
                buttRef_inv_pc_leftHand = -1;
                buttRef_inv_pc_rightHand = -1;

                //held right button
                buttRef_inv_pc_rightHand = CreateButtonX_Overlay(*pWinRef_Inventory, 245, 286, 90, 61, 1006, -1, 1006, -1, 0, 0, 0, 0);
                if (buttRef_inv_pc_rightHand != -1)
                    fall_Button_SetFunctions(buttRef_inv_pc_rightHand, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
                //held left button
                buttRef_inv_pc_leftHand = CreateButtonX_Overlay(*pWinRef_Inventory, 154, 286, 90, 61, 1007, -1, 1007, -1, 0, 0, 0, 0);
                if (buttRef_inv_pc_leftHand != -1)
                    fall_Button_SetFunctions(buttRef_inv_pc_leftHand, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);
                //armor button
                buttRef_inv_pc_armor = CreateButtonX_Overlay(*pWinRef_Inventory, 154, 183, 90, 61, 1008, -1, 1008, -1, 0, 0, 0, 0);
                if (buttRef_inv_pc_armor != -1)
                    fall_Button_SetFunctions(buttRef_inv_pc_armor, pfall_inv_hover_on, pfall_inv_hover_off, nullptr, nullptr);

                //done button
                buttRef = CreateButtonX(*pWinRef_Inventory, 437, 329, 15, 16, -1, -1, -1, 27, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
                if (buttRef != -1)
                    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
            }
            else if (inv_type == INV_TYPE::use_on) {
                //done button
                buttRef = CreateButtonX(*pWinRef_Inventory, 233, 328, 15, 16, -1, -1, -1, 27, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
                if (buttRef != -1)
                    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
            }

        }

    }

    *ppObj_PC_Wearing = nullptr;
    *ppObj_PC_LeftHand = nullptr;
    *ppObj_PC_RightHand = nullptr;
    LONG item_num = 0;
    ITEMblock* pItem = (*pp_pud_PC)->general.p_item;

    //equip marked items
    while (item_num < (*pp_pud_PC)->general.inv_size) {
        pItem = &(*pp_pud_PC)->general.p_item[item_num];

        if (pItem->p_obj && pItem->num > 0) {
            if (pItem->p_obj->flags & FLG_IsHeldSlot1) {
                if (pItem->p_obj->flags & FLG_IsHeldSlot2)//if item is two handed - I don't think this ever happens.
                    *ppObj_PC_RightHand = pItem->p_obj;
                *ppObj_PC_LeftHand = pItem->p_obj;

            }
            else if (pItem->p_obj->flags & FLG_IsHeldSlot2)
                *ppObj_PC_RightHand = pItem->p_obj;
            else if (pItem->p_obj->flags & FLG_IsWornArmor)
                *ppObj_PC_Wearing = pItem->p_obj;
        }
        item_num++;
    }

    //remove equipped items from the inventory
    if (*ppObj_PC_LeftHand) {
        fall_Obj_Inventory_RemoveItems(*ppObj_PC_for_INV, *ppObj_PC_LeftHand, 1);
    }
    if (*ppObj_PC_RightHand) {
        if (*ppObj_PC_RightHand != *ppObj_PC_LeftHand)//if not two handed
            fall_Obj_Inventory_RemoveItems(*ppObj_PC_for_INV, *ppObj_PC_RightHand, 1);
    }
    if (*ppObj_PC_Wearing) {
        fall_Obj_Inventory_RemoveItems(*ppObj_PC_for_INV, *ppObj_PC_Wearing, 1);
    }
    fall_inv_adjust_pc_frm();

    LONG were_game_events_disabled = fall_DisableGameEvents();
    fall_GameMouse_Disable(0);

    return were_game_events_disabled;
}


//____________________________________________
void __declspec(naked) h_inventory_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Inventory_Setup
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


//______________________________________________
void __declspec(naked) h_inventory_win_destroy() {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call fall_Win_Destroy
        add esp, 0x4
        mov eax, pWinRef_Inventory
        mov dword ptr ds : [eax] , -1

        mov winRef_Inv_PC_Two_Handed_Item_BG, -1
        mov winRef_Inv_Console, -1

        call Inventory_DestroyPortraits

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
DWORD GetInventoryItemFrm(OBJStruct* pObj) {
    if (!pObj)
        return -1;
    PROTO* pPro = nullptr;

    fall_GetPro(pObj->proID, &pPro);
    if (!pPro)
        return -1;
    return pPro->item.invFrmID;
}


//_________________________________________________________________________________________________________________________________
LONG Inventory_Draw_Item_Quantity_Text(OBJStruct* p_obj, LONG numItems, ButtonStruct_DX* p_butt, LONG xPos, LONG yPos, bool picked) {///reversed the logic on "picked" variable to original fallout function this is based on.

    if (!p_butt)
        return -1;
    DWORD width = p_butt->rect.right - p_butt->rect.left;
    DWORD height = p_butt->rect.bottom - p_butt->rect.top;
    if (!p_butt->buttDx->isTexture()) {
        p_butt->buttDx->Overlay_CreateTexture(false, false, true);
        //p_butt->buttDx->OverLay_SetPixelShader(pd3d_PS_DrawTextColour, &OverlayShader_Button_Item_Quantity_Text);
        p_butt->buttDx->OverLay_SetAsColouredButton(FLG_BUTT_UP);
        DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
        if (color_pal)
            colour = color_pal->GetColour(*pPalColour_LightGrey & 0x000000FF);

        XMFLOAT4 fcolour = { ((colour & 0x000000FF)) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f,  ((colour & 0x00FF0000) >> 16) / 256.0f,  ((colour & 0xFF000000) >> 24) / 256.0f };
        p_butt->buttDx->OverLay_SetColours(&fcolour, nullptr, nullptr, nullptr);

    }
    p_butt->buttDx->Clear_Staging();

    if (!p_obj)
        return -1;

    LONG itemType = fall_Obj_Item_GetType(p_obj);

    if (itemType == ITEM_TYPE::ammo) {
        numItems--;
        LONG maxAmmo = fall_Obj_Item_GetMaxAmmo(p_obj);
        numItems *= maxAmmo;
        if (!picked) {
            LONG currentAmmo = fall_Obj_Item_GetCurrentAmmo(p_obj);
            numItems += currentAmmo;
        }
    }
    else {
        if (picked)
            numItems--;
    }
    if (numItems <= 1)
        return numItems;
    if (numItems > 99999)
        numItems = 99999;

    char msg[12];
    sprintf_s(msg, "x%d", numItems);

    int oldFont = fall_GetFont();
    fall_SetFont(101);

    FONT_FUNC_STRUCT* font = GetCurrentFont();

    LONG txtWidth = font->GetTextWidth(msg);
    LONG txtHeight = font->GetFontHeight();
    if (txtWidth <= 0 || txtHeight <= 0) {
        fall_SetFont(oldFont);
        return numItems;
    }
    if (yPos + txtHeight > (LONG)height) {
        fall_SetFont(oldFont);
        return numItems;
    }

    if (xPos + txtWidth > (LONG)width)
        txtWidth = (LONG)width - xPos;

    LONG char_count = 0;
    BYTE* tBuff = nullptr;
    UINT pitchBytes = 0;
    if (SUCCEEDED(p_butt->buttDx->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
        memset(tBuff, 0, pitchBytes * height);
        char_count = font->PrintText(tBuff + xPos + pitchBytes * yPos, msg, pitchBytes, txtWidth);
        p_butt->buttDx->Unlock(nullptr);
    }
    tBuff = nullptr;

    fall_SetFont(oldFont);
    return numItems;
}


//__________________________________________________________________________________________________________________________________________________
void Inventory_Update_Item_List(LONG listPos, LONG itemNum_selected, LONG item_x, LONG item_y, LONG item_w, LONG item_h, PUD* pPud, LONG* p_buttRef) {
    ButtonStruct_DX* p_butt = nullptr;
    DWORD frmID = -1;


    //Set up item list graphics
    ITEMblock* pItem = nullptr;
    LONG item_num = listPos;
    bool pickup = false;

    for (LONG buttNum = 0; buttNum < *p_inv_num_items_visible; buttNum++) {
        if (buttNum == itemNum_selected)
            pickup = true;
        else
            pickup = false;
        pItem = nullptr;
        if (item_num >= 0 && item_num < pPud->general.inv_size) {
            pItem = &pPud->general.p_item[pPud->general.inv_size - 1 - item_num];
            if (pItem)
                frmID = GetInventoryItemFrm(pItem->p_obj);
            else
                frmID = -1;
        }
        else
            frmID = -1;

        p_butt = (ButtonStruct_DX*)fall_Button_Get(p_buttRef[buttNum], nullptr);

        if (p_butt) {
            if (pItem) {
                if (Inventory_Draw_Item_Quantity_Text(pItem->p_obj, pItem->num, p_butt, 4, 4, pickup) <= 0)
                    frmID = -1;
                p_butt->buttDx->Overlay_SetFrm(frmID, (float)item_x, (float)item_y, item_w, item_h);

            }
            else {
                p_butt->buttDx->Overlay_SetFrm(-1, (float)item_x, (float)item_y, item_w, item_h);
                Inventory_Draw_Item_Quantity_Text(nullptr, 0, p_butt, 0, 0, 0);
            }
        }
        item_num++;
    }
}


//_________________________________________________________________________________
void Inventory_Update_Items_PC(LONG listPos, LONG itemNum_selected, LONG type_long) {
    INV_TYPE inv_type = static_cast<INV_TYPE>(type_long);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (!pWin || !pWin->winDx)
        return;

    ButtonStruct_DX* p_butt = nullptr;
    DWORD frmID = -1;

    LONG item_x = 4;
    LONG item_y = 4;
    LONG item_w = 56;
    LONG item_h = 40;

    Window_DX* subwin_held = nullptr;

    switch (inv_type) {
    case INV_TYPE::inv:
        if (*ppObj_PC_LeftHand != nullptr && *ppObj_PC_LeftHand == *ppObj_PC_RightHand) {//if a two handed item - I don't think this ever happens.
            //Set background for two handed items.
            if (winRef_Inv_PC_Two_Handed_Item_BG == -1) {
                subwin_held = new Window_DX(152, 284, 188, 67, 0x00000000, pWin->winDx, &winRef_Inv_PC_Two_Handed_Item_BG);
                DWORD frmID = 0;
                FRMCached* pfrm = nullptr;
                FRMframeDx* pFrame = nullptr;
                frmID = fall_GetFrmID(ART_INTRFACE, 32, 0, 0, 0); //SATTKBUP.FRM; single attack big up
                pfrm = new FRMCached(frmID);
                if (pfrm) {
                    pFrame = pfrm->GetFrame(0, 0);
                    if (pFrame)
                        subwin_held->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
                    delete pfrm;
                    pfrm = nullptr;
                    pFrame = nullptr;
                }
            }
            //Set two handed item graphic, centred between both hand buttons using left hand frm.
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_pc_leftHand, nullptr);
            if (p_butt) {
                if (itemNum_selected == 7)
                    frmID = -1;
                else
                    frmID = GetInventoryItemFrm(*ppObj_PC_LeftHand);
                p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 180, 61);
            }
            //Set right hand to not draw.
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_pc_rightHand, nullptr);
            if (p_butt)
                p_butt->buttDx->Overlay_SetFrm(-1, 0, 0, 90, 61);
        }
        else {
            //Remove background for two handed items if present.
            if (winRef_Inv_PC_Two_Handed_Item_BG != -1)
                pWin->winDx->DeleteSubWin(winRef_Inv_PC_Two_Handed_Item_BG);
            //Draw left hand item.
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_pc_leftHand, nullptr);
            if (p_butt) {
                if (itemNum_selected == 7)
                    frmID = -1;
                else
                    frmID = GetInventoryItemFrm(*ppObj_PC_LeftHand);
                p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 90, 61);
            }
            //Draw right hand item.
            p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_pc_rightHand, nullptr);
            if (p_butt) {
                if (itemNum_selected == 6)
                    frmID = -1;
                else
                    frmID = GetInventoryItemFrm(*ppObj_PC_RightHand);
                p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 90, 61);
            }
        }
        //Draw item being worn.
        p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_pc_armor, nullptr);
        if (p_butt) {
            if (itemNum_selected == 8)
                frmID = -1;
            else
                frmID = GetInventoryItemFrm(*ppObj_PC_Wearing);
            p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 90, 61);
        }
        //break;
    case INV_TYPE::use_on:
        //break;
    case INV_TYPE::loot:
        //Set pc inv buttons to enabled/disabled depending on the item list position - this is not done on the barter screen for some reason.
        if (*p_inv_scroll_up_buttId_pc != -1) {
            if (listPos > 0)
                SetButtonEnabledX(*p_inv_scroll_up_buttId_pc);
            else
                SetButtonDisabledX(*p_inv_scroll_up_buttId_pc);
        }
        if (*p_inv_scroll_dn_buttId_pc != -1) {
            if ((*pp_pud_PC)->general.inv_size - listPos > *p_inv_num_items_visible)
                SetButtonEnabledX(*p_inv_scroll_dn_buttId_pc);
            else
                SetButtonDisabledX(*p_inv_scroll_dn_buttId_pc);
        }
        break;
    case INV_TYPE::barter:
        //max item size is a little different on the barter screen ?
        //item_x = 2;
        //item_y = 4;
        //item_w = 59;
        //item_h = 40;
        break;
    default:
        break;
    }

    Inventory_Update_Item_List(listPos, itemNum_selected, item_x, item_y, item_w, item_h, *pp_pud_PC, p_buttRef_inv_pc);
}


//______________________________________________________
void __declspec(naked) h_inventory_update_items_pc(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call Inventory_Update_Items_PC
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//_____________________________________________________________
void __declspec(naked) h_inventory_update_items_pc_picked(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        mov edx, edi//edi has pressed button number
        sub edx, 1000//minus 1000 to get item number. - reserved item list = 0-5, right hand = 6, left hand = 7, armor = 8.
        push ebx
        push edx
        push eax
        call Inventory_Update_Items_PC
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//________________________________________________________________________________________________________
void Inventory_Update_Items_Target(LONG listPos, DWORD itemNum_selected, PUD* pTarget_pud, LONG type_long) {
    INV_TYPE inv_type = static_cast<INV_TYPE>(type_long);
    if (inv_type == INV_TYPE::loot) {
        //Set target inv buttons to enabled/disabled depending on the item list position - this is not done on the barter screen for some reason.
        if (*p_inv_scroll_up_buttId_target != -1) {
            if (listPos > 0)
                SetButtonEnabledX(*p_inv_scroll_up_buttId_target);
            else
                SetButtonDisabledX(*p_inv_scroll_up_buttId_target);
        }
        if (*p_inv_scroll_dn_buttId_target != -1) {
            if (pTarget_pud->general.inv_size - listPos > *p_inv_num_items_visible)
                SetButtonEnabledX(*p_inv_scroll_dn_buttId_target);
            else
                SetButtonDisabledX(*p_inv_scroll_dn_buttId_target);
        }
    }

    Inventory_Update_Item_List(listPos, itemNum_selected, 4, 4, 56, 40, pTarget_pud, p_buttRef_inv_target);
}


//__________________________________________________________
void __declspec(naked) h_inventory_update_items_target(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ecx
        push ebx
        push edx
        push eax
        call Inventory_Update_Items_Target
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//_________________________________________________________________________________________________________________________________
void Inventory_Update_Items_Tables(LONG winRef, OBJStructDx* pObj_PC_Table, OBJStructDx* pObj_Target_Table, DWORD itemNum_selected) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (!pWin || !pWin->winDx)
        return;

    LONG msg_y = 48 * *p_inv_num_items_visible + 24;
    char msg[32];

    int oldFont = fall_GetFont();
    fall_SetFont(101);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG fontHeight = (LONG)font->GetFontHeight();


    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_LightGrey & 0x000000FF);

    if (pObj_PC_Table) {
        Inventory_Update_Item_List(*p_inv_pc_table_listPos, itemNum_selected, 4, 4, 56, 40, &pObj_PC_Table->pud, p_buttRef_inv_pc_table);
        LONG msg_x = 169;
        if (*p_PC_party_size)
            sprintf_s(msg, "%s %d", (char*)GetMsg(pMsgList_Inventory, 30, 2), fall_Obj_Inventory_GetTotalWeight((OBJStruct*)pObj_PC_Table));

        else
            sprintf_s(msg, "$%d", fall_Obj_Inventory_GetTotalCost((OBJStruct*)pObj_PC_Table));
        RECT rect{ msg_x, msg_y, msg_x + 64, msg_y + fontHeight };
        pWin->winDx->ClearRect(&rect);
        pWin->winDx->Draw_Text(msg, msg_x, msg_y, colour, 0, TextEffects::none);
    }

    if (pObj_Target_Table) {
        Inventory_Update_Item_List(*p_inv_target_table_listPos, itemNum_selected, 4, 4, 56, 40, &pObj_Target_Table->pud, p_buttRef_inv_target_table);
        LONG msg_x = 254;
        if (*p_PC_party_size)
            sprintf_s(msg, "%s %d", (char*)GetMsg(pMsgList_Inventory, 30, 2), fall_Obj_Inventory_GetTotalWeight((OBJStruct*)pObj_Target_Table));

        else
            sprintf_s(msg, "$%d", fall_Obj_Inventory_GetTotalCost((OBJStruct*)pObj_Target_Table));

        RECT rect{ msg_x, msg_y, msg_x + 64, msg_y + fontHeight };
        pWin->winDx->ClearRect(&rect);
        pWin->winDx->Draw_Text(msg, msg_x, msg_y, colour, 0, TextEffects::none);
    }

    fall_SetFont(oldFont);
}


//__________________________________________________________
void __declspec(naked) h_inventory_update_items_tables(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ecx
        push ebx
        push edx
        push eax
        call Inventory_Update_Items_Tables
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//_________________________________________________________________
void Inventory_Update_Portraits(DWORD frmID_Target, LONG type_long) {
    INV_TYPE inv_type = static_cast<INV_TYPE>(type_long);
    static ULONGLONG  last_time = 0;
    ULONGLONG time = GetTickCount64();

    if (time < last_time + 166)
        return;

    static LONG ori_rotation = 0;
    ori_rotation++;
    if (ori_rotation >= 6)
        ori_rotation = 0;


    if (inv_type == INV_TYPE::inv || inv_type == INV_TYPE::use_on) {
        //setup the rotating PC portrait for "inventory" and "use on" screens.
        FRMCached* pfrm = new FRMCached(*p_frmID_inv_pc);
        if (pfrm) {
            int frameNum = 0;
            FRMframeDx* pFrame = pfrm->GetFrame(ori_rotation, frameNum);
            if (pFrame) {
                ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_pc, nullptr);
                if (p_butt) {
                    LONG butt_w = p_butt->rect.right - p_butt->rect.left + 1;
                    LONG butt_h = p_butt->rect.bottom - p_butt->rect.top + 1;
                    p_butt->buttDx->Overlay_SetFrm(*p_frmID_inv_pc, ori_rotation, frameNum, (float)(butt_w - pFrame->GetWidth()) / 2, (float)(butt_h - pFrame->GetHeight()) / 2, 0, 0);
                }
            }
            delete pfrm;
            pfrm = nullptr;
            pFrame = nullptr;
        }
        last_time = time;
    }
    else {
        //setup the PC portrait
        if (*p_frmID_inv_pc != -1) {
            FRMCached* pfrm = new FRMCached(*p_frmID_inv_pc);
            if (pfrm) {
                int frameNum = 0;
                FRMframeDx* pFrame = pfrm->GetFrame(3, frameNum);
                if (pFrame) {
                    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_pc, nullptr);
                    if (p_butt) {
                        LONG butt_w = p_butt->rect.right - p_butt->rect.left + 1;
                        LONG butt_h = p_butt->rect.bottom - p_butt->rect.top + 1;
                        p_butt->buttDx->Overlay_SetFrm(*p_frmID_inv_pc, 3, frameNum, (float)(butt_w - pFrame->GetWidth()) / 2, (float)(butt_h - pFrame->GetHeight()) / 2, 0, 0);
                    }
                }
                delete pfrm;
                pfrm = nullptr;
                pFrame = nullptr;
            }
        }
        //setup the Target portrait
        if (frmID_Target != -1) {
            FRMCached* pfrm = new FRMCached(frmID_Target);
            if (pfrm) {
                FRMdx* frmDx = pfrm->GetFrm();

                int frameNum = 0;
                if (frmDx)
                    frameNum = frmDx->GetNumFrames() - 1;

                int target_ori = 2;
                OBJStruct* pObj = pp_Obj_stack_Target[*p_stack_offset_Target];
                if (pObj)
                    target_ori = pObj->ori;

                FRMframeDx* pFrame = pfrm->GetFrame(target_ori, frameNum);
                if (pFrame) {
                    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_inv_portrait_target, nullptr);
                    if (p_butt) {
                        LONG butt_w = p_butt->rect.right - p_butt->rect.left + 1;
                        LONG butt_h = p_butt->rect.bottom - p_butt->rect.top + 1;
                        p_butt->buttDx->Overlay_SetFrm(frmID_Target, target_ori, frameNum, (float)(butt_w - pFrame->GetWidth()) / 2, (float)(butt_h - pFrame->GetHeight()) / 2, 0, 0);
                    }
                }
                delete pfrm;
                pfrm = nullptr;
                frmDx = nullptr;
                pFrame = nullptr;
            }
        }
    }
}


//_______________________________________________________
void __declspec(naked) h_inventory_update_portraits(void) {
    //00470650 / $  53            PUSH EBX; fallout2.void INV_REFRESH_CRITTER_PORTRAITS(EAX frmID_target, EDX invNum) 0 = inv, 1 = use - on, 2 = loot, 3 = barter, 4 = move - multi, 5 = move - multi(guessed void)
    //00470651 | .  51            PUSH ECX
    //00470652 | .  56            PUSH ESI
    //00470653 | .  57            PUSH EDI
    //00470654 | .  55            PUSH EBP
    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Inventory_Update_Portraits
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//______________________________________
void Inventory_Update_Console_PC_Stats() {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (!pWin || !pWin->winDx)
        return;
    int oldFont = fall_GetFont();
    fall_SetFont(101);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG fontHeight = (LONG)font->GetFontHeight();

    Window_DX* pWinSub = nullptr;
    if (winRef_Inv_Console == -1)
        //pWinSub = new Window_DX(297, 44, 152, 19 * fontHeight, 0x00000000, pWin->winDx, &winRef_Inv_Console);
        pWinSub = new Window_DX(297, 44, 148, 19 * fontHeight, 0x00000000, pWin->winDx, &winRef_Inv_Console);
    else
        pWinSub = pWin->winDx->GetSubWin(winRef_Inv_Console);

    if (!pWinSub) {
        fall_SetFont(oldFont);
        return;
    }
    pWinSub->ClearRenderTarget(nullptr);


    LONG y_Pos = 0;

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    //pc name text
    pWinSub->Draw_Text(fall_obj_GetNameFromScript(*pp_Obj_stack_PC), 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

    y_Pos += fontHeight;

    BYTE divid_Line[143];
    memset(divid_Line, *pPalColour_ConsoleGreen & 0x000000FF, 143);
    pWinSub->Draw(divid_Line, nullptr, false, 143, 1, 0, 0, 143, 1, 1, y_Pos + fontHeight / 2);

    y_Pos += fontHeight;

    char msg[64];

    LONG stats_thresh[7] = {
        STAT::current_hp,
        STAT::ac,
        STAT::dmg_thresh,
        STAT::dmg_thresh_laser,
        STAT::dmg_thresh_fire,
        STAT::dmg_thresh_plasma,
        STAT::dmg_thresh_explosion
    };
    LONG stats_max[7] = {
        STAT::max_hit_points,
        -1,
        STAT::dmg_resist,
        STAT::dmg_resist_laser,
        STAT::dmg_resist_fire,
        STAT::dmg_resist_plasma,
        STAT::dmg_resist_explosion
    };

    for (int i = 0; i < 7; i++) {
        // SPECIAL System stats
        pWinSub->Draw_Text(GetMsg(pMsgList_Inventory, i, 2), 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
        sprintf_s(msg, "%d", fall_Obj_GetStat(*pp_Obj_stack_PC, i));
        pWinSub->Draw_Text(msg, 24, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

        //other stats
        pWinSub->Draw_Text(GetMsg(pMsgList_Inventory, i + 7, 2), 40, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

        if (stats_max[i] == -1)
            sprintf_s(msg, "   %d", fall_Obj_GetStat(*pp_Obj_stack_PC, stats_thresh[i]));
        else {
            if (i == 0)
                sprintf_s(msg, "%d/%d", fall_Obj_GetStat(*pp_Obj_stack_PC, stats_thresh[i]), fall_Obj_GetStat(*pp_Obj_stack_PC, stats_max[i]));
            else
                sprintf_s(msg, "%d/%d%%", fall_Obj_GetStat(*pp_Obj_stack_PC, stats_thresh[i]), fall_Obj_GetStat(*pp_Obj_stack_PC, stats_max[i]));
        }
        pWinSub->Draw_Text(msg, 104, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

        y_Pos += fontHeight;
    }

    pWinSub->Draw(divid_Line, nullptr, false, 143, 1, 0, 0, 143, 1, 1, y_Pos + fontHeight / 2);
    y_Pos += fontHeight;


    //stuff being held text
    OBJStruct* p_Obj_hand = *ppObj_PC_LeftHand;
    LONG attack_hand = ATTACK_TYPE::left_primary;
    for (int i = 0; i < 2; i++) {
        if (i == 1) {
            p_Obj_hand = *ppObj_PC_RightHand;
            attack_hand = ATTACK_TYPE::right_primary;
        }
        if (p_Obj_hand) {
            pWinSub->Draw_Text(fall_obj_GetProtoName(p_Obj_hand), 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
            y_Pos += fontHeight;
            LONG itemType = fall_Obj_Item_GetType(p_Obj_hand);


            if (itemType == ITEM_TYPE::weapon) {
                LONG range = fall_Obj_Critter_GetWeaponRange(*pp_Obj_stack_PC, attack_hand);
                LONG damage_min = 0;
                LONG damage_max = 0;
                fall_Obj_Item_Weapon_GetDamage(p_Obj_hand, &damage_min, &damage_max);

                LONG subType = fall_Obj_Item_Weapon_GetSubType(p_Obj_hand, attack_hand);

                LONG melee_dmg = 0;
                if (subType == ATTACK_SUB_TYPE::unarmed || subType == ATTACK_SUB_TYPE::melee)
                    melee_dmg = fall_Obj_GetStat(*pp_Obj_stack_PC, STAT::melee_dmg);

                char* pMsg_damage = GetMsg(pMsgList_Inventory, 15, 2);

                if (subType == ATTACK_SUB_TYPE::guns || range > 1) {
                    char* pMsg_range = GetMsg(pMsgList_Inventory, 16, 2);
                    sprintf_s(msg, "%s %d-%d   %s %d", pMsg_damage, damage_min + melee_dmg, damage_max, pMsg_range, range);
                }
                else//throwing ?
                    sprintf_s(msg, "%s %d-%d", pMsg_damage, damage_min, damage_max);
                pWinSub->Draw_Text(msg, 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

                y_Pos += fontHeight;

                LONG ammo_max = fall_Obj_Item_GetMaxAmmo(p_Obj_hand);

                if (ammo_max > 0) {
                    char* pMsg_ammo = GetMsg(pMsgList_Inventory, 17, 2);
                    DWORD proID = fall_Obj_Item_Weapon_GetAmmoProID(p_Obj_hand);
                    LONG ammo_current = fall_Obj_Item_GetCurrentAmmo(p_Obj_hand);

                    if (ammo_current != 0 && proID != -1)
                        sprintf_s(msg, "%s %d/%d %s", pMsg_ammo, ammo_current, ammo_max, fall_Get_Prototype_Name(proID));
                    else
                        sprintf_s(msg, "%s %d/%d", pMsg_ammo, ammo_current, ammo_max);

                    pWinSub->Draw_Text(msg, 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
                }
                y_Pos += fontHeight;
            }
            else {
                if (itemType == ITEM_TYPE::armor)
                    pWinSub->Draw_Text(GetMsg(pMsgList_Inventory, 18, 2), 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
                y_Pos += fontHeight;
                y_Pos += fontHeight;
            }
        }
        else {//hand to hand
            pWinSub->Draw_Text(GetMsg(pMsgList_Inventory, 14, 2), 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
            y_Pos += fontHeight;

            char* pMsg_unarmed_damage = GetMsg(pMsgList_Inventory, 24, 2);
            sprintf_s(msg, "%s 1-%d", pMsg_unarmed_damage, fall_Obj_GetStat(*pp_Obj_stack_PC, STAT::melee_dmg));
            pWinSub->Draw_Text(msg, 0, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);
            y_Pos += fontHeight;

            y_Pos += fontHeight;
        }
        pWinSub->Draw(divid_Line, nullptr, false, 143, 1, 0, 0, 143, 1, 1, y_Pos + fontHeight / 2);
        y_Pos += fontHeight;

    }

    LONG carryWeight = fall_Obj_Inventory_GetTotalWeight(*pp_Obj_stack_PC);
    LONG carryWeight_max = fall_Obj_GetStat(*pp_Obj_stack_PC, STAT::carry_amt);
    sprintf_s(msg, "%s %d/%d", GetMsg(pMsgList_Inventory, 20, 2), carryWeight, carryWeight_max);
    //centre weight text in relation to divide lines, original was 15 xPos.
    LONG textWidth = (LONG)font->GetTextWidth(msg);
    if (carryWeight > carryWeight_max && color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleRed & 0x000000FF);
    pWinSub->Draw_Text(msg, 71 - textWidth / 2, y_Pos, colour, 0xFFFFFFFF, TextEffects::none);

    fall_SetFont(oldFont);
}


//______________________________________________________________
void __declspec(naked) h_inventory_update_console_pc_stats(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Inventory_Update_Console_PC_Stats
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


LONG inv_console_line_y = 1;
//___________________________________________________
void Inventory_Update_Console_Text(const char* pText) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (!pWin || !pWin->winDx)
        return;
    if (winRef_Inv_Console == -1)
        return;
    Window_DX* pWinSub = pWin->winDx->GetSubWin(winRef_Inv_Console);
    if (!pWinSub)
        return;


    int oldFont = fall_GetFont();
    fall_SetFont(101);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG fontHeight = (LONG)font->GetFontHeight();

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    RECT margins = { 0, inv_console_line_y, (LONG)pWinSub->GetWidth(), (LONG)pWinSub->GetHeight() };
    pWinSub->Draw_Text_Formated(pText, &margins, colour, 0xFFFFFFFF, 0, TextEffects::none);

    inv_console_line_y = margins.top;

    fall_SetFont(oldFont);
}


//________________________________________________________
void __declspec(naked) inventory_update_console_text(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Inventory_Update_Console_Text
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


//________________________________________________________________________________________
void Inventory_Update_Console_Item_Details(OBJStruct* p_obj_holder, OBJStruct* p_obj_item) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (!pWin || !pWin->winDx)
        return;
    int oldFont = fall_GetFont();
    fall_SetFont(101);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG fontHeight = (LONG)font->GetFontHeight();

    Window_DX* pWinSub = nullptr;
    if (winRef_Inv_Console == -1)
        //pWinSub = new Window_DX(297, 44, 152, 19 * fontHeight, 0x00000000, pWin->winDx, &winRef_Inv_Console);
        pWinSub = new Window_DX(297, 44, 148, 19 * fontHeight, 0x00000000, pWin->winDx, &winRef_Inv_Console);
    else
        pWinSub = pWin->winDx->GetSubWin(winRef_Inv_Console);

    if (!pWinSub) {
        fall_SetFont(oldFont);
        return;
    }
    pWinSub->ClearRenderTarget(nullptr);


    inv_console_line_y = fontHeight;


    Inventory_Update_Console_Text(fall_obj_GetName(p_obj_item));

    BYTE divid_Line[143];
    memset(divid_Line, *pPalColour_ConsoleGreen & 0x000000FF, 143);
    pWinSub->Draw(divid_Line, nullptr, false, 143, 1, 0, 0, 143, 1, 0, inv_console_line_y);

    inv_console_line_y += fontHeight;

    fall_Obj_Examine(p_obj_holder, p_obj_item, &inventory_update_console_text);

    LONG itemWeight = fall_Obj_Item_GetWeight(p_obj_item);
    if (itemWeight > 0) {
        LONG msgNum = 540;
        if (itemWeight == 1)
            msgNum = 541;
        char* pMsg = GetMsg(pMsgList_Proto_Main, msgNum, 2);
        if (!pMsg)
            Fallout_Debug_Error(" INV Item - Couldn't get message!");
        else {
            char msg[64];
            sprintf_s(msg, pMsg, itemWeight);
            Inventory_Update_Console_Text(msg);
        }
    }
    fall_SetFont(oldFont);
}


//__________________________________________________________________
void __declspec(naked) h_inventory_update_console_item_details(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Inventory_Update_Console_Item_Details
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_______________________________________________
void Inventory_MoveMulti_Or_SetTimer_Destructor() {
    if(*p_winRef_MoveItems != -1)
        fall_Win_Destroy(*p_winRef_MoveItems);
    *p_winRef_MoveItems = -1;
}


//___________________________________________________________
void OnScreenResize_Inventory_MoveMulti(Window_DX* pWin_This) {
    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    LONG x_win = 0;
    LONG y_win = 0;
    if (*pWinRef_DialogMain != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_DialogMain);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else if (*pWinRef_Inventory != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else {
        x_win = ((LONG)SCR_WIDTH - 640) / 2;
        y_win = (Get_GameWin_Height() - 480) / 2;
        if (y_win < 0)
            y_win = 0;
    }
    x_win += inv_details[static_cast<LONG>(INV_TYPE::move_items)].x;
    y_win += inv_details[static_cast<LONG>(INV_TYPE::move_items)].y;

    MoveWindowX(pWin, x_win, y_win);
}


//__________________________________________________________
void OnScreenResize_Inventory_SetTimer(Window_DX* pWin_This) {
    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    LONG x_win = 0;
    LONG y_win = 0;
    if (*pWinRef_DialogMain != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_DialogMain);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else if (*pWinRef_Inventory != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else {
        x_win = ((LONG)SCR_WIDTH - 640) / 2;
        y_win = (Get_GameWin_Height() - 480) / 2;
        if (y_win < 0)
            y_win = 0;
    }
    x_win += inv_details[static_cast<LONG>(INV_TYPE::set_timer)].x;
    y_win += inv_details[static_cast<LONG>(INV_TYPE::set_timer)].y;

    MoveWindowX(pWin, x_win, y_win);
}


//_________________________________________________________________________________________
WinStructDx* Inventory_MoveMulti_Or_SetTimer_Setup(INV_TYPE inv_type, OBJStruct* pObj_Item) {
    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;


    frmID = fall_GetFrmID(ART_INTRFACE, inv_details[static_cast<LONG>(inv_type)].frm_list_num, 0, 0, 0);

    pfrm = new FRMCached(frmID);
    if (!pfrm)
        return nullptr;
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = nullptr;
        return nullptr;
    }

    DWORD winHeight = pFrame->GetHeight();

    LONG x_win = 0;
    LONG y_win = 0;
    if (*pWinRef_DialogMain != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_DialogMain);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else if (*pWinRef_Inventory != -1) {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
        if (pWin) {
            x_win = pWin->rect.left;
            y_win = pWin->rect.top;
        }
    }
    else {
        x_win = ((LONG)SCR_WIDTH - 640) / 2;
        y_win = (Get_GameWin_Height() - 480) / 2;
        if (y_win < 0)
            y_win = 0;
    }
    x_win += inv_details[static_cast<LONG>(inv_type)].x;
    y_win += inv_details[static_cast<LONG>(inv_type)].y;

    *p_winRef_MoveItems = fall_Win_Create(x_win, y_win, inv_details[static_cast<LONG>(inv_type)].width, inv_details[static_cast<LONG>(inv_type)].height, 0x101, FLG_WinToFront | FLG_WinExclusive);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*p_winRef_MoveItems);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return nullptr;
    }

    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    if (inv_type == INV_TYPE::move_items)
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Inventory_MoveMulti);
    else
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Inventory_SetTimer);

    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    int oldFont = fall_GetFont();
    fall_SetFont(103);

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG font_height = font->GetFontHeight();

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_Mustard & 0x000000FF);

    char* pMsg = nullptr;

    if (inv_type == INV_TYPE::move_items) {//draw move items text
        pMsg = GetMsg(pMsgList_Inventory, 21, 2);
        if (pMsg) {
            LONG textWidth = (LONG)font->GetTextWidth(pMsg);
            pWin->winDx->Draw_Text(pMsg, (inv_details[static_cast<LONG>(inv_type)].width - textWidth) / 2, 9, colour, 0, TextEffects::none);
        }
    }
    else {//draw set timer text and timer background overlay
        pMsg = GetMsg(pMsgList_Inventory, 23, 2);
        if (pMsg) {
            LONG textWidth = (LONG)font->GetTextWidth(pMsg);
            pWin->winDx->Draw_Text(pMsg, (inv_details[static_cast<LONG>(inv_type)].width - textWidth) / 2, 9, colour, 0, TextEffects::none);
        }
        frmID = fall_GetFrmID(ART_INTRFACE, 306, 0, 0, 0);//TIMER.FRM      ; timer overlay for move multiple items interface
        pfrm = new FRMCached(frmID);
        if (pfrm) {
            pFrame = pfrm->GetFrame(0, 0);
            if (pFrame)
                pWin->winDx->RenderTargetDrawFrame(113, 34, pFrame, nullptr, nullptr);

            delete pfrm;
            pfrm = nullptr;
            pFrame = nullptr;
        }
    }

    //create a button for the item image - this wont do anything else.
    frmID = GetInventoryItemFrm(pObj_Item);
    LONG buttRef = CreateButtonX_Overlay(*p_winRef_MoveItems, 16, 46, 90, 61, -1, -1, -1, -1, 0, 0, 0, 0);
    //set item image
    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (p_butt)
        p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 90, 61);

    //[plus] button
    //0xC1 SPLSOFF.FRM; Character editor
    //0xC2 SPLSON.FRM; Character editor
    if (inv_type == INV_TYPE::move_items)
        buttRef = CreateButtonX(*p_winRef_MoveItems, 200, 46, 16, 12, -1, -1, 6000, -1, 0x060000C1, 0x060000C2, 0, FLG_ButtTrans);
    else //set_timer
        buttRef = CreateButtonX(*p_winRef_MoveItems, 194, 64, 16, 12, -1, -1, 6000, -1, 0x060000C1, 0x060000C2, 0, FLG_ButtTrans);

    if (buttRef != -1)
        SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    //[negative] button
    //0xBF SNEGOFF.FRM; Character editor
    //0xC0 SNEGON.FRM; Character editor
    if (inv_type == INV_TYPE::move_items)
        buttRef = CreateButtonX(*p_winRef_MoveItems, 200, 46 + 12, 17, 12, -1, -1, 7000, -1, 0x060000BF, 0x060000C0, 0, FLG_ButtTrans);
    else  //set_timer
        buttRef = CreateButtonX(*p_winRef_MoveItems, 194, 64 + 12, 17, 12, -1, -1, 7000, -1, 0x060000BF, 0x060000C0, 0, FLG_ButtTrans);

    if (buttRef != -1)
        SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    //0x8 lilredup.frm; little red button up
    //0x9 lilreddn.frm; little red button down
    //[done] button
    buttRef = CreateButtonX(*p_winRef_MoveItems, 98, 128, 15, 16, -1, -1, -1, 0x0D, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
    if (buttRef != -1)
        SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
    //[cancel] button
    buttRef = CreateButtonX(*p_winRef_MoveItems, 148, 128, 15, 16, -1, -1, -1, 0x1B, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
    if (buttRef != -1)
        SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    if (inv_type == INV_TYPE::move_items) {
        //[all] button
        //0x133 ALLBON.FRM; ALL button(pressed) for move multiple items interface
        //0x134 ALLBOFF.FRM; ALL button(unpressed) for move multiple items interface
        buttRef = CreateButtonX_Overlay(*p_winRef_MoveItems, 120, 80, 94, 33, -1, -1, -1, 5000, 0x06000133, 0x06000134, 0, 0);
        if (buttRef != -1)
            SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);
        pMsg = GetMsg(pMsgList_Inventory, 22, 2);//22 all text
        p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
        if (pMsg && p_butt) {
            unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
            p_butt->buttDx->Overlay_CreateTexture(0, 0, 0, 0);
            p_butt->buttDx->SetOverlayRenderTarget();
            DirectX::XMMATRIX Ortho2D;
            p_butt->buttDx->GetOrthoMatrix(&Ortho2D);
            DWORD msg_width = font->GetTextWidth(pMsg);

            FRMframeDx* pFrame = new FRMframeDx(nullptr, msg_width, font_height, nullptr, nullptr, false, true);
            BYTE* tBuff = nullptr;
            UINT pitchBytes = 0;
            if (SUCCEEDED(pFrame->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
                font->PrintText(tBuff + 1 + pitchBytes, pMsg, pitchBytes, msg_width);
                pFrame->Unlock(nullptr);
                GEN_SURFACE_BUFF_DATA genSurfaceData;
                XMFLOAT4 colour_text = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f };
                genSurfaceData.genData4_1 = colour_text;
                pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
                pFrame->DrawFrame(&Ortho2D, (float)(94 - msg_width) / 2, (float)(33 - font_height) / 2 - 3, pd3d_PS_Colour_32_Alpha);
            }
            tBuff = nullptr;
            delete pFrame;
            pFrame = nullptr;
        }
    }

    fall_Mouse_SetImageFromList(1);
    fall_SetFont(oldFont);
    return pWin;
}


#define DIGIT_COUNT  5
#define DIGIT_WIDTH  14
#define DIGIT_HEIGHT  24
#define DIGIT_RED  DIGIT_WIDTH * 12
//__________________________________________________________________________________________________________________________
void Inventory_MoveMulti_Or_SetTimer_DrawNumbers(Window_DX* pWin_Counter, LONG number, INV_TYPE inv_type, bool isOutOfScope) {

    if (!pWin_Counter)
        return;
    pWin_Counter->ClearRenderTarget(nullptr);

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    DirectX::XMMATRIX Ortho2D;
    pWin_Counter->GetOrthoMatrix(&Ortho2D);

    LONG offset_colour = 0;
    if (isOutOfScope)
        offset_colour = DIGIT_RED;

    FRMCached* cfrm = new FRMCached(0x060000AA);//BIGNUM.FRM     ; Character editor
    FRMframeDx* pFrame = cfrm->GetFrame(0, 0);
    if (!pFrame) {
        delete cfrm;
        cfrm = nullptr;
        return;
    }
    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    pWin_Counter->SetRenderTarget(nullptr);

    if (inv_type == INV_TYPE::move_items) {
        LONG digit = 0;
        LONG digit_x = 0;
        D3D11_RECT digit_rect;
        LONG xPos = DIGIT_COUNT * DIGIT_WIDTH;

        for (int i = 0; i < DIGIT_COUNT; i++) {
            digit = number % 10;
            digit_x = digit * DIGIT_WIDTH + offset_colour;
            xPos -= DIGIT_WIDTH;
            digit_rect = { xPos , 0, xPos + DIGIT_WIDTH, DIGIT_HEIGHT };
            number /= 10;
            //DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect, digit_x);
            pD3DDevContext->RSSetScissorRects(1, &digit_rect);
            pFrame->DrawFrame(&Ortho2D, (float)(digit_rect.left - digit_x), 0.0f);
        }
    }
    else {
        LONG digit = 0;
        LONG digit_x = 0;
        D3D11_RECT digit_rect;

        LONG xPos = 4 * DIGIT_WIDTH;
        //char msg[12];

        for (int i = 0; i < 3; i++) {
            xPos -= DIGIT_WIDTH;
            if (i == 0) {
                digit = number % 10;
            }
            else if (i == 1) {
                digit = number % 60;
                digit /= 10;
                number /= 60;
            }
            else if (i == 2) {
                digit = number % 10;
                xPos -= DIGIT_WIDTH;
            }

            digit_x = digit * DIGIT_WIDTH + offset_colour;
            digit_rect = { xPos , 0, xPos + DIGIT_WIDTH, DIGIT_HEIGHT };

            //DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect, digit_x);
            pD3DDevContext->RSSetScissorRects(1, &digit_rect);
            pFrame->DrawFrame(&Ortho2D, (float)(digit_rect.left - digit_x), 0.0f);
        }
    }
    delete cfrm;
    cfrm = nullptr;
}


//______________________________________________________________________________________
LONG Inventory_MoveMulti_Or_SetTimer(LONG type_long, OBJStruct* pObj_Item, LONG num_max) {
    INV_TYPE inv_type = static_cast<INV_TYPE>(type_long);
    if (inv_type != INV_TYPE::move_items && inv_type != INV_TYPE::set_timer)
        return -1;
    WinStructDx* pWin = Inventory_MoveMulti_Or_SetTimer_Setup(inv_type, pObj_Item);
    if (!pWin)
        return -1;
    Window_DX* pWin_Counter = nullptr;
    if (inv_type == INV_TYPE::move_items)
        pWin_Counter = new Window_DX(125, 45, DIGIT_COUNT * DIGIT_WIDTH, DIGIT_HEIGHT, 0xFF0000FF, pWin->winDx, nullptr);
    else //set_timer
        pWin_Counter = new Window_DX(133, 64, 4 * DIGIT_WIDTH, DIGIT_HEIGHT, 0x00000000, pWin->winDx, nullptr);


    LONG num_min = 1;
    LONG num_current = 1;
    if (inv_type == INV_TYPE::move_items) {
        num_current = 1;
        num_min = 1;
        if (num_max > 99999)
            num_max = 99999;
    }
    else {//set_timer
        num_current = 60;
        num_min = 10;
    }

    bool isOutOfScope = false;
    bool key_input = false;

    LONG input = -1;
    bool exit_loop = false;

    while (!exit_loop) {
        if (num_current < num_min || num_current > num_max)
            isOutOfScope = true;
        else
            isOutOfScope = false;
        Inventory_MoveMulti_Or_SetTimer_DrawNumbers(pWin_Counter, num_current, inv_type, isOutOfScope);

        bool draw_number = false;
        while (!draw_number) {
            input = fall_Get_Input();

            if (input == 0x1B) {
                Inventory_MoveMulti_Or_SetTimer_Destructor();
                return -1;
            }
            else if (input == 0x0D) {
                if (num_current < num_min || num_current > num_max)
                    fall_PlayAcm("iisxxxx1");
                else {
                    if (inv_type == INV_TYPE::set_timer) {
                        if (num_current % 10 != 0)//time must be in increments of 10 seconds
                            fall_PlayAcm("iisxxxx1");
                        else {
                            Inventory_MoveMulti_Or_SetTimer_Destructor();
                            return num_current;
                        }
                    }
                    else {
                        Inventory_MoveMulti_Or_SetTimer_Destructor();
                        return num_current;
                    }
                }
            }
            else if (input == 6000) {//[plus] button
                key_input = false;
                if (num_current >= num_max)
                    continue;
                if (inv_type == INV_TYPE::move_items) {
                    if (GetMouseFlags() & FLG_MouseL_Hold) {
                        LONG count = 100;
                        while (GetMouseFlags() & FLG_MouseL_Hold) {
                            if (num_current < num_max)
                                num_current++;
                            if (num_current < num_min || num_current > num_max)
                                isOutOfScope = true;
                            else
                                isOutOfScope = false;
                            Inventory_MoveMulti_Or_SetTimer_DrawNumbers(pWin_Counter, num_current, inv_type, isOutOfScope);
                            Dx_Present_Main();
                            fall_Get_Input();
                            if (count > 1) {
                                count--;
                                Wait_ms(count);
                            }
                        }
                        draw_number = true;
                        continue;
                    }
                    if (num_current < num_max)
                        num_current++;
                    draw_number = true;
                }
                else {//set_timer
                    num_current += 10;
                    draw_number = true;
                }
            }
            else if (input == 7000) {//[neg] button
                key_input = false;
                if (num_current <= num_min)
                    continue;
                if (inv_type == INV_TYPE::move_items) {
                    if (GetMouseFlags() & FLG_MouseL_Hold) {
                        LONG count = 100;
                        while (GetMouseFlags() & FLG_MouseL_Hold) {
                            if (num_current > num_min)
                                num_current--;
                            if (num_current < num_min || num_current > num_max)
                                isOutOfScope = true;
                            else
                                isOutOfScope = false;
                            Inventory_MoveMulti_Or_SetTimer_DrawNumbers(pWin_Counter, num_current, inv_type, isOutOfScope);
                            Dx_Present_Main();
                            fall_Get_Input();
                            if (count > 1) {
                                count--;
                                Wait_ms(count);
                            }
                        }
                        draw_number = true;
                        continue;
                    }
                    if (num_current > num_min)
                        num_current--;
                    draw_number = true;
                }
                else {//set_timer
                    num_current -= 10;
                    draw_number = true;
                }
            }
            else if (input == 5000) {//[all] button
                key_input = false;
                num_current = num_max;
                draw_number = true;
            }
            else if (inv_type == INV_TYPE::move_items) {
                switch (input) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (key_input == false)
                        num_current = 0;
                    input -= '0';
                    num_current *= 10;
                    num_current %= 100000;//keep number under 100000
                    num_current += input;
                    key_input = true;
                    draw_number = true;
                    break;
                case '\b'://back space
                    if (key_input == false)
                        num_current = 0;
                    num_current /= 10;
                    draw_number = true;
                    break;
                default:
                    break;
                }
            }
        }
    }

    Inventory_MoveMulti_Or_SetTimer_Destructor();
    return -1;
}


//______________________________________________________________
void __declspec(naked) h_inventory_move_multi_or_set_timer(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call Inventory_MoveMulti_Or_SetTimer
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//______________________________________________________________
LONG Inventory_Mouse_MultiMenu(DWORD* pfIdLstNum, LONG numItems) {
    //no longer necessary to contain the mouse menu within the inv window border, just use the screen rect.

    //WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    //if(pWin)
    //    return MouseDX_MenuMulti(pfIdLstNum, numItems, pWin->rect.right, pWin->rect.bottom);
    return MouseDX_MenuMulti(pfIdLstNum, numItems, pFALL_RC->right, pFALL_RC->bottom);
}


//_______________________________________________________
void __declspec(naked) h_inventory_mouse_multi_menu(void) {

    __asm {


        push esi
        push edi
        push ebp

        push ecx
        push ebx
        call Inventory_Mouse_MultiMenu
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}


//______________________________________________
LONG Inventory_Mouse_SingleMenu(DWORD fIdLstNum) {
    //no longer nessesary to contain the mouse menu within the inv window border, just use the screen rect.

    //WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    //if(pWin)
    //    return MouseDX_MenuMulti(pfIdLstNum, numItems, pWin->rect.right, pWin->rect.bottom);
    return MouseDX_MenuSingle(fIdLstNum, pFALL_RC->right, pFALL_RC->bottom);
}


//________________________________________________________
void __declspec(naked) h_inventory_mouse_single_menu(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ebx
        call Inventory_Mouse_SingleMenu
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        ret 0x4
    }
}


//__________________________________________________________________________
void Inventory_Adjust_Mouse_Drop_Position_In_Item_List(LONG* p_x, LONG* p_y) {
    fall_Mouse_GetPos(p_x, p_y);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Inventory);
    if (pWin) {
        *p_x -= pWin->rect.left;
        *p_y -= pWin->rect.top;
    }
}


//____________________________________________________________________
void __declspec(naked) h_inv_adjust_mouse_drop_position_in_item_list() {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Inventory_Adjust_Mouse_Drop_Position_In_Item_List
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________________________________________________
bool Mouse_Wheel_Inventory(int zDelta, int* p_keyCode, int* p_pageSize) {

    int upCode = 0x148;
    int dnCode = 0x150;

    bool isFocusOnPrimaryMenu = false;
    if (ConfigReadInt(L"INPUT", L"SCROLLWHEEL_FOCUS_PRIMARY_MENU", 1))
        isFocusOnPrimaryMenu = true;

    WinStruct* pWin = nullptr;

    pWin = fall_Win_Get(*pWinRef_Inventory);
    if (!pWin) {
        *p_keyCode = -1;
        return false;
    }
    *p_pageSize = *pInvListItemsVis;
    switch (invType_current) {
    case INV_TYPE::barter:
        if (fall_Mouse_IsInArea(rc_Items_Table_PC.left + pWin->rect.left, rc_Items_Table_PC.top + pWin->rect.top, rc_Items_Table_PC.right + pWin->rect.left, rc_Items_Table_PC.bottom + pWin->rect.top)) {
            upCode = 0x149;//PgUp
            dnCode = 0x151;//PgDn
            break;
        }
        if (fall_Mouse_IsInArea(rc_Items_Table_Target.left + pWin->rect.left, rc_Items_Table_Target.top + pWin->rect.top, rc_Items_Table_Target.right + pWin->rect.left, rc_Items_Table_Target.bottom + pWin->rect.top)) {
            upCode = 0x184;//Ctrl-PgUp
            dnCode = 0x176;//Ctrl-PgDn
            break;
        }
    case INV_TYPE::loot:
        if (fall_Mouse_IsInArea(rc_Items_Target.left + pWin->rect.left, rc_Items_Target.top + pWin->rect.top, rc_Items_Target.right + pWin->rect.left, rc_Items_Target.bottom + pWin->rect.top)) {
            upCode = 0x18D;//Ctrl-Up, 
            dnCode = 0x191;//Ctrl-Dn
            break;
        }
    case INV_TYPE::inv:
    case INV_TYPE::use_on:
        if (isFocusOnPrimaryMenu || fall_Mouse_IsInArea(rc_Items_PC.left + pWin->rect.left, rc_Items_PC.top + pWin->rect.top, rc_Items_PC.right + pWin->rect.left, rc_Items_PC.bottom + pWin->rect.top)) {
            upCode = 0x148;//Up
            dnCode = 0x150;//Dn
            break;
        }
    default:
        return false;
        break;
    }

    if (zDelta > 0)
        *p_keyCode = upCode;
    else if (zDelta < 0)
        *p_keyCode = dnCode;
    else
        return false;

    return true;
}


//________________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_armor() {

    __asm {
        push esi
        push edi
        push ebp

        push buttRef_inv_pc_armor
        call IsMouseInButtonRect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//___________________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_portrait() {

    __asm {
        push esi
        push edi
        push ebp

        push buttRef_inv_portrait_pc
        call IsMouseInButtonRect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//_______________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_left() {

    __asm {
        push esi
        push edi
        push ebp

        push buttRef_inv_pc_leftHand
        call IsMouseInButtonRect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//________________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_right() {

    __asm {
        push esi
        push edi
        push ebp

        push buttRef_inv_pc_rightHand
        call IsMouseInButtonRect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//________________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_items() {

    __asm {
        push esi
        push edi
        push ebp

        lea eax, rc_Items_PC
        push eax
        mov edi, pWinRef_Inventory
        push dword ptr ds : [edi]
        call IsMouseInWindowRect2
        add esp, 0x8

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//____________________________________________________
void __declspec(naked) h_inv_mouse_drop_target_items() {

    __asm {
        push esi
        push edi
        push ebp

        lea eax, rc_Items_Target
        push eax
        mov edi, pWinRef_Inventory
        push dword ptr ds : [edi]
        call IsMouseInWindowRect2
        add esp, 0x8

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//______________________________________________________
void __declspec(naked) h_inv_mouse_drop_pc_items_table() {

    __asm {
        push esi
        push edi
        push ebp

        lea eax, rc_Items_Table_PC
        push eax
        mov edi, pWinRef_Inventory
        push dword ptr ds : [edi]
        call IsMouseInWindowRect2
        add esp, 0x8

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//__________________________________________________________
void __declspec(naked) h_inv_mouse_drop_target_items_table() {

    __asm {
        push esi
        push edi
        push ebp

        lea eax, rc_Items_Table_Target
        push eax
        mov edi, pWinRef_Inventory
        push dword ptr ds : [edi]
        call IsMouseInWindowRect2
        add esp, 0x8

        pop ebp
        pop edi
        pop esi

        ret
    }
}


//_______________________________
void Modifications_Inventory_CH() {

    ///height in list items visible (6 for inv)
    pInvListItemsVis = (LONG*)0x528E44;

    inv_details = (INV_DETAILS*)0x528E58;


    //fix for dropping stuff in bags
    FuncReplace32(0x47085B, 0x05B773, (DWORD)&h_inv_adjust_mouse_drop_position_in_item_list);

    //MOUSE DROP RECTS

    //INV PC ITEMS LIST
    FuncWrite32(0x470846, 0x05B64A, (DWORD)&h_inv_mouse_drop_pc_items);
    //INV LEFT HAND
    FuncWrite32(0x470971, 0x05B51F, (DWORD)&h_inv_mouse_drop_pc_left);
    //INV RIGHT HAND
    FuncWrite32(0x470A02, 0x05B48E, (DWORD)&h_inv_mouse_drop_pc_right);
    //INV ARMOR
    FuncWrite32(0x470A8C, 0x05B404, (DWORD)&h_inv_mouse_drop_pc_armor);
    //INV PC PORTRAIT
    FuncWrite32(0x470B58, 0x05B338, (DWORD)&h_inv_mouse_drop_pc_portrait);

    //LOOT TARGET
    FuncWrite32(0x473F56, 0x057F3A, (DWORD)&h_inv_mouse_drop_target_items);
    //LOOT PC
    FuncWrite32(0x474020, 0x057E70, (DWORD)&h_inv_mouse_drop_pc_items);

    //BARTER PC ITEM LIST TABLE
    FuncWrite32(0x474573, 0x05791D, (DWORD)&h_inv_mouse_drop_pc_items_table);
    //BARTER TARGET ITEM LIST TABLE
    FuncWrite32(0x4745F6, 0x05789A, (DWORD)&h_inv_mouse_drop_target_items);
    //BARTER PC ITEM LIST
    FuncWrite32(0x47483E, 0x057652, (DWORD)&h_inv_mouse_drop_pc_items);
    //BARTER TARGET ITEM LIST
    FuncWrite32(0x4748BF, 0x0575D1, (DWORD)&h_inv_mouse_drop_target_items);

    //To-Do Modifications_Inventory_CH

    /*
    ///00470833  |.  BB BC000000   MOV EBX,0BC
    pRgt_Inv = (LONG*)0x470834;
    ///0047083B  |.  BA 23000000   MOV EDX,23
    pTop_Inv = (LONG*)0x47083C;
    ///00470840  |.  B8 7C000000   MOV EAX,7C
    pLft_Inv = (LONG*)0x470841;

    ///00473F44  |.  BB B9010000   MOV EBX,1B9
    pRgtNpc_Loot = (LONG*)0x473F45;
    ///00473F49  |.  BA 25000000   MOV EDX,25
    pTopNpc_Loot = (LONG*)0x473F4A;
    ///00473F50  |.  B8 79010000   MOV EAX,179
    pLftNpc_Loot = (LONG*)0x473F51;

    ///0047400E  |> \BB 40010000   MOV EBX,140
    pRgtPc_Loot = (LONG*)0x47400F;
    ///00474013  |.  BA 25000000   MOV EDX,25
    pTopPc_Loot = (LONG*)0x474014;
    ///0047401A  |.  B8 00010000   MOV EAX,100
    pLftPc_Loot = (LONG*)0x47401B;


    //0046E40E  |. E8 AADC0600    CALL Fallout2.004DC0BD
    //FuncWrite32(0x46E40F, 0x06DCAA, (DWORD)&h_inventory);

    //0046E4C9  |.  E8 EFDB0600   CALL 004DC0BD
    //FuncWrite32(0x46E4CA, 0x06DBEF, (DWORD)&h_dialog_sub_win_barter);

    ////0046F3B0  |.  E8 0BD00600   CALL 004DC3C0                            ; [Fallout2.004DC3C0
    //FuncReplace32(0x46F3B1, 0x06D00B, (DWORD)&h_inv_win_destroy);



    ///00474554  |.  05 36010000   ADD EAX,136
    //pBtmTrade_Barter = (LONG*)0x474555;
    ///00474561  |.  BB 35010000   MOV EBX,135
    pRgtPcTrade_Barter = (LONG*)0x474562;
    ///00474566  |.  BA 36010000   MOV EDX,136
    pTopPcTrade_Barter = (LONG*)0x474567;
    ///0047456D  |.  B8 F5000000   MOV EAX,0F5
    pLftPcTrade_Barter = (LONG*)0x47456E;

    ///004745E4  |> \BB 8A010000   MOV EBX,18A
    pRgtNpcTrade_Barter = (LONG*)0x4745E5;
    ///004745E9  |.  BA 36010000   MOV EDX,136
    pTopNpcTrade_Barter = (LONG*)0x4745EA;
    ///004745F0  |.  B8 4A010000   MOV EAX,14A
    pLftNpcTrade_Barter = (LONG*)0x4745F1;


    ///0047481F  |.  05 36010000   ADD EAX,136
   // pBtmInv_Barter = (LONG*)0x474820;
    ///0047482C  |.  BB 90000000   MOV EBX,90
    pRgtPcInv_Barter = (LONG*)0x47482D;
    ///00474831  |.  BA 36010000   MOV EDX,136
    pTopPcInv_Barter = (LONG*)0x474832;
    ///00474838  |.  B8 50000000   MOV EAX,50
    pLftPcInv_Barter = (LONG*)0x474839;

    ///004748AD  |> \BB 1B020000   MOV EBX,21B
    pRgtNpcInv_Barter = (LONG*)0x4748AE;
    ///004748B2  |.  BA 36010000   MOV EDX,136
    pTopNpcInv_Barter = (LONG*)0x4748B3;
    ///004748B9  |.  B8 DB010000   MOV EAX,1DB
    pLftNpcInv_Barter = (LONG*)0x4748BA;
    


    ///done///0046E4DD  |.  891D F4EE5A00 MOV DWORD PTR DS:[5AEEF4],EBX            ; invMouseMenuMaxX
    invMouseEdgeRight = (int*)0x5AEEF4;
    ///done///0046E4E3  |.  890D F0EE5A00 MOV DWORD PTR DS:[5AEEF0],ECX            ; invMouseMenuMaxY
    invMouseEdgeBottom = (int*)0x5AEEF0;
    */

}

//__________________________________
void Modifications_Inventory_MULTI() {

    //num list items visible (6 for inv)
    pInvListItemsVis = (LONG*)FixAddress(0x519054);

    inv_details = (INV_DETAILS*)FixAddress(0x519068);

    //fix for dropping stuff in bags
    FuncReplace32(0x47115B, 0x05987D, (DWORD)&h_inv_adjust_mouse_drop_position_in_item_list);

    //MOUSE DROP RECTS

    //INV PC ITEMS LIST
    FuncReplace32(0x471146, 0x0597EA, (DWORD)&h_inv_mouse_drop_pc_items);
    //INV LEFT HAND
    FuncReplace32(0x471271, 0x0596BF, (DWORD)&h_inv_mouse_drop_pc_left);
    //INV RIGHT HAND
    FuncReplace32(0x471302, 0x05962E, (DWORD)&h_inv_mouse_drop_pc_right);
    //INV ARMOR
    FuncReplace32(0x47138C, 0x0595A4, (DWORD)&h_inv_mouse_drop_pc_armor);
    //INV PC PORTRAIT
    FuncReplace32(0x471458, 0x0594D8, (DWORD)&h_inv_mouse_drop_pc_portrait);
    //LOOT TARGET
    FuncReplace32(0x47495A, 0x055FD6, (DWORD)&h_inv_mouse_drop_target_items);
    //LOOT PC
    FuncReplace32(0x474A24, 0x055F0C, (DWORD)&h_inv_mouse_drop_pc_items);
    //BARTER PC ITEM LIST
    FuncReplace32(0x475242, 0x0556EE, (DWORD)&h_inv_mouse_drop_pc_items);
    //BARTER PC ITEM LIST TABLE
    FuncReplace32(0x474F77, 0x0559B9, (DWORD)&h_inv_mouse_drop_pc_items_table);
    //BARTER TARGET ITEM LIST
    FuncReplace32(0x4752C3, 0x05566D, (DWORD)&h_inv_mouse_drop_target_items);
    //BARTER TARGET ITEM LIST TABLE
    FuncReplace32(0x474FFA, 0x055936, (DWORD)&h_inv_mouse_drop_target_items_table);

    ppObj_PC_for_INV = (OBJStruct**)FixAddress(0x519058);

    pp_Obj_stack_PC = (OBJStruct**)FixAddress(0x59E86C);

    pp_pud_PC = (PUD**)FixAddress(0x59E960);

    p_stack_offset_PC = (DWORD*)FixAddress(0x59E844);

    p_current_stack_PC = (DWORD*)FixAddress(0x59E96C);

    p_inv_num_items_visible = (LONG*)FixAddress(0x519054);

    p_inv_dropped_explosive = (LONG*)FixAddress(0x5190E0);

    ppfall_INV_Display_Text = (void**)FixAddress(0x59E938);

    pfall_INV_Display_Text_Dialog_NPC = (void*)FixAddress(0x445448);
    pfall_INV_Display_Text_IMonitor = (void*)FixAddress(0x43186C);

    fall_inv_adjust_pc_frm = (DWORD(*)())FixAddress(0x4716E8);

    ppObj_PC_Wearing = (OBJStruct**)FixAddress(0x59E954);
    ppObj_PC_LeftHand = (OBJStruct**)FixAddress(0x59E958);
    ppObj_PC_RightHand = (OBJStruct**)FixAddress(0x59E968);

    pfall_inv_hover_off = (void*)FixAddress(0x470D1C);
    pfall_inv_hover_on = (void*)FixAddress(0x470C2C);



    p_inv_scroll_up_buttId_pc = (LONG*)FixAddress(0x5190E4);
    p_inv_scroll_up_buttId_target = (LONG*)FixAddress(0x5190EC);
    p_inv_scroll_dn_buttId_pc = (LONG*)FixAddress(0x5190E8);
    p_inv_scroll_dn_buttId_target = (LONG*)FixAddress(0x5190F0);


    MemWrite8(0x46EC90, 0x53, 0xE9);
    FuncWrite32(0x46EC91, 0x57565251, (DWORD)&h_inventory_setup);

    FuncReplace32(0x46FCB1, 0x0667B3, (DWORD)&h_inventory_win_destroy);

    MemWrite8(0x46FDF4, 0x51, 0xE9);
    FuncWrite32(0x46FDF5, 0x83555756, (DWORD)&h_inventory_update_items_pc);

    MemWrite8(0x470650, 0x53, 0xE9);
    FuncWrite32(0x470651, 0x55575651, (DWORD)&h_inventory_update_portraits);

    MemWrite8(0x47036C, 0x56, 0xE9);
    FuncWrite32(0x47036D, 0xEC835557, (DWORD)&h_inventory_update_items_target);

    //set the max area to the screen as locking the mouse to the inv window is no longer necessary. - Mouse menus were previously drawn to the inv window itself, these are now seperate and are handled in Dx_Mouse.cpp
    FuncReplace32(0x470C88, 0xFFFDC314, (DWORD)&h_inventory_mouse_single_menu);
    FuncReplace32(0x473265, 0xFFFD9FAB, (DWORD)&h_inventory_mouse_multi_menu);



    //inv skip "items <= 1" check when redrawing item list - so that item in list is not drawn when picked up with mouse.
    MemWrite16(0x470ED6, 0x127E, 0x9090);
    //loot pc
    MemWrite16(0x47476F, 0x6C7E, 0x9090);
    //loot target
    MemWrite16(0x4747BB, 0x207E, 0x9090);
    //barter pc & target
    MemWrite16(0x474DF1, 0x2F7E, 0x9090);
    //barter tables pc & target
    MemWrite16(0x4750B4, 0x357E, 0x9090);

    //dont draw bg for armor and hands when picked with mouse
    MemWrite16(0x470EBA, 0x2E74, 0x9090);
    //pass picked armor or hand to DRAW_ITEM_LIST by way of their pressed button number.
    FuncReplace32(0x470EE1, 0xFFFFEF0F, (DWORD)&h_inventory_update_items_pc_picked);

    p_inv_pc_table_listPos = (LONG*)FixAddress(0x59E8A0);
    p_inv_target_table_listPos = (LONG*)FixAddress(0x59E89C);

    MemWrite8(0x475334, 0x56, 0xE9);
    FuncWrite32(0x475335, 0xEC815557, (DWORD)&h_inventory_update_items_tables);

    pMsgList_Inventory = (MSGList*)FixAddress(0x59E814);

    p_frmID_inv_pc = (DWORD*)FixAddress(0x59E95C);

    p_current_stack_Target = (DWORD*)FixAddress(0x59E948);

    p_stack_offset_Target = (DWORD*)FixAddress(0x59E7EC);

    pp_Obj_stack_Target = (OBJStruct**)FixAddress(0x59E81C);

    pp_pud_Target = (PUD**)FixAddress(0x59E978);


    MemWrite8(0x471D5C, 0x53, 0xE9);
    FuncWrite32(0x471D5D, 0x57565251, (DWORD)&h_inventory_update_console_pc_stats);

    MemWrite8(0x472EB8, 0x53, 0xE9);
    FuncWrite32(0x472EB9, 0x55575651, (DWORD)&h_inventory_update_console_item_details);

    p_winRef_MoveItems = (LONG*)FixAddress(0x59E894);

    MemWrite8(0x47688C, 0x51, 0xE9);
    FuncWrite32(0x47688D, 0x83555756, (DWORD)&h_inventory_move_multi_or_set_timer);
    MemWrite16(0x476891, 0x04EC, 0x9090);
}



//____________________________
void Modifications_Inventory() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Inventory_CH();
    else
        Modifications_Inventory_MULTI();

}