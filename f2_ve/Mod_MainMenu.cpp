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
#include "version.h"

#include "Fall_General.h"
#include "Fall_Graphics.h"
#include "Fall_Text.h"
#include "Fall_Msg.h"

#include "Dx_Windows.h"
#include "Dx_Graphics.h"

//for hires win setup
LONG p_buttRef_MainMenu_Options=-1;

LONG MAIN_MENU_SIZE = 0;
LONG USE_HIRES_IMAGES = 0;
//LONG SCALE_BUTTONS_AND_TEXT_MENU = 0;
bool isUsingOriginalBackgroundImage = false;

MSGList* pMsgList_Misc = nullptr;
BYTE** pp_colour_MM_MenuText = nullptr;
BYTE** pp_colour_MM_CopyrightText = nullptr;
BYTE** pp_colour_MM_VersionText = nullptr;

BOOL* p_is_main_menu_initiated = nullptr;
BOOL* p_is_main_menu_hidden = nullptr;

void* pfall_get_version_text = nullptr;

LONG *p_buttRefs_MainMenu = nullptr;
LONG *p_butt_key_values_MainMenu = nullptr;


int winRef_MM_BG_Image = -1;
int winRef_MM_Menu = -1;
int winRef_MM_CopyrightText = -1;
int winRef_MM_VersionText = -1;

bool main_menu_exit = false;


//_______________________
void MainMenu_SetToExit() {
    main_menu_exit = true;
}


//_______________
bool IsMainMenu() {
   if(*p_is_main_menu_initiated && !*p_is_main_menu_hidden)
      return true;
   return false;
}


//_______________________________________
char* fall_GetVersionText(char* outText) {
    __asm {
        mov eax, outText
        call pfall_get_version_text
    }
    return outText;
}


//__________________________________
DWORD MainMenu_GetMenuMaxTextWidth() {
    LONG oldFont = fall_GetFont();
    fall_SetFont(104);
    FONT_FUNC_STRUCT* font = GetCurrentFont();

    DWORD textWidth = 0;
    DWORD maxWidth = 0;
    MSGNode* msgNode = nullptr;
    for (DWORD i = 0; i < 6; i++) {// find the max text width in menu - for centring
        msgNode = GetMsgNode(pMsgList_Misc, 9 + i);
        if (msgNode)
            textWidth = font->GetTextWidth(msgNode->msg2);
        if (textWidth > maxWidth)
            maxWidth = textWidth;
    }

    fall_SetFont(oldFont);
    return maxWidth;
}


//________________________________________________________________________________________________________
void MainMenu_PrintMenuText(Window_DX* winDx, LONG xPos, LONG yCentre, DWORD colour, bool use_glyph_alpha) {
    if (fallout_exe_region == EXE_Region::Chinese)
        return;
    LONG oldFont = fall_GetFont();
    fall_SetFont(104);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG fontCentreOffset = (LONG)font->GetFontHeight() / 2;
    LONG textCentreOffset = 0;


    DWORD textWidth = 0;
    DWORD maxWidth = 0;
    MSGNode* msgNode = nullptr;
    for (DWORD i = 0; i < 6; i++) {// find the max text width in menu - for centring
        msgNode = GetMsgNode(pMsgList_Misc, 9 + i);
        if (msgNode)
            textWidth = font->GetTextWidth(msgNode->msg2);
        if (textWidth > maxWidth)
            maxWidth = textWidth;
    }

    LONG xCentre = xPos + (LONG)maxWidth / 2;

    for (DWORD i = 0; i < 6; i++) {
        msgNode = GetMsgNode(pMsgList_Misc, 9 + i);
        if (msgNode) {
            textCentreOffset = (LONG)font->GetTextWidth(msgNode->msg2) / 2;
            winDx->Draw_Text(msgNode->msg2, xCentre - textCentreOffset, yCentre - fontCentreOffset, colour, 0, TextEffects::none);
        }
        yCentre += 41;
    }
    fall_SetFont(oldFont);
}


//________________________________________________
void OnScreenResize_MainMenu(Window_DX* pWin_This) {

    if (!pWin_This)
        return;

    if (!*p_is_main_menu_initiated)
        return;

    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    ResizeWindowX(pWin, 0, 0, SCR_WIDTH, SCR_HEIGHT);

    LONG menuBGOffX = ConfigReadInt(L"MAINMENU", L"MENU_BG_OFFSET_X", -14);
    LONG menuBGOffY = ConfigReadInt(L"MAINMENU", L"MENU_BG_OFFSET_Y", -4);

    LONG buttX = 30;
    LONG buttY = 19;
    buttX += SfallReadInt(L"Misc", L"MainMenuOffsetX", 0);
    buttY += SfallReadInt(L"Misc", L"MainMenuOffsetY", 0);
    if (buttX <= 0)buttX = 0;
    if (buttY <= 0)buttY = 0;



    Window_DX* subwin = pWin_This->GetSubWin(winRef_MM_BG_Image);
    if (!subwin)
        return;

    float point_w = 1.0f;
    float point_h = 1.0f;
    RECT rect_bg_scrn = { 0,0,0,0 };
    ScaleWindowToScreen(subwin, MAIN_MENU_SIZE, &point_w, &point_h, &rect_bg_scrn);

    DWORD buttWidth = 0;
    DWORD buttHeight = 0;
    ButtonStruct_DX* butt = (ButtonStruct_DX*)fall_Button_Get(p_buttRefs_MainMenu[0], nullptr);
    butt->buttDx->GetWidthHeight(&buttWidth, &buttHeight);

    LONG yOffset = 0;
    subwin = pWin_This->GetSubWin(winRef_MM_Menu);
    if (isUsingOriginalBackgroundImage) {
        buttX += (LONG)(rect_bg_scrn.left * point_w);// rect_bg_scaled.left;
        buttY += (LONG)(rect_bg_scrn.top * point_h);// rect_bg_scaled.top;
        if (subwin) {
            subwin->SetPosition((buttX + menuBGOffX) / point_w, (buttY + menuBGOffY) / point_h);
            subwin->SetScale(1 / point_w, 1 / point_h);
        }

        //move and scale menu buttons
        for (int i = 0; i < 6; i++) {
            butt = (ButtonStruct_DX*)fall_Button_Get(p_buttRefs_MainMenu[i], nullptr);
            butt->rect.left = (LONG)(buttX / point_w);//buttX - 1;
            butt->rect.right = butt->rect.left + (LONG)(buttWidth / point_w) - 1;
            butt->rect.top = (LONG)((buttY + yOffset) / point_h);//buttY + yOffset - 1;
            butt->rect.bottom = butt->rect.top + (LONG)(buttHeight / point_h) - 1;
            if (butt->buttDx) {
                butt->buttDx->SetPosition((float)butt->rect.left, (float)butt->rect.top);
                butt->buttDx->SetScale(1 / point_w, 1 / point_h);
            }
            yOffset += 41;
        }
    }
    else {//hr menu and buttons aren't scaled.
        buttX += rect_bg_scrn.left;
        buttY += rect_bg_scrn.top;
        if (subwin)
            subwin->SetPosition((float)(buttX + menuBGOffX), (float)(buttY + menuBGOffY));
        //move menu buttons
        for (int i = 0; i < 6; i++) {
            butt = (ButtonStruct_DX*)fall_Button_Get(p_buttRefs_MainMenu[i], nullptr);
            butt->rect.left = buttX;
            butt->rect.right = butt->rect.left + buttWidth - 1;
            butt->rect.top = buttY + yOffset;// +1;
            butt->rect.bottom = butt->rect.top + buttHeight - 1;
            if (butt->buttDx)
                butt->buttDx->SetPosition((float)butt->rect.left, (float)butt->rect.top);
            yOffset += 41;
        }
    }


    subwin = pWin_This->GetSubWin(winRef_MM_CopyrightText);
    if (subwin)
        subwin->SetPosition((float)(rect_bg_scrn.left + 15), (float)(rect_bg_scrn.bottom - subwin->GetHeight() - 8));

    subwin = pWin_This->GetSubWin(winRef_MM_VersionText);
    if (subwin)
        subwin->SetPosition((float)(rect_bg_scrn.right - subwin->GetWidth() - 15), (float)(rect_bg_scrn.bottom - subwin->GetHeight() - 8));

    return;
}


//________________________
void MainMenu_Destructor() {
    if (*p_is_main_menu_initiated == FALSE)
        return;
    for (int i = 0; i < 6; i++) {
        if (p_buttRefs_MainMenu[i] != -1)
            fall_Button_Destroy(p_buttRefs_MainMenu[i]);
        p_buttRefs_MainMenu[i] = -1;
    }
    if (*pWinRef_MainMenu != -1)
        fall_Win_Destroy(*pWinRef_MainMenu);
    *pWinRef_MainMenu = -1;

    winRef_MM_BG_Image = -1;
    winRef_MM_Menu = -1;
    winRef_MM_CopyrightText = -1;
    winRef_MM_VersionText = -1;

    *p_is_main_menu_initiated = FALSE;
}


//___________________________________________
void __declspec(naked) main_menu_destructor() {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        call MainMenu_Destructor
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//___________________
LONG MainMenu_Setup() {
    if (main_menu_exit == true) {
        *pGAME_EXIT_FLAGS = 3;
        return -1;
    }
    
    if (*p_is_main_menu_initiated)
        return 0;
    MAIN_MENU_SIZE = ConfigReadInt(L"MAINMENU", L"MAIN_MENU_SIZE", 0);
    USE_HIRES_IMAGES = ConfigReadInt(L"MAINMENU", L"USE_HIRES_IMAGES", 0);
    //SCALE_BUTTONS_AND_TEXT_MENU = ConfigReadInt("MAINMENU", "SCALE_BUTTONS_AND_TEXT_MENU", 0);

    LONG menuBGOffX = ConfigReadInt(L"MAINMENU", L"MENU_BG_OFFSET_X", -14);
    LONG menuBGOffY = ConfigReadInt(L"MAINMENU", L"MENU_BG_OFFSET_Y", -4);

    LONG buttX = 30;
    LONG buttY = 19;
    buttX += SfallReadInt(L"Misc", L"MainMenuOffsetX", 0);
    buttY += SfallReadInt(L"Misc", L"MainMenuOffsetY", 0);
    if (buttX <= 0)buttX = 0;
    if (buttY <= 0)buttY = 0;

    DWORD palColour = 0;
    if (pp_colour_MM_MenuText)
        palColour = (**pp_colour_MM_MenuText) | 0x06000000;

    WinStructDx* win = nullptr;
    *pWinRef_MainMenu = fall_Win_Create(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, FLG_WinToFront | FLG_WinHidden);
    win = (WinStructDx*)fall_Win_Get(*pWinRef_MainMenu);

    if (!win || !win->winDx) {
        MainMenu_Destructor();
        return -1;
    }
    win->winDx->SetDrawFlag(false);
    win->winDx->Set_OnScreenResizeFunction(&OnScreenResize_MainMenu);

    FRMdx* menuBG = nullptr;

    FRMdx* pMainMenuFrmDx = nullptr;
    FRMframeDx* pMainMenuFrame = nullptr;
    //load background images
    if (USE_HIRES_IMAGES) {
        pMainMenuFrmDx = new FRMdx("HR_MAINMENU", ART_INTRFACE, -1);
        pMainMenuFrame = pMainMenuFrmDx->GetFrame(0, 0);
        if (fallout_exe_region == EXE_Region::Chinese)
            menuBG = new FRMdx("HR_MENU_BG_CHI", ART_INTRFACE, -1);
        else
            menuBG = new FRMdx("HR_MENU_BG", ART_INTRFACE, -1);
        //check if menuBG exists
        if (!menuBG->GetFrame(0, 0)) {
            delete menuBG;
            menuBG = nullptr;
        }

    }
    //if not using hr-background or hr-background failed to load
    if (!pMainMenuFrame || !USE_HIRES_IMAGES) {
        isUsingOriginalBackgroundImage = true;//the menu is part of the background image
        if (pMainMenuFrmDx)
            delete pMainMenuFrmDx;
        pMainMenuFrmDx = new FRMdx("MAINMENU.frm", ART_INTRFACE, -1);
        pMainMenuFrame = pMainMenuFrmDx->GetFrame(0, 0);
    }
    //if background completly failed to load
    if (!pMainMenuFrame) {
        if (pMainMenuFrmDx)
            delete pMainMenuFrmDx;
        pMainMenuFrmDx = nullptr;
        pMainMenuFrame = nullptr;
        MainMenu_Destructor();
        return -1;
    }

    Window_DX* subwin = nullptr;
    subwin = new Window_DX(0, 0, pMainMenuFrame->GetWidth(), pMainMenuFrame->GetHeight(), 0x00000000, win->winDx, &winRef_MM_BG_Image);
    subwin->ClearRenderTarget(nullptr);
    subwin->RenderTargetDrawFrame(0, 0, pMainMenuFrame, nullptr, nullptr);
    delete pMainMenuFrmDx;
    pMainMenuFrmDx = nullptr;
    pMainMenuFrame = nullptr;

    float point_w = 1.0f;
    float point_h = 1.0f;
    RECT rect_bg_scrn = { 0,0,0,0 };
    ScaleWindowToScreen(subwin, MAIN_MENU_SIZE, &point_w, &point_h, &rect_bg_scrn);

    delete pMainMenuFrmDx;
    pMainMenuFrmDx = nullptr;
    pMainMenuFrame = nullptr;

    //button frm ID's
    DWORD frmID_menuUp = (ART_INTRFACE << 24 | 0x12B);
    DWORD frmID_menuDn = (ART_INTRFACE << 24 | 0x12C);
    //get the width and height of the up button
    FRMCached* frmMenuUp = new FRMCached(frmID_menuUp);
    if (!frmMenuUp) {
        MainMenu_Destructor();
        if (menuBG)
            delete menuBG;
        menuBG = nullptr;
        return -1;
    }
    FRMframeDx* pFrame = frmMenuUp->GetFrame(0, 0);
    if (!pFrame) {
        MainMenu_Destructor();
        if (menuBG)
            delete menuBG;
        menuBG = nullptr;
        return -1;
    }

    DWORD buttWidth = pFrame->GetWidth();
    DWORD buttHeight = pFrame->GetHeight();
    delete frmMenuUp;
    frmMenuUp = nullptr;
    pFrame = nullptr;


    DWORD colour;//for holding colour extracted from palette "color.pal"
    if (color_pal == nullptr)
        return false;

    colour = color_pal->GetColour(palColour & 0x000000FF);


    if (menuBG) { //if the hr menu graphic exists, create a sub window and draw menu background and text to.
        pFrame = menuBG->GetFrame(0, 0);
        subwin = new Window_DX(0, 0, pFrame->GetWidth(), pFrame->GetHeight(), 0x00000000, win->winDx, &winRef_MM_Menu);
        subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
        delete menuBG;
        menuBG = nullptr;
        pFrame = nullptr;
        //set use_glyph_alpha to false so that the text blends with menu background rather than overwriting it.
        MainMenu_PrintMenuText(subwin, buttWidth + 3 - menuBGOffX, buttHeight / 2 - menuBGOffY, colour, false);
    }
    else {//create a sub window, big enough to draw the menu text to.
        subwin = new Window_DX(0, 0, buttWidth + 3 - menuBGOffX + MainMenu_GetMenuMaxTextWidth(), buttHeight / 2 - menuBGOffY + 41 * 6, 0x00000000, win->winDx, &winRef_MM_Menu);
        //set use_glyph_alpha to true since we want the text on this window to blend with the main background when rendered.
        MainMenu_PrintMenuText(subwin, buttWidth + 3 - menuBGOffX, buttHeight / 2 - menuBGOffY, colour, true);
    }

    LONG yOffset = 0;//4;
    ButtonStruct_DX* butt = nullptr;

    if (isUsingOriginalBackgroundImage) {//the menu is part of the background image, button position needs to be scaled to match
        buttX += (LONG)(rect_bg_scrn.left * point_w);// rect_bg_scaled.left;
        buttY += (LONG)(rect_bg_scrn.top * point_h);// rect_bg_scaled.top;

        subwin->SetPosition((buttX + menuBGOffX) / point_w, (buttY + menuBGOffY) / point_h);
        subwin->SetScale(1 / point_w, 1 / point_h);
        //create menu buttons
        for (int i = 0; i < 6; i++) {
            p_buttRefs_MainMenu[i] = CreateButtonX(*pWinRef_MainMenu, (LONG)(buttX / point_w), (LONG)((buttY + yOffset) / point_h), (LONG)(buttWidth / point_w), (LONG)(buttHeight / point_h), -1, -1, 1111, p_butt_key_values_MainMenu[i], frmID_menuUp, frmID_menuDn, 0, FLG_ButtTrans);
            if (p_butt_key_values_MainMenu[i] == 'o')
                p_buttRef_MainMenu_Options = p_buttRefs_MainMenu[i];
            //SetButtonPointerMaskX(p_buttRefs_MainMenu[i], frmID_menuUp);
            butt = (ButtonStruct_DX*)fall_Button_Get(p_buttRefs_MainMenu[i], nullptr);
            butt->buttDx->SetScale(1 / point_w, 1 / point_h);
            yOffset += 41;
        }
    }
    else {
        buttX += rect_bg_scrn.left;
        buttY += rect_bg_scrn.top;
        subwin->SetPosition((float)(buttX + menuBGOffX), (float)(buttY + menuBGOffY));

        for (int i = 0; i < 6; i++) {
            p_buttRefs_MainMenu[i] = CreateButtonX(*pWinRef_MainMenu, buttX, buttY + yOffset, buttWidth, buttHeight, -1, -1, 1111, p_butt_key_values_MainMenu[i], frmID_menuUp, frmID_menuDn, 0, FLG_ButtTrans);
            if (p_butt_key_values_MainMenu[i] == 'o')
                p_buttRef_MainMenu_Options = p_buttRefs_MainMenu[i];
            SetButtonPointerMaskX(p_buttRefs_MainMenu[i], frmID_menuUp);
            yOffset += 41;
        }
    }


    LONG oldFont = fall_GetFont();
    fall_SetFont(100);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    DWORD fontHeight = font->GetFontHeight();

    //create a sub window and draw copyright text
    MSGNode* msgNode = GetMsgNode(pMsgList_Misc, 20);
    if (msgNode) {
        if (pp_colour_MM_CopyrightText)
            palColour = (**pp_colour_MM_CopyrightText) | 0x06000000;
        colour = color_pal->GetColour(palColour & 0x000000FF);
        DWORD textWidth = font->GetTextWidth(msgNode->msg2);
        if (textWidth > 0 && fontHeight > 0) {
            subwin = new Window_DX((float)(rect_bg_scrn.left + 15), (float)(rect_bg_scrn.bottom - fontHeight - 8), textWidth, fontHeight, 0x00000000, win->winDx, &winRef_MM_CopyrightText);
            subwin->Draw_Text(msgNode->msg2, 0, 0, colour, 0, TextEffects::none);
        }
        msgNode = nullptr;
    }

    //get the version text for the game.
    char verText[256];
    fall_GetVersionText(verText);

    if (pp_colour_MM_VersionText)
        palColour = (**pp_colour_MM_VersionText) | 0x06000000;
    colour = color_pal->GetColour(palColour & 0x000000FF);
    DWORD textWidth = font->GetTextWidth(verText);

    //get the version text for the hi-res patch.
    char verText_HiRes[64];
    sprintf_s(verText_HiRes, 64, "HI-RES-V.E. %s", VER_FILE_VERSION_STR);
    DWORD textWidth_HiRes = font->GetTextWidth(verText_HiRes);
    if (textWidth_HiRes > textWidth)
        textWidth = textWidth_HiRes;

    DWORD ver_win_height = fontHeight * 2;

    //get the version text for sfall.
    char verText_Sfall[64];
    bool isSfallVersion = false;
    if (GetSfall_VersionInfo(verText_Sfall, 64)) {
        isSfallVersion = true;
        ver_win_height += fontHeight;
        DWORD textWidth_sfall = font->GetTextWidth(verText_Sfall);
        if (textWidth_sfall > textWidth)
            textWidth = textWidth_sfall;
    }

    //create a sub window and draw the version text
    if (textWidth > 0 && ver_win_height > 0) {
        subwin = new Window_DX((float)(rect_bg_scrn.right - textWidth - 15), (float)(rect_bg_scrn.bottom - ver_win_height - 8), textWidth, ver_win_height, 0x00000000, win->winDx, &winRef_MM_VersionText);
        subwin->Draw_Text(verText, 0, 0, colour, 0, TextEffects::none);
        subwin->Draw_Text(verText_HiRes, 0, fontHeight, colour, 0, TextEffects::none);
        if (isSfallVersion)
            subwin->Draw_Text(verText_Sfall, 0, fontHeight * 2, colour, 0, TextEffects::none);
    }
    fall_SetFont(oldFont);

    *p_is_main_menu_initiated = TRUE;
    *p_is_main_menu_hidden = TRUE;
    return 0;
}


//______________________________________
void __declspec(naked) main_menu_setup() {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        call MainMenu_Setup
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//______________________________
void Modifications_MainMenu_CH() {

    pMsgList_Misc = (MSGList*)0x59EEC0;

    pp_colour_MM_CopyrightText = (BYTE**)0x480C04;
    pp_colour_MM_VersionText = (BYTE**)0x480C3C;

    p_is_main_menu_initiated = (BOOL*)0x5292F8;
    p_is_main_menu_hidden = (BOOL*)0x624DD8;

    p_buttRefs_MainMenu = (LONG*)0x624DC0;
    p_butt_key_values_MainMenu = (LONG*)0x529300;

    pfall_get_version_text = (void*)0x4B3110;

    MemWrite8(0x480B10, 0x53, 0xE9);
    FuncWrite32(0x480B11, 0x57565251, (DWORD)&main_menu_setup);

    MemWrite8(0x480D98, 0x53, 0xE9);
    FuncWrite32(0x480D99, 0x57565251, (DWORD)&main_menu_destructor);
}


//_________________________________
void Modifications_MainMenu_MULTI() {

    pMsgList_Misc = (MSGList*)FixAddress(0x58E940);
    pp_colour_MM_MenuText = (BYTE**)FixAddress(0x481906);

    pp_colour_MM_CopyrightText = (BYTE**)FixAddress(0x481748);
    pp_colour_MM_VersionText = (BYTE**)FixAddress(0x481780);

    p_is_main_menu_initiated = (BOOL*)FixAddress(0x519508);
    p_is_main_menu_hidden = (BOOL*)FixAddress(0x614858);

    p_buttRefs_MainMenu = (LONG*)FixAddress(0x614840);
    p_butt_key_values_MainMenu = (LONG*)FixAddress(0x519510);

    pfall_get_version_text = (void*)FixAddress(0x4B4580);

    MemWrite8(0x481650, 0x53, 0xE9);
    FuncWrite32(0x481651, 0x57565251, (DWORD)&main_menu_setup);

    MemWrite8(0x481968, 0x53, 0xE9);
    FuncWrite32(0x481969, 0x57565251, (DWORD)&main_menu_destructor);
}


//___________________________
void Modifications_MainMenu() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_MainMenu_CH();
    else
        Modifications_MainMenu_MULTI();
}






