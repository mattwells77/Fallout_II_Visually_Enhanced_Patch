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
#include "game_map.h"
#include "modifications.h"
#include "memwrite.h"
#include "win_fall.h"
#include "configTools.h"

#include "Fall_General.h"
#include "Fall_Objects.h"
#include "Fall_File.h"
#include "Fall_Graphics.h"

#include "Dx_Game.h"
#include "Dx_Windows.h"

#include "mapper\mapper_tools.h"

RECT rcGame_GUI;

LONG game_PortalWidth=0;
LONG game_PortalHeight=0;

LONG xMouse_GUI = 0;
LONG yMouse_GUI = 0;

bool isMapLevelChanged = false;
bool isZoomBoundByMapEdges = false;


LONG* isMouseScrollBoundaryEnabled = nullptr;
LONG* isMouseAtScrollBoundary = nullptr;
//doesn't seem to ever be enabled
LONG* ignore_isMouseAtScrollBoundary = nullptr;


LONG scaleLevel_Game = 1;
float scaleGame_RO = 1.0f;
float scaleSubUnit = 1.0f;

LONG numPathNodes = 2000;
DWORD* PathFindNodes01 = nullptr;
DWORD* PathFindNodes02 = nullptr;


//____________________________
BOOL IsHexVisible(LONG hexPos) {
    //Is hex tile within 5 hexes of game portal centre.

    LONG hex_x = 0;
    LONG hex_y = 0;

    HexNumToHexPos(hexPos, &hex_x, &hex_y);

    if (abs(*pVIEW_HEX_X - hex_x) < 5 || (abs(*pVIEW_HEX_Y - hex_y) < 5))
        return TRUE;
    return FALSE;

}


//_________________________________________
void __declspec(naked) is_hex_visible(void) {

    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call IsHexVisible
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


//________________________________________________
BOOL GetMousePosOnGamePortal(LONG* p_x, LONG* p_y) {

    GetMousePos(p_x, p_y);

    xMouse_GUI = *p_x;
    yMouse_GUI = *p_y;
    if (GetWinAtPos(*p_x, *p_y) != *pWinRef_GameArea)
        return FALSE;
    WinStruct* win = fall_Win_Get(*pWinRef_GameArea);
    if (!(win->flags & FLG_WinHidden)) {
        *p_x = (LONG)((float)*p_x / scaleGame_RO);
        *p_y = (LONG)((float)*p_y / scaleGame_RO);
    }
    return TRUE;
}


//_______________________________________________________
void __declspec(naked) get_mouse_pos_on_game_portal(void) {
    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetMousePosOnGamePortal
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//_________________________________________________
BOOL GetMousePosOnGameMap(LONG* pXPos, LONG* pYPos) {

    GetMousePos(pXPos, pYPos);

    xMouse_GUI = *pXPos;
    yMouse_GUI = *pYPos;
    if (GetWinAtPos(*pXPos, *pYPos) != *pWinRef_GameArea)
        return FALSE;
    WinStruct* win = fall_Win_Get(*pWinRef_GameArea);
    if (win && !(win->flags & FLG_WinHidden)) {
        *pXPos = (LONG)((float)*pXPos / scaleGame_RO);
        *pXPos += rcGame_PORTAL.left;
        *pYPos = (LONG)((float)*pYPos / scaleGame_RO);
        *pYPos += rcGame_PORTAL.top;
    }
    return TRUE;
}


//____________________________________________________
void __declspec(naked) get_mouse_pos_on_game_map(void) {
    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetMousePosOnGameMap
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//for keeping the mouse hex position set to a valid hex when the mouse is out of bounds.
//_______________________________________________
LONG GetNearestValidHexPos(LONG xPos, LONG yPos) {

    LONG xHex = 0;
    LONG yHex = 0;
    LONG hexPos = SqrToHexPos_GameOffset(xPos, yPos, &xHex, &yHex);
    if (hexPos == -1) {
        if (xHex < 0)
            xHex = 0;
        else if (xHex >= *pNUM_HEX_X)
            xHex = *pNUM_HEX_X - 1;
        if (yHex < 0)
            yHex = 0;
        else if (yHex >= *pNUM_HEX_Y)
            yHex = *pNUM_HEX_Y - 1;

        hexPos =  yHex * *pNUM_HEX_X + xHex;
    }
    return hexPos;
}


//____________________________________________________
void __declspec(naked) get_nearest_valid_hexpos(void) {
    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetNearestValidHexPos
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}



//________________________________________________________________________
void __declspec(naked) adjust_game_mouse_pos_to_nearest_valid_hexpos(void) {

    __asm {
        push ebx
        push ecx
        push esi

        add eax, rcGame_PORTAL.left
        add edx, rcGame_PORTAL.top
        push edx
        push eax
        call GetNearestValidHexPos
        add esp, 0x8

        pop esi
        pop ecx
        pop ebx
        ret
    }

}



//___________________________________________________________________________
void __declspec(naked) get_mouse_pic_ref_adjust_position_on_game_portal(void) {

    __asm {
        lea eax, [esp + 0x84]//p_return_mouse_x
        lea edx, [esp + 0x88]//p_return_mouse_y

        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetMousePosOnGamePortal
        add esp, 0x8

        mov eax, pMousePicID
        mov eax, dword ptr ds : [eax]

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//________________________________________________
DWORD CheckMouseScrnRect(LONG mouseX, LONG mouseY) {

    //if (!isGameMode && !isMapperSelecting && !isMapperScrolling) {//free mouse when not in game mode.
    if (!isGameMode && !IsMapperScrolling()) {//free mouse when not in game mode.
            ClipCursor(nullptr);
        return 0;
    }

    RECT rcClient;
    POINT p = { 0,0 };
    ClientToScreen(hGameWnd, &p);
    GetClientRect(hGameWnd, &rcClient);
    rcClient.left += p.x;
    rcClient.top += p.y;
    rcClient.right += p.x;
    rcClient.bottom += p.y;
    ClipCursor(&rcClient);

    DWORD flags = 0;
    if (mouseX <= pFALL_RC->left)
        flags = flags | 0x01;
    if (mouseX >= pFALL_RC->right)
        flags = flags | 0x02;
    if (mouseY <= pFALL_RC->top)
        flags = flags | 0x04;
    if (mouseY >= pFALL_RC->bottom)
        flags = flags | 0x08;
    return flags;
}


//________________________________________
LONG Scroll_Game(LONG mouseX, LONG mouseY) {
    if (*isMouseScrollBoundaryEnabled == 0)
        return -1;

    DWORD flags = CheckMouseScrnRect(mouseX, mouseY);
    if (flags == 0)
        *isMouseAtScrollBoundary = 0;
    else
        *isMouseAtScrollBoundary = 1;

    LONG xDir = 0;
    LONG yDir = 0;
    LONG mouseNum = 0;

    switch (flags) {
    case 1://left
        xDir = -1;
        mouseNum = 11;
        break;
    case 2://right
        xDir = 1;
        mouseNum = 7;
        break;
    case 4://top
        yDir = -1;
        mouseNum = 5;
        break;
    case 8://bottom
        yDir = 1;
        mouseNum = 9;
        break;
    case 5://left & top
        xDir = -1;
        yDir = -1;
        mouseNum = 4;
        break;
    case 6://right & top
        xDir = 1;
        yDir = -1;
        mouseNum = 6;
        break;
    case 9://left & bottom
        xDir = -1;
        yDir = 1;
        mouseNum = 10;
        break;
    case 10://right & bottom
        xDir = 1;
        yDir = 1;
        mouseNum = 8;
        break;
    default:
        break;
    }
    if (xDir == 0 && yDir == 0)
        return -1;
    if (*ignore_isMouseAtScrollBoundary == 0) {
        if (*isMouseAtScrollBoundary != 0) {
            LONG scrollVal = MapScroller(xDir, yDir);
            if (scrollVal == 0)
                fall_Mouse_SetImage(mouseNum);
            else if (scrollVal == -1)
                fall_Mouse_SetImage(mouseNum += 8);
        }
        //else 
            //Fallout_Debug_Info("isMouseAtScrollBoundary == 0");

    }
    //else 
        //Fallout_Debug_Info("ignore_isMouseAtScrollBoundary != 0");

    return 0;
}


//___________________________________
void __declspec(naked) scroller(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call Scroll_Game
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
void OnScreenResize_GameMap(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    ResizeGameWin();
}


//__________________________________________________
void OnDisplay_Instead_GameMap(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    GameAreas_Display();
}


//______________________________________________________________________________________
LONG CreateGameWin(LONG x, LONG y, DWORD width, DWORD height, DWORD colour, DWORD flags) {

    if (ConfigReadInt(L"MAPS", L"IS_ZOOM_BOUND_BY_EDGES", 0))
        isZoomBoundByMapEdges = true;

    scaleLevel_Game = ConfigReadInt(L"MAPS", L"ZOOM_LEVEL", 1);
    if (scaleLevel_Game < 1)
        scaleLevel_Game = 1;

    scaleSubUnit = 1.0f / scaleLevel_GUI;

    rcGame_GUI.left = 0;
    rcGame_GUI.top = 0;
    rcGame_GUI.right = SCR_WIDTH - 1;
    rcGame_GUI.bottom = SCR_HEIGHT - 1;
    if (IFACE_BAR_MODE == 0 && isGameMode)
        rcGame_GUI.bottom -= IFACE_BAR_HEIGHT;

    DWORD width_gui = rcGame_GUI.right - rcGame_GUI.left + 1;
    DWORD height_gui = rcGame_GUI.bottom - rcGame_GUI.top + 1;

    if (scaleLevel_Game < scaleLevel_GUI)
        scaleGame_RO = scaleSubUnit * (float)scaleLevel_Game;
    else
        scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;

    game_PortalWidth = (LONG)(width_gui / scaleGame_RO);
    game_PortalHeight = (LONG)(height_gui / scaleGame_RO);

    LONG winRef = fall_Win_Create(rcGame_GUI.left, rcGame_GUI.top, width_gui, height_gui, colour, flags);
    *pWinRef_GameArea = winRef;

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    pWin->winDx->Set_OnDisplayFunctions(nullptr, nullptr, &OnDisplay_Instead_GameMap);
    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_GameMap);
    ResizeGameWin();

    return winRef;
}


//__________________________________________
void __declspec(naked) create_game_win(void) {
    __asm {
        push ebp
        push esi
        push edi

        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        push ecx
        push ebx
        push edx
        push eax
        call CreateGameWin
        add esp, 0x18

        pop edi
        pop esi
        pop ebp
        ret 0x8
    }
}


//__________________
bool ResizeGameWin() {

    //if (isMapperSizing) 
        //imonitorInsertText("hello isMapperSizing ResizeGameWin");
 //To_Do working on scaleLevel_GUI scaling in game-------------------
    //float test_scaleSubUnit = 1.0f / scaleLevel_GUI;
    //if (scaleSubUnit != test_scaleSubUnit) {
    //    scaleSubUnit = test_scaleSubUnit;
        //if (scaleLevel_Game < scaleLevel_GUI)
        //    scaleGame_RO = scaleSubUnit * (float)scaleLevel_Game;
        //else
        //    scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
   // }
    //---------------------------------------------------
    rcGame_GUI.left = 0;
    rcGame_GUI.top = 0;
    rcGame_GUI.right = SCR_WIDTH - 1;
    rcGame_GUI.bottom = SCR_HEIGHT - 1;
    if (IFACE_BAR_MODE == 0 && isGameMode)
        rcGame_GUI.bottom -= IFACE_BAR_HEIGHT;

    DWORD width_gui = rcGame_GUI.right - rcGame_GUI.left + 1;
    DWORD height_gui = rcGame_GUI.bottom - rcGame_GUI.top + 1;

    game_PortalWidth = (LONG)(width_gui / scaleGame_RO);
    game_PortalHeight = (LONG)(height_gui / scaleGame_RO);

    EDGE_X_DIFF = (game_PortalWidth / 2) & 0x1F;
    EDGE_Y_DIFF = (game_PortalHeight / 2) % 24;

    if (isZoomBoundByMapEdges) {
        GAME_AREA* pGameArea = GameAreas_GetCurrentArea();
        if (pGameArea && (game_PortalWidth > (LONG)pGameArea->width || game_PortalHeight > (LONG)pGameArea->height)) {
            scaleLevel_Game++;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;

            if (ResizeGameWin())
                return true;
            else
                return false;
        }
    }

    if (*pWinRef_GameArea == -1)
        return false;
    WinStruct* gameWin = fall_Win_Get(*pWinRef_GameArea);
    if (gameWin == nullptr)
        return false;

    gameWin->width = width_gui;
    gameWin->height = height_gui;
    gameWin->rect.left = rcGame_GUI.left;
    gameWin->rect.top = rcGame_GUI.top;
    gameWin->rect.right = rcGame_GUI.left + gameWin->width - 1;
    gameWin->rect.bottom = rcGame_GUI.top + gameWin->height - 1;
    gameWin->buff = (BYTE*)fall_Mem_Reallocate(gameWin->buff, gameWin->width * gameWin->height);

    GameAreas_SetScale();
    UpdateFloatingTextAreaWH(gameWin->width, gameWin->height);
    return true;
}


//___________________________
bool GameMap_Trans(int delta) {

    GameAreas_ShiftWallRoofOpaqueness(delta);
    DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Obj);// | FLG_Roof);
    return true;
}

/*
//_____________________________________________
bool GameMap_Zoom(int delta, bool focusOnMouse) {

    WinStruct* win = fall_Win_Get(*pWinRef_GameArea);
    if (!win)
        return false;

    LONG xPos = 0;
    LONG yPos = 0;

    if (focusOnMouse) {
        //fall_Mouse_GetPos(&xPos, &yPos);
        //if (GetWinAtPos(xPos, yPos) == *pWinRef_GameArea)
        //    GetMousePosOnGameMap(&xPos, &yPos);
        //else
        //    return false;
        if (!GetMousePosOnGameMap(&xPos, &yPos))
            return false;
    }
    else
        HexNumToSqr(*pVIEW_HEXPOS, &xPos, &yPos);

    xPos += EDGE_OFF_X;
    yPos += EDGE_OFF_Y;

    LONG hex_x = 0;
    LONG hex_y = 0;
    LONG hexPos = SqrToHexPos_GameOffset(xPos, yPos, &hex_x, &hex_y);
    if (hexPos == -1) {
        if (xPos < 0)
            xPos = 0;
        else if (xPos >= *pNUM_HEX_X)
            xPos = *pNUM_HEX_X - 1;
        if (yPos < 0)
            yPos = 0;
        else if (yPos >= *pNUM_HEX_Y)
            yPos = *pNUM_HEX_Y - 1;

        hexPos = yPos * *pNUM_HEX_X + xPos;
    }
  
    if (!isHexWithinMapEdges(hexPos) && isGameMode)
        return true;

    SetViewPosition_Hex(hexPos, 0x0);
    if (delta > 0) {
        scaleLevel_Game++;
        if (scaleLevel_Game < scaleLevel_GUI)
            scaleGame_RO = scaleLevel_Game * scaleSubUnit;
        else
            scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;


        if ((float)win->width / scaleGame_RO < 128.0f) {
            scaleLevel_Game--;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
        }
    }
    else if (delta < 0) {
        if (scaleLevel_Game > 1) {
            scaleLevel_Game--;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
        }
        else if (scaleLevel_Game == 1) {
            float widthRO = 1.0f;
            float heightRO = 1.0f;
            GAME_AREA* pGameArea = GameAreas_GetCurrentArea();
            if (pGameArea) {
                widthRO = ((float)game_PortalWidth / (float)pGameArea->width) / scaleLevel_GUI;
                heightRO = ((float)game_PortalHeight / (float)pGameArea->height) / scaleLevel_GUI;
            }
            if (widthRO < scaleGame_RO || heightRO < scaleGame_RO) {
                scaleLevel_Game = 0;
                if (widthRO < heightRO)
                    scaleGame_RO = widthRO;
                else
                    scaleGame_RO = heightRO;
            }
        }
    }

    ResizeGameWin();
    SetViewPosition_Hex(hexPos, 0x0);// 0x2);
    if (focusOnMouse) {
        LONG xView = 0;
        LONG yView = 0;
        HexNumToSqr_Grid16x12(*pVIEW_HEXPOS, &xView, &yView);
        HexNumToSqr_Grid16x12(hexPos, &xPos, &yPos);

        xPos = (game_PortalWidth >> 1) - ((xPos - xView) << 4);
        yPos = (game_PortalHeight >> 1) - ((yView - yPos) * 12);

        SetMousePosGame(xPos, yPos);
    }
    return true;
}
*/

//_____________________________________________
bool GameMap_Zoom(int delta, bool focusOnMouse) {

    WinStruct* win = fall_Win_Get(*pWinRef_GameArea);
    if (!win)
        return false;

    LONG sqr_x = 0;
    LONG sqr_y = 0;

    if (focusOnMouse) {
        if (!GetMousePosOnGameMap(&sqr_x, &sqr_y))
            return false;
    }
    else
        HexNumToSqr(*pVIEW_HEXPOS, &sqr_x, &sqr_y);

    LONG hex_x = 0;
    LONG hex_y = 0;
    SqrToHexPos_GameOffset(sqr_x, sqr_y, &hex_x, &hex_y);

    if (!isHexWithinMapEdges(hex_x, hex_y) && isGameMode)
        return true;

    SetViewPosition_Hex_Unbound(hex_x, hex_y, 0x0);
    if (delta > 0) {
        scaleLevel_Game++;
        if (scaleLevel_Game < scaleLevel_GUI)
            scaleGame_RO = scaleLevel_Game * scaleSubUnit;
        else
            scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;


        if ((float)win->width / scaleGame_RO < 128.0f) {
            scaleLevel_Game--;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
        }
    }
    else if (delta < 0) {
        if (scaleLevel_Game > 1) {
            scaleLevel_Game--;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
        }
        else if (scaleLevel_Game == 1) {
            float widthRO = 1.0f;
            float heightRO = 1.0f;
            GAME_AREA* pGameArea = GameAreas_GetCurrentArea();
            if (pGameArea) {
                widthRO = ((float)game_PortalWidth / (float)pGameArea->width) / scaleLevel_GUI;
                heightRO = ((float)game_PortalHeight / (float)pGameArea->height) / scaleLevel_GUI;
            }
            if (widthRO < scaleGame_RO || heightRO < scaleGame_RO) {
                scaleLevel_Game = 0;
                if (widthRO < heightRO)
                    scaleGame_RO = widthRO;
                else
                    scaleGame_RO = heightRO;
            }
        }
    }

    ResizeGameWin();
    SetViewPosition_Hex_Unbound(hex_x, hex_y, 0x0);

    if (focusOnMouse) {
        HexToSqr(hex_x, hex_y, &sqr_x, &sqr_y);
        sqr_x -= rcGame_PORTAL.left;
        sqr_y -= rcGame_PORTAL.top;
        SetMousePosGame(sqr_x, sqr_y);
    }
    return true;
}


//_____________________
bool GameMap_HasFocus() {
    if (*pWinRef_GameArea == -1)
        return false;
    WinStruct* win = fall_Win_Get(*pWinRef_GameArea);
    if (!win)
        return false;
    if (win->flags & FLG_WinHidden)
        return false;
    if (pWin_Array[*p_num_windows - 1]->ref != *pWinRef_GameArea &&
        pWin_Array[*p_num_windows - 1]->ref != *pWinRef_Iface &&
        pWin_Array[*p_num_windows - 1]->ref != *pWinRef_NotifyBar)
        return false;
    return true;
}



//____________________________________________________
bool Mouse_Wheel_GameWindow(int zDelta, bool setTrans) {

    if (!GameMap_HasFocus())
        return false;
    if (setTrans)
        return GameMap_Trans(-zDelta);
    return GameMap_Zoom(zDelta, true);
}


//___________________________________
LONG Game_CheckControlKeys(LONG code) {

    if (code == 0x149) {//Page_Up - zoom in
        GameMap_Zoom(1, false);
        return -1;
    }
    else if (code == 0x151) {//Page_Dn - zoom out
        GameMap_Zoom(-1, false);
        return -1;
    }
    else if (code == 0x184) {//CTRL + Page_Up - more transparent
            GameMap_Trans(-1);
        return -1;
    }
    else if (code == 0x176) {//CTRL + Page_Dn - more opaque
            GameMap_Trans(1);
        return -1;
    }
    
    return code;
}


//__________________________________________________
void __declspec(naked) game_check_control_keys(void) {

    __asm {
        cmp ebx, -2
        je exitFunc

        push eax
        push edx
        push ecx
        push esi
        push edi

        push ebx
        call Game_CheckControlKeys
        add esp, 0x4
        mov ebx, eax

        pop edi
        pop esi
        pop ecx
        pop edx
        pop eax

        exitFunc :
        xor ebp, ebp
        cmp eax, 0x5
        ret
    }
}


//______________________
LONG ResetLevelScaling() {
    isMapLevelChanged = true;
    return *pMousePicID;
}


//______________________________________________
void __declspec(naked) reset_level_scaling(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        call ResetLevelScaling
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//___________________
void ResetZoomLevel() {

    if (!isMapLevelChanged && !isMapperSizing)
        return;
    isMapLevelChanged = false;

    if (isZoomBoundByMapEdges) {
        GAME_AREA* pGameArea = GameAreas_GetCurrentArea();
        if (pGameArea && (game_PortalWidth > (LONG)pGameArea->width || game_PortalHeight > (LONG)pGameArea->height)) {
            scaleLevel_Game++;
            if (scaleLevel_Game < scaleLevel_GUI)
                scaleGame_RO = scaleLevel_Game * scaleSubUnit;
            else
                scaleGame_RO = (float)scaleLevel_Game - scaleLevel_GUI + 1;
            ResizeGameWin();
        }
    }
    else if (scaleGame_RO < scaleSubUnit) {
        scaleLevel_Game = 1;
        scaleGame_RO = scaleSubUnit;
        ResizeGameWin();
    }

}


//_____________________________________
void __declspec(naked) sqr_to_hex(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        //push ebx
        push edx
        push eax
        call SqrToHexNum_GameOffset
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//_____________________________________
void __declspec(naked) hex_to_sqr(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call HexNumToSqr_GameOffset
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//________________________________________________
void __declspec(naked) adjust_game_mouse_pos(void) {

    __asm {
        push ebx
        push ecx
        push esi

        add eax, rcGame_PORTAL.left
        add edx, rcGame_PORTAL.top
        push edx
        push eax
        call SqrToHexNum_GameOffset
        add esp, 0x8

        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//_________________________________________________
void __declspec(naked) adjust_game_mouse_pos2(void) {

    __asm {
        add eax, rcGame_PORTAL.left
        add edx, rcGame_PORTAL.top

        mov ebp, eax
        mov edi, edx
        mov esi, ebx
        ret
    }
}



//____________________________________________
void __declspec(naked) sqr_to_floor_tile(void) {

    __asm {
        push esi
        push edi
        push ebp

        mov ebx, dword ptr ss : [esp + 0x10]

        push ebx
        push ecx
        push edx
        push eax
        call SqrToTile_GameOffset
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret 0x4
    }
}


//___________________________________________
void __declspec(naked) sqr_to_roof_tile(void) {

    __asm {
        push esi
        push edi
        push ebp

        mov ebx, dword ptr ss : [esp + 0x10]

        push ebx
        push ecx
        push edx
        push eax
        call SqrToTile_Roof_GameOffset
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret 0x4
    }
}


//____________________________________________
void __declspec(naked) floor_tile_to_sqr(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call TileToSqr_GameOffsets
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//___________________________________________
void __declspec(naked) roof_tile_to_sqr(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call TileToSqr_Roof_GameOffsets
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//___________________________________________
void __declspec(naked) get_next_hex_pos(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call GetNextHexPos
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }

}



//_______________________________________
void __declspec(naked) get_hex_dist(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetHexDistance
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//________________________________________________________________________________
LONG Proto_File_Close_For_Reading(void* FileStream, const char* path, PROTO* pPro) {
    VE_PROTO_LightColour_Read(path, pPro);
    return fall_fclose(FileStream);
}


//_______________________________________________________
void __declspec(naked) proto_file_close_for_reading(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        lea ebx, dword ptr ds: [esp+0x1C]//prototype file path
        mov ecx, dword ptr ds : [esp + 0x108 + 0x1C]//PROTO**
        mov ecx, dword ptr ds : [ecx]//PROTO*
        push ecx
        push ebx
        push eax//FileStream
        call Proto_File_Close_For_Reading//fall_fclose
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//___________________________________________________________________
void Game_Data_Files_Copy(const char* pFromPath, const char* pToPath) {

    FogOfWarMap_CopyFiles(pFromPath, pToPath);
    VE_MAP_CopyFiles(pFromPath, pToPath);
}


//_______________________________________________
void __declspec(naked) game_data_files_copy(void) {

   __asm {
      push eax

      push ebx
      push esi
      push edi
      push ebp

      push dword ptr ss:[esp+0x1C] //pToPath
      push dword ptr ss:[esp+0x1C] //pFromPath
      call Game_Data_Files_Copy
      add esp, 0x08

      pop ebp
      pop edi
      pop esi
      pop ebx

      pop eax
      cmp eax, -1
      ret 0x8
   }
}


//____________________________________________________________
LONG Game_Data_Files_Delete(const char* path, const char* ext) {

    FogOfWarMap_DeleteTmps(path);
    VE_MAP_DeleteTmps(path);
    return fall_Files_Delete(path, ext);
}


//_________________________________________________
void __declspec(naked) game_data_files_delete(void) {

    __asm {
        push ebx
        push ecx
        push esi

        push edx// extension
        push eax//maps\(mapName).ext
        call Game_Data_Files_Delete
        add esp, 0x8

        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_________________________________________________________________
void* Mapfile_Open_for_Writing(const char* path, const char* flags) {

    void* FileStream_MAP = fall_fopen(path, flags);

    VE_MAP_Open_WRITE(path, FileStream_MAP);
    FogOfWarMap_Save(path);

    return FileStream_MAP;
}


//___________________________________________________
void __declspec(naked) mapfile_open_for_writing(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push edx// "wb"
        push eax//maps\(mapName).ext
        call Mapfile_Open_for_Writing
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


//__________________________________________________
LONG Mapfile_Close_For_Writing(void* FileStream_MAP) {
    VE_MAP_Close_WRITE();
    return fall_fclose(FileStream_MAP);
}


//____________________________________________________
void __declspec(naked) mapfile_close_for_writing(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax//FileStream
        call Mapfile_Close_For_Writing
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


//_________________________________________________________________
void* Mapfile_Open_For_Reading(const char* path, const char* flags) {

    void* FileStream_MAP = fall_fopen(path, flags);

    VE_MAP_Open_READ(path, FileStream_MAP);

    return FileStream_MAP;
}


//___________________________________________________
void __declspec(naked) mapfile_open_for_reading(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push edx// "rb"
        push eax//maps\(mapName).ext
        call Mapfile_Open_For_Reading
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


//____________________________________________________________________
LONG Mapfile_Close_For_Reading(void* FileStream_MAP, const char* path) {

    VE_MAP_Close_READ();
    FogOfWarMap_Load(path);

    return fall_fclose(FileStream_MAP);
}


//____________________________________________________
void __declspec(naked) mapfile_close_for_reading(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push ebx//mapName (+ extension)
        push eax//FileStream
        call Mapfile_Close_For_Reading
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


//__________________________________________________________________
void* SAVE_DAT_Open_For_Writing(const char* path, const char* flags) {

    void* FileStream = fall_fopen(path, flags);

    VE_SAVE_DAT_Open_WRITE(path, FileStream);
    LS_Save_Picture_To_File(path);

    return FileStream;
}


//____________________________________________________
void __declspec(naked) save_dat_open_for_writing(void) {

    __asm {


        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx// flags
        push eax//save.dat path
        call SAVE_DAT_Open_For_Writing
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
void* SAVE_DAT_Open_For_Reading(const char* path, const char* flags) {

    void* FileStream = fall_fopen(path, flags);

    VE_SAVE_DAT_Open_READ(path, FileStream);

    return FileStream;
}


//____________________________________________________
void __declspec(naked) save_dat_open_for_reading(void) {

    __asm {


        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx// flags
        push eax//save.dat path
        call SAVE_DAT_Open_For_Reading
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//_________________________________________
void __declspec(naked) save_dat_close(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax//FileStream
        call VE_SAVE_DAT_Close
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


//___________________________________________________________________________________________
LONG Object_Light_Colour_And_Radius_WRITE(void* FileStream, DWORD lightData, OBJStruct* pObj) {

    LightColour_Write(FileStream, pObj);
    //write light_radius from object structure.
    return fall_fwrite32_BE(FileStream, lightData);
}


//_______________________________________________________________
void __declspec(naked) object_light_colour_and_radius_write(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push ecx//pObj
        push edx//dword
        push eax//FileStream
        call Object_Light_Colour_And_Radius_WRITE//fall_fwrite32_BE
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_____________________________________________________________________________________________
LONG Object_Light_Colour_And_Radius_READ(void* FileStream, DWORD* p_lightData, OBJStruct* pObj) {

    LightColour_Read(FileStream, pObj);
    //read light radius into object structure.
    return fall_fread32_BE(FileStream, p_lightData);
}


//______________________________________________________________
void __declspec(naked) object_light_colour_and_radius_read(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push esi//pObj
        push edx//p_dword
        push eax//FileStream
        call Object_Light_Colour_And_Radius_READ
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________
void CheckOpenDoor() {
    if (!FOG_OF_WAR)
        return;
    //ReDrawViewWin();
    //pcHexNum=-1;
    //Fallout_Debug_Info("CheckOpenDoor");
}
//______________________________________
void __declspec(naked) check_open_door() {
    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp
        pushad
        call CheckOpenDoor
        popad
        push edx
        push eax
        call fall_GetPro//(DWORD proID, PROTOall **proto);
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//_______________________________________
void __declspec(naked) set_view_pos(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx//current level
        push eax//current view hex pos
        call SetViewPosition_Hex
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//_________________________________________
LONG SetViewFocus(LONG hexPos, DWORD flags) {
    SetViewPosition_Hex(hexPos, 2);
    return 0;
}


//_________________________________________
void __declspec(naked) set_view_focus(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call SetViewFocus
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
void __declspec(naked) check_angled_roof_tile_edge2(void) {
    __asm {
        push edx
        push dword ptr ss : [esp + 0x10]
        push dword ptr ss : [esp + 0x10]
        call CheckAngledRoofTileEdge
        add esp, 0xC

        cmp eax, -1
        jne exitFunc
        xor eax, eax
        add dword ptr ss : [esp] , 0x9//skip tile draw if pos invalid
        exitFunc :
        ret 4
    }
}


//___________________________________________
void __declspec(naked) h_get_objects_at_pos() {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ss : [esp + 0x10]
        push ecx
        push ebx
        push edx
        push eax
        call GetObjectsAtPos
        add esp, 0x14

        pop ebp
        pop edi
        pop esi
        ret 0x4
    }
}


//______________________________
void Modifications_Game_Map_CH() {

    MemWrite32(0x4811CE, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x4811DD, 0x1, 0x0);

    MemWrite32(0x4811E5, 0x6BCF68, (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x4811F1, 0xE883, 0x9090);
    MemWrite8(0x4811F3, 0x63, 0x90);

    MemWrite32(0x48123F, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x48124B, 0x1, 0x0);

    MemWrite32(0x481253, 0x6BCF68, (DWORD)&rcGame_GUI.bottom);
    MemWrite8(0x48125E, 0x9D, 0x0);

    MemWrite32(0x481BBD, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481BC9, 0x1, 0x0);

    MemWrite32(0x481BD1, 0x6BCF68, (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x481BD7, 0xE883, 0x9090);
    MemWrite8(0x481BD9, 0x63, 0x90);

    MemWrite32(0x48201D, 0x6BCF68, (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x482023, 0xE883, 0x9090);
    MemWrite8(0x482025, 0x63, 0x90);

    MemWrite32(0x48202E, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x482038, 0x1, 0x0);

    MemWrite32(0x481C77, 0x6BCF68, (DWORD)&rcGame_GUI.bottom);
    MemWrite8(0x481C7F, 0x64, 0x1);

    MemWrite32(0x481C18, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481C22, 0x40, 0x90);

    MemWrite32(0x481C54, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481C5A, 0x40, 0x90);

    MemWrite32(0x481CA6, 0x6BCF64, (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481CAC, 0x40, 0x90);

    //To-Do Modifications_Game_Map_CH

    /*
       //Mouse single choice pointer area show------
       //0044B190  |. A1 68CF6B00    MOV EAX,DWORD PTR DS:[6BCF68]
       //0044B19B  |. 83E8 63        SUB EAX,63
       MemWrite32(0x44B191, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
       MemWrite16(0x44B19B, 0xE883, 0x9090);
       MemWrite8(0x44B19D, 0x63, 0x90);

       //0044B1A5  |. A1 64CF6B00    MOV EAX,DWORD PTR DS:[6BCF64]
       MemWrite32(0x44B1A6, 0x6BCF64, (DWORD)&GAME_RECT.right);
       //0044B1B3  |. 8D48 01        LEA ECX,DWORD PTR DS:[EAX+1]
       MemWrite8(0x44B1B5, 0x1, 0x0);

       //Mouse menu choice pointer area show------
       //0044BCE7  |> A1 68CF6B00    MOV EAX,DWORD PTR DS:[6BCF68]            ;  Default case of switch 0044BB8B
       //0044BCF2  |. 83E8 63        SUB EAX,63
       MemWrite32(0x44BCE8, 0x6BCF68, (DWORD)&GAME_RECT.bottom);
       MemWrite16(0x44BCF2, 0xE883, 0x9090);
       MemWrite8(0x44BCF4, 0x63, 0x90);

       //0044BCFC  |. A1 64CF6B00    MOV EAX,DWORD PTR DS:[6BCF64]
       MemWrite32(0x44BCFD, 0x6BCF64, (DWORD)&GAME_RECT.right);
       //0044BD07  |. 40             INC EAX
       MemWrite8(0x44BD07, 0x40, 0x90);
    */
    /*
    //Mouse single choice pointer area show------
    //0044B190  |. A1 68CF6B00    MOV EAX,DWORD PTR DS:[6BCF68]
    //0044B19B  |. 83E8 63        SUB EAX,63
    MemWrite8(0x44B190, 0xA1, 0xE8);
    FuncWrite32(0x44B191, 0x6BCF68, (DWORD)&fix_mouse_menu_y);
    MemWrite16(0x44B19B, 0xE883, 0x9090);
    MemWrite8(0x44B19D, 0x63, 0x90);

    //0044B1A5  |. A1 64CF6B00    MOV EAX,DWORD PTR DS:[6BCF64]
    MemWrite8(0x44B1A5, 0xA1, 0xE8);
    FuncWrite32(0x44B1A6, 0x6BCF64, (DWORD)&fix_mouse_menu_x);
    //0044B1B3  |. 8D48 01        LEA ECX,DWORD PTR DS:[EAX+1]
    MemWrite8(0x44B1B5, 0x1, 0x0);

    //Mouse menu choice pointer area show------
    //0044BCE7  |> A1 68CF6B00    MOV EAX,DWORD PTR DS:[6BCF68]            ;  Default case of switch 0044BB8B
    //0044BCF2  |. 83E8 63        SUB EAX,63
    MemWrite8(0x44BCE7, 0xA1, 0xE8);
    FuncWrite32(0x44BCE8, 0x6BCF68, (DWORD)&fix_mouse_menu_y);
    MemWrite16(0x44BCF2, 0xE883, 0x9090);
    MemWrite8(0x44BCF4, 0x63, 0x90);

    //0044BCFC  |. A1 64CF6B00    MOV EAX,DWORD PTR DS:[6BCF64]
    MemWrite8(0x44BCFC, 0xA1, 0xE8);
    FuncWrite32(0x44BCFD, 0x6BCF64, (DWORD)&fix_mouse_menu_x);
    //0044BD07  |. 40             INC EAX
    MemWrite8(0x44BD07, 0x40, 0x90);
    */


    //0044B768  |. E8 27070800    CALL Fallout2.004CBE94
    //FuncWrite32(0x44B769, 0x080727,  (DWORD)&check_mouse_in_game_rect);

    FuncWrite32(0x48112C, 0x05AF8D, (DWORD)&create_game_win);

    FuncReplace32(0x4815A2, 0xFFFCAB92, (DWORD)&reset_level_scaling);

    MemWrite32(0x489CDE, 0x6BCF5C, (DWORD)&rcGame_GUI);



    ///0044BD85  |> /E8 9A030800   /CALL 004CC124                           ; [Fallout2.004CC124
   // FuncReplace32(0x44BD86, 0x08039A,  (DWORD)&hold_multi_menu_pos);
    ///0044BE15  |.  E8 FD010800   CALL 004CC017                            ; [Fallout2.004CC017, set_mouse_pos(EAX x, EDX y)
  //  FuncReplace32(0x44BE16, 0x0801FD,  (DWORD)&release_multi_menu_pos);

    ///0044BDA9  |.  E8 24020800   |CALL 004CBFD2                           ; [Fallout2.004CBFD2, get_mouse_pos(EAX *x, EDX *y)
    ///FuncReplace32(0x44BDAA, 0x080224,  (DWORD)&get_multi_menu_pos);


 ///0044BDB7  |.  29D0          |SUB EAX,EDX
 ///0044BDB9  |.  83F8 0A       |CMP EAX,0A
  //  MemWrite8(0x44BDB7, 0x29, 0xE8);
  //  FuncWrite32(0x44BDB9, 0x0AF883D0, (DWORD)&fix_multi_menu_pos);

    ///0044B1BF  |.  E8 2C150000   CALL 0044C6F0                            ; \Fallout2.0044C6F0, mouse_single_menu(EAX xPos, EDX yPos, EBX fidListNum, ECX xMax, Arg1 yMax)
   // FuncReplace32(0x44B1C0, 0x152C, (DWORD)&mouse_menu_single);

    ///0044BD13  |.  E8 4C0C0000   CALL 0044C964                            ; \Fallout2.0044C964, mouse_multi_menu(EAX mouseX, EDX mouseY, EBX listSize?, ECX listNum?, Arg1 toWidth, Arg2 toHeight>
  //  FuncReplace32(0x44BD14, 0x0C4C, (DWORD)&mouse_menu_multi);



    ///0044C6F0  /$  56            PUSH ESI                                 ; MOUSE_SINGLE_MENU(EAX xPos, EDX yPos, EBX fidListNum, ECX xMax, Arg1 yMax)
  //  pMouseMenuSingle = (void*)FixAddress(0x44C6F0);
    ///0044C964  /$  56            PUSH ESI                                 ; MOUSE_MULTI_MENU(EAX xPos, EDX yPos, EBX *fIDLstNumArry, ECX numItems, Arg1 toWidth, Arg2 toHeight)
   // pMouseMenuMulti = (void*)FixAddress(0x44C964);


    //this needs to be updated
    ///004426DA  |.  81FB 50010000 CMP EBX,150
 ///   MemWrite16(0x4426DA, 0xFB81, 0xE890);
 ///   FuncWrite32(0x4426DC, 0x150, (DWORD)&check_zoom_keys);
 //}
    ///0044DBA1  |.  31C9              XOR ECX,ECX
    ///0044DBA3  |.  31C0              XOR EAX,EAX
 ///   MemWrite16(0x44DB9F, 0xD231, 0xE890);
 ///   FuncWrite32(0x44DBA1, 0xC031C931, (DWORD)&check_mouse_edges);


    FuncReplace32(0x44C683, 0x03F33D, (DWORD)&h_get_objects_at_pos);

    if (ConfigReadInt(L"MAPS", L"IGNORE_MAP_EDGES", 0))
        FuncWrite32(0x4812A7, 0x02F665, (DWORD)0x4B091C);

    //IGNORE_PLAYER_SCROLL_LIMITS
    FuncWrite32(0x4812AC, 0x02F680, (DWORD)0x4B093C);

    //path finding
    int multi = ConfigReadInt(L"MAPS", L"NumPathNodes", 1);

    if (multi > 1 && multi < 21) {

        int NumPathNodes = multi * 2000;

        PathFindNodes01 = new DWORD[NumPathNodes * 20];
        PathFindNodes02 = new DWORD[NumPathNodes * 20];

        MemWrite32(0x416177, 2000, NumPathNodes);

        MemWrite32(0x415FE4, 40000, NumPathNodes * 20);
        MemWrite32(0x4160D8, 40000, NumPathNodes * 20);

        MemWrite32(0x415FDF, 0x572B88, (DWORD)PathFindNodes01);

        MemWrite32(0x415F8D, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416035, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416070, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x41607A, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416153, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416189, 0x572B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x415FB6, 0x572BA0, 24 + (DWORD)PathFindNodes01);
        MemWrite32(0x415FA2, 0x572BA4, 28 + (DWORD)PathFindNodes01);
        MemWrite32(0x415FC9, 0x572BA8, 32 + (DWORD)PathFindNodes01);
        MemWrite32(0x415FD6, 0x572BAC, 36 + (DWORD)PathFindNodes01);
        MemWrite32(0x416160, 0x572BB0, 40 + (DWORD)PathFindNodes01);

        MemWrite32(0x4160B9, 0x552FD4, (DWORD)PathFindNodes02);
        MemWrite32(0x41628E, 0x552FD4, (DWORD)PathFindNodes02);
        MemWrite32(0x4162BA, 0x552FD4, (DWORD)PathFindNodes02);
        MemWrite32(0x4162A6, 0x552FE8, 20 + (DWORD)PathFindNodes02);
    }


    //FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
    //NOW PLAYER POSITION SET BEFORE SCROLL
    MemWrite32(0x482260, 0x67C3B4, 0x52934C);

    MemWrite8(0x4B24B4, 0x53, 0xE9);
    FuncWrite32(0x4B24B5, 0x55575651, (DWORD)&set_view_focus);

    MemWrite16(0x4B05FC, 0x5651, 0xE990);
    FuncWrite32(0x4B05FE, 0xDE895557, (DWORD)&get_next_hex_pos);

    MemWrite8(0x4B03EC, 0x53, 0xE9);
    FuncWrite32(0x4B03ED, 0x55575651, (DWORD)&get_hex_dist);

    MemWrite8(0x4AFE88, 0x53, 0xE9);
    FuncWrite32(0x4AFE89, 0x55575651, (DWORD)&set_view_pos);

    FuncWrite32(0x4B1788, 0xFFF68494, (DWORD)&check_angled_roof_tile_edge2);
    //To-Do Fog of War chinese
    /*
    ///done///00482D40  |.  E8 47220400   CALL 004C4F8C                            ; [Fallout2.004C4F8C, fileStream* f_open_file(EAX *FileName, EDX *flags)
    FuncReplace32(0x482D41, 0x042247, (DWORD)&mapfile_open_for_saving);

    ///done///0047AE35  |.  E8 7A460000   CALL 0047F4B4                            ; [Fallout2.0047F4B4, int delete_save_files(EAX *pPath, EDX *pExt)
    FuncReplace32(0x47AE36, 0x467A, (DWORD)&game_data_files_delete);
    ///done///0047AE67  |.  E8 48460000   CALL 0047F4B4                            ; [Fallout2.0047F4B4, int delete_save_files(EAX *pPath, EDX *pExt)
    FuncReplace32(0x47AE68, 0x4648, (DWORD)&game_data_files_delete);
    ///done///0047EB35  |.  E8 7A090000   CALL 0047F4B4                            ; [Fallout2.0047F4B4, int delete_save_files(EAX *pPath, EDX *pExt)
    FuncReplace32(0x47EB36, 0x097A, (DWORD)&game_data_files_delete);
    ///done///0047EF27  |.  E8 88050000   CALL 0047F4B4                            ; [Fallout2.0047F4B4, int delete_save_files(EAX *pPath, EDX *pExt)
    FuncReplace32(0x47EF28, 0x0588, (DWORD)&game_data_files_delete);
    ///done///0047F4A5  |.  E8 0A000000   CALL 0047F4B4                            ; [Fallout2.0047F4B4, int delete_save_files(EAX *pPath, EDX *pExt)
    FuncReplace32(0x47F4A6, 0x0A, (DWORD)&game_data_files_delete);

    ///done///0047ECA9  |.  83C4 08       |ADD ESP,8
    ///0047ECAC  |.  83F8 FF       |CMP EAX,-1
    MemWrite16(0x47ECA9, 0xC483, 0xE890);
    FuncWrite32(0x47ECAB, 0xFFF88308, (DWORD)&game_data_files_copy);

    ///done///0047F10C  |.  83C4 08       |ADD ESP,8
    ///0047F10F  |.  83F8 FF       |CMP EAX,-1
    MemWrite16(0x47F10C, 0xC483, 0xE890);
    FuncWrite32(0x47F10E, 0xFFF88308, (DWORD)&game_data_files_copy);
    */
    ///done///00481F25  |.  E8 1F300400   CALL 004C4F49                            ; [Fallout2.004C4F49, f_close_file(EAX *FileStrea )
    //FuncReplace32(0x481F26, 0x04301F, (DWORD)&fog_of_war_load);

 //block fallout method for marking objects as visible.
    if (FOG_OF_WAR)
        MemWrite8(0x48BBA0, 0x53, 0xC3);

}


//_________________________________
void Modifications_Game_Map_MULTI() {

    MemWrite32(0x481D9E, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481DAD, 0x1, 0x0);

    MemWrite32(0x481DB5, FixAddress(0x6AC9FC), (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x481DC1, 0xE883, 0x9090);
    MemWrite8(0x481DC3, 0x63, 0x90);

    MemWrite32(0x481E0F, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x481E1B, 0x1, 0x0);

    MemWrite32(0x481E23, FixAddress(0x6AC9FC), (DWORD)&rcGame_GUI.bottom);
    MemWrite8(0x481E2E, 0x9D, 0x0);

    MemWrite32(0x48278D, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x482799, 0x1, 0x0);

    MemWrite32(0x4827A1, FixAddress(0x6AC9FC), (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x4827A7, 0xE883, 0x9090);
    MemWrite8(0x4827A9, 0x63, 0x90);

    MemWrite32(0x482BED, FixAddress(0x6AC9FC), (DWORD)&rcGame_GUI.bottom);
    MemWrite16(0x482BF3, 0xE883, 0x9090);
    MemWrite8(0x482BF5, 0x63, 0x90);

    MemWrite32(0x482BFE, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x482C08, 0x1, 0x0);

    MemWrite32(0x482847, FixAddress(0x6AC9FC), (DWORD)&rcGame_GUI.bottom);
    MemWrite8(0x48284F, 0x64, 0x1);

    MemWrite32(0x4827E8, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x4827F2, 0x40, 0x90);

    MemWrite32(0x482824, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x48282A, 0x40, 0x90);

    MemWrite32(0x482876, FixAddress(0x6AC9F8), (DWORD)&rcGame_GUI.right);
    MemWrite8(0x48287C, 0x40, 0x90);

    FuncReplace32(0x481CFC, 0x054538, (DWORD)&create_game_win);

    FuncReplace32(0x482172, 0xFFFCA872, (DWORD)&reset_level_scaling);

    //In move object function
    MemWrite32(0x48A8DE, FixAddress(0x6AC9F0), (DWORD)&rcGame_GUI);

    MemWrite8(0x442D56, 0x31, 0xE8);
    FuncWrite32(0x442D57, 0x05F883ED, (DWORD)&game_check_control_keys);

    MemWrite8(0x44E42C, 0x53, 0xE9);
    FuncWrite32(0x44E42D, 0x55575651, (DWORD)&scroller);

    FuncReplace32(0x412AB1, 0x0B7F27, (DWORD)&get_mouse_pos_on_game_portal);
    FuncReplace32(0x418015, 0x0B29C3, (DWORD)&get_mouse_pos_on_game_portal);
    FuncReplace32(0x442D8A, 0x087C4E, (DWORD)&get_mouse_pos_on_game_portal);

    MemWrite8(0x44B748, 0xA1, 0xE8);
    FuncWrite32(0x44B749, 0x518C0C, (DWORD)&get_mouse_pic_ref_adjust_position_on_game_portal);

    FuncReplace32(0x44CAAC, 0x07DF2C, (DWORD)&get_mouse_pos_on_game_portal);
    FuncReplace32(0x44CEE2, 0x07DAF6, (DWORD)&get_mouse_pos_on_game_map);
    FuncReplace32(0x44DA82, 0x07CF56, (DWORD)&get_mouse_pos_on_game_portal);


    isMouseScrollBoundaryEnabled = (LONG*)FixAddress(0x518C08);
    //doesn't seem to ever be enabled
    ignore_isMouseAtScrollBoundary = (LONG*)FixAddress(0x518C04);
    isMouseAtScrollBoundary = (LONG*)FixAddress(0x518D98);

    // script obj_on_screen fix
    MemWrite32(0x45C886, FixAddress(0x453FC0), (DWORD)&rcGame_PORTAL);


    MemWrite16(0x4B1754, 0x5651, 0xE990);
    FuncWrite32(0x4B1756, 0xC1895557, (DWORD)&sqr_to_hex);

    MemWrite8(0x4B1674, 0x56, 0xE9);
    FuncWrite32(0x4B1675, 0xC6895557, (DWORD)&hex_to_sqr);


    //keep the mouse hex position set to a valid hex when the mouse is out of bounds (when drawing).
    FuncReplace32(0x44DFD7, 0x00063779, (DWORD)&get_nearest_valid_hexpos);
    FuncReplace32(0x44E229, 0x00063527, (DWORD)&get_nearest_valid_hexpos);


    FuncReplace32(0x412ABF, 0x09EC91, (DWORD)&adjust_game_mouse_pos);
    //keep the mouse hex position set to a valid hex when the mouse is out of bounds (when clicking).
    FuncReplace32(0x418022, 0x09972E, (DWORD)&adjust_game_mouse_pos_to_nearest_valid_hexpos);

    MemWrite16(0x44DF9D, 0xC589, 0xE890);
    FuncWrite32(0x44DF9F, 0xDE89D789, (DWORD)&adjust_game_mouse_pos2);

    MemWrite16(0x4B1F94, 0x5756, 0xE990);
    FuncWrite32(0x4B1F96, 0x245C8B55, (DWORD)&sqr_to_floor_tile);

    MemWrite8(0x4B1DC0, 0x56, 0xE9);
    FuncWrite32(0x4B1DC1, 0xC6895557, (DWORD)&floor_tile_to_sqr);

    MemWrite16(0x4B203C, 0x5756, 0xE990);
    FuncWrite32(0x4B203E, 0x245C8B55, (DWORD)&sqr_to_roof_tile);

    MemWrite8(0x4B1E60, 0x56, 0xE9);
    FuncWrite32(0x4B1E61, 0xC6895557, (DWORD)&roof_tile_to_sqr);




    FuncReplace32(0x44CF33, 0x03F68D, (DWORD)&h_get_objects_at_pos);

    if (ConfigReadInt(L"MAPS", L"IGNORE_MAP_EDGES", 0))
        FuncReplace32(0x481E77, 0x02FF05, (DWORD)0x4B1D8C);

    //IGNORE_PLAYER_SCROLL_LIMITS
    FuncReplace32(0x481E7C, 0x02FF20, (DWORD)0x4B1DAC);

    //path finding
    int multi = ConfigReadInt(L"MAPS", L"NumPathNodes", 1);

    if (multi > 1 && multi < 21) {
        int NumPathNodes = multi * 2000;

        PathFindNodes01 = new DWORD[NumPathNodes * 20];
        PathFindNodes02 = new DWORD[NumPathNodes * 20];

        MemWrite32(0x416177, 2000, NumPathNodes);

        MemWrite32(0x415FE4, 40000, NumPathNodes * 20);
        MemWrite32(0x4160D8, 40000, NumPathNodes * 20);

        MemWrite32(0x415FDF, 0x562B88, (DWORD)PathFindNodes01);

        MemWrite32(0x415F8D, 0x562B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416035, 0x562B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416070, 0x562B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x41607A, 0x562B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416153, 0x562B9C, 20 + (DWORD)PathFindNodes01);
        MemWrite32(0x416189, 0x562B9C, 20 + (DWORD)PathFindNodes01);

        MemWrite32(0x415FB6, 0x562BA0, 24 + (DWORD)PathFindNodes01);

        MemWrite32(0x415FA2, 0x562BA4, 28 + (DWORD)PathFindNodes01);

        MemWrite32(0x415FC9, 0x562BA8, 32 + (DWORD)PathFindNodes01);

        MemWrite32(0x415FD6, 0x562BAC, 36 + (DWORD)PathFindNodes01);

        MemWrite32(0x416160, 0x562BB0, 40 + (DWORD)PathFindNodes01);

        MemWrite32(0x4160B9, 0x542FD4, (DWORD)PathFindNodes02);
        MemWrite32(0x41628E, 0x542FD4, (DWORD)PathFindNodes02);
        MemWrite32(0x4162BA, 0x542FD4, (DWORD)PathFindNodes02);

        MemWrite32(0x4162A6, 0x542FE8, 20 + (DWORD)PathFindNodes02);
    }


    //FIX - ORIGINALLY PLAYER POSITION SET TO SCROLL POSITION FOR JUMP TO MAP
    //NOW PLAYER POSITION SET BEFORE SCROLL
    MemWrite32(0x482E30, 0x66BE34, FixAddress(0x51955C));


    MemWrite8(0x4B3924, 0x53, 0xE9);
    FuncWrite32(0x4B3925, 0x55575651, (DWORD)&set_view_focus);

    MemWrite16(0x4B1A6C, 0x5651, 0xE990);
    FuncWrite32(0x4B1A6E, 0xDE895557, (DWORD)&get_next_hex_pos);

    MemWrite8(0x4B185C, 0x53, 0xE9);
    FuncWrite32(0x4B185D, 0x55575651, (DWORD)&get_hex_dist);

    MemWrite8(0x4B12F8, 0x53, 0xE9);
    FuncWrite32(0x4B12F9, 0x55575651, (DWORD)&set_view_pos);

    FuncReplace32(0x4B2BF8, 0xFFF6708C, (DWORD)&check_angled_roof_tile_edge2);


    //Reading/Writing light colour and radius for maps and pc in save.dat.
    FuncReplace32(0x4892FE, 0x0003CF12, (DWORD)&object_light_colour_and_radius_write);
    FuncReplace32(0x488BF2, 0x0003D556, (DWORD)&object_light_colour_and_radius_read);

    //fog of war data is saved here.
    FuncReplace32(0x483911, 0x000425B3, (DWORD)&mapfile_open_for_writing);
    FuncReplace32(0x483925, 0x0004258B, (DWORD)&mapfile_close_for_writing);
 
    FuncReplace32(0x482AE2, 0x000433E2, (DWORD)&mapfile_open_for_reading);
    //fog of war data is loaded here.
    FuncReplace32(0x482AF6, 0x000433BA, (DWORD)&mapfile_close_for_reading);
 
    FuncReplace32(0x47B980, 0x0004A544, (DWORD)&save_dat_open_for_reading);
    FuncReplace32(0x47DD13, 0x000481B1, (DWORD)&save_dat_open_for_reading);
    FuncReplace32(0x47DA15, 0x000484AF, (DWORD)&save_dat_open_for_writing);

    FuncReplace32(0x47B9A0, 0x0004A510, (DWORD)&save_dat_close);
    FuncReplace32(0x47DAB6, 0x000483FA, (DWORD)&save_dat_close);
    FuncReplace32(0x47DB4F, 0x00048361, (DWORD)&save_dat_close);
    FuncReplace32(0x47DBE5, 0x000482CB, (DWORD)&save_dat_close);
    FuncReplace32(0x47DD67, 0x00048149, (DWORD)&save_dat_close);
    FuncReplace32(0x47DDEF, 0x000480C1, (DWORD)&save_dat_close);
    FuncReplace32(0x47DE50, 0x00048060, (DWORD)&save_dat_close);
    //FuncReplace32(0x47E61B, 0x000478A9, (DWORD)&save_dat_open_for_reading);
    //FuncReplace32(0x47E6A0, 0x00047810, (DWORD)&save_dat_close);

    //delete game data, ve and fog data.
    FuncReplace32(0x47B836, 0x00004806, (DWORD)&game_data_files_delete);
    FuncReplace32(0x47B868, 0x000047D4, (DWORD)&game_data_files_delete);
    FuncReplace32(0x47F6C2, 0x0000097A, (DWORD)&game_data_files_delete);
    FuncReplace32(0x47FAB4, 0x00000588, (DWORD)&game_data_files_delete);
    FuncReplace32(0x480032, 0x0000000A, (DWORD)&game_data_files_delete);

    //Saving game data, copy ve/fog files from maps folder to save slot.
    MemWrite16(0x47F835, 0xC483, 0xE890);
    FuncWrite32(0x47F837, 0xFFF88308, (DWORD)&game_data_files_copy);

    //Loading game data, copy ve/fog files from save slot to maps folder.
    MemWrite16(0x47FC98, 0xC483, 0xE890);
    FuncWrite32(0x47FC9A, 0xFFF88308, (DWORD)&game_data_files_copy);

    //read prototype light colour when closing .pro/sav file after loading prototype data.
    FuncReplace32(0x4A1D81, 0x0002412F, (DWORD)&proto_file_close_for_reading);

    //To-Do Fog of War

    //block fallout method for marking objects as visible. //fallout2.0048C7A0(guessed void)  MARK_VISIBLE_OBJS() within 400 pix from PC
    if (FOG_OF_WAR)
        MemWrite8(0x48C7A0, 0x53, 0xC3);



    //is_hex_visible
    //CPU Disasm
    //0045404C / $  53            PUSH EBX; is_hex_visible(EAX hexNum) is hex with - in a box 320x240 around view centre hex
    //0045404D | .  51            PUSH ECX
    //0045404E | .  52            PUSH EDX
    //0045404F | .  8B1D 34BE6600 MOV EBX, DWORD PTR DS : [view_hexPos] ; used to check hex vis
    MemWrite16(0x45404C, 0x5153, 0xE890);
    FuncWrite32(0x45404E, 0x341D8B52, (DWORD)&is_hex_visible);
}


//___________________________
void Modifications_Game_Map() {

    FOG_OF_WAR = ConfigReadInt(L"MAPS", L"FOG_OF_WAR", 0);
    if (FOG_OF_WAR < 0)
        FOG_OF_WAR = 0;

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Game_Map_CH();
    else
        Modifications_Game_Map_MULTI();
}


