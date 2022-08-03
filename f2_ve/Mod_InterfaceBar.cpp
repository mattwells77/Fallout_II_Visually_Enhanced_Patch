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

#include "Fall_Graphics.h"
#include "Fall_General.h"
#include "Fall_Objects.h"
#include "Fall_Text.h"

#include "Dx_Windows.h"
#include "Dx_Graphics.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

DWORD IFACE_BAR_WIDTH = 640;
DWORD IFACE_BAR_GRAPHIC_WIDTH = 640;
UINT IFACE_BAR_MODE = 0;
int IFACE_BAR_LOCATION = 0;
int IFACE_BAR_SIDE_ART = 0;
DWORD IFACE_BG_COLOUR = 0x484848BD;


struct IFACE_WIELDED_OBJ {
    OBJStruct *pObj;
    DWORD typeFlags;//flags byte1 = itemDoesNothing?, byte2 = itemIsAWeapon. byte3 = ?, byte4 = ?;
    LONG attackType1;
    LONG attackType2;
    LONG rClickNum;//current option count
    DWORD frmID;
};

IFACE_WIELDED_OBJ *p_iface_wielded = nullptr;

LONG *p_iface_active_slot = nullptr;

BOOL* p_is_iface_initilizing = nullptr;
BOOL* p_is_iface_enabled = nullptr;
BOOL* p_is_iface_hidden = nullptr;
BOOL* p_is_combat_box_opened = nullptr;

LONG *p_buttRef_ifaceInv = nullptr;
LONG *p_buttRef_ifaceOptions = nullptr;
LONG *p_buttRef_ifaceSkills = nullptr;
LONG *p_buttRef_ifaceMap = nullptr;
LONG *p_buttRef_ifacePip = nullptr;
LONG *p_buttRef_ifaceChar = nullptr;
LONG *p_buttRef_ifaceAttack = nullptr;
LONG *p_buttRef_ifaceSwitchHands = nullptr;

LONG *p_buttRef_ifaceSwitchTurn = nullptr;
LONG *p_buttRef_ifaceEndCombat = nullptr;

LONG* p_buttRef_imonitor_up = nullptr;
LONG* p_buttRef_imonitor_down = nullptr;


BYTE **p_buff_ifaceWinBuff = nullptr;

RECT *icombatRect = nullptr;
RECT *imonitorRect = nullptr;
RECT *iactionRect = nullptr;
RECT *iitemSlotRect = nullptr;

BYTE **pp_imonitor_colour = nullptr;//0xD7;
LONG* p_imonitor_font = nullptr;//101;

#define IMON_TXT_WIDTH_CHARS    256
#define IMON_TXT_NUM_LINES      100
#define IMON_TXT_BUFF_SIZE      IMON_TXT_WIDTH_CHARS * IMON_TXT_NUM_LINES
char imonitorTxtBuff[IMON_TXT_BUFF_SIZE];

LONG* p_imonitor_line_last = nullptr;
LONG* p_imonitor_line_view = nullptr;
LONG* p_imonitor_num_visible_lines = nullptr;

BOOL *p_is_imonitor = nullptr;

ULONGLONG ull_iMonitor_Sound_Time = 0;

int imonitorTxtHeight = 0;
int imonitorAreaX = 0;
int imonitorAreaY = 0;
int imonitorAreaWidth = 0;
int imonitorAreaHeight = 0;

LONG(*fall_Iface_DrawAmmoBar)() = nullptr;

LONG(*fall_NotifyBar_Setup)() = nullptr;
LONG(*fall_NotifyBar_Destructor)() = nullptr;
LONG(*fall_NotifyBar_Enable)() = nullptr;
LONG(*fall_NotifyBar_Disable)() = nullptr;

int subWin_num_iMonitor = -1;
int subWin_num_AmmoBar = -1;
int subWin_num_APBar = -1;
int subWin_num_CombatBox = -1;
int subWin_num_CombatBoxLights = -1;
int subWin_num_HPCounter = -1;
int subWin_num_ACCounter = -1;


int subWin_num_ifaceLeft_a = -1;
int subWin_num_ifaceLeft_b = -1;
int subWin_num_ifaceRight_a = -1;
int subWin_num_ifaceRight_b = -1;


//______________________
void imonitorPrintText() {
    if (!*p_is_imonitor)
        return;
    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;

    Window_DX* winDx_iMonitor = win->winDx->GetSubWin(subWin_num_iMonitor);
    if (!winDx_iMonitor)
        return;

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    if (font == nullptr)
        return;
    if (color_pal == nullptr)
        return;

    int oldFont = fall_GetFont();
    fall_SetFont(*p_imonitor_font);

    DWORD colour = color_pal->GetColour(**pp_imonitor_colour & 0x000000FF);

    winDx_iMonitor->Clear();

    int xOffset = 0;
    char* txtBuff = 0;
    for (int i = 0; i < *p_imonitor_num_visible_lines; i++) {
        txtBuff = imonitorTxtBuff;
        txtBuff += ((*p_imonitor_line_view + i + IMON_TXT_NUM_LINES - *p_imonitor_num_visible_lines) % IMON_TXT_NUM_LINES) * IMON_TXT_WIDTH_CHARS;
        winDx_iMonitor->Draw_Text(txtBuff, xOffset, i * imonitorTxtHeight, colour, 0, TextEffects::none);
        if (fallout_exe_region != EXE_Region::Chinese)
            xOffset++;
    }

    fall_SetFont(oldFont);
}


//____________________________________________
void __declspec(naked) h_imonitor_print_text() {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call imonitorPrintText

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//______________________________________
void imonitorInsertText(const char* msg) {
    if (!*p_is_imonitor)
        return;

    char* txtBuff = imonitorTxtBuff;

    char lineStart_Char = '•';//149 0x95;//bullet point
    int oldFont = fall_GetFont();
    fall_SetFont(*p_imonitor_font);

    FONT_FUNC_STRUCT* font = GetCurrentFont();
    int minTextWidth = font->GetCharWidth(lineStart_Char);
    int msgWidth = 0;

    WinStruct* win = fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    int lineWidth = imonitorAreaWidth;
    if (fallout_exe_region != EXE_Region::Chinese)
        lineWidth -= *p_imonitor_num_visible_lines;
    lineWidth -= minTextWidth;

    if (!(*p_combat_state_flags & 1)) {
        ULONGLONG current_tick = GetTickCount64();
        if (ull_iMonitor_Sound_Time + 500 < current_tick) {
            ull_iMonitor_Sound_Time = current_tick;
            fall_PlayAcm("monitor");
        }
    }

    LONG lineLength = 1;

    while (*msg != '\0') {
        txtBuff = imonitorTxtBuff + *p_imonitor_line_last * IMON_TXT_WIDTH_CHARS;//point txtBuff at next line
        if (lineStart_Char) {
            *txtBuff = lineStart_Char;
            txtBuff++;
        }
        else {
            if (fallout_exe_region == EXE_Region::Chinese) {
                *txtBuff = ' ';
                txtBuff++;
            }
        }

        font->GetTextWidth2(msg, (DWORD)lineWidth, &lineLength);
        if (*(msg + lineLength) == ' ')//add next char to end of line if equals space
            lineLength++;

        strncpy_s(txtBuff, IMON_TXT_BUFF_SIZE, msg, lineLength);
        txtBuff[lineLength] = '\0';

        msg += lineLength;

        lineStart_Char = 0;
        *p_imonitor_line_last = (*p_imonitor_line_last + 1) % IMON_TXT_NUM_LINES;
    }

    *p_imonitor_line_view = *p_imonitor_line_last;
    fall_SetFont(oldFont);

    imonitorPrintText();
}


//_____________________________________________
void __declspec(naked) h_imonitor_insert_text() {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        
        push eax
        call imonitorInsertText
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


//______________________
void imonitorCleatText() {
    if (!*p_is_imonitor)
        return;
    char* ptxt = imonitorTxtBuff;
    for (int i = 0; i < IMON_TXT_NUM_LINES; i++) {
        *ptxt = '\0';
        ptxt += IMON_TXT_WIDTH_CHARS;
    }
    *p_imonitor_line_last = 0;
    *p_imonitor_line_view = 0;
    imonitorPrintText();
}


//____________________________________________
void __declspec(naked) h_imonitor_clear_text() {

    __asm {
        push ebx
        push ecx
        push edx

        call imonitorCleatText

        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_________________________
void iMonitorMouseDefault() {
    fall_Mouse_SetImage(1);
}


//____________________
void iMonitorMouseUp() {
    fall_Mouse_SetImage(2);
}


//______________________
void iMonitorMouseDown() {
    fall_Mouse_SetImage(3);
}


//_____________________
void iMonitorScrollUp() {
    int newLineNum = (IMON_TXT_NUM_LINES + *p_imonitor_line_view - 1) % IMON_TXT_NUM_LINES;
    if (newLineNum != *p_imonitor_line_last)
        *p_imonitor_line_view = newLineNum;
    imonitorPrintText();
}


//_______________________
void iMonitorScrollDown() {
    if (*p_imonitor_line_view == *p_imonitor_line_last)
        return;
    *p_imonitor_line_view = (*p_imonitor_line_view + 1) % IMON_TXT_NUM_LINES;
    imonitorPrintText();
}


//____________________________________________________
bool Mouse_Wheel_Imonitor(int zDelta, bool scrollPage) {

    if (!*p_is_imonitor)
        return false;
    WinStruct* win = fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return false;
    if (win->flags & FLG_WinHidden)
        return false;

    RECT rcImon;
    rcImon.left = imonitorRect->left + win->rect.left;
    rcImon.top = imonitorRect->top + win->rect.top;
    rcImon.right = imonitorRect->right + win->rect.left;
    rcImon.bottom = imonitorRect->bottom + win->rect.top;

    int numLines = 1;
    if (scrollPage)
        numLines = *p_imonitor_num_visible_lines;

    if (IsMouseInRect(&rcImon)) {
        while (numLines > 0) {
            if (zDelta > 0)
                iMonitorScrollUp();
            else if (zDelta < 0)
                iMonitorScrollDown();
            numLines--;
        }
        return true;
    }
    return false;
}


//__________________
int iMonitor_Setup() {

    if (*p_is_imonitor)
        return 0;

    int oldFont = fall_GetFont();
    fall_SetFont(*p_imonitor_font);
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    imonitorTxtHeight = font->GetFontHeight();// fall_GetTextHeight();
    fall_SetFont(oldFont);

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return -1;
    if (!win->winDx)
        return -1;

    imonitorAreaX = 23;
    imonitorAreaWidth = 167 + (win->width - 640);
    int buttHeight;
    if (fallout_exe_region == EXE_Region::Chinese) {//chi
        imonitorAreaY = 12;
        imonitorAreaHeight = 72;
        imonitorTxtHeight += 2;
        buttHeight = 36;
    }
    else if (pGvx) {
        imonitorAreaY = 24;
        imonitorAreaHeight = 60;
        imonitorTxtHeight += 2;
        buttHeight = 30;
    }
    else {
        imonitorAreaY = 24;
        imonitorAreaHeight = 60;
        buttHeight = 30;
    }

    *p_imonitor_num_visible_lines = imonitorAreaHeight / imonitorTxtHeight;
    *p_imonitor_line_last = 0;
    *p_imonitor_line_view = 0;

    Window_DX* winDx_iMonitor = win->winDx->GetSubWin(subWin_num_iMonitor);
    if (!winDx_iMonitor) {
        winDx_iMonitor = new Window_DX((float)(imonitorAreaX), (float)(imonitorAreaY), imonitorAreaWidth, imonitorAreaHeight, 0x00000000, win->winDx, &subWin_num_iMonitor);
        if (!winDx_iMonitor)
            return -1;
    }
    *p_buttRef_imonitor_up = fall_Button_Create(*pWinRef_Iface, imonitorAreaX, imonitorAreaY, imonitorAreaWidth, buttHeight, -1, -1, -1, -1, 0, 0, 0, 0);
    fall_Button_SetFunctions(*p_buttRef_imonitor_up, &iMonitorMouseUp, &iMonitorMouseDefault, &iMonitorScrollUp, 0);

    *p_buttRef_imonitor_down = fall_Button_Create(*pWinRef_Iface, imonitorAreaX, imonitorAreaY + buttHeight, imonitorAreaWidth, buttHeight, -1, -1, -1, -1, 0, 0, 0, 0);
    fall_Button_SetFunctions(*p_buttRef_imonitor_down, &iMonitorMouseDown, &iMonitorMouseDefault, &iMonitorScrollDown, 0);

    *p_is_imonitor = TRUE;
    imonitorCleatText();
    imonitorPrintText();
    return 0;
}


//_____________________________________________
void DrawAmmoBar(LONG barX, LONG barFillHeight) {

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    //don't redraw if hasn't changed
    static long last_barFillHeight = 0;
    if (barFillHeight == last_barFillHeight)
        return;
    last_barFillHeight = barFillHeight;

    //move bar position away from rivits
    barX -= 4;
    //fix barX if iface longer
    barX += win->width - 640;

    LONG barY = 26;
    LONG barFillHeightMax = 70;

    Window_DX* winDx_AmmoBar = win->winDx->GetSubWin(subWin_num_AmmoBar);
    if (!winDx_AmmoBar) {
        winDx_AmmoBar = new Window_DX((float)(barX), (float)(barY), 1, barFillHeightMax, 0x00000000, win->winDx, &subWin_num_AmmoBar);
        if (!winDx_AmmoBar)
            return;
    }
    if (barFillHeight & 1)
        barFillHeight--;
    if (barFillHeight > barFillHeightMax) 
        barFillHeight = barFillHeightMax;
    if (barFillHeight < 0)
        barFillHeight = 0;


    LONG emptyHeight = barFillHeightMax - barFillHeight - 1;

    BYTE* pTextBack = new BYTE[barFillHeightMax];
    for (int i = 0; i < barFillHeightMax - 1; i += 2) {
        if (i < emptyHeight)
            pTextBack[i] = 0;
        else
            pTextBack[i] = 0xC4;

        pTextBack[i + 1] = 0;
    }
    winDx_AmmoBar->Draw(pTextBack, nullptr, false, 1, barFillHeightMax, 0, 0, 1, barFillHeightMax, 0, 0);
    delete[] pTextBack;

}


//______________________________________
void __declspec(naked) h_draw_ammo_bar() {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call DrawAmmoBar
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//_______________________________________________________
void OnScreenResize_IfaceSubWindows(Window_DX* pWin_This) {

    //this is for the notify and skill windows
    //nothing is done here as these windows are moved in the ReSizeIfaceX function.
    //this function exists to prevent them being moved in the default OnScreenResize_Window function.
    //imonitorInsertText("OnScreenResize_IfaceSubWindows");
}


//___________________________________________________________________________________
int IfaceNotifyWin(int x, int y, int width, int height, DWORD colour, DWORD winFlags) {
    //To-Do Notify Bar - should replace the whole setup at some point.
    WinStruct* ifaceWin = fall_Win_Get(*pWinRef_Iface);
    int winRef = fall_Win_Create(x + ifaceWin->rect.left, ifaceWin->rect.bottom - (478 - y), width, height, colour, winFlags);
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (pWin && pWin->winDx) {
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_IfaceSubWindows);
    }
    return winRef;
}


//_____________________________________________
void __declspec(naked) h_iface_notify_win(void) {

    __asm {
        push dword ptr ss : [esp + 0x8]
        push dword ptr ss : [esp + 0x8]

        push ecx
        push ebx
        push edx
        push eax
        call IfaceNotifyWin
        add esp, 0x18
        ret 0x8
    }

}


//___________________________________________________________________________________
int IfaceSkillWin( int x, int y, int width, int height, DWORD colour, DWORD winFlags) {
    //To-Do Iface Skill Menu - should replace the whole setup at some point.
   WinStruct *ifaceWin = fall_Win_Get(*pWinRef_Iface);
   int winRef = fall_Win_Create(x+ifaceWin->rect.left+(ifaceWin->width-640), ifaceWin->rect.bottom-(478-y), width, height, colour, winFlags);
   WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
   if (pWin && pWin->winDx) {
       pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_IfaceSubWindows);
   }
   return winRef;
}


//____________________________________________
void __declspec(naked) h_iface_skill_win(void) {

   __asm {
	  push dword ptr ss:[esp+0x8]
	  push dword ptr ss:[esp+0x8]

      push ecx
      push ebx
      push edx
      push eax
      call IfaceSkillWin
      add esp, 0x18
      ret 0x8
   }

}


//_____________________________________________
void __declspec(naked) h_iface_skill_kill(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call fall_Win_Destroy
        add esp, 0x4
        mov eax, pWinRef_Skills
        mov dword ptr ds : [eax] , -1

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//__________________________________
FRMdx* Get_Iface_Background_Image() {
    char path[128];
    char s_region[6] = { 0,0,0,0,0,0 };

    if (fallout_exe_region == EXE_Region::Chinese)
        sprintf_s(s_region, _countof(s_region), "CHI_");

    char s_ACPBar[4] = { 0, 0, 0, 0 };
    bool sfall_ActionPointsBar = false;

    //Make thing work with sfalls "ActionPointsBar" mod.
    if (SfallReadInt(L"Interface", L"ActionPointsBar", 0) != 0)
        sfall_ActionPointsBar = true, sprintf_s(s_ACPBar, _countof(s_ACPBar), "E");


    FRMdx* pfrmDx = nullptr;
    FRMframeDx* pFrame = nullptr;
    sprintf_s(path, _countof(path), "HR_IFACE_%s%i%s", s_region, IFACE_BAR_GRAPHIC_WIDTH, s_ACPBar);
    pfrmDx = new FRMdx(path, ART_INTRFACE, -1);
    pFrame = pfrmDx->GetFrame(0, 0);
    if (!pFrame) {
        delete pfrmDx;
        pfrmDx = nullptr;

        if (sfall_ActionPointsBar)
            sprintf_s(s_ACPBar, _countof(s_ACPBar), "_E");
        sprintf_s(path, _countof(path), "IFACE%s", s_ACPBar);
        pfrmDx = new FRMdx(path, ART_INTRFACE, -1);
        pFrame = pfrmDx->GetFrame(0, 0);
    }

    if (!pFrame) {
        delete pfrmDx;
        pfrmDx = nullptr;
        
    }
    return pfrmDx;
}


//______________________________________
DWORD Get_Iface_Background_Image_Width() {
    FRMdx* pfrmDx = Get_Iface_Background_Image();
    FRMframeDx* pFrame = nullptr;
    DWORD width = 0;
    if (pfrmDx)
        pFrame = pfrmDx->GetFrame(0, 0);
    if (pFrame)
        width =  pFrame->GetWidth();

    pFrame = nullptr;
    if(pfrmDx)
        delete pfrmDx;
    pfrmDx = nullptr;

    return width;
}


//________________________________________________
bool Draw_Iface_Background_Image(Window_DX* winDx) {

    FRMdx* pfrmDx = Get_Iface_Background_Image();
    FRMframeDx* pFrame = nullptr;
    if (pfrmDx)
        pFrame = pfrmDx->GetFrame(0, 0);

    if (pFrame) {
        //If graphic width is longer than the iface window - draw so that right side of graphic and window align, left side of  graphic will be cut off.
        winDx->RenderTargetDrawFrame((float)((LONG)IFACE_BAR_WIDTH - (LONG)IFACE_BAR_GRAPHIC_WIDTH), 0, pFrame, nullptr, nullptr);

        //Draw part of the left side of graphic aligned to the left side of window - to create a shorter iMonitor background.
        if (IFACE_BAR_WIDTH < IFACE_BAR_GRAPHIC_WIDTH) {
            RECT scissorRect = { 0,0,80,(LONG)winDx->GetHeight() - 1 };
            winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, &scissorRect);
        }
    }
    if (pfrmDx)
        delete pfrmDx;
    pfrmDx = nullptr;

    if (!pFrame)
        return false;

    pFrame = nullptr;
    return true;
}


//_____________________________________________
bool LoadAndDrawIfaceBarSides(Window_DX* winDx) {

    static bool no_art_found_left_a = false;//if initial art loading failed, stop trying to load art when resizing.
    static bool no_art_found_left_b = false;
    static bool no_art_found_right_a = false;
    static bool no_art_found_right_b = false;

    DWORD backGround_colour = 0x00000000;
    
    if (IFACE_BAR_SIDE_ART <= 0) {
        no_art_found_left_a = true;//if initial art loading failed, stop trying to load art when resizing.
        no_art_found_left_b = true;
        no_art_found_right_a = true;
        no_art_found_right_b = true;
        backGround_colour = IFACE_BG_COLOUR;
    }

    char path[128];
    if (IFACE_BAR_WIDTH < SCR_WIDTH) {
        FRMdx* pfrmDx = nullptr;
        FRMframeDx* pFrame = nullptr;
 
 
        float f_iface_x = 0.0f;
        float f_iface_y = 0.0f;
        winDx->GetPosition(&f_iface_x, &f_iface_y);

        DWORD iface_h = winDx->GetHeight();

        Window_DX* subWin = nullptr;
        if (f_iface_x > 0.0f) {
            if (subWin_num_ifaceLeft_a == -1 && no_art_found_left_a == false) {
                sprintf_s(path, _countof(path), "HR_IFACELFT%d", IFACE_BAR_SIDE_ART);
                pfrmDx = new FRMdx(path, ART_INTRFACE, IFACE_BAR_SIDE_ART);
                pFrame = pfrmDx->GetFrame(0, 0);
                if (pFrame) {
                    subWin = new Window_DX(0, 0, pFrame->GetWidth(), iface_h, backGround_colour, winDx, &subWin_num_ifaceLeft_a);
                    subWin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
                }
                else {
                    subWin = nullptr;
                    no_art_found_left_a = true;
                }
                delete pfrmDx;
                pfrmDx = nullptr;
                pFrame = nullptr;
            }
            else
                subWin = winDx->GetSubWin(subWin_num_ifaceLeft_a);
            DWORD ifaceLeft_w = 0;
            if (subWin) {
                ifaceLeft_w = subWin->GetWidth();
                subWin->SetPosition(-(float)ifaceLeft_w, 0);
            }
            LONG ext_w = (LONG)f_iface_x - ifaceLeft_w;
            if (ext_w > 0) {
                bool redraw = false;
                if (subWin_num_ifaceLeft_b == -1) {
                    subWin = new Window_DX(0, 0, (DWORD)ext_w, iface_h, backGround_colour, winDx, &subWin_num_ifaceLeft_b);
                    redraw = true;
                }
                else
                    subWin = winDx->GetSubWin(subWin_num_ifaceLeft_b);
                if (subWin) {
                    if ((LONG)subWin->GetWidth() < ext_w) {
                        subWin->ResizeWindow((DWORD)ext_w, iface_h);
                        redraw = true;
                    }
                    if (redraw) {
                        subWin->ClearRenderTarget(nullptr);
                        if (no_art_found_left_b == false) {
                            sprintf_s(path, _countof(path), "HR_IFACELFT_EXT%d", IFACE_BAR_SIDE_ART);
                            pfrmDx = new FRMdx(path, ART_INTRFACE, IFACE_BAR_SIDE_ART);
                            pFrame = pfrmDx->GetFrame(0, 0);
                            if (pFrame) {
                                int num_draws = subWin->GetWidth() / pFrame->GetWidth() + 1;
                                LONG posX = subWin->GetWidth() - pFrame->GetWidth();
                                for (int i = 0; i < num_draws; i++) {
                                    subWin->RenderTargetDrawFrame((float)posX, 0, pFrame, nullptr, nullptr);
                                    posX -= pFrame->GetWidth();
                                }
                            }
                            else {
                                no_art_found_left_b = true;
                                subWin->SetBackGroungColour(IFACE_BG_COLOUR);
                                subWin->ClearRenderTarget(nullptr);
                            }
                            delete pfrmDx;
                            pfrmDx = nullptr;
                            pFrame = nullptr;
                        }

                    }
                    subWin->SetPosition(-(float)(ifaceLeft_w + subWin->GetWidth()), 0);
                }
            }
        }
        
        if (f_iface_x + winDx->GetWidth() < SCR_WIDTH) {
            if (subWin_num_ifaceRight_a == -1 && no_art_found_right_a == false) {
                sprintf_s(path, _countof(path), "HR_IFACERHT%d", IFACE_BAR_SIDE_ART);
                pfrmDx = new FRMdx(path, ART_INTRFACE, IFACE_BAR_SIDE_ART);
                pFrame = pfrmDx->GetFrame(0, 0);
                if (pFrame) {
                    subWin = new Window_DX(0, 0, pFrame->GetWidth(), pFrame->GetHeight(), backGround_colour, winDx, &subWin_num_ifaceRight_a);
                    subWin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
                }
                else {
                    subWin = nullptr;
                    no_art_found_right_a = true;
                }
                delete pfrmDx;
                pfrmDx = nullptr;
                pFrame = nullptr;
            }
            else
                subWin = winDx->GetSubWin(subWin_num_ifaceRight_a);
            DWORD ifaceRight_w = 0;
            if (subWin) {
                subWin->SetPosition((float)winDx->GetWidth(), 0);
                ifaceRight_w = subWin->GetWidth();
            }
            LONG ext_w = (LONG)SCR_WIDTH - ifaceRight_w - winDx->GetWidth() - (LONG)f_iface_x;
            if (ext_w > 0) {
                bool redraw = false;
                if (subWin_num_ifaceRight_b == -1) {
                    subWin = new Window_DX(0, 0, (DWORD)ext_w, iface_h, backGround_colour, winDx, &subWin_num_ifaceRight_b);
                    redraw = true;
                }
                else
                    subWin = winDx->GetSubWin(subWin_num_ifaceRight_b);

                if (subWin) {
                    if ((LONG)subWin->GetWidth() < ext_w) {
                        subWin->ResizeWindow((DWORD)ext_w, iface_h);
                        redraw = true;
                    }
                    if (redraw) {
                        subWin->ClearRenderTarget(nullptr);
                        if (no_art_found_right_b == false) {
                            sprintf_s(path, _countof(path), "HR_IFACERHT_EXT%d", IFACE_BAR_SIDE_ART);
                            pfrmDx = new FRMdx(path, ART_INTRFACE, IFACE_BAR_SIDE_ART);
                            pFrame = pfrmDx->GetFrame(0, 0);
                            if (pFrame) {
                                int num_draws = subWin->GetWidth() / pFrame->GetWidth() + 1;
                                for (int i = 0; i < num_draws; i++) {
                                    subWin->RenderTargetDrawFrame((float)i * pFrame->GetWidth(), 0, pFrame, nullptr, nullptr);
                                }
                            }
                            else {
                                no_art_found_right_b = true;
                                subWin->SetBackGroungColour(IFACE_BG_COLOUR);
                                subWin->ClearRenderTarget(nullptr);
                            }
                            delete pfrmDx;
                            pfrmDx = nullptr;
                            pFrame = nullptr;
                        }
                    }
                    subWin->SetPosition((float)winDx->GetWidth() + ifaceRight_w, 0);
                }
            }
        }
    }
    return true;
}


//_____________________________________________
void OnScreenResize_Iface(Window_DX* pWin_This) {

    if (!pWin_This)
        return;

    if (*pWinRef_Iface == -1)
        return;
    WinStructDx* ifaceWin = pWin_This->Get_FalloutParent();
    if (ifaceWin == nullptr)
        return;

    IFACE_BAR_WIDTH = IFACE_BAR_GRAPHIC_WIDTH;
    if (IFACE_BAR_WIDTH > SCR_WIDTH)
        IFACE_BAR_WIDTH = SCR_WIDTH;
    LONG winX = 0;
    LONG winY = SCR_HEIGHT - ifaceWin->height;

    if (IFACE_BAR_LOCATION == 0)
        winX = ((LONG)SCR_WIDTH - (LONG)IFACE_BAR_WIDTH) / 2;
    else if (IFACE_BAR_LOCATION == 2)
        winX = (LONG)SCR_WIDTH - (LONG)IFACE_BAR_WIDTH;

    if (SCR_WIDTH < IFACE_BAR_WIDTH)
        IFACE_BAR_WIDTH = SCR_WIDTH;

    if (ifaceWin->width == IFACE_BAR_WIDTH) 
        MoveWindowX(ifaceWin, winX, winY);
    else {
        DWORD oldIfaceWidth = ifaceWin->width;
        //Resize and position Main Window
        ResizeWindowX(ifaceWin, winX, winY, IFACE_BAR_WIDTH, ifaceWin->height);
   
        icombatRect->left = 580 + ifaceWin->width - 640;
        icombatRect->right = 637 + ifaceWin->width - 640;

        iactionRect->left = 316 + ifaceWin->width - 640;
        iactionRect->right = 406 + ifaceWin->width - 640;

        iitemSlotRect->left = 267 + ifaceWin->width - 640;
        iitemSlotRect->right = 455 + ifaceWin->width - 640;

        Draw_Iface_Background_Image(ifaceWin->winDx);

        ButtonStruct_DX* button = nullptr;
        //Resize iMonitor
        Window_DX* winDx = ifaceWin->winDx->GetSubWin(subWin_num_iMonitor);
        if (winDx) {
            imonitorRect->right = 189 + ifaceWin->width - 640;

            imonitorAreaX = 23;
            imonitorAreaWidth = imonitorRect->right - imonitorRect->left + 1;

            winDx->ResizeWindow(imonitorAreaWidth, imonitorAreaHeight);

            *p_imonitor_num_visible_lines = imonitorAreaHeight / imonitorTxtHeight;
            *p_imonitor_line_last = 0;
            *p_imonitor_line_view = 0;

            if (*p_buttRef_imonitor_up != -1) {
                button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_imonitor_up, nullptr);
                if (button)
                    button->rect.right = imonitorRect->right;
            }
            if (*p_buttRef_imonitor_down != -1) {
                button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_imonitor_down, nullptr);
                if (button)
                    button->rect.right = imonitorRect->right;
            }
            if (*p_is_imonitor) {
                imonitorCleatText();
                imonitorPrintText();
            }
        }
        //Move other sub windows
        float old_x = 0;
        float old_y = 0;
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_AmmoBar);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_APBar);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_CombatBox);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_CombatBoxLights);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_HPCounter);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }
        winDx = ifaceWin->winDx->GetSubWin(subWin_num_ACCounter);
        if (winDx) {
            winDx->GetPosition(&old_x, &old_y);
            winDx->SetPosition(ifaceWin->width - (oldIfaceWidth - old_x), old_y);
        }

        //Move other buttons
        if (*p_buttRef_ifaceInv != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceInv, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 211);
                button->rect.right = button->rect.left + 32 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceOptions != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceOptions, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 210);
                button->rect.right = button->rect.left + 34 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceSkills != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceSkills, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 523);
                button->rect.right = button->rect.left + 22 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceMap != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceMap, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 526);
                button->rect.right = button->rect.left + 41 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifacePip != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifacePip, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 526);
                button->rect.right = button->rect.left + 41 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceChar != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceChar, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 526);
                button->rect.right = button->rect.left + 41 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceAttack != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceAttack, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 267);
                button->rect.right = button->rect.left + 188 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceSwitchHands != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceSwitchHands, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 218);
                button->rect.right = button->rect.left + 22 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceSwitchTurn != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceSwitchTurn, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 590);
                button->rect.right = button->rect.left + 38 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
        if (*p_buttRef_ifaceEndCombat != -1) {
            button = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceEndCombat, nullptr);
            if (button) {
                button->rect.left = ifaceWin->width - (640 - 590);
                button->rect.right = button->rect.left + 38 - 1;
                button->buttDx->SetPosition((float)button->rect.left, (float)button->rect.top);
            }
        }
    }
    LoadAndDrawIfaceBarSides(ifaceWin->winDx);

    //move notify bar
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_NotifyBar);
    if (pWin)
        MoveWindowX(pWin, ifaceWin->rect.left, ifaceWin->rect.top - pWin->height);
    //move skills window
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Skills);
    if (pWin)
        MoveWindowX(pWin, ifaceWin->rect.right - pWin->width - 3, ifaceWin->rect.top - pWin->height - 6);
 
    return;
}


//_____________________
void Iface_Destructor() {
    if (*pWinRef_Iface == -1)
        return;
    //imonitor buttons aren't destroyed in the fallout destructor.
    if (*p_buttRef_imonitor_up != -1)
        fall_Button_Destroy(*p_buttRef_imonitor_up);
    *p_buttRef_imonitor_up = -1;
    if (*p_buttRef_imonitor_down != -1)
        fall_Button_Destroy(*p_buttRef_imonitor_down);
    *p_buttRef_imonitor_down = -1;
    *p_is_imonitor = FALSE;

    if (*p_buttRef_ifaceInv != -1)
        fall_Button_Destroy(*p_buttRef_ifaceInv);
    *p_buttRef_ifaceInv = -1;
    if (*p_buttRef_ifaceOptions != -1)
        fall_Button_Destroy(*p_buttRef_ifaceOptions);
    *p_buttRef_ifaceOptions = -1;
    if (*p_buttRef_ifaceSkills != -1)
        fall_Button_Destroy(*p_buttRef_ifaceSkills);
    *p_buttRef_ifaceSkills = -1;
    if (*p_buttRef_ifaceMap != -1)
        fall_Button_Destroy(*p_buttRef_ifaceMap);
    *p_buttRef_ifaceMap = -1;
    if (*p_buttRef_ifacePip != -1)
        fall_Button_Destroy(*p_buttRef_ifacePip);
    *p_buttRef_ifacePip = -1;
    if (*p_buttRef_ifaceChar != -1)
        fall_Button_Destroy(*p_buttRef_ifaceChar);
    *p_buttRef_ifaceChar = -1;
    if (*p_buttRef_ifaceAttack != -1)
        fall_Button_Destroy(*p_buttRef_ifaceAttack);
    *p_buttRef_ifaceAttack = -1;
    if (*p_buttRef_ifaceSwitchHands != -1)
        fall_Button_Destroy(*p_buttRef_ifaceSwitchHands);
    *p_buttRef_ifaceSwitchHands = -1;

    //these buttons should already be destroyed but lets check anyway.
    if (*p_buttRef_ifaceSwitchTurn != -1)
        fall_Button_Destroy(*p_buttRef_ifaceSwitchTurn);
    *p_buttRef_ifaceSwitchTurn = -1;
    if (*p_buttRef_ifaceEndCombat != -1)
        fall_Button_Destroy(*p_buttRef_ifaceEndCombat);
    *p_buttRef_ifaceEndCombat = -1;


    fall_Win_Destroy(*pWinRef_Iface);
    *pWinRef_Iface = -1;
    //set all dx sub win refs to -1
    subWin_num_iMonitor = -1;
    subWin_num_AmmoBar = -1;
    subWin_num_APBar = -1;
    subWin_num_CombatBox = -1;
    subWin_num_CombatBoxLights = -1;
    subWin_num_HPCounter = -1;
    subWin_num_ACCounter = -1;

    subWin_num_ifaceLeft_a = -1;
    subWin_num_ifaceLeft_b = -1;
    subWin_num_ifaceRight_a = -1;
    subWin_num_ifaceRight_b = -1;

    fall_NotifyBar_Destructor();
}


//_____________________________________________
void __declspec(naked) h_iface_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Iface_Destructor

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//________________
LONG Iface_Setup() {
    if (*pWinRef_Iface != -1)
        return -1;

    *p_is_iface_initilizing = TRUE;

    IFACE_BAR_GRAPHIC_WIDTH = ConfigReadInt(L"IFACE", L"IFACE_BAR_WIDTH", 640);

    DWORD bgImage_width = Get_Iface_Background_Image_Width();
    if (IFACE_BAR_GRAPHIC_WIDTH != bgImage_width) {
        Fallout_Debug_Error("Iface_Setup - ini IFACE_BAR_GRAPHIC_WIDTH %d doesn't match the loaded image width %d.", IFACE_BAR_GRAPHIC_WIDTH, bgImage_width);
        IFACE_BAR_GRAPHIC_WIDTH = bgImage_width;
    }
    IFACE_BAR_WIDTH = IFACE_BAR_GRAPHIC_WIDTH;

    if (SCR_WIDTH < IFACE_BAR_WIDTH)
        IFACE_BAR_WIDTH = SCR_WIDTH;

    LONG winX = 0;
    LONG winY = (LONG)SCR_HEIGHT - (LONG)IFACE_BAR_HEIGHT;

    if (IFACE_BAR_LOCATION == 0)
        winX = ((LONG)SCR_WIDTH - (LONG)IFACE_BAR_WIDTH) / 2;
    else if (IFACE_BAR_LOCATION == 2)
        winX = (LONG)SCR_WIDTH - (LONG)IFACE_BAR_WIDTH;

    *pWinRef_Iface = fall_Win_Create(winX, winY, IFACE_BAR_WIDTH, IFACE_BAR_HEIGHT, 0, 0x8);
    if (*pWinRef_Iface == -1) {
        Iface_Destructor();
        return -1;
    }
    WinStructDx* pWin = nullptr;
    pWin = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);

    *p_buff_ifaceWinBuff = pWin->buff;


    if (!pWin || !pWin->winDx) {
        Iface_Destructor();
        return -1;
    }

    icombatRect->left = 580 + pWin->width - 640;
    icombatRect->right = 637 + pWin->width - 640;

    imonitorRect->right = 189 + pWin->width - 640;

    iactionRect->left = 316 + pWin->width - 640;
    iactionRect->right = 406 + pWin->width - 640;

    iitemSlotRect->left = 267 + pWin->width - 640;
    iitemSlotRect->right = 455 + pWin->width - 640;



    if (!Draw_Iface_Background_Image(pWin->winDx)) {
        Fallout_Debug_Error("Iface_Setup - iface load backGround image failed");
        Iface_Destructor();
        return -1;
    }
    LoadAndDrawIfaceBarSides(pWin->winDx);

    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Iface);
    FRMframeDx* pFrame = nullptr;

    ButtonStruct_DX* butt = nullptr;

    //get the width and height of the up button
    FRMCached* frmMenuUp = new FRMCached(0x0600002F);
    pFrame = frmMenuUp->GetFrame(0, 0);

    DWORD buttWidth = pFrame->GetWidth();
    DWORD buttHeight = pFrame->GetHeight();
    delete frmMenuUp;
    frmMenuUp = nullptr;
    pFrame = nullptr;


    *p_buttRef_ifaceInv = CreateButtonX(*pWinRef_Iface, 211 + pWin->width - 640, 40, 32, 21, -1, -1, -1, 105, 0x0600002F, 0x0600002E, 0, 0);
    SetButtonSoundsX(*p_buttRef_ifaceInv, &button_sound_Dn_2, &button_sound_Up_2);


    *p_buttRef_ifaceOptions = CreateButtonX(*pWinRef_Iface, 210 + pWin->width - 640, 62, 34, 34, -1, -1, -1, 111, 0x06000012, 0x06000011, 0, 0);
    SetButtonSoundsX(*p_buttRef_ifaceOptions, &button_sound_Dn_2, &button_sound_Up_2);


    *p_buttRef_ifaceSkills = CreateButtonX(*pWinRef_Iface, 523 + pWin->width - 640, 6, 22, 21, -1, -1, -1, 115, 0x06000006, 0x06000007, 0, FLG_ButtTrans);
    SetButtonPointerMaskX(*p_buttRef_ifaceSkills, 0x06000006);
    SetButtonSoundsX(*p_buttRef_ifaceSkills, &button_sound_Dn_2, &button_sound_Up_2);

    *p_buttRef_ifaceMap = CreateButtonX(*pWinRef_Iface, 526 + pWin->width - 640, 40, 41, 19, -1, -1, -1, 9, 0x0600000D, 0x0600000A, 0, FLG_ButtTrans);
    SetButtonPointerMaskX(*p_buttRef_ifaceMap, 0x0600000D);
    SetButtonSoundsX(*p_buttRef_ifaceMap, &button_sound_Dn_2, &button_sound_Up_2);

    *p_buttRef_ifacePip = CreateButtonX(*pWinRef_Iface, 526 + pWin->width - 640, 78, 41, 19, -1, -1, -1, 112, 0x0600003B, 0x0600003A, 0, 0);
    SetButtonPointerMaskX(*p_buttRef_ifacePip, 0x0600003B);
    SetButtonSoundsX(*p_buttRef_ifacePip, &button_sound_Dn_2, &button_sound_Up_2);

    *p_buttRef_ifaceChar = CreateButtonX(*pWinRef_Iface, 526 + pWin->width - 640, 59, 41, 19, -1, -1, -1, 99, 0x06000039, 0x06000038, 0, 0);
    SetButtonPointerMaskX(*p_buttRef_ifaceChar, 0x06000039);
    SetButtonSoundsX(*p_buttRef_ifaceChar, &button_sound_Dn_2, &button_sound_Up_2);

    *p_buttRef_ifaceAttack = CreateButtonX_Overlay(*pWinRef_Iface, 267 + pWin->width - 640, 26, 188, 67, -1, -1, -1, -20, 0x06000020, 0x0600001F, 0, FLG_ButtTrans);

    SetButtonRightClickX(*p_buttRef_ifaceAttack, -1, 110, nullptr, nullptr);
    SetButtonSoundsX(*p_buttRef_ifaceAttack, &button_sound_Dn_3, &button_sound_Up_3);

    *p_buttRef_ifaceSwitchHands = CreateButtonX(*pWinRef_Iface, 218 + pWin->width - 640, 6, 22, 21, -1, -1, -1, 98, 0x06000006, 0x06000007, 0, FLG_ButtTrans);
    SetButtonPointerMaskX(*p_buttRef_ifaceSwitchHands, 0x06000006);
    SetButtonSoundsX(*p_buttRef_ifaceSwitchHands, &button_sound_Dn_2, &button_sound_Up_2);




    fall_NotifyBar_Setup();

    *p_iface_active_slot = 0;
    ///p_iface_wielded1->pObj=(OBJStruct*)-1;
    ///p_iface_wielded2->pObj=(OBJStruct*)-1;
    //*p_iface_wielded1=-1;//(OBJStruct*)-1;
    //*p_iface_wielded2=-1;//(OBJStruct*)-1;
    //p_iface_wielded[0].pObj=(OBJStruct*)-1;// this is how its set in fallout.exe - why?
    //p_iface_wielded[1].pObj=(OBJStruct*)-1;

    //this makes more sense to me
    p_iface_wielded[0].pObj = nullptr;
    p_iface_wielded[1].pObj = nullptr;
    p_iface_wielded[0].frmID = -1;
    p_iface_wielded[1].frmID = -1;


    iMonitor_Setup();///-------------------------------

    *p_is_iface_initilizing = FALSE;
    *p_is_iface_enabled = TRUE;
    *p_is_iface_hidden = TRUE;
    return 0;
}


//________________________________________
void __declspec(naked) h_iface_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Iface_Setup

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//_____________________________________________
void Draw_AP_Bar(LONG actionPoints, LONG extra) {

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;
    Window_DX* winDx_APBar = win->winDx->GetSubWin(subWin_num_APBar);
    if (!winDx_APBar) {
        winDx_APBar = new Window_DX((float)(316 + win->width - 640), (float)(14), 90, 5, 0x00000000, win->winDx, &subWin_num_APBar);
        if (!winDx_APBar)
            return;
    }
    winDx_APBar->ClearRenderTarget(nullptr);


    FRMCached* frm_dot = nullptr;
    if (actionPoints == -1) {
        actionPoints = 10;
        extra = 0;
        frm_dot = new FRMCached(0x06000055);//red dot
    }
    else {
        frm_dot = new FRMCached(0x06000053);//green dot
        if (actionPoints < 0)
            actionPoints = 0;
        else if (actionPoints > 10)
            actionPoints = 10;
        if (extra > 0)
            extra = 0;
        else {
            if (actionPoints + extra > 10)
                extra = 10 - actionPoints;
        }
    }

    FRMframeDx* pFrame = frm_dot->GetFrame(0, 0);

    for (int xPos = 0; xPos < actionPoints * 9; xPos += 9)
        winDx_APBar->RenderTargetDrawFrame((float)xPos, (float)0, pFrame, nullptr, nullptr);

    delete frm_dot;
    frm_dot = nullptr;
    pFrame = nullptr;

    if (extra <= 0) {
        //dxPresentFlag = true;
        return;
    }


    frm_dot = new FRMCached(0x06000054);//yellow dot

    pFrame = frm_dot->GetFrame(0, 0);

    for (int xPos = actionPoints * 9; xPos < actionPoints * 9 + extra * 9; xPos += 9)
        winDx_APBar->RenderTargetDrawFrame((float)xPos, (float)0, pFrame, nullptr, nullptr);

    delete frm_dot;
    frm_dot = nullptr;
    pFrame = nullptr;

    //imonitorInsertText("drew ap bar");
}



//________________________________________
void __declspec(naked) h_draw_ap_bar(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Draw_AP_Bar
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


//_________________________________________
DWORD CanItemBeUsedOnSomething(DWORD proID) {
    PROTO* pPro = nullptr;
    if (fall_GetPro(proID, &pPro) == -1)
        return 0;
    if ((pPro->item.actionFlags & FLG_UseOn))
        return FLG_UseOn;
    if ((proID & 0xFF000000) != 0)//if ProID obj type is not an item "item objects == 0"
        return 0;
    if (pPro->item.itemType == ITEM_TYPE::drug)
        return 1;
    return 0;
}


//______________________________
DWORD CanItemBeUsed(DWORD proID) {
    PROTO* pPro = nullptr;
    if (fall_GetPro(proID, &pPro) == -1)
        return 0;
    if ((pPro->item.actionFlags & FLG_Use))
        return FLG_Use;
    if ((proID & 0xFF000000) != 0)//if ProID obj type is not an item "item objects == 0"
        return 0;
    if (pPro->item.itemType == ITEM_TYPE::container)
        return 1;
    return 0;//pPro->item.itemType;
}


//______________________________________
DWORD CanItemBeUsed_Obj(OBJStruct* pObj) {
    if (!pObj)
        return 0;
    if (pObj->proID == 0xCD || pObj->proID == 0xCE || pObj->proID == 0xD1)
        return 0;
    return CanItemBeUsed(pObj->proID);
}


//________________________
LONG Draw_Wielded_Button() {
    if (*pWinRef_Iface == -1)
        return -1;
    if (*p_buttRef_ifaceAttack == -1)
        return -1;

    ButtonStruct_DX* butt = (ButtonStruct_DX*)fall_Button_Get(*p_buttRef_ifaceAttack, nullptr);
    if (butt == nullptr || butt->buttDx == nullptr)
        return -1;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    DirectX::XMMATRIX Ortho2D;
    butt->buttDx->GetOrthoMatrix(&Ortho2D);
    SetButtonEnabledX(*p_buttRef_ifaceAttack);

    LONG mpCost = -1;
    FRMCached* cfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    butt->buttDx->Overlay_CreateTexture(0, 0, 1, -2);
    butt->buttDx->ClearRenderTarget(nullptr);
    DWORD buttWidth = butt->buttDx->GetWidth();
    DWORD buttHeight = butt->buttDx->GetHeight();
    if (!(p_iface_wielded[*p_iface_active_slot].typeFlags & 0x000000FF)) {//if item does nothing flag not set

        if ((p_iface_wielded[*p_iface_active_slot].typeFlags & 0x0000FF00)) {//if item used as weapon flag set
            LONG attackType = -1;
            bool isCalledShot = 0;
            switch (p_iface_wielded[*p_iface_active_slot].rClickNum) {
            case 2:
                isCalledShot = 1;
            case 1:
                attackType = p_iface_wielded[*p_iface_active_slot].attackType1;
                break;
            case 4:
                isCalledShot = 1;
            case 3:
                attackType = p_iface_wielded[*p_iface_active_slot].attackType2;
                break;
            case 5:
                mpCost = fall_Obj_GetHeldItemMovementPointCost_WeaponCheck(*ppObj_PC, *p_iface_active_slot + 6, 0);
                cfrm = new FRMCached(0x06000123);//RELOAD.FRM     ; reload text
                break;
            default:
                cfrm = nullptr;
                break;
            }
            if (isCalledShot && !cfrm) {//draw called shot bullseye
                cfrm = new FRMCached(0x06000120);//BULLSEYE.FRM   ; bullseye for interface button
                if (cfrm) {
                    pFrame = cfrm->GetFrame(0, 0);
                    butt->buttDx->SetOverlayRenderTarget();
                    pFrame->DrawFrame(&Ortho2D, (float)(buttWidth - 7 - pFrame->GetWidth()), (float)(buttHeight - 7 - pFrame->GetHeight()));
                    delete cfrm;
                    cfrm = nullptr;
                    pFrame = nullptr;
                }
            }
            if (attackType != -1) {
                mpCost = fall_Obj_GetHeldItemMovementPointCost(*ppObj_PC, attackType, isCalledShot);
                LONG anim = fall_Obj_GetHeldItemAnimation(*ppObj_PC, attackType);
                DWORD frmID = -1;
                if (anim == 0x29)
                    frmID = 0x0600002D;//THRUST.FRM     ; thrust text
                else if (anim == 0x2D)
                    frmID = 0x0600002B;//SINGLE.FRM     ; single text
                else if (anim == 0x2E || anim == 0x2F)
                    frmID = 0x06000028;//BURST.FRM      ; burst text
                else if (anim == 0x2A)
                    frmID = 0x0600002C;//SWING.FRM      ; swing text
                else if (anim == 0x12)
                    frmID = 0x06000075;//throw.frm      ; throw text
                else if (anim == 0x11) {
                    switch (attackType) {
                    case ATTACK_TYPE::strong_kick:
                        frmID = 0x060001AE;//skick.frm      ; strong kick text
                        break;
                    case ATTACK_TYPE::snap_kick:
                        frmID = 0x060001AF;//snapkick.frm   ; snap kick text
                        break;
                    case ATTACK_TYPE::power_kick:
                        frmID = 0x060001AD;//cm_pwkck.frm   ; roundk.frm     ; roundhouse kick text
                        break;
                    case ATTACK_TYPE::hip_kick:
                        frmID = 0x060001AA;//hipk.frm       ; hip kick text
                        break;
                    case ATTACK_TYPE::hook_kick:
                        frmID = 0x060001AB;//cm_hookk.frm   ; jumpk.frm      ; jump kick text
                        break;
                    case ATTACK_TYPE::piercing_kick:
                        frmID = 0x060001A6;//cm_prckk.frm   ; dblossk.frm    ; death blossom kick text
                        break;
                    default:
                        frmID = 0x06000029;//KICK.FRM       ; kick text
                        break;
                    }
                }
                else if (anim == 0x10) {
                    switch (attackType) {
                    case ATTACK_TYPE::strong_punch:
                        frmID = 0x060001B0;//spunch.frm     ; strong punch text
                        break;
                    case ATTACK_TYPE::hammer_punch:
                        frmID = 0x060001A9;//hampnch.frm    ; hammer punch text
                        break;
                    case ATTACK_TYPE::hay_maker:
                        frmID = 0x060001AC;//cm_hymkr.frm   ; lignpuch.frm   ; lightning punch text
                        break;
                    case ATTACK_TYPE::jab:
                        frmID = 0x060001A5;//cm_jab.frm     ; chopuch.frm    ; chop punch text
                        break;
                    case ATTACK_TYPE::palm_strike:
                        frmID = 0x060001A7;//cm_plmst.frm   ; dragpuch.frm   ; dragon punch text
                        break;
                    case ATTACK_TYPE::piercing_strike:
                        frmID = 0x060001A8;//cm_pstrk.frm   ; forcpuch.frm   ; force punch text
                        break;
                    default:
                        frmID = 0x0600002A;//PUNCH.FRM      ; punch text
                        break;
                    }
                }
                if (frmID != -1 && !cfrm)
                    cfrm = new FRMCached(frmID);
            }

            if (cfrm) {//draw relaoded or attack text
                pFrame = cfrm->GetFrame(0, 0);
                butt->buttDx->SetOverlayRenderTarget();
                pFrame->DrawFrame(&Ortho2D, (float)(buttWidth - 7 - pFrame->GetWidth()), 7.0f);
                delete cfrm;
                cfrm = nullptr;
                pFrame = nullptr;
            }

        }
        else {//if not used as a weapon
            OBJStruct* pObj = p_iface_wielded[*p_iface_active_slot].pObj;

            if (CanItemBeUsedOnSomething(pObj->proID))
                cfrm = new FRMCached(0x06000126);//USEON.FRM      ; use on text
            else if (CanItemBeUsed_Obj(pObj))
                cfrm = new FRMCached(0x06000124);//USET.FRM       ; use text
            if (cfrm) {//draw use/useon text
                pFrame = cfrm->GetFrame(0, 0);
                butt->buttDx->SetOverlayRenderTarget();
                pFrame->DrawFrame(&Ortho2D, (float)(buttWidth - 7 - pFrame->GetWidth()), 7.0f);
                delete cfrm;
                cfrm = nullptr;
                pFrame = nullptr;
                mpCost = fall_Obj_GetHeldItemMovementPointCost_WeaponCheck(*ppObj_PC, p_iface_wielded[*p_iface_active_slot].attackType1, 0);
            }
        }
        if (mpCost >= 0 && mpCost < 10) {
            DWORD mpTextWidth = 0;
            cfrm = new FRMCached(0x06000121);//MVEPNT.FRM     ; movement point text
            if (cfrm) {//draw mp text
                pFrame = cfrm->GetFrame(0, 0);
                mpTextWidth = pFrame->GetWidth();
                butt->buttDx->SetOverlayRenderTarget();
                pFrame->DrawFrame(&Ortho2D, 7.0f, (float)(buttHeight - 7 - pFrame->GetHeight()));
                delete cfrm;
                cfrm = nullptr;
                pFrame = nullptr;
            }
            cfrm = new FRMCached(0x06000122);//MVENUM.FRM     ; movement point numbers
            if (cfrm) {//draw mp numbers

                pFrame = cfrm->GetFrame(0, 0);
                LONG num_x = 7 + mpTextWidth + 5 + 1;
                LONG num_y = buttHeight - 7 - (pFrame->GetHeight());
                butt->buttDx->SetOverlayRenderTarget();
                LONG glyph_width = 10;// (pFrame->GetWidth() + 1) / 10;
                //create a scissor rect to obsure the other numbers in the frame
                D3D11_RECT rect = { num_x,num_y,num_x + glyph_width,num_y + (LONG)pFrame->GetHeight() };
                pD3DDevContext->RSSetScissorRects(1, &rect);
                //shift the x position of the frame so that the correct number aligns in the scissor rect;
                LONG glyph_x = mpCost * glyph_width - 1;
                pFrame->DrawFrame(&Ortho2D, (float)(num_x - glyph_x), (float)(num_y));
                delete cfrm;
                cfrm = nullptr;
                pFrame = nullptr;
            }
        }

    }

    if (p_iface_wielded[*p_iface_active_slot].frmID != -1) {
        cfrm = new FRMCached(p_iface_wielded[*p_iface_active_slot].frmID);
        if (cfrm) {//set item frm and position on button
            pFrame = cfrm->GetFrame(0, 0);
            butt->buttDx->Overlay_SetFrm(p_iface_wielded[*p_iface_active_slot].frmID, (float)((buttWidth - (pFrame->GetWidth())) / 2), (float)((buttHeight - (pFrame->GetHeight())) / 2), 0, 0);
            delete cfrm;
            cfrm = nullptr;
            pFrame = nullptr;
        }
    }
    else
        butt->buttDx->Overlay_SetFrm(0, 0, 0, 0, 0);

    if (!*p_is_iface_initilizing) {
        fall_Iface_DrawAmmoBar();
        if ((p_iface_wielded[*p_iface_active_slot].typeFlags & 0x000000FF))//if item does nothing flag is set
            SetButtonDisabledX(*p_buttRef_ifaceAttack);
        else
            SetButtonEnabledX(*p_buttRef_ifaceAttack);
    }

    return 0;
}


//________________________________________________
void __declspec(naked) h_draw_wielded_button(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call Draw_Wielded_Button


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
bool CombatBox_Create_Buttons() {
    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return false;
    *p_buttRef_ifaceSwitchTurn = CreateButtonX(*pWinRef_Iface, 590 + win->width - 640, 43, 38, 22, -1, -1, -1, 32, 0x06000069, 0x0600006A, 0, 0);
    SetButtonPointerMaskX(*p_buttRef_ifaceSwitchTurn, 0x06000069);
    SetButtonSoundsX(*p_buttRef_ifaceSwitchTurn, &button_sound_Dn_2, &button_sound_Up_2);

    *p_buttRef_ifaceEndCombat = CreateButtonX(*pWinRef_Iface, 590 + win->width - 640, 65, 38, 22, -1, -1, -1, 32, 0x0600006B, 0x0600006C, 0, 0);
    SetButtonPointerMaskX(*p_buttRef_ifaceEndCombat, 0x0600006B);
    SetButtonSoundsX(*p_buttRef_ifaceEndCombat, &button_sound_Dn_2, &button_sound_Up_2);

    return true;
}


//______________________________
bool CombatBox_Destroy_Buttons() {

    fall_Button_Destroy(*p_buttRef_ifaceSwitchTurn);
    *p_buttRef_ifaceSwitchTurn = -1;
    fall_Button_Destroy(*p_buttRef_ifaceEndCombat);
    *p_buttRef_ifaceEndCombat = -1;

    return true;
}


//_____________________________
void CombatBox_Enable_Buttons() {
    if (!*p_is_combat_box_opened)
        return;
    SetButtonEnabledX(*p_buttRef_ifaceSwitchTurn);
    SetButtonEnabledX(*p_buttRef_ifaceEndCombat);

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    FRMCached* cfrm = new FRMCached(0x0600006D);//endltgrn.frm   ; green lights around end turn/combat window
    if (!cfrm)
        return;

    FRMframeDx* pFrame = cfrm->GetFrame(0, 0);
    if (!pFrame) {
        delete cfrm;
        cfrm = nullptr;
        return;
    }
    Window_DX* winDx_CombatBoxLights = win->winDx->GetSubWin(subWin_num_CombatBoxLights);
    if (!winDx_CombatBoxLights) {
        winDx_CombatBoxLights = new Window_DX((float)(580 + win->width - 640), (float)(38), pFrame->GetWidth(), pFrame->GetHeight(), 0x00000000, win->winDx, &subWin_num_CombatBoxLights);
        if (!winDx_CombatBoxLights)
            return;
    }
    fall_PlayAcm("icombat2");

    winDx_CombatBoxLights->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);

    delete cfrm;
    cfrm = nullptr;
    pFrame = nullptr;

}


//___________________________________________________________
void __declspec(naked) h_draw_enable_combat_box_buttons(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call CombatBox_Enable_Buttons

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
void CombatBox_Disable_Buttons() {
    if (!*p_is_combat_box_opened)
        return;
    SetButtonDisabledX(*p_buttRef_ifaceSwitchTurn);
    SetButtonDisabledX(*p_buttRef_ifaceEndCombat);

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    FRMCached* cfrm = new FRMCached(0x0600006E);//endltred.frm   ; red lights around end turn/combat window
    if (!cfrm)
        return;

    FRMframeDx* pFrame = cfrm->GetFrame(0, 0);
    if (!pFrame) {
        delete cfrm;
        cfrm = nullptr;
        return;
    }
    Window_DX* winDx_CombatBoxLights = win->winDx->GetSubWin(subWin_num_CombatBoxLights);
    if (!winDx_CombatBoxLights) {
        winDx_CombatBoxLights = new Window_DX((float)(580 + win->width - 640), (float)(38), pFrame->GetWidth(), pFrame->GetHeight(), 0x00000000, win->winDx, &subWin_num_CombatBoxLights);
        if (!winDx_CombatBoxLights)
            return;
    }

    fall_PlayAcm("icombat1");

    winDx_CombatBoxLights->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);

    delete cfrm;
    cfrm = nullptr;
    pFrame = nullptr;

}


//____________________________________________________________
void __declspec(naked) h_draw_disable_combat_box_buttons(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call CombatBox_Disable_Buttons

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
void Draw_CombatBox_Open(LONG isAnimated) {
    if (*p_is_combat_box_opened)
        return;

    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    FRMCached* cfrm_CombatBox = new FRMCached(0x06000068);//endanim.frm    ; endturn window open/close animation
    if (!cfrm_CombatBox)
        return;

    FRMframeDx* pFrame = cfrm_CombatBox->GetFrame(0, 0);
    if (!pFrame) {
        delete cfrm_CombatBox;
        cfrm_CombatBox = nullptr;
        return;
    }

    Window_DX* winDx_CombatBox = win->winDx->GetSubWin(subWin_num_CombatBox);
    if (!winDx_CombatBox) {
        winDx_CombatBox = new Window_DX((float)(580 + win->width - 640), (float)(38), pFrame->GetWidth(), pFrame->GetHeight(), 0x00000000, win->winDx, &subWin_num_CombatBox);
        if (!winDx_CombatBox)
            return;
        winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);
    }

    FRMdx* pFrmDx = cfrm_CombatBox->GetFrm();
    if (!pFrmDx) {
        delete cfrm_CombatBox;
        cfrm_CombatBox = nullptr;
        pFrame = nullptr;
        return;
    }
    WORD numFrames = pFrmDx->GetNumFrames();
    WORD frameTime = 1000 / pFrmDx->GetFPS();
    pFrmDx = nullptr;

    fall_PlayAcm("iciboxx1");

    if (isAnimated) {
        ULONGLONG count = GetTickCount64();
        ULONGLONG lastFrameCount = count + frameTime;
        LONG frameNum = 0;

        while (frameNum < numFrames) {
            CheckMessages();
            fall_Process_Map_Mouse();
            count = GetTickCount64();
            if (count >= lastFrameCount) {
                lastFrameCount = count + frameTime;
                pFrame = cfrm_CombatBox->GetFrame(0, frameNum);
                winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);
                frameNum++;

            }
        }
    }
    else {
        pFrame = cfrm_CombatBox->GetFrame(0, numFrames - 1);
        winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);

    }

    *p_is_combat_box_opened = TRUE;

    delete cfrm_CombatBox;
    cfrm_CombatBox = nullptr;
    pFrame = nullptr;

    CombatBox_Create_Buttons();
    CombatBox_Disable_Buttons();
}


//_________________________________________________
void __declspec(naked) h_draw_combat_box_open(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Draw_CombatBox_Open
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
void Draw_CombatBox_Close(LONG isAnimated) {
    if (!*p_is_combat_box_opened)
        return;
    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    FRMCached* cfrm_CombatBox = new FRMCached(0x06000068);//endanim.frm    ; endturn window open/close animation
    if (!cfrm_CombatBox)
        return;

    FRMframeDx* pFrame = cfrm_CombatBox->GetFrame(0, 0);
    if (!pFrame) {
        delete cfrm_CombatBox;
        cfrm_CombatBox = nullptr;
        return;
    }

    Window_DX* winDx_CombatBox = win->winDx->GetSubWin(subWin_num_CombatBox);
    if (!winDx_CombatBox) {
        winDx_CombatBox = new Window_DX((float)(580 + win->width - 640), (float)(38), pFrame->GetWidth(), pFrame->GetHeight(), 0x00000000, win->winDx, &subWin_num_CombatBox);
        if (!winDx_CombatBox)
            return;
        winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);
    }

    FRMdx* pFrmDx = cfrm_CombatBox->GetFrm();
    if (!pFrmDx) {
        delete cfrm_CombatBox;
        cfrm_CombatBox = nullptr;
        pFrame = nullptr;
        return;
    }

    CombatBox_Destroy_Buttons();

    WORD numFrames = pFrmDx->GetNumFrames();
    WORD frameTime = 1000 / pFrmDx->GetFPS();
    pFrmDx = nullptr;

    fall_PlayAcm("icibcxx1");

    if (isAnimated) {
        ULONGLONG count = GetTickCount64();
        ULONGLONG lastFrameCount = count + frameTime;
        LONG frameNum = numFrames - 1;

        while (frameNum >= 0) {
            CheckMessages();
            fall_Process_Map_Mouse();
            //CheckMessages_ProccessMapMouse();
            ULONGLONG count = GetTickCount64();
            if (count >= lastFrameCount) {
                lastFrameCount = count + frameTime;
                pFrame = cfrm_CombatBox->GetFrame(0, frameNum);
                winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);
                frameNum--;
            }
        }
    }
    else {
        pFrame = cfrm_CombatBox->GetFrame(0, 0);
        winDx_CombatBox->RenderTargetDrawFrame((float)0, (float)0, pFrame, nullptr, nullptr);
    }

    Window_DX* winDx_CombatBoxLights = win->winDx->GetSubWin(subWin_num_CombatBoxLights);
    if (winDx_CombatBoxLights)//clear combat lights
        winDx_CombatBoxLights->ClearRenderTarget(nullptr);

    *p_is_combat_box_opened = FALSE;

    delete cfrm_CombatBox;
    cfrm_CombatBox = nullptr;
    pFrame = nullptr;

}


//__________________________________________________
void __declspec(naked) h_draw_combat_box_close(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call Draw_CombatBox_Close
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


#define DIGIT_WIDTH  9
#define DIGIT_SIGN_WIDTH  6
#define DIGIT_SIGN_NEG_X  108
#define DIGIT_SIGN_POS_X  108 + DIGIT_SIGN_WIDTH
#define DIGIT_98_POS_X  90
#define DIGIT_43_POS_X  DIGIT_98_POS_X + DIGIT_SIGN_WIDTH
#define DIGIT_BUFF_X_1  DIGIT_SIGN_WIDTH
#define DIGIT_BUFF_X_10  DIGIT_SIGN_WIDTH + DIGIT_WIDTH
#define DIGIT_BUFF_X_100  DIGIT_SIGN_WIDTH + DIGIT_WIDTH + DIGIT_WIDTH
#define DIGIT_BUFF_X_MAX  DIGIT_SIGN_WIDTH + DIGIT_WIDTH + DIGIT_WIDTH + DIGIT_WIDTH
#define NUM_DIGITS  3


//______________________________________________________________________________________________________
LONG NumToCounterOffsets(LONG number, LONG* digit, LONG* digit_x, UINT num_digits, DWORD numColour_xPos) {
    LONG sign = DIGIT_SIGN_POS_X;
    if (number < 0) {
        sign = DIGIT_SIGN_NEG_X;
        number = abs(number);
    }
    sign += numColour_xPos;
    for (UINT u = 0; u < num_digits; u++) {
        digit[u] = number % 10;
        digit_x[u] = digit[u] * DIGIT_WIDTH;
        digit_x[u] += numColour_xPos;
        number /= 10;
    }
    return sign;
}


//________________________________________________________________________________________________________________________________
void DrawCounterDigit(ID3D11DeviceContext* pD3DDevContext, XMMATRIX* pOrtho2D, FRMframeDx* pFrame, D3D11_RECT* pRect, LONG offset) {
    pD3DDevContext->RSSetScissorRects(1, pRect);
    pFrame->DrawFrame(pOrtho2D, (float)(pRect->left - offset), 0.0f);
}


//___________________________________________________________________________________________________________
void DrawCounter(Window_DX* winDx, LONG countCurrent, LONG countEnd, DWORD numColour_xPos, LONG flipTime_ms) {

    if (countEnd > 999)
        countEnd = 999;
    else if (countEnd < -999)
        countEnd = -999;

    LONG digit_43_pos_x = DIGIT_43_POS_X + numColour_xPos;
    LONG digit_98_pos_x = DIGIT_98_POS_X + numColour_xPos;
    LONG digit_x[NUM_DIGITS];
    LONG digit[NUM_DIGITS];
    LONG digit_prev[NUM_DIGITS];

    D3D11_RECT digit_rect[NUM_DIGITS];

    LONG xPos = DIGIT_SIGN_WIDTH + (NUM_DIGITS - 1) * DIGIT_WIDTH;
    for (int i = 0; i < NUM_DIGITS; i++) {
        digit_rect[i] = { xPos , 0, xPos + DIGIT_WIDTH, 17 };
        xPos -= DIGIT_WIDTH;
    }
    //flipTime_ms = 20;
    bool animate = false;
    if (*p_is_iface_initilizing || flipTime_ms <= 0) {
        countCurrent = countEnd;
    }
    else {
        animate = true;
    }

    LONG sign_x = NumToCounterOffsets(countCurrent, digit, digit_x, NUM_DIGITS, numColour_xPos);
    D3D11_RECT signRect = { 0,0,DIGIT_SIGN_WIDTH,17 };

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    DirectX::XMMATRIX Ortho2D;
    winDx->GetOrthoMatrix(&Ortho2D);

    FRMCached* cfrm = new FRMCached(0x06000052);//NUMBERS.FRM    ; numbers for the hit points and fatigue counters
    FRMframeDx* pFrame = cfrm->GetFrame(0, 0);

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    winDx->SetRenderTarget(nullptr);
    DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &signRect, sign_x);
    for (int i = 0; i < NUM_DIGITS; i++) {
        DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect[i], digit_x[i]);
    }

    fall_Update_Mouse_State();
    CheckMessages();
    fall_Process_Map_Mouse();


    bool isDrawingHalfdigit = false;
    if (animate) {
        long diffCount = countEnd - countCurrent;
        while (countCurrent != countEnd) {
            if (countCurrent < countEnd) {
                countCurrent++;
            }
            else {
                countCurrent--;
            }
            memcpy(digit_prev, digit, sizeof(LONG[NUM_DIGITS]));
            sign_x = NumToCounterOffsets(countCurrent, digit, digit_x, 3, numColour_xPos);
            winDx->SetRenderTarget(nullptr);
            DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &signRect, sign_x);

            for (int i = 0; i < NUM_DIGITS; i++) {
                if (digit[i] == 9 && digit_prev[i] == 8 || digit[i] == 8 && digit_prev[i] == 9) {
                    DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect[i], digit_98_pos_x);
                    isDrawingHalfdigit = true;
                }
                else if (digit[i] == 4 && digit_prev[i] == 3 || digit[i] == 3 && digit_prev[i] == 4) {
                    DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect[i], digit_43_pos_x);
                    isDrawingHalfdigit = true;
                }
                if (isDrawingHalfdigit) {
                    fall_Update_Mouse_State();
                    fall_Process_Map_Mouse();
                    Dx_Present_Main();
                    Wait_ms(flipTime_ms);
                    isDrawingHalfdigit = false;
                }
            }

            winDx->SetRenderTarget(nullptr);

            for (int i = 0; i < NUM_DIGITS; i++) {
                DrawCounterDigit(pD3DDevContext, &Ortho2D, pFrame, &digit_rect[i], digit_x[i]);
            }

            fall_Update_Mouse_State();
            fall_Process_Map_Mouse();

            Dx_Present_Main();
            Wait_ms(flipTime_ms);
        }
    }

    delete cfrm;
    cfrm = nullptr;
    pFrame = nullptr;
}


//________________________________________________________________________________________________________
void DrawHPCounter(LONG x, LONG y, LONG lastCount, LONG thisCount, DWORD numColour_xPos, LONG flipTime_ms) {
    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;
    Window_DX* winDx_HPCounter = win->winDx->GetSubWin(subWin_num_HPCounter);
    if (!winDx_HPCounter) {
        DWORD counterWidth = DIGIT_BUFF_X_MAX;
        DWORD counterHeight = 17;
        winDx_HPCounter = new Window_DX((float)(x + win->width - 640), (float)(y), counterWidth, counterHeight, 0x00000000, win->winDx, &subWin_num_HPCounter);
        if (!winDx_HPCounter)
            return;
    }
    winDx_HPCounter->ClearRenderTarget(nullptr);
    DrawCounter(winDx_HPCounter, lastCount, thisCount, numColour_xPos, flipTime_ms);
}


//________________________________________________________________________________________________________
void DrawACCounter(LONG x, LONG y, LONG lastCount, LONG thisCount, DWORD numColour_xPos, LONG flipTime_ms) {
    WinStructDx* win = (WinStructDx*)fall_Win_Get(*pWinRef_Iface);
    if (!win)
        return;
    if (!win->winDx)
        return;

    Window_DX* winDx_ACCounter = win->winDx->GetSubWin(subWin_num_ACCounter);
    if (!winDx_ACCounter) {
        DWORD counterWidth = DIGIT_BUFF_X_MAX;
        DWORD counterHeight = 17;
        winDx_ACCounter = new Window_DX((float)(x + win->width - 640), (float)(y), counterWidth, counterHeight, 0x00000000, win->winDx, &subWin_num_ACCounter);

        if (!winDx_ACCounter)
            return;
    }
    winDx_ACCounter->ClearRenderTarget(nullptr);
    DrawCounter(winDx_ACCounter, lastCount, thisCount, numColour_xPos, flipTime_ms);
}


//____________________________________________
void __declspec(naked) h_draw_hp_counter(void) {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ds : [esp + 0x14]
        push dword ptr ds : [esp + 0x14]
        push ecx
        push ebx
        push edx
        push eax
        call DrawHPCounter
        add esp, 0x18

        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}


//____________________________________________
void __declspec(naked) h_draw_ac_counter(void) {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ds : [esp + 0x14]
        push dword ptr ds : [esp + 0x14]
        push ecx
        push ebx
        push edx
        push eax
        call DrawACCounter
        add esp, 0x18

        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}


//___________________________________
void Modifications_Interface_Bar_CH() {
    //To-Do Modifications_Interface_Bar_CH lots to do



    p_buttRef_ifaceInv = (LONG*)0x528D08;
    p_buttRef_ifaceOptions = (LONG*)0x528D14;
    p_buttRef_ifaceSkills = (LONG*)0x528D20;
    p_buttRef_ifaceMap = (LONG*)0x528D30;
    p_buttRef_ifacePip = (LONG*)0x528D40;
    p_buttRef_ifaceChar = (LONG*)0x528D4C;
    p_buttRef_ifaceAttack = (LONG*)0x528D58;
    p_buttRef_ifaceSwitchHands = (LONG*)0x528D7C;
    p_buttRef_ifaceSwitchTurn = (LONG*)0x528DA0;
    p_buttRef_ifaceEndCombat = (LONG*)0x528DAC;

    p_buttRef_imonitor_up = (LONG*)0x528314;
    p_buttRef_imonitor_down = (LONG*)0x528310;


    FuncWrite32(0x460D00, 0x07B3B9, (DWORD)&h_iface_notify_win);

    FuncWrite32(0x4AAF90, 0x031129, (DWORD)&h_iface_skill_win);
    FuncWrite32(0x4AB21F, 0x03119D, (DWORD)&h_iface_skill_kill);

    //IFACE SETUP
    //0045CFBC  |. E8 FCF00700    CALL Fallout2.004DC0BD
    //FuncWrite32(0x45CFBD, 0x07F0FC,  (DWORD)&IfaceSetup);


    //0045DF46  |. E8 75E40700    CALL Fallout2.004DC3C0
   // FuncWrite32(0x45DF47, 0x07E475,  (DWORD)&h_iface_window_destroy);
 /*
    //0045DAF5  |. E8 04F60700    CALL Fallout2.004DD0FE
    FuncWrite32(0x45DAF6, 0x07F604,  (DWORD)&h_iface_bar_hide);

    //0045DFFB   . E8 FEF00700    CALL Fallout2.004DD0FE
    FuncWrite32(0x45DFFC, 0x07F0FE,  (DWORD)&h_iface_bar_hide);

    //0045E0FD   . E8 FCEF0700    CALL Fallout2.004DD0FE
    FuncWrite32(0x45E0FE, 0x07EFFC,  (DWORD)&h_iface_bar_hide);

    //0045E148  |. E8 81EE0700    CALL Fallout2.004DCFCE
    FuncWrite32(0x45E149, 0x07EE81,  (DWORD)&h_iface_bar_show);
 */



    icombatRect = (RECT*)0x528D90;
    imonitorRect = (RECT*)0x528300;
    //iface action bar rect
    iactionRect = (RECT*)0x528DC4;
    //iface item slot rect
    iitemSlotRect = (RECT*)0x528D6C;

    p_imonitor_line_last = (LONG*)0x57FB54;
    p_imonitor_line_view = (LONG*)0x57FB48;

    p_is_imonitor = (BOOL*)0x5282FC;
    p_imonitor_num_visible_lines = (LONG*)0x57FB40;
    pp_imonitor_colour = (BYTE**)0x43171A;
    p_imonitor_font = (LONG*)0x4316F7;


    MemWrite8(0x4314A0, 0x53, 0xE9);
    FuncWrite32(0x4314A1, 0x57565251, (DWORD)&h_imonitor_insert_text);

    MemWrite8(0x431694, 0x53, 0xE9);
    FuncWrite32(0x431695, 0x57565251, (DWORD)&h_imonitor_print_text);

    MemWrite8(0x431434, 0x53, 0xE9);
    FuncWrite32(0x431435, 0x3D835251, (DWORD)&h_imonitor_clear_text);

    MemWrite8(0x431648, 0x53, 0xE9);
    FuncWrite32(0x431649, 0x3D835251, (DWORD)&h_imonitor_clear_text);


    //ammo bar-----
    MemWrite16(0x4601A0, 0x5153, 0xE990);
    FuncWrite32(0x4601A2, 0x10EC8356, (DWORD)&h_draw_ammo_bar);

    fall_Iface_DrawAmmoBar = (LONG(*)())0x45EF38;

    ///done///0045E2D8  /$  53                        PUSH EBX                                           ; DRAW_HP_COUNTER(EAX flag)
    //F_DRAW_IFACE_HP_NUM = (void*)0x45E2D8;
    ///done///0045E4A8  /$  53                        PUSH EBX                                           ; DRAW_AC_COUNTER(EAX flag)
    //F_DRAW_IFACE_AC_NUM = (void*)0x45E4A8;
    ///done///0045E50C  /$  53                        PUSH EBX                                           ; DRAW_AP_BAR(EAX actionPoints)
    //F_DRAW_IFACE_AP_BAR = (void*)0x45E50C;

    ///done///0045CFDE  |.  A3 74D95A00               MOV DWORD PTR DS:[5AD974],EAX                      ; iface buff ptr
    p_buff_ifaceWinBuff = (BYTE**)0x5AD974;

}


//______________________________________
void Modifications_Interface_Bar_MULTI() {

    p_buttRef_ifaceInv = (LONG*)FixAddress(0x518F18);
    p_buttRef_ifaceOptions = (LONG*)FixAddress(0x518F24);
    p_buttRef_ifaceSkills = (LONG*)FixAddress(0x518F30);
    p_buttRef_ifaceMap = (LONG*)FixAddress(0x518F40);
    p_buttRef_ifacePip = (LONG*)FixAddress(0x518F50);
    p_buttRef_ifaceChar = (LONG*)FixAddress(0x518F5C);
    p_buttRef_ifaceAttack = (LONG*)FixAddress(0x518F68);
    p_buttRef_ifaceSwitchHands = (LONG*)FixAddress(0x518F8C);
    p_buttRef_ifaceSwitchTurn = (LONG*)FixAddress(0x518FB0);
    p_buttRef_ifaceEndCombat = (LONG*)FixAddress(0x518FBC);

    p_buttRef_imonitor_up = (LONG*)FixAddress(0x518524);
    p_buttRef_imonitor_down = (LONG*)FixAddress(0x518520);


    FuncReplace32(0x461600, 0x074C34, (DWORD)&h_iface_notify_win);

    FuncReplace32(0x4AC261, 0x029FD3, (DWORD)&h_iface_skill_win);
    FuncReplace32(0x4AC683, 0x029DE1, (DWORD)&h_iface_skill_kill);

    icombatRect = (RECT*)FixAddress(0x518FA0);
    imonitorRect = (RECT*)FixAddress(0x518510);
    //iface action bar rect
    iactionRect = (RECT*)FixAddress(0x518FD4);
    //iface item slot rect
    iitemSlotRect = (RECT*)FixAddress(0x518F7C);

    p_imonitor_line_last = (LONG*)FixAddress(0x56FB54);
    p_imonitor_line_view = (LONG*)FixAddress(0x56FB48);

    p_is_imonitor = (BOOL*)FixAddress(0x51850C);
    p_imonitor_num_visible_lines = (LONG*)FixAddress(0x56FB40);
    pp_imonitor_colour = (BYTE**)FixAddress(0x431AF2);
    p_imonitor_font = (LONG*)FixAddress(0x431ADB);


    MemWrite8(0x43186C, 0x53, 0xE9);
    FuncWrite32(0x43186D, 0x57565251, (DWORD)&h_imonitor_insert_text);

    MemWrite8(0x431A78, 0x53, 0xE9);
    FuncWrite32(0x431A79, 0x57565251, (DWORD)&h_imonitor_print_text);

    MemWrite8(0x431800, 0x53, 0xE9);
    FuncWrite32(0x431801, 0x3D835251, (DWORD)&h_imonitor_clear_text);

    MemWrite8(0x431A2C, 0x53, 0xE9);
    FuncWrite32(0x431A2D, 0x3D835251, (DWORD)&h_imonitor_clear_text);


    MemWrite16(0x460AA0, 0x5153, 0xE990);
    FuncWrite32(0x460AA2, 0x10EC8356, (DWORD)&h_draw_ammo_bar);

    fall_Iface_DrawAmmoBar = (LONG(*)())FixAddress(0x45F838);


    fall_NotifyBar_Setup = (LONG(*)())FixAddress(0x461134);
    fall_NotifyBar_Destructor = (LONG(*)())FixAddress(0x461454);
    fall_NotifyBar_Enable = (LONG(*)())FixAddress(0x461740);
    fall_NotifyBar_Disable = (LONG(*)())FixAddress(0x461760);

    p_is_iface_initilizing = (BOOL*)FixAddress(0x518F08);
    p_is_iface_enabled = (BOOL*)FixAddress(0x518F10);
    p_is_iface_hidden = (BOOL*)FixAddress(0x518F14);



    p_iface_active_slot = (LONG*)FixAddress(0x518F78);
    p_iface_wielded = (IFACE_WIELDED_OBJ*)FixAddress(0x5970F8);

    p_buff_ifaceWinBuff = (BYTE**)FixAddress(0x59D3F4);


    MemWrite8(0x45D880, 0x53, 0xE9);
    FuncWrite32(0x45D881, 0x57565251, (DWORD)&h_iface_setup);

    MemWrite8(0x45E440, 0x53, 0xE9);
    FuncWrite32(0x45E441, 0x57565251, (DWORD)&h_iface_destructor);


    MemWrite8(0x45EE0C, 0x53, 0xE9);
    FuncWrite32(0x45EE0D, 0x55575651, (DWORD)&h_draw_ap_bar);


    MemWrite8(0x45FD88, 0x53, 0xE9);
    FuncWrite32(0x45FD89, 0x57565251, (DWORD)&h_draw_wielded_button);


    p_is_combat_box_opened = (BOOL*)FixAddress(0x518F9C);

    MemWrite8(0x45F96C, 0x53, 0xE9);
    FuncWrite32(0x45F96D, 0x57565251, (DWORD)&h_draw_combat_box_open);

    MemWrite8(0x45FAC0, 0x53, 0xE9);
    FuncWrite32(0x45FAC1, 0x57565251, (DWORD)&h_draw_combat_box_close);

    MemWrite8(0x45FC04, 0x53, 0xE9);
    FuncWrite32(0x45FC05, 0xEC835251, (DWORD)&h_draw_enable_combat_box_buttons);

    MemWrite8(0x45FC98, 0x53, 0xE9);
    FuncWrite32(0x45FC99, 0xEC835251, (DWORD)&h_draw_disable_combat_box_buttons);


    //hp
    FuncReplace32(0x45ED64, 0x1E38, (DWORD)&h_draw_hp_counter);
    FuncReplace32(0x45ED87, 0x1E15, (DWORD)&h_draw_hp_counter);
    //ac
    FuncReplace32(0x45EDFB, 0x1DA1, (DWORD)&h_draw_ac_counter);
}



//________________________________
void Modifications_Interface_Bar() {

    IFACE_BAR_SIDE_ART = ConfigReadInt(L"IFACE", L"IFACE_BAR_SIDE_ART", 0);
    if (IFACE_BAR_SIDE_ART > 99)
        IFACE_BAR_SIDE_ART = 0;

    IFACE_BAR_LOCATION = ConfigReadInt(L"IFACE", L"IFACE_BAR_LOCATION", 0);
    if (IFACE_BAR_LOCATION > 2)
        IFACE_BAR_LOCATION = 0;

    IFACE_BG_COLOUR = ConfigReadInt(L"IFACE", L"IFACE_BG_COLOUR", 0x000000FF);

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Interface_Bar_CH();
    else
        Modifications_Interface_Bar_MULTI();
}







