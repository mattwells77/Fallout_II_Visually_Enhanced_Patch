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
#include "graphics.h"
#include "version.h"

#include "Fall_General.h"
#include "Fall_File.h"
#include "Fall_Text.h"
#include "Fall_Msg.h"
#include "Fall_GameMap.h"

#include "Dx_General.h"
#include "Dx_Windows.h"
#include "Dx_Game.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;


LONG oldFont_LoadSave = 0;
BOOL* p_LoadSave_GameEventsDisabled = nullptr;
MSGList* pMsgList_Load_Save = nullptr;
BYTE** pp_buff_LoadSave_pic_mem = nullptr;
BYTE** pp_buff_LoadSave_pic_1 = nullptr;
BYTE** pp_buff_LoadSave_pic_2 = nullptr;

LONG* p_LoadSave_frm_list_num = nullptr;

LONG* p_LS_SlotState = nullptr;
LONG* p_LS_CurrentlySelectedSlot = nullptr;

char* p_LS_GeneralPurposeTextBuffer = nullptr;

void* p_LS_Sfall_ExtraSaveSlots_Get_Page_Number = nullptr;
LONG buttRef_LS_Sfall_ExtraSaveSlots_Edit_Page_Number = -1;
LONG ls_sfall_ExtraSaveSlots_LastPageOffset = 0;

int winRef_LS_Load_Picture = -1;
int winRef_LS_Save_Picture = -1;
int winRef_LS_Cover = -1;
int winRef_LS_Description = -1;
int winRef_LS_SaveSlots = -1;

bool is_loading_from_mainmenu = false;

Window_DX* p_LS_Quick_Save_Picture = nullptr;//quick-save window needs to be independent from the main load/save window as quick-save never loads the main load/save window.

enum class LS_TYPE : LONG {
    save = 0,
    quick_save = 1,
    load_in_game = 2,
    load_from_main = 3,
    quick_load = 4
};

struct LS_SAVE_DATA {//size 136
    char data_description[24];//start 0, size 24? made 29 //"FALLOUT SAVE FILE" text.
    char version[5];//start 29, size 32  //version eg "0x00010002" followed by 'R' character.
    char character_name[32];//start 29, size 32  //charater name
    char save_title[30];// [59] ;//start 61, size ? made 59 //save title
    
    char save_date_real[13];//real time not game time

    char month[2]; //start 104
    char day[2]; 
    char year[2];
    char space[2];
    char seconds[4];
    char map_level[2];
    char map_index[2];
    char map_file_name[16];//start 120, size ? made 16 //"[maptitle].sav" text

};
LS_SAVE_DATA* p_ls_SaveData = nullptr;


//____________________________________________
LONG LS_Sfall_ExtraSaveSlots_Get_Page_Number() {//ugly hack to grab the save slot page number when using sfall - ExtraSaveSlots "Who the hell wrote that crap!!!"
    LONG retVal = 0;
    if (!p_LS_Sfall_ExtraSaveSlots_Get_Page_Number)
        return retVal;
    __asm {
        push ebx
        mov eax, -1// in "sfall - ExtraSaveSlots - add_page_offset_hack3" eax is incremened by 1.
        call p_LS_Sfall_ExtraSaveSlots_Get_Page_Number//dword ptr ds : [ebx]
        pop ebx
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________
void __declspec(naked) h_ls_sfall_extra_save_slots_skip_p_key_edit(void) {//this is to avoid a crash when using "sfall - ExtraSaveSlots - SetPageNum" - don't have a work around as yet/////////////////////////////

    __asm {
        cmp eax, 'p'
        je skip_p
        cmp eax, 'P'
        jne endFunc
        skip_p :
        mov eax, -1
            endFunc :
            cmp eax, 501
            ret
    }
}


//__________________________________________
void Load_Arrange_Picture_Cover(BOOL toBack) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;
    
    Window_DX* subwin = nullptr;
    if (winRef_LS_Cover == -1) {
        //create sub windows for picture cover
        subwin = new Window_DX(340, 39, 275, 173, 0x00000000, pWin->winDx, &winRef_LS_Cover);

        //draw picture cover
        DWORD frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[2], 0, 0, 0);//239 LSCOVER.FRM    ; Load/Save game
        FRMCached* pfrm = new FRMCached(frmID);

        if (pfrm) {
            FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
            if (pFrame)
                subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
            delete pfrm;
            pfrm = nullptr;
            pFrame = nullptr;
        }
    }
    else
        subwin = pWin->winDx->GetSubWin(winRef_LS_Cover);

    if (subwin)
        subwin->ArrangeWin(toBack);
}


//_____________________________________________________
void __declspec(naked) h_load_arrange_cover_to_back(void) {

    __asm {
        pushad

        push TRUE
        call Load_Arrange_Picture_Cover
        add esp, 0x4

        popad
        ret
    }

}


//______________________________________________________
void __declspec(naked) h_load_arrange_cover_to_front(void) {

    __asm {
        pushad

        push FALSE
        call Load_Arrange_Picture_Cover
        add esp, 0x4

        popad
        ret
    }

}


//_________________________________________
void Save_ArrangeSavingPicture(BOOL toBack) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;
    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_LS_Save_Picture);

    if (subwin)
        subwin->ArrangeWin(toBack);
}


//______________________________________________________
void __declspec(naked) h_save_arrange_saving_to_back(void) {

    __asm {
        pushad

        push TRUE
        call Save_ArrangeSavingPicture
        add esp, 0x4

        popad
        ret
    }

}


//_______________________________________________________
void __declspec(naked) h_save_arrange_saving_to_front(void) {

    __asm {
        pushad

        push FALSE
        call Save_ArrangeSavingPicture
        add esp, 0x4

        popad
        ret
    }

}


//___________________________
void LS_Create_Save_Picture() {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_LS_Save_Picture == -1) 
        subwin = new Window_DX(366, 58, 224, 133, 0x00000000, pWin->winDx, &winRef_LS_Save_Picture);
    else
        subwin = pWin->winDx->GetSubWin(winRef_LS_Save_Picture);

    if (subwin)
        GameAreas_DrawToWindow(subwin, *pVIEW_HEXPOS, 640, 380);
}


//_________________________________________________________
void LS_Load_Picture_From_File(const char* p_save_dat_path) {
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;
    

    char picture_path[MAX_PATH];
    sprintf_s(picture_path, MAX_PATH, "%s", p_save_dat_path);

    char* pCH = picture_path;
    while (*pCH != '\0') {
        *pCH = (char)tolower(*pCH);
        pCH++;
    }

    char* pExt = strstr(picture_path, ".dat");
    if (!pExt)
        return;
    strcpy_s(pExt, 5, ".bmp");

    bool is32bitColour = false;
    UNLSTDframe* p_frame = nullptr;
    UNLSTDfrm* p_unlisted_frm = LoadBmpAsFrm(picture_path);
    if (p_unlisted_frm) {
        if (p_unlisted_frm->version == FRM_VER_32BIT)
            is32bitColour = true;
        p_frame = p_unlisted_frm->frames[0][0];
    }
    
    Window_DX* subwin = nullptr;

    if (winRef_LS_Load_Picture == -1) 
        subwin = new Window_DX(366, 58, 224, 133, 0xFF0000FF, pWin->winDx, &winRef_LS_Load_Picture);
    else
        subwin = pWin->winDx->GetSubWin(winRef_LS_Load_Picture);
    
    if (subwin && p_frame)
        subwin->Draw(p_frame->buff, p_unlisted_frm->pPal, is32bitColour, p_frame->width, p_frame->height, 0, 0, p_frame->width, p_frame->height, 0, 0);
    else if(subwin && pp_buff_LoadSave_pic_1)
        subwin->Draw(*pp_buff_LoadSave_pic_1, nullptr, false, 224, 133, 0, 0, 224, 133, 0, 0);

    if(p_unlisted_frm)
        delete p_unlisted_frm;
    p_unlisted_frm = nullptr;
    p_frame = nullptr;
}


//___________________________________________________
void __declspec(naked) h_load_picture_from_file(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax//fileStream
        call fall_fclose
        add esp, 0x4


        mov eax, p_LS_GeneralPurposeTextBuffer
        push eax//save.dat path
        call LS_Load_Picture_From_File
        add esp, 0x4

        xor eax, eax
        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//_______________________________________________________
void LS_Save_Picture_To_File(const char* p_save_dat_path) {

    char picture_path[MAX_PATH];
    sprintf_s(picture_path, MAX_PATH, "%s", p_save_dat_path);


    char* pCH = picture_path;
    while (*pCH != '\0') {
        *pCH = (char)tolower(*pCH);
        pCH++;
    }

    char* pExt = strstr(picture_path, ".dat");
    if (!pExt)
        return;
    strcpy_s(pExt, 5, ".bmp");

    Window_DX* subwin = nullptr;



    if (p_LS_Quick_Save_Picture) 
        subwin = p_LS_Quick_Save_Picture;
    else {
        WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
        if (!pWin || !pWin->winDx)
            return;
        subwin = pWin->winDx->GetSubWin(winRef_LS_Save_Picture);
    }
    if (!subwin)
        return;


    ID3D11Texture2D* pTex_Picture = subwin->GetTexture();
    if(SUCCEEDED (SaveTextureToFile(pTex_Picture, picture_path, true)))
        Fallout_Debug_Info("SaveGame Saved picture - %s\n", picture_path);

    if (p_LS_Quick_Save_Picture) {
        delete p_LS_Quick_Save_Picture;
        p_LS_Quick_Save_Picture = nullptr;
    }
}


//__________________________________________________
void __declspec(naked) h_save_picture_to_file(void) {

    __asm {
        
        
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        push edx

        push eax//save.dat path
        call LS_Save_Picture_To_File
        add esp, 0x4
        
        pop edx
        pop eax


        push edx// flags
        push eax//save.dat path
        call fall_fopen
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//____________________________________
void LoadSave_Destructor(LS_TYPE type) {
    
    fall_SetFont(oldFont_LoadSave);

    fall_MsgList_Destroy(pMsgList_Load_Save);
    if(*pp_buff_LoadSave_pic_mem)
        fall_Mem_Deallocate(*pp_buff_LoadSave_pic_mem);
    *pp_buff_LoadSave_pic_1 = nullptr;
    *pp_buff_LoadSave_pic_2 = nullptr;

    if(*pWinRef_LoadSave != -1)
        fall_Win_Destroy(*pWinRef_LoadSave);
    *pWinRef_LoadSave = -1;
    winRef_LS_Cover = -1;
    winRef_LS_Load_Picture = -1;
    winRef_LS_Save_Picture = -1;
    winRef_LS_Description = -1;
    winRef_LS_SaveSlots = -1;
    //winRef_LS_LoadBackGround = -1;

    buttRef_LS_Sfall_ExtraSaveSlots_Edit_Page_Number = -1;

    if (type == LS_TYPE::load_from_main) {
        if(*p_LoadSave_GameEventsDisabled == TRUE)
            fall_EnableGameEvents();
        *p_LoadSave_GameEventsDisabled = FALSE;
    }
    fall_Mouse_SetImage(1);
}

//convert the int passed from fallout to LS_TYPE
//_________________________________________
void LoadSave_Destructor_INT(LONG type_long) {
    LoadSave_Destructor(static_cast<LS_TYPE>(type_long));
}


//__________________________________________
void __declspec(naked) h_ls_destructor(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call LoadSave_Destructor_INT
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop ecx

        xor eax, eax
        pop edx
        pop ebx
        ret
    }
}


//______________________________
void LS_Draw_8bit_Save_Picture() {
    if (!*pp_buff_LoadSave_pic_mem || !*pp_buff_LoadSave_pic_2)
        return;
    LONG oldFont = fall_GetFont();
    fall_SetFont(3);
    FONT_FUNC_STRUCT* p_font = GetCurrentFont();
    if (p_font) {
        char verText_HiRes[MAX_PATH];
        sprintf_s((char*)verText_HiRes, MAX_PATH, "Saved using\nHI-RES-V.E. %s\n\n If your reading this than the HI-RES-V.E patch is disabled. Saving without the HI-RES-V.E patch enabled will result in the altering or loss of some features, eg. coloured lighting, fog of war etc.", VER_FILE_VERSION_STR);
        RECT rc_text{ 0,0,224,133 };
        p_font->PrintText_Formated(*pp_buff_LoadSave_pic_2, verText_HiRes, 224, 133, &rc_text, FLG_TEXT_CENTRED);
        BYTE* buff = *pp_buff_LoadSave_pic_2;
        for (int i = 0; i < 224 * 133; i++) {
            if (buff[i] != 0)
                buff[i] = 0xD7;//green
            else
                buff[i] = 0xCF;//black-ish
        }
    }
    fall_SetFont(oldFont);
}


//_________________________________
LONG LS_Create_Quick_Save_Picture() {
    *pp_buff_LoadSave_pic_mem = (BYTE*)fall_Mem_Allocate(224 * 133);//61632
    memset(*pp_buff_LoadSave_pic_mem, 0, 224 * 133);
    if (*pp_buff_LoadSave_pic_mem == nullptr) {
        LoadSave_Destructor(LS_TYPE::quick_save);
        return -1;
    }
    *pp_buff_LoadSave_pic_2 = *pp_buff_LoadSave_pic_mem;

    LS_Draw_8bit_Save_Picture();

    if (!p_LS_Quick_Save_Picture)
        p_LS_Quick_Save_Picture = new Window_DX(366, 58, 224, 133, 0x00000000, nullptr, nullptr);

    if (p_LS_Quick_Save_Picture)
        GameAreas_DrawToWindow(p_LS_Quick_Save_Picture, *pVIEW_HEXPOS, 640, 380);

    return 1;
}


//_________________________________________________________
void __declspec(naked) h_ls_create_quick_save_picture(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp


        call LS_Create_Quick_Save_Picture

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
LONG LoadSave_Setup(LONG type_long) {
    LS_TYPE type = static_cast<LS_TYPE>(type_long);
    *p_LoadSave_GameEventsDisabled = FALSE;
    oldFont_LoadSave = fall_GetFont();
    fall_SetFont(103);
    pMsgList_Load_Save->MsgNodes = 0;
    pMsgList_Load_Save->numMsgs = 0;

    if (fall_LoadMsgList(pMsgList_Load_Save, "game\\LSGAME.MSG") != TRUE) {
        
        LoadSave_Destructor(type);
        return -1;
    }

    *pp_buff_LoadSave_pic_mem = (BYTE*)fall_Mem_Allocate(224 * 133 * 2 + 2048);//61632
    memset(*pp_buff_LoadSave_pic_mem, 0, 224 * 133 * 2 + 2048);
    if (*pp_buff_LoadSave_pic_mem == nullptr) {
        LoadSave_Destructor(type);
        return -1;
    }
    *pp_buff_LoadSave_pic_1 = *pp_buff_LoadSave_pic_mem;
    *pp_buff_LoadSave_pic_2 = *pp_buff_LoadSave_pic_mem + 224 * 133;

    
    if (type == LS_TYPE::load_from_main) {
        is_loading_from_mainmenu = true;
        fall_DisableGameEvents();
        *p_LoadSave_GameEventsDisabled = TRUE;
    }
    else
        is_loading_from_mainmenu = false;

    fall_Mouse_SetImage(1);

    if (type == LS_TYPE::save || type == LS_TYPE::quick_save)
        LS_Draw_8bit_Save_Picture();
    
    
    if (type == LS_TYPE::load_from_main)
        *pWinRef_LoadSave = Win_Create_CenteredOnScreen(640, 480, 0x100, FLG_WinExclusive | FLG_WinToFront);
    else
        *pWinRef_LoadSave = Win_Create_CenteredOnGame(640, 480, 0x100, FLG_WinExclusive | FLG_WinToFront);

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx) {
        LoadSave_Destructor(type);
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    
    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    //p_LoadSave_frm_list_num[0] //237 LSGAME.FRM     ; Load/Save game
    //p_LoadSave_frm_list_num[1] //238 LSGBOX.FRM     ; Load/Save game
    //p_LoadSave_frm_list_num[2] //239 LSCOVER.FRM    ; Load/Save game
    //p_LoadSave_frm_list_num[3] //9 lilreddn.frm   ; little red button down
    //p_LoadSave_frm_list_num[4] //8 lilredup.frm   ; little red button up
    //p_LoadSave_frm_list_num[5] //181 DNARWOFF.FRM   ; Character editor
    //p_LoadSave_frm_list_num[6] //182 DNARWON.FRM    ; Character editor
    //p_LoadSave_frm_list_num[7] //199 UPARWOFF.FRM   ; Character editor
    //p_LoadSave_frm_list_num[8] //200 UPARWON.FRM    ; Character editor
 
    //draw load/save background.
    frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[0], 0, 0, 0); //237 LSGAME.FRM     ; Load/Save game
    
    pfrm = new FRMCached(frmID);
    if (pfrm) {
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame)
            pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }


    //grab map pic for save picture
    if (type == LS_TYPE::save || type == LS_TYPE::quick_save)
        LS_Create_Save_Picture();

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_DarkYellow & 0x000000FF);


    LONG msg_num = 0;
    switch (type) {
    case LS_TYPE::save:
        msg_num = 102;
        break;
    case LS_TYPE::quick_save:
        msg_num = 103;
        break;
    case LS_TYPE::load_in_game:
    case LS_TYPE::load_from_main:
        msg_num = 100;
        break;
    case LS_TYPE::quick_load:
        msg_num = 101;
        break;
    default:
        break;
    };
    
    char* pText = nullptr;

    //title text
    pText = GetMsg(pMsgList_Load_Save, msg_num, 2);
    pWin->winDx->Draw_Text(pText, 48, 27, colour, 0, TextEffects::none);

    //[done] text
    pText = GetMsg(pMsgList_Load_Save, 104, 2);
    pWin->winDx->Draw_Text(pText, 410, 348, colour, 0, TextEffects::none);
    //[cancel] text
    pText = GetMsg(pMsgList_Load_Save, 105, 2);
    pWin->winDx->Draw_Text(pText, 515, 348, colour, 0, TextEffects::none);

    LONG butt_ref = -1;

    frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[3], 0, 0, 0);
    pfrm = new FRMCached(frmID);
    if (pfrm) {
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            //[done] button
            butt_ref = CreateButtonX(*pWinRef_LoadSave, 391, 349, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, 500, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[4], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if(butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
            //[cancel] button
            butt_ref = CreateButtonX(*pWinRef_LoadSave, 495, 349, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, 501, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[4], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if (butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }


    DWORD up_arrow_height = 0;//needed to get down arrow y position.
    frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[8], 0, 0, 0);
    pfrm = new FRMCached(frmID);
    if (pfrm) {
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            //[up arrow] button
            up_arrow_height = pFrame->GetHeight();
            butt_ref = CreateButtonX(*pWinRef_LoadSave, 35, 58, pFrame->GetWidth(), up_arrow_height, -1, 505, 506, 505, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[7], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if (butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }
    
    frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[6], 0, 0, 0);
    pfrm = new FRMCached(frmID);
    if (pfrm) {
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            //[down arrow] button
            butt_ref = CreateButtonX(*pWinRef_LoadSave, 35, 58 + up_arrow_height, pFrame->GetWidth(), pFrame->GetHeight(), -1, 503, 504, 503, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[5], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if (butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }

    //big button for selection list
    butt_ref = CreateButtonX(*pWinRef_LoadSave, 55, 87, 230, 353, -1, -1, -1, 502, 0, 0, 0, FLG_ButtTrans);


    fall_SetFont(101);

    //
    if (SfallReadInt(L"Misc", L"ExtraSaveSlots", 0) != 0) {

        FONT_FUNC_STRUCT* font = GetCurrentFont();

        LONG txtWidth = 0;
        LONG txtHeight = font->GetFontHeight();
        
        char button_text[12];
        ButtonStruct_DX* p_butt = nullptr;
        

        DWORD colour = 0xFFFFFFFF;
        if (color_pal) 
            colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
        XMFLOAT4 fcolour_green(((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f);
        if (color_pal)
            colour = color_pal->GetColour(*pPalColour_ConsoleYellow & 0x000000FF);
        XMFLOAT4 fcolour_yellow(((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f);

        for (int num = 0; num < 5; num++) {

            switch (num) {
            case 0:
                sprintf_s(button_text, "<");
                txtWidth = font->GetTextWidth(button_text);
                // left button -10                   | X | Y | W | H |HOn |HOff |BDown |BUp |PicUp |PicDown |? |ButType
                butt_ref = CreateButtonX_Overlay(*pWinRef_LoadSave, 100, 60, 24, 20, -1, 0x500, 0x54B, 0x14B, 0, 0, 0, FLG_ButtTrans);
                break;
            case 1:
                sprintf_s(button_text, "<<");
                txtWidth = font->GetTextWidth(button_text);
                // left button -100
                butt_ref = CreateButtonX_Overlay(*pWinRef_LoadSave, 68, 60, 24, 20, -1, 0x500, 0x549, 0x149, 0, 0, 0, FLG_ButtTrans);
                break;
            case 2:
                sprintf_s(button_text, ">");
                txtWidth = font->GetTextWidth(button_text);
                // right button +10
                butt_ref = CreateButtonX_Overlay(*pWinRef_LoadSave, 216, 60, 24, 20, -1, 0x500, 0x54D, 0x14D, 0, 0, 0, FLG_ButtTrans);
                break;
            case 3:
                sprintf_s(button_text, ">>");
                txtWidth = font->GetTextWidth(button_text);
                // right button +100
                butt_ref = CreateButtonX_Overlay(*pWinRef_LoadSave, 248, 60, 24, 20, -1, 0x500, 0x551, 0x151, 0, 0, 0, FLG_ButtTrans);
                break;
            case 4:
                ls_sfall_ExtraSaveSlots_LastPageOffset = LS_Sfall_ExtraSaveSlots_Get_Page_Number();
                sprintf_s(button_text, 12, "[ %d ]", ls_sfall_ExtraSaveSlots_LastPageOffset / 10);
                txtWidth = font->GetTextWidth(button_text);
                // Set Number button
                butt_ref = CreateButtonX_Overlay(*pWinRef_LoadSave, 140, 60, 60, 20, -1, -1, 'p', -1, 0, 0, 0, FLG_ButtTrans);
                buttRef_LS_Sfall_ExtraSaveSlots_Edit_Page_Number = butt_ref;
                break;
            default:
                butt_ref = -1;
                break;
            }

            if (butt_ref != -1) {
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
                p_butt = (ButtonStruct_DX*)fall_Button_Get(butt_ref, nullptr);
                if (p_butt) {
                    p_butt->buttDx->Overlay_CreateTexture(false, false, true);
                    p_butt->buttDx->OverLay_SetAsColouredButton(FLG_BUTT_UP| FLG_BUTT_DN);
                    p_butt->buttDx->OverLay_SetColours(&fcolour_green, &fcolour_yellow, nullptr, nullptr);
                    p_butt->buttDx->Clear_Staging();


                    DWORD width = p_butt->rect.right - p_butt->rect.left;
                    DWORD height = p_butt->rect.bottom - p_butt->rect.top;
                    int text_x = (width - txtWidth) / 2;
                    int text_y = (height - txtHeight) / 2;
                    if (txtWidth > 0 || txtHeight > 0) {
                        if (text_y + txtHeight < (LONG)height) {
                            if (text_x + txtWidth > (LONG)width)
                                txtWidth = (LONG)width - text_x;
                            BYTE* tBuff=nullptr;
                            UINT pitchBytes = 0;
                            LONG char_count = 0;
                            if (SUCCEEDED(p_butt->buttDx->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
                                memset(tBuff, 0, pitchBytes * height);
                                char_count = font->PrintText(tBuff + text_x + pitchBytes * text_y, button_text, pitchBytes, txtWidth);
                                p_butt->buttDx->Unlock(nullptr);
                            }
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}


//_____________________________________
void __declspec(naked) h_ls_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call LoadSave_Setup
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


//___________________________________________
void LS_Update_SaveSlots(LONG save_load_flag) {//is either 0 for save or 2 for load.

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_LS_SaveSlots == -1) {
        subwin = new Window_DX(55, 87, 230, 353, 0x00000000, pWin->winDx, &winRef_LS_SaveSlots);
    }
    else
        subwin = pWin->winDx->GetSubWin(winRef_LS_SaveSlots);

    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    char* pText = nullptr;
    LONG yPos = 0;

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG textHeight = (LONG)font->GetFontHeight();

    char msg[512];

    BOOL isLoading = FALSE;
    if (save_load_flag != 0)
        isLoading = TRUE;

    DWORD colourGreen = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colourGreen = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
    DWORD colourYellow = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colourYellow = color_pal->GetColour(*pPalColour_ConsoleYellow & 0x000000FF);

    LONG ls_sfall_ExtraSaveSlots_PageOffset = LS_Sfall_ExtraSaveSlots_Get_Page_Number();

    if (buttRef_LS_Sfall_ExtraSaveSlots_Edit_Page_Number != -1 && ls_sfall_ExtraSaveSlots_LastPageOffset != ls_sfall_ExtraSaveSlots_PageOffset) {
        ls_sfall_ExtraSaveSlots_LastPageOffset = ls_sfall_ExtraSaveSlots_PageOffset;
        ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_LS_Sfall_ExtraSaveSlots_Edit_Page_Number, nullptr);
        if (p_butt) {
            p_butt->buttDx->Clear_Staging();
            LONG txtWidth = 0;
            char button_text[32];
            sprintf_s(button_text, 32, "[ %d ]", ls_sfall_ExtraSaveSlots_PageOffset / 10);
            txtWidth = font->GetTextWidth(button_text);
            DWORD width = p_butt->rect.right - p_butt->rect.left;
            DWORD height = p_butt->rect.bottom - p_butt->rect.top;
            int text_x = (width - txtWidth) / 2;
            int text_y = (height - textHeight) / 2;
            if (txtWidth > 0 || textHeight > 0) {
                if (text_y + textHeight < (LONG)height) {
                    if (text_x + txtWidth > (LONG)width)
                        txtWidth = (LONG)width - text_x;
                    BYTE* tBuff = nullptr;
                    UINT pitchBytes = 0;
                    LONG char_count = 0;
                    if (SUCCEEDED(p_butt->buttDx->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
                        memset(tBuff, 0, pitchBytes * height);
                        char_count = font->PrintText(tBuff + text_x + pitchBytes * text_y, button_text, pitchBytes, txtWidth);
                        p_butt->buttDx->Unlock(nullptr);
                    }
                }
            }
        }
    }

    DWORD* p_colour = nullptr;

    for (int slot = 0; slot < 10; slot++) {

        if (slot == *p_LS_CurrentlySelectedSlot)
            p_colour = &colourYellow;
        else
            p_colour = &colourGreen;
        pText = GetMsg(pMsgList_Load_Save, 109 + isLoading, 2);
        sprintf_s(msg, "[   %s %.2d:   ]", pText, slot + 1 + ls_sfall_ExtraSaveSlots_PageOffset);
        subwin->Draw_Text(msg, 0, yPos, *p_colour, 0, TextEffects::none);
        yPos += textHeight;

        LONG slotState = p_LS_SlotState[slot];
        switch (slotState) {
        case 1: //valid
            pText = p_ls_SaveData[slot].save_title;
            break;
        case 0://empty
            pText = GetMsg(pMsgList_Load_Save, 111, 2);

            break;
        case 2://corrupt save file
            pText = GetMsg(pMsgList_Load_Save, 112, 2);

            break;
        case 3://old version
            pText = GetMsg(pMsgList_Load_Save, 113, 2);
            break;

        default:
            pText = nullptr;
            break;
        }
        subwin->Draw_Text(pText, 0, yPos, *p_colour, 0, TextEffects::none);
        yPos += textHeight;
        yPos += textHeight;
        yPos += 4;
    }
}


//_________________________________________________
void __declspec(naked) h_ls_update_save_slots(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call LS_Update_SaveSlots
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


//________________________________________________
void LS_Update_DescriptionText(LONG selected_slot) {
    //Fallout_Debug_Ihfo("LS_Update_DescriptionText");
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;

    if (winRef_LS_Description == -1) {
        subwin = new Window_DX(396, 254, 164, 60, 0x00000000, pWin->winDx, &winRef_LS_Description);
    }
    else
        subwin = pWin->winDx->GetSubWin(winRef_LS_Description);

    if (!subwin)
        return;

    subwin->ClearRenderTarget(nullptr);

    char* pText = nullptr;
    LONG xPos = 0;
    LONG yPos = 0;

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG textHeight = (LONG)font->GetFontHeight();

    char msg[512];


    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    LONG slotState = p_LS_SlotState[selected_slot];
    switch (slotState) {
    case 1: //valid
        xPos = 0;
        yPos = 0;
        {
            //print character name
            subwin->Draw_Text(p_ls_SaveData[selected_slot].character_name, xPos, yPos, colour, 0, TextEffects::none);

            //print game date and time
            DWORD minuets = *(DWORD*)p_ls_SaveData[selected_slot].seconds / 600;
            DWORD hours = minuets / 60;
            minuets = minuets - hours * 60;
            DWORD days = hours / 24;
            hours = hours - days * 24;
            DWORD time = hours * 100 + minuets;
            sprintf_s(msg, "%.2d %s %.4d   %.4d", *(WORD*)p_ls_SaveData[selected_slot].day, GetMsg(pMsgList_Load_Save, *(WORD*)p_ls_SaveData[selected_slot].month + 116, 2), *(WORD*)p_ls_SaveData[selected_slot].year, time);
            xPos += 1;
            yPos = 2 + textHeight;
            subwin->Draw_Text(msg, xPos, yPos, colour, 0, TextEffects::none);

            //print the location
            sprintf_s(msg, "%s %s", fall_AUTOMAP_GetShortCityName(*(WORD*)(p_ls_SaveData[selected_slot].map_index)), fall_AUTOMAP_GetCityElevationName(*(WORD*)(p_ls_SaveData[selected_slot].map_index), *(WORD*)p_ls_SaveData[selected_slot].map_level));
            xPos += 2;
            yPos += 3 + textHeight;
            RECT rc_margins{ xPos, yPos, 164 - xPos, 60 };
            subwin->Draw_Text_Formated(msg, &rc_margins, colour, 0, FLG_TEXT_SLOPED_L | FLG_TEXT_SLOPED_R | (1 << 24), TextEffects::none);
            return;
        }
        break;
    case 0://empty
        pText = GetMsg(pMsgList_Load_Save, 114, 2);
        xPos = 8;
        yPos = 8;
        break;
    case 2://error
        pText = GetMsg(pMsgList_Load_Save, 115, 2);
        xPos = 8;
        yPos = 8;
        break;
    case 3://old version
        pText = GetMsg(pMsgList_Load_Save, 116, 2);
        xPos = 4;
        yPos = 8;
        break;

    default:
        return;
        break;
    }
    subwin->Draw_Text(pText, xPos, yPos, colour, 0, TextEffects::none);
}


//_______________________________________________________
void __declspec(naked) h_ls_update_description_text(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call LS_Update_DescriptionText
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


//___________________________________________
LONG Save_Description_Box(LONG save_slot_num) {

    WinStructDx* pWin_LS = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (!pWin_LS || !pWin_LS->winDx)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    frmID = fall_GetFrmID(ART_INTRFACE, 238, 0, 0, 0);//238 LSGBOX.FRM     ; Load/Save game

    pfrm = new FRMCached(frmID);

    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        delete pfrm;
        pfrm = nullptr;
        return -1;
    }


    LONG winRef = fall_Win_Create(pWin_LS->rect.left + 169, pWin_LS->rect.top + 116, pFrame->GetWidth(), pFrame->GetHeight(), 0x100, FLG_WinToFront | FLG_WinExclusive);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (!pWin || !pWin->winDx) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);

    pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_DarkYellow & 0x000000FF);


    char* pText = nullptr;


    fall_SetFont(103);
    //done text
    pText = GetMsg(pMsgList_Load_Save, 104, 2);
    pWin->winDx->Draw_Text(pText, 56, 57, colour, 0, TextEffects::none);
    //cancel text
    pText = GetMsg(pMsgList_Load_Save, 105, 2);
    pWin->winDx->Draw_Text(pText, 181, 57, colour, 0, TextEffects::none);
    //description text
    pText = GetMsg(pMsgList_Load_Save, 130, 2);
    FONT_FUNC_STRUCT* p_font = GetCurrentFont();
    pWin->winDx->Draw_Text(pText, (pWin->winDx->GetWidth() - p_font->GetTextWidth(pText)) / 2, 7, colour, 0, TextEffects::none);

    fall_SetFont(101);
    p_font = GetCurrentFont();

    LONG butt_ref = -1;

    frmID = fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[3], 0, 0, 0);
    pfrm = new FRMCached(frmID);
    if (pfrm) {
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            //[done] button
            butt_ref = CreateButtonX(winRef, 34, 58, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, 507, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[4], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if (butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
            //[cancel] button
            butt_ref = CreateButtonX(winRef, 160, 58, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, 508, fall_GetFrmID(ART_INTRFACE, p_LoadSave_frm_list_num[4], 0, 0, 0), frmID, 0, FLG_ButtTrans);
            if (butt_ref != -1)
                SetButtonSoundsX(butt_ref, &button_sound_Dn_1, &button_sound_Up_1);
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }

    LONG slotState = p_LS_SlotState[save_slot_num];
    char msg[30];
    msg[0] = '\0';
    if (slotState == 1) //slot currently occupied
        strncpy_s(msg, p_ls_SaveData[save_slot_num].save_title, 30);

    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

    Window_DX* subwin = new Window_DX(24, 35, 244, 12, 0x00000000, pWin->winDx, nullptr);
    subwin->ClearRenderTarget(nullptr);
    subwin->Draw_Text(msg, 0, 0, colour, 0, TextEffects::none);
    float cursor_offset = 24.0f + p_font->GetHorizontalGapWidth() * 2;

    DWORD textWidth = p_font->GetTextWidth(msg);

    Window_DX* subwin_cursor = new Window_DX(cursor_offset + textWidth, 35.0f, p_font->GetTextWidth(" "), p_font->GetFontHeight(), colour, pWin->winDx, nullptr);
    subwin_cursor->Clear();

    ULONGLONG oldTick = GetTickCount64();
    ULONGLONG newTick = oldTick;
    ULONGLONG cursorTime = 200;

    BOOL flashCursor = FALSE;
    LONG input = -1;
    LONG exit_code = -1;
    size_t lastChar = strlen(msg);

    bool firstKey = true;

    while (exit_code == -1) {
        input = fall_Get_Input();
        if (input == 0xD || input == 507)
            exit_code = 1;
        else if (input == 0x1B || input == 508)
            exit_code = 0;
        else if (input == '\b' || input == 339) {
            if (firstKey)//allow for the whole line to be erased if this is the first key press
                lastChar = 1;

            if (lastChar > 0) {
                lastChar--;
                msg[lastChar] = '\0';
                textWidth = p_font->GetTextWidth(msg);
                subwin->ClearRenderTarget(nullptr);
                subwin->Draw_Text(msg, 0, 0, colour, 0, TextEffects::none);
                subwin_cursor->SetPosition(cursor_offset + textWidth, 35.0f);
            }
        }
        else if (input >= 0x20 && input <= 0x7A) {
            if (lastChar < 30 - 1) {
                msg[lastChar] = (char)(input & 0x000000FF);
                lastChar++;
                msg[lastChar] = '\0';
                textWidth = p_font->GetTextWidth(msg);
                subwin->ClearRenderTarget(nullptr);
                subwin->Draw_Text(msg, 0, 0, colour, 0, TextEffects::none);
                subwin_cursor->SetPosition(cursor_offset + textWidth, 35.0f);
            }
        }
        if (input != -1 && firstKey) {
            firstKey = false;
        }

        newTick = GetTickCount64();
        if (newTick > oldTick + cursorTime) {
            oldTick = newTick;
            flashCursor = 1 - flashCursor;
            subwin_cursor->ArrangeWin(flashCursor);
        }
    }

    if (exit_code == 1) {
        if (msg[0] == '\0')
            exit_code = -1;
        else {
            strncpy_s(p_ls_SaveData[save_slot_num].save_title, msg, 30);
            p_ls_SaveData[save_slot_num].save_title[29] = '\0';
        }
    }
    fall_Win_Destroy(winRef);

    //-1 failed, 0 escaped, 1 ready to save
    return exit_code;
}


//_____________________________________________________
void __declspec(naked) h_ls_save_description_box(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Save_Description_Box
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


//_______________________________________________
void Load_From_MainMenu_Fade_Effect(LONG type_long) {
    LS_TYPE type = static_cast<LS_TYPE>(type_long);
    if (type == LS_TYPE::load_from_main)
        FadeToPalette(pLOADED_PAL);
}


//________________________________________________________
void __declspec(naked) h_load_from_main_menu_fade_effect() {

    __asm {

        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        mov eax, dword ptr ss : [esp + 0x21C + 0x1C]
        push eax
        call Load_From_MainMenu_Fade_Effect
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


//________________________________________________
void LS_Adjust_MousePosition(LONG* p_x, LONG* p_y) {
    fall_Mouse_GetPos(p_x, p_y);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_LoadSave);
    if (pWin) {
        *p_x -= pWin->rect.left;
        *p_y -= pWin->rect.top;
    }
}


//_________________________________________________
void __declspec(naked) h_ls_adjust_mouse_position() {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call LS_Adjust_MousePosition
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________________________________
void __declspec(naked) h_ls_check_double_click_saving() {//a different way to check for double clicking. So the sluggishness inducing delay in the original code can be removed.
//0047BE74 | .  8B8C24 28020000 | MOV ECX, DWORD PTR SS : [ESP + 228]
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax //temporarily store current slot num
        call IsMouseDoubleClick
        test eax, eax
        pop eax //restore current slot num

        je no_double_click
        //if double click is true make the stored "esp+0x228" first click slot number match the current slot number. 
        mov dword ptr ss : [esp + 0x228 + 0x1C] , eax
        no_double_click :


        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        //restore original code
        mov ecx, dword ptr ss : [esp + 0x228 + 0x4]
        ret
    }

}


//______________________________________________________
void __declspec(naked) h_ls_check_double_click_loading() {//a different way to check for double clicking. So the sluggishness inducing delay in the original code can be removed.
    //0047CC41 | > \8B9424 2C020000 | MOV EDX, DWORD PTR SS : [ESP + 22C]
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        
        push eax //temporarily store current slot num
        call IsMouseDoubleClick
        test eax, eax
        pop eax //restore current slot num

        je no_double_click
        //if double click is true make the stored "esp+0x22C" first click slot number match the current slot number. 
        mov dword ptr ss : [esp + 0x22C + 0x1C] , eax
        no_double_click :


        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        //restore original code
        mov edx, dword ptr ss : [esp + 0x22C+0x4]
        ret
    }

}


//______________________________
void Modifications_LoadSave_CH() {

    //To-Do all Modifications_LoadSave_CH

    //SAVEGAME NAME BOX POSITION
    //0047E2A4  |.  E8 14DE0500   CALL 004DC0BD
  //  FuncWrite32(0x47E2A5, 0x05DE14,  (DWORD)&h_save_name_box_win);

    //QUICK SAVE GAME PIC WIDTH & HEIGHT
    //0047BC27  |.  E8 FECF0500   CALL 004D8C2A
   // FuncWrite32(0x47BC28, 0x05CFFE,  (DWORD)&h_save_pic_stretch);

    //QUICK LOAD BACKGROUND SCREEN BLACKOUT WIDTH & HEIGHT
    //0047BCF2  |.  E8 C6030600   CALL 004DC0BD
   /// FuncWrite32(0x47BCF3, 0x0603C6,  (DWORD)&h_fade_win_setup);

    //QUICK LOAD BACKGROUND FILL BLACK
    //0047BD1B  |.  E8 12D40500   CALL 004D9132
   /// FuncWrite32(0x47BD1C, 0x05D412,  (DWORD)&h_colour_fill);

    //LOAD SAVE GAME PIC LARGE
    //0047CA2F  |.  E8 F6C10500   CALL 004D8C2A
  //  FuncWrite32(0x47CA30, 0x05C1F6,  (DWORD)&h_save_pic_stretch);

    //LOAD_SCRN - fade OUT BLACK
    //0047FF8D  |.  E8 2BC10500   |CALL 004DC0BD
    ///FuncWrite32(0x47FF8E, 0x05C12B,  (DWORD)&h_fade_win_setup);

    //FADE TO MAP AT START
    //00480211  |.  E8 A7BE0500   CALL 004DC0BD
   /// FuncWrite32(0x480212, 0x05BEA7,  (DWORD)&h_fade_win_setup);

    //0047CB29  |. E8 8FF50500    CALL Fallout2.004DC0BD
 //   FuncWrite32(0x47CB2A, 0x05F58F,  (DWORD)&h_load_save_scrn);
    /*
    MemWrite8(0x47C227, 0x83, 0xE8);
    FuncWrite32(0x47C228, 0xD0894FEA, (DWORD)&h_load_save_mouse);

    MemWrite8(0x47B455, 0x83, 0xE8);
    FuncWrite32(0x47B456, 0xD0894FEA, (DWORD)&h_load_save_mouse);*/
}


//_________________________________
void Modifications_LoadSave_MULTI() {

    //adjust mouse relative to window position - save
    FuncReplace32(0x47BE3E, 0x0004EB9A, (DWORD)&h_ls_adjust_mouse_position);

    //adjust mouse relative to window position - load
    FuncReplace32(0x47CC10, 0x0004DDC8, (DWORD)&h_ls_adjust_mouse_position);

    p_LoadSave_GameEventsDisabled = (BOOL*)FixAddress(0x5193C0);

    pMsgList_Load_Save = (MSGList*)FixAddress(0x613D28);

    pp_buff_LoadSave_pic_mem = (BYTE**)FixAddress(0x6142EC);

    pp_buff_LoadSave_pic_1 = (BYTE**)FixAddress(0x6142A8);
    pp_buff_LoadSave_pic_2 = (BYTE**)FixAddress(0x6142AC);

    p_LoadSave_frm_list_num = (LONG*)FixAddress(0x47B7C0);


    //save save.bmp
    FuncReplace32(0x47DA15, 0x000484AF, (DWORD)&h_save_picture_to_file);
    //load save.bmp
    FuncReplace32(0x47ED50, 0x00047160, (DWORD)&h_load_picture_from_file);


    //Saving-Fixes------------------------------------------------------------------------------------------------------------------------------------------------------

    //dont need to draw window
    MemWrite8(0x47BB60, 0xE8, 0x90);
    MemWrite32(0x47BB61, 0x05B3F7, 0x90909090);

    MemWrite8(0x47BCF1, 0xE8, 0x90);
    MemWrite32(0x47BCF2, 0x05B266, 0x90909090);

    MemWrite8(0x47C0C3, 0xE8, 0x90);
    MemWrite32(0x47C0C4, 0x05AE94, 0x90909090);

    MemWrite8(0x47C1CA, 0xE8, 0x90);
    MemWrite32(0x47C1CB, 0x05AD8D, 0x90909090);

    MemWrite8(0x47C308, 0xE8, 0x90);
    MemWrite32(0x47C309, 0x05AC4F, 0x90909090);

    MemWrite8(0x47C496, 0xE8, 0x90);
    MemWrite32(0x47C497, 0x05AAC1, 0x90909090);

    //set saving pic
    MemWrite8(0x47BC77, 0xA1, 0xE8);
    FuncWrite32(0x47BC78, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_front);
    MemWrite8(0x47BC7C, 0x68, 0xEB);
    MemWrite32(0x47BC7D, 0x00000280, 0x90909058);

    //set loading pic
    MemWrite8(0x47BCA8, 0xA1, 0xE8);
    FuncWrite32(0x47BCA9, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_back);
    MemWrite8(0x47BCAD, 0x68, 0xEB);
    MemWrite32(0x47BCAE, 0x00000280, 0x90909027);

    //set saving pic
    MemWrite8(0x47C04E, 0xA1, 0xE8);
    FuncWrite32(0x47C04F, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_front);
    MemWrite8(0x47C053, 0x68, 0xEB);
    MemWrite32(0x47C054, 0x00000280, 0x90909058);

    //set loading pic
    MemWrite8(0x47C07F, 0xA1, 0xE8);
    FuncWrite32(0x47C080, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_back);
    MemWrite8(0x47C084, 0x68, 0xEB);
    MemWrite32(0x47C085, 0x00000280, 0x90909027);

    //set saving pic
    MemWrite8(0x47C155, 0xA1, 0xE8);
    FuncWrite32(0x47C156, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_front);
    MemWrite8(0x47C15A, 0x68, 0xEB);
    MemWrite32(0x47C15B, 0x00000280, 0x90909058);

    //set loading pic
    MemWrite8(0x47C186, 0xA1, 0xE8);
    FuncWrite32(0x47C187, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_back);
    MemWrite8(0x47C18B, 0x68, 0xEB);
    MemWrite32(0x47C18C, 0x00000280, 0x90909027);

    //set saving pic
    MemWrite8(0x47C420, 0xA1, 0xE8);
    FuncWrite32(0x47C421, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_front);
    MemWrite8(0x47C425, 0x68, 0xEB);
    MemWrite32(0x47C426, 0x00000280, 0x90909059);

    //set loading pic
    MemWrite8(0x47C452, 0xA1, 0xE8);
    FuncWrite32(0x47C453, FixAddress(0x614700), (DWORD)&h_save_arrange_saving_to_back);
    MemWrite8(0x47C457, 0x68, 0xEB);
    MemWrite32(0x47C458, 0x00000280, 0x90909027);

    //removing the sluggishness from the original code by using a different method to check for double clicking
    MemWrite16(0x47BE74, 0x8C8B, 0x9090);
    MemWrite8(0x47BE76, 0x24, 0xE8);
    FuncWrite32(0x47BE77, 0x0228, (DWORD)&h_ls_check_double_click_saving);
    //jmp over original delay
    MemWrite16(0x47C1F4, 0xF889, 0x0AEB);



    //Loading-Fixes------------------------------------------------------------------------------------------------------------------------------------------------------

    //moved the fade-in effect between the main-menu and load screen, was previously being done before load screen creation causing load screen to pop in after fade-in. Now set within the load_screen function after window creation. - looks better.
    MemWrite8(0x480B70, 0xE8, 0x90);
    MemWrite32(0x480B71, 0x012F5F, 0x90909090);
    FuncReplace32(0x47CAC6, 0x05A492, (DWORD)&h_load_from_main_menu_fade_effect);


    //dont draw win
    MemWrite8(0x47C92B, 0xE8, 0x90);
    MemWrite32(0x47C92C, 0x05A62C, 0x90909090);

    MemWrite8(0x47CE9B, 0xE8, 0x90);
    MemWrite32(0x47CE9C, 0x05A0BC, 0x90909090);

    MemWrite8(0x47CFD9, 0xE8, 0x90);
    MemWrite32(0x47CFDA, 0x059F7E, 0x90909090);


    //set lscover
    MemWrite8(0x47CA48, 0xA1, 0xE8);
    FuncWrite32(0x47CA49, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_front);
    //jump over draw to window function
    MemWrite8(0x47CA4D, 0x68, 0xEB);
    MemWrite32(0x47CA4E, 0x00000280, 0x90909058);

    //set load pic
    MemWrite8(0x47CA79, 0xA1, 0xE8);
    FuncWrite32(0x47CA7A, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_back);
    //jump over draw to window function
    MemWrite8(0x47CA7E, 0x68, 0xEB);
    MemWrite32(0x47CA7F, 0x00000280, 0x90909027);

    //set lscover
    MemWrite8(0x47CDEE, 0xA1, 0xE8);
    FuncWrite32(0x47CDEF, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_front);
    //jump over draw to window function
    MemWrite8(0x47CDF3, 0x68, 0xE9);
    MemWrite32(0x47CDF4, 0x00000280, 0x0000008A);

    //set load pic
    MemWrite8(0x47CE1F, 0xA1, 0xE8);
    FuncWrite32(0x47CE20, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_back);
    //jump over draw to window function
    MemWrite8(0x47CE24, 0x68, 0xEB);
    MemWrite32(0x47CE25, 0x00000280, 0x9090905C);

    //set lscover
    MemWrite8(0x47CF2D, 0xA1, 0xE8);
    FuncWrite32(0x47CF2E, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_front);
    //jump over draw to window function
    MemWrite8(0x47CF32, 0x68, 0xE9);
    MemWrite32(0x47CF33, 0x00000280, 0x00000089);


    //set load pic
    MemWrite8(0x47CF5D, 0xA1, 0xE8);
    FuncWrite32(0x47CF5E, FixAddress(0x614700), (DWORD)&h_load_arrange_cover_to_back);
    //jump over draw to window function
    MemWrite8(0x47CF62, 0x68, 0xEB);
    MemWrite32(0x47CF63, 0x00000280, 0x9090905C);


    MemWrite8(0x47E6D8, 0x53, 0xE9);
    FuncWrite32(0x47E6D9, 0x57565251, (DWORD)&h_ls_update_save_slots);

    MemWrite8(0x47E8E0, 0x53, 0xE9);
    FuncWrite32(0x47E8E1, 0x57565251, (DWORD)&h_ls_update_description_text);


    //removing the sluggishness from the original code by using a different method to check for double clicking
    MemWrite16(0x47CC41, 0x948B, 0x9090);
    MemWrite8(0x47CC43, 0x24, 0xE8);
    FuncWrite32(0x47CC44, 0x022C, (DWORD)&h_ls_check_double_click_loading);
    //jmp over original delay
    MemWrite16(0x47D004, 0xD889, 0x0AEB);

    //End-Loading-Fixes--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    //Load/Save Setup function
    MemWrite8(0x47D2E4, 0x53, 0xE9);
    FuncWrite32(0x47D2E5, 0x57565251, (DWORD)&h_ls_setup);


    //done here for compatability  with sfall - ExtraSaveSlots
    MemWrite8(0x47D82D, 0xE8, 0xE9);
    FuncWrite32(0x47D82E, 0x058C36, (DWORD)&h_ls_destructor);

    p_LS_SlotState = (LONG*)FixAddress(0x614280);

    p_ls_SaveData = (LS_SAVE_DATA*)FixAddress(0x613D30);

    p_LS_CurrentlySelectedSlot = (LONG*)FixAddress(0x5193B8);

    p_LS_GeneralPurposeTextBuffer = (char*)FixAddress(0x6145FC);



    //save naming box function
    MemWrite8(0x47ED5C, 0x53, 0xE9);
    FuncWrite32(0x47ED5D, 0x57565251, (DWORD)&h_ls_save_description_box);

    //quick save picture creation function
    MemWrite8(0x47C5B4, 0x53, 0xE9);
    FuncWrite32(0x47C5B5, 0x60B85251, (DWORD)&h_ls_create_quick_save_picture);



    //Sfall ExtraSaveSlots compatability
    if (SfallReadInt(L"Misc", L"ExtraSaveSlots", 0) != 0) {
        p_LS_Sfall_ExtraSaveSlots_Get_Page_Number = (void*)FixAddress(0x47E756);
        MemWrite8(0x47E75B, 0x50, 0xC3);

        //To-Do Sfall ExtraSaveSlots - typing page offset
        //typing page offset is proving problematic - skip over for now -----------
        //save
        MemWrite8(0x47BD31, 0x3D, 0xE8);
        FuncWrite32(0x47BD32, 0x01F5, (DWORD)&h_ls_sfall_extra_save_slots_skip_p_key_edit);
        //load
        MemWrite8(0x47CB05, 0x3D, 0xE8);
        FuncWrite32(0x47CB06, 0x01F5, (DWORD)&h_ls_sfall_extra_save_slots_skip_p_key_edit);
        //---------------------------------------------------------------------------
    }

}



//___________________________
void Modifications_LoadSave() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_LoadSave_CH();
    else
        Modifications_LoadSave_MULTI();
}

