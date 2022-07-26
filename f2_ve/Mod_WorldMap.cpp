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
#include "Fall_Text.h"
#include "Fall_Scripts.h"
#include "Fall_Msg.h"
#include "Fall_GameMap.h"

#include "Dx_General.h"
#include "Dx_Windows.h"
#include "Dx_RenderTarget.h"
#include "Dx_Game.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

//WORLD_MAP.msg
#define WM_MSG_OFFSET_CITY_ENTRY_POINTS     200
#define WM_MSG_OFFSET_TERRAIN_TYPES         1000
#define WM_MSG_OFFSET_TERRAIN_UNKNOWN       1004
//MAP.msg
#define WM_MSG_OFFSET_SHORT_CITY_NAMES       1500


#define WM_NUM_SUB_TILES_X  7
#define WM_NUM_SUB_TILES_Y  6
#define WM_NUM_SUB_TILES   WM_NUM_SUB_TILES_X * WM_NUM_SUB_TILES_Y


LONG SFALL_ExpandWorldMap = 0;
LONG SFALL_WorldMapTerrainInfo = 0;
LONG SFALL_WorldMapTravelMarkers = 0;
LONG SFALL_WorldMapSlots = 17;


struct SCRIPT_DATA {
    DWORD unk00;
    DWORD unk08;
    DWORD unk0C;
    DWORD unk10;
};

struct TILE_DATA_CELL {
    DWORD terrain;//0x00;
    DWORD fill;//0x04;
    DWORD morning_chance;//0x08;
    DWORD afternoon_chance;//0x0C;
    DWORD night_chance;//0x10;
    DWORD enc_type;//0x14;
    DWORD fog_level;//0x18;
};

struct TILE_DATA {
    DWORD frmID;//0x00;
    DWORD frmObj;//0x04;
    BYTE* frmBuff;//0x08;
    char walk_mask_name[40];//0x0C
    BYTE* walk_mask_buff;//0x34; pointer to mask data 13200 bytes
    DWORD encounter_difficulty;//0x38;
    TILE_DATA_CELL cell[WM_NUM_SUB_TILES];//0x3C
};

struct ENTRANCE_DATA {
    LONG on_off;// DWORD unknown_0x00;
    LONG x_pos;// DWORD unknown_0x04;
    LONG y_pos;// DWORD unknown_0x08;
    LONG map_index;// DWORD unknown_0x0C;
    LONG elevation;// DWORD unknown_0x10;
    LONG tile_number;// DWORD unknown_0x14;
    LONG orientation;// DWORD unknown_0x18;
};

struct CITY_DATA {
    char area_name[40];
    LONG area_num;//0x28;
    LONG world_pos_x;//0x2C;
    LONG world_pos_y;//0x30;
    LONG size;//0x34;
    LONG start_state;//_0x38;
    LONG lock_state;//0x3C;
    DWORD unknown_0x40;
    DWORD townmap_art_frmID;//0x44;
    DWORD townmap_label_art_frmID;//0x48;
    LONG num_of_entrances;//0x4C;
    ENTRANCE_DATA entrance[10];//0x50
};

struct IMAGE_DETAILS {
    DWORD frmID;//0x00;
    DWORD width;//0x04;
    DWORD height;//0x08;
    FRMframeDx* pFrame;//0x0C; //previously void* frmObj;
    BYTE* buff;//0x10;
};

LONG* p_wm_num_tiles = nullptr;
LONG* p_wm_num_horizontal_tiles = nullptr;
TILE_DATA** pp_wm_tile_data = nullptr;

TILE_DATA_CELL** pp_wm_current_tile_cell = nullptr;

LONG* p_wm_num_cities = nullptr;
CITY_DATA** pp_wm_city_data = nullptr;


DWORD* p_wm_tick_count = nullptr;
LONG* p_wm_last_font = nullptr;

BYTE** pp_wm_win_buff = nullptr;


BOOL* p_wm_has_vehicle = nullptr;

IMAGE_DETAILS* p_worldSphereImage = nullptr;


DWORD* p_hotspot_width = nullptr;
DWORD* p_hotspot_height = nullptr;

LONG* p_wm_city_menu_scroll_limit = nullptr;// p_wmtabs_height = nullptr;

LONG* p_wm_city_list_final_pos = nullptr;
LONG* p_wm_city_list_offset_unit = nullptr;
LONG* p_wm_city_list_y_pos = nullptr;


FRMCached* pfrm_hotspot1 = nullptr;
FRMCached* pfrm_hotspot2 = nullptr;
FRMCached* pfrm_wmaptarg = nullptr;//wmaptarg.frm; World map move target maker #1
FRMCached* pfrm_wmaploc = nullptr;//wmaploc.frm; World map location maker
//FRMCached* pfrm_wmtabs = nullptr;//WMTABS.FRM; Worldmap Town Tabs Underlay
//FRMCached* pfrm_wmtbedge = nullptr;//WMTBEDGE.FRM   ; Worldmap Town Tabs Edging overlay
FRMCached* pfrm_wmdial = nullptr;//WMDIAL.FRM     ; Worldmap Night/Day Dial
//FRMCached* pfrm_wmscreen = nullptr;//WMSCREEN.FRM; Worldmap Overlay Screen
//FRMCached* pfrm_wmglobe = nullptr;//WMGLOBE.FRM; Worldmap Globe Stamp overlay
FRMCached* pfrm_months = nullptr;//months.frm     ; month strings for pip boy
FRMCached* pfrm_numbers = nullptr;//NUMBERS.FRM    ; numbers for the hit points and fatigue counters
FRMCached* pfrm_wmcarmve = nullptr;//WMCARMVE.FRM   ; WorldMap Car Movie

FRMCached* pfrm_town_art = nullptr;//CITY_DATA->townmap_art_frmID



FRMdx** ppfrmdx_wmdial = nullptr;
FRMdx** ppfrmdx_wmcarmve = nullptr;

FRMframeDx* pframe_wm_fuel_empty = nullptr;
FRMframeDx* pframe_wm_fuel_full = nullptr;

//dx sub window refs
int winRef_WM_Background = -1;
int winRef_WM_TownView = -1;
int winRef_WM_TownView_Text = -1;
int winRef_WM_Date_Time = -1;
int winRef_WM_Fuel = -1;
int winRef_WM_City_List = -1;
int winRef_WM_City_Edge = -1;
int winRef_WM_Map = -1;


LONG buttRef_WM_Vehicle = -1;
LONG buttRef_WM_Dial = -1;


DWORD* p_list_num_encounters = nullptr;
#define num_encounter_icons 4
FRMCached* pfrm_encounter_icons[num_encounter_icons] = {0};


char* p_wm_world_music = nullptr;
char* p_wm_world_music_car = nullptr;

void(*fall_WM_SetMouseImage)() = nullptr;
void* pfall_wm_create_area_list = nullptr;

//The number of accessable locations in the city menu.
LONG* p_wm_city_list_current_size = nullptr;
//holds a list of area number for the accessable locations in the city menu, arranged in alphabetical order.
DWORD** pp_wm_city_list_current = nullptr; 


//The number of scrollable items in the city menu, this will be the number of menu buttons if there are more buttons than the number of cities currently in the list.
LONG wm_city_list_menu_size = 0;

BOOL* p_wm_setup_flag = nullptr;


LONG* p_wm_area_number_selected = nullptr;
LONG* p_wm_area_number_town_view = nullptr;


LONG* p_wm_town_view_button = nullptr;


MSGList* pMsgList_WorldMap = nullptr;
MSGList* pMsgList_Map = nullptr;

LONG* p_wm_dial_frame_num = nullptr;
LONG* p_wm_vehicle_frame_num = nullptr;
LONG* p_wm_vehicle_fuel_level = nullptr;


LONG* p_wm_view_pos_x = nullptr;
LONG* p_wm_view_pos_y = nullptr;

LONG* p_wm_selected_pos_x = nullptr;
LONG* p_wm_selected_pos_y = nullptr;

LONG* p_wm_current_pos_x = nullptr;
LONG* p_wm_current_pos_y = nullptr;

LONG* p_wm_encounter_flag = nullptr;
LONG* p_wm_encounter_mapID = nullptr;
LONG* p_wm_encounter_type = nullptr;
LONG* p_wm_encounter_num_in_table = nullptr;

LONG* p_wm_encounter_icon_type = nullptr;

LONG* p_wm_hot_spot_selected = nullptr;

LONG* p_wm_num_terrain_types = nullptr;
char** ppfall_wm_terrain_types = nullptr;


LONG* p_wm_is_pc_moving = nullptr;

RenderTarget2* p_RT_WorldMap = nullptr;
RenderTarget* p_RT_WorldMap_Fog = nullptr;

ID3D11Buffer* pd3dVB_pixel_quad = nullptr;

XMFLOAT4 wm_travel_dot_colour = { 0.0f,0.0f,0.0f,0.0f };


char** terrain_types = nullptr;

int* pterrain_types_markers[2] = { nullptr };

bool wm_is_mouse_over_hotspot = false;
bool wm_was_mouse_off_screen_on_start = false;

void* pfall_wm_is_area_visible = nullptr;

LONG* p_wm_view_right_limit = nullptr;
LONG* p_wm_view_bottom_limit = nullptr;

//to prevent scrolling if mouse is outside the window boundary when world map is first opened.
bool wm_map_mouse_scrolling_enabled = false;

BYTE* p_CHECK_VOODOO_remove_circle_name = nullptr;
bool wm_draw_cities = true;

//last x position on world map
static LONG wm_last_pos_x = -1;
//last y position on world map
static LONG wm_last_pos_y = -1;


LONG* p_wm_menu_buttRef = nullptr;

LONG* p_wm_city_button_code_first = nullptr;
//LONG* p_wm_city_button_code_last_plus_one = nullptr;

LONG num_menu_buttons = 7;

//BYTE* p_wm_city_button_code_mem_offset_01 = nullptr;
//BYTE* p_wm_city_button_code_mem_offset_02 = nullptr;
//BYTE* p_wm_city_button_code_mem_offset_03 = nullptr;
//BYTE* p_wm_city_button_code_mem_offset_04 = nullptr;

//LONG* p_wm_city_list_menu_y_max = nullptr;





//world map scale level
LONG wm_scale_level = 1;

//world map scale ratio
float wm_scale_RO = 1.0f;
//world map scale ratio max zoom out
float wm_scale_RO_max_out = 1.0f;
//world map scale ratio max zoom in
float wm_scale_RO_max_in = 1.0f;


//_______________________________________
BOOL fall_WM_IsAreaVisible(LONG area_num) {
    BOOL retVal = FALSE;
    __asm {

        mov eax, area_num
        call pfall_wm_is_area_visible
        mov retVal, eax
    }
    return retVal;

}


//_______________________________________________________________________
LONG fall_WM_CreateAreaList(DWORD** p_area_name_list, LONG * p_num_areas) {//creates a list of area numbers with "townmap_label_art_frmID != -1" and sorts them in alphabetical order.
        LONG retVal = 0;
    __asm {
        mov edx, p_num_areas
        mov eax, p_area_name_list
        call pfall_wm_create_area_list
        mov retVal, eax
    }
    return retVal;

}


//_______________________
LONG WM_TownView_Update() {//nothing needs to happen here, everything done in "WM_TownView_Setup".
    //Fallout_Debug_Info("WM_TownView_Update success...");
    return 0;
}


//________________________________________________
void __declspec(naked) h_wm_town_view_update(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_TownView_Update

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
LONG WM_TownView_Destructor() {
    //Fallout_Debug_Info("WM_TownView_Destructor start...");
    if (*pWinRef_WorldMap == -1)
        return -1;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return -1;



    if (*p_wm_area_number_town_view != -1) {
        //CITY_DATA* pCity = pp_wm_city_data[*p_wm_area_number_town_view];
        CITY_DATA* pCity = &(*pp_wm_city_data)[*p_wm_area_number_town_view];
        if (pCity->num_of_entrances > 0) {
            for (int i = 0; i < pCity->num_of_entrances; i++) {
                fall_Button_Destroy(p_wm_town_view_button[i]);
                p_wm_town_view_button[i] = -1;
            }
        }
    }


    if (winRef_WM_TownView != -1)
        pWin->winDx->DeleteSubWin(winRef_WM_TownView);
    winRef_WM_TownView = -1;
    if (winRef_WM_TownView_Text != -1)
        pWin->winDx->DeleteSubWin(winRef_WM_TownView_Text);
    winRef_WM_TownView_Text = -1;
    
    //Fallout_Debug_Info("WM_TownView_Destructor success...");
    fall_Event_Add(fall_WM_SetMouseImage);
    return 0;
}


//____________________________________________________
void __declspec(naked) h_wm_town_view_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_TownView_Destructor

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
LONG WM_TownView_Setup() {
    //Fallout_Debug_Info("WM_TownView_Setup start...");
    if (*pWinRef_WorldMap == -1)
        return -1;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return -1;

    *p_wm_area_number_town_view = *p_wm_area_number_selected;
    if (*p_wm_area_number_town_view == -1)
        return -1;

    CITY_DATA* pCity = &(*pp_wm_city_data)[*p_wm_area_number_town_view];

    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    pfrm = new FRMCached(pCity->townmap_art_frmID);
    pFrame = pfrm->GetFrame(0, 0);
    float scale = 1.0f;
    Window_DX* subwin = nullptr;
    if (pFrame) {
        subwin = new Window_DX((float)22, (float)21, pFrame->GetWidth(), pFrame->GetHeight(), 0x000000FF, pWin->winDx, &winRef_WM_TownView);
        subwin->ClearRenderTarget(nullptr);
        subwin->ArrangeWin(2);//put just behind main window
        subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);

        scale = (float)(pWin->width - (640 - (22 + 450)) - 22) / pFrame->GetWidth();
        subwin->SetScale(scale, scale);
        subwin->SetClippingRect(nullptr);
    }
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;


    subwin = new Window_DX((float)22, (float)21, pWin->width - (640 - (22 + 450)) - 22, pWin->height - (480 - (21 + 450)) - 21, 0x00000000, pWin->winDx, &winRef_WM_TownView_Text);
    subwin->ClearRenderTarget(nullptr);
    subwin->ArrangeWin(2);

    ButtonStruct_DX* p_butt = nullptr;

    DWORD colour = 0xFFFFFFFF;
    if (color_pal)
        colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
    XMFLOAT4 fcolour_green(((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f);

    char* pText = nullptr;
    FONT_FUNC_STRUCT* font = GetCurrentFont();

    LONG txtWidth = 0;
    LONG txtHeight = font->GetFontHeight();

    DWORD msgNum = *p_wm_area_number_town_view * 10 + WM_MSG_OFFSET_CITY_ENTRY_POINTS;
    if (pCity->num_of_entrances > 0) {
        for (int i = 0; i < pCity->num_of_entrances; i++) {
            if (pCity->entrance[i].on_off != 0 && pCity->entrance[i].x_pos != -1 && pCity->entrance[i].y_pos != -1) {
                LONG button_x = (LONG)(scale * (pCity->entrance[i].x_pos - 22)) + 22;
                LONG button_y = (LONG)(scale * (pCity->entrance[i].y_pos - 21)) + 21;

                p_wm_town_view_button[i] = CreateButtonX_Overlay(*pWinRef_WorldMap, button_x, button_y, *p_hotspot_width, *p_hotspot_height, -1, 0x815, -1, 0x31 + i, 0x060000A8, 0x060000DF, 0, FLG_ButtTrans);
                //Fallout_Debug_Info("WM_TownView_ entrance button created");
                p_butt = (ButtonStruct_DX*)fall_Button_Get(p_wm_town_view_button[i], nullptr);
                if (p_butt) {
                    pText = (char*)GetMsg(pMsgList_WorldMap, msgNum + i, 2);
                    if (pText) {
                        txtWidth = font->GetTextWidth(pText);
                        if (txtWidth > 0) {
                            if (subwin)
                                subwin->Draw_Text(pText, button_x - 22 + ((LONG)*p_hotspot_width - txtWidth) / 2, button_y - 21 + (LONG)*p_hotspot_height + 2, colour, 0xFF000000, TextEffects::dropShadow);
                        }
                    }
                }
            }
            else
                p_wm_town_view_button[i] = -1;
        }
    }
    //Fallout_Debug_Info("WM_TownView_Setup success...");
    fall_Event_Remove(fall_WM_SetMouseImage);
    return 0;
}


//_______________________________________________
void __declspec(naked) h_wm_town_view_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_TownView_Setup

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


#define FUEL_GAUGE_WIDTH    3
#define FUEL_GAUGE_HEIGHT   70
#define FUEL_GAUGE_SIZE     FUEL_GAUGE_WIDTH * FUEL_GAUGE_HEIGHT

//________________________
void WM_Fuel_Gauge_Setup() {

    BYTE* buff = new BYTE[FUEL_GAUGE_SIZE];
    int offset = 0;
    while (offset < FUEL_GAUGE_SIZE) {
        buff[offset] = 0xC;
        offset++;
        buff[offset] = 0xB;
        offset++;
        buff[offset] = 0xC;
        offset++;
        buff[offset] = 0xF;
        offset++;
        buff[offset] = 0xE;
        offset++;
        buff[offset] = 0xF;
        offset++;
    }
    pframe_wm_fuel_empty = new FRMframeDx(nullptr, FUEL_GAUGE_WIDTH, FUEL_GAUGE_HEIGHT, buff, nullptr, false, false);
    offset = 0;
    while (offset < FUEL_GAUGE_SIZE) {
        buff[offset] = 0xC4;
        offset++;
        buff[offset] = 0xC4;
        offset++;
        buff[offset] = 0xC4;
        offset++;
        buff[offset] = 0xC8;
        offset++;
        buff[offset] = 0xC5;
        offset++;
        buff[offset] = 0xC8;
        offset++;
    }
    pframe_wm_fuel_full = new FRMframeDx(nullptr, FUEL_GAUGE_WIDTH, FUEL_GAUGE_HEIGHT, buff, nullptr, false, false);
    delete[] buff;
    buff = nullptr;
}


//_________________________
void WM_Fuel_Gauge_Update() {
    if (*pWinRef_WorldMap == -1)
        return;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;
    if (winRef_WM_Fuel != -1)
        subwin = pWin->winDx->GetSubWin(winRef_WM_Fuel);
    else
        subwin = new Window_DX((float)pWin->width - (640 - (499.0f - 3)), (float)pWin->height - (480 - 339.0f), FUEL_GAUGE_WIDTH, FUEL_GAUGE_HEIGHT, 0x00000000, pWin->winDx, &winRef_WM_Fuel);


    if (!subwin)
        return;
    subwin->ClearRenderTarget(nullptr);
    //return;
    if (pframe_wm_fuel_empty)
        subwin->RenderTargetDrawFrame(0, 0, pframe_wm_fuel_empty, nullptr, nullptr);


    LONG fuel_level = *p_wm_vehicle_fuel_level * FUEL_GAUGE_HEIGHT / 80000;
    //Fallout_Debug_Info("WM_Update_Fuel_Gauge %d", fuel_level);

    fuel_level &= 0xFFFFFFFE;
    if (fuel_level < 0)
        fuel_level = 0;
    else if (fuel_level > FUEL_GAUGE_HEIGHT)
        fuel_level = FUEL_GAUGE_HEIGHT;
    D3D11_RECT rect{ 0, FUEL_GAUGE_HEIGHT - fuel_level, FUEL_GAUGE_WIDTH, FUEL_GAUGE_HEIGHT };
    if (pframe_wm_fuel_full)
        subwin->RenderTargetDrawFrame(0, 0, pframe_wm_fuel_full, nullptr, &rect);


}


#define DIGIT_WIDTH  9
#define DIGIT_HEIGHT  17
//#define DIGIT_RED  DIGIT_WIDTH * 12

//______________________________________________________________________________________________________________________________
void WM_Draw_Number(ID3D11DeviceContext* pD3DDevContext, XMMATRIX* pOrtho2D, LONG xPos, LONG yPos, LONG number, int digit_count) {

    if (!pfrm_numbers)
        return;
    FRMframeDx* pFrame = pfrm_numbers->GetFrame(0, 0);
    if (!pFrame)
        return;

    LONG digit = 0;
    LONG digit_x = 0;
    D3D11_RECT digit_rect;

    xPos += digit_count * DIGIT_WIDTH;
    for (int i = 0; i < digit_count; i++) {
        digit = number % 10;
        digit_x = digit * DIGIT_WIDTH;// +offset_colour;

        xPos -= DIGIT_WIDTH;
        digit_rect = { xPos, yPos, xPos + DIGIT_WIDTH, yPos + DIGIT_HEIGHT };
        number /= 10;

        pD3DDevContext->RSSetScissorRects(1, &digit_rect);
        pFrame->DrawFrame(pOrtho2D, (float)(digit_rect.left - digit_x), (float)yPos);
    }
}


#define MONTH_HEIGHT  15
//___________________________________________________________________________________________________________
void WM_Draw_Month(ID3D11DeviceContext* pD3DDevContext, XMMATRIX* pOrtho2D, LONG xPos, LONG yPos, LONG month) {

    if (!pfrm_months)
        return;
    FRMframeDx* pFrame = pfrm_months->GetFrame(0, 0);
    if (!pFrame)
        return;

    month -= 1;
    LONG month_y = month * MONTH_HEIGHT;
    D3D11_RECT rect{ xPos, yPos, xPos + (LONG)pFrame->GetWidth(), yPos + MONTH_HEIGHT };
    pD3DDevContext->RSSetScissorRects(1, &rect);
    pFrame->DrawFrame(pOrtho2D, (float)(rect.left), (float)rect.top - month_y);

}


//________________________
void WM_Update_Date_Time() {
    if (*pWinRef_WorldMap == -1)
        return;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return;

    Window_DX* subwin = nullptr;
    if (winRef_WM_Date_Time != -1)
        subwin = pWin->winDx->GetSubWin(winRef_WM_Date_Time);
    else {
        FRMframeDx* pFrame = pfrm_numbers->GetFrame(0, 0);
        if (pFrame)
            subwin = new Window_DX((float)pWin->width - (640 - 487.0f), 12.0f, 143, pFrame->GetHeight(), 0x00000000, pWin->winDx, &winRef_WM_Date_Time);
        pFrame = nullptr;
    }
    if (!subwin)
        return;
    subwin->ClearRenderTarget(nullptr);

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    DirectX::XMMATRIX Ortho2D;
    subwin->GetOrthoMatrix(&Ortho2D);
    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    subwin->SetRenderTarget(nullptr);

    LONG month = 0;
    LONG day = 0;
    LONG year = 0;
    fall_GameTime_Get_Date(&month, &day, &year);
    WM_Draw_Number(pD3DDevContext, &Ortho2D, 0, 0, day, 2);
    WM_Draw_Month(pD3DDevContext, &Ortho2D, 26, 0, month);
    WM_Draw_Number(pD3DDevContext, &Ortho2D, 62, 0, year, 4);

    LONG time = fall_GameTime_Get_Time();
    WM_Draw_Number(pD3DDevContext, &Ortho2D, 107, 0, time, 4);
}


//________________________________________________
void __declspec(naked) h_wm_update_date_time(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_Update_Date_Time

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
void WM_Update_Dial() {
    ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_WM_Dial, nullptr);
    if (p_butt)
        p_butt->buttDx->OverLay_Frm_SetFrameNum(*p_wm_dial_frame_num);
}


//___________________________________________
void __declspec(naked) h_wm_update_dial(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_Update_Dial

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
LONG WM_City_List_Update() {

    if (*pWinRef_WorldMap == -1)
        return -1;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;
    Window_DX* subwin = nullptr;

    LONG original_menu_height = 27 * 17;

    LONG menu_height = wm_city_list_menu_size * 27;

    LONG menu_display_height = (num_menu_buttons - 1) * 27 + 16;

    if (winRef_WM_City_List == -1) {
        subwin = new Window_DX((float)pWin->width - (640 - 501.0f), 135.0f, 119, menu_height, 0x00000000, pWin->winDx, &winRef_WM_City_List);

        //set the clipping rect to match the dimensions of the - WMTBEDGE.FRM ; Worldmap Town Tabs Edging overlay.
        D3D11_RECT rect{ (LONG)pWin->width - (640 - 501),135,(LONG)pWin->width - (640 - 501) + 119, 135 + menu_display_height };
        subwin->SetClippingRect(&rect);

        frmID = fall_GetFrmID(ART_INTRFACE, 0x16C, 0, 0, 0);//WMTABS.FRM     ; Worldmap Town Tabs Underlay
        pfrm = new FRMCached(frmID);
        FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
        //draw menu background relative to the bottom of the sub window, repeat if larger than original menu size;
        if (pFrame) {
            LONG num_draws = menu_height / original_menu_height;
            if (menu_height % original_menu_height)
                num_draws++;
            LONG y_offset = menu_height - original_menu_height;
            while (num_draws > 0) {
                subwin->RenderTargetDrawFrame(-9.0f, (float)y_offset - 27, pFrame, nullptr, nullptr);
                y_offset -= original_menu_height;
                num_draws--;
            }
            pFrame = nullptr;
        }
        delete pfrm;
        pfrm = nullptr;
        //draw menu items
        LONG y_offset = 3;
        for (int i = 0; i < *p_wm_city_list_current_size; i++) {
            frmID = (*pp_wm_city_data)[(*pp_wm_city_list_current)[i]].townmap_label_art_frmID;
            pfrm = new FRMCached(frmID);
            pFrame = pfrm->GetFrame(0, 0);
            if (pFrame) {
                subwin->RenderTargetDrawFrame(29.0f, (float)y_offset, pFrame, nullptr, nullptr);
                pFrame = nullptr;
            }
            delete pfrm;
            pfrm = nullptr;
            y_offset += 27;
        }

    }

    if (winRef_WM_City_Edge == -1) {
        frmID = fall_GetFrmID(ART_INTRFACE, 0x16F, 0, 0, 0);//WMTBEDGE.FRM   ; Worldmap Town Tabs Edging overlay
        pfrm = new FRMCached(frmID);
        FRMframeDx* pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            subwin = new Window_DX((float)pWin->width - (640 - 501.0f), 135.0f, 119, menu_display_height, 0x00000000, pWin->winDx, &winRef_WM_City_Edge);
            subwin->ClearRenderTarget(nullptr);
            D3D11_RECT rect{ 0, 0, 135, 27 };
            subwin->RenderTargetDrawFrame(0, 0, pFrame, nullptr, &rect);
            for (int i = 27; i < (LONG)subwin->GetHeight() - 27; i += 27) {
                rect.top = i;
                rect.bottom = i + 27;
                subwin->RenderTargetDrawFrame(0, (float)i - 27, pFrame, nullptr, &rect);
            }
            rect.bottom = subwin->GetHeight();
            rect.top = rect.bottom - 27;
            subwin->RenderTargetDrawFrame(0, (float)subwin->GetHeight() - (float)pFrame->GetHeight(), pFrame, nullptr, &rect);
            pFrame = nullptr;
        }
        delete pfrm;
        pfrm = nullptr;
    }

    subwin = pWin->winDx->GetSubWin(winRef_WM_City_List);
    if (subwin) {
        subwin->SetPosition((float)pWin->width - (640 - 501.0f), 135.0f - *p_wm_city_list_y_pos);
    }

    return 0;
}


//________________________
LONG WM_Update_Interface() {

    //draw city list
    WM_City_List_Update();

    if (*ppfrmdx_wmdial) {
        LONG hours = fall_GameTime_Get_Time() / 100;
        int numFrames = (*ppfrmdx_wmdial)->GetNumFrames();
        *p_wm_dial_frame_num = (hours + 12) % numFrames;
        WM_Update_Dial();
    }

    if (*p_wm_has_vehicle == TRUE) {
        ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_WM_Vehicle, nullptr);
        if (p_butt)
            p_butt->buttDx->OverLay_Frm_SetFrameNum(*p_wm_vehicle_frame_num);
        WM_Fuel_Gauge_Update();
    }

    WM_Update_Date_Time();
    return 0;
}


//________________________________________________
void __declspec(naked) h_wm_update_interface(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_Update_Interface

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}
                        

//________________________________________________________________________
void WM_Update_Map_Icons(XMMATRIX* pOrtho2D, float xOffset, float yOffset) {

    FRMCached* pFrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    LONG win_current_x = (*p_wm_current_pos_x - *p_wm_view_pos_x);
    LONG win_current_y = (*p_wm_current_pos_y - *p_wm_view_pos_y);


    if (*p_wm_selected_pos_x < 1 && *p_wm_selected_pos_y < 1) {

        if (*p_wm_encounter_flag == 1)
            pFrm = pfrm_encounter_icons[*p_wm_encounter_icon_type];
        else {
            if (*p_wm_hot_spot_selected == 0)
                pFrm = pfrm_hotspot1;
            else
                pFrm = pfrm_hotspot2;
        }

        DWORD hotspot_height = 0;
        if (pFrm) {
            pFrame = pFrm->GetFrame(0, 0);
            if (pFrame) {
                hotspot_height = pFrame->GetHeight();
                pFrame->DrawFrame(pOrtho2D, xOffset + (float)win_current_x - (pFrame->GetWidth() / 2), yOffset + (float)win_current_y - (hotspot_height / 2));
            }
        }

        if (SFALL_WorldMapTerrainInfo && wm_is_mouse_over_hotspot) {
            char* txt = nullptr;
            if (*p_wm_area_number_selected != -1) {
                CITY_DATA* pCity = &(*pp_wm_city_data)[*p_wm_area_number_selected];
                if (fall_WM_IsAreaVisible(pCity->area_num) != FALSE)
                    txt = GetMsg(pMsgList_Map, pCity->area_num + WM_MSG_OFFSET_SHORT_CITY_NAMES, 2);
                else
                    txt = GetMsg(pMsgList_WorldMap, WM_MSG_OFFSET_TERRAIN_UNKNOWN, 2);
            }
            if (!txt && *pp_wm_current_tile_cell)
                txt = terrain_types[(*pp_wm_current_tile_cell)->terrain];
            if (txt) {
                FONT_FUNC_STRUCT* font = GetCurrentFont();
                LONG txt_width = (LONG)font->GetTextWidth(txt);
                LONG txt_height = (LONG)font->GetFontHeight();
                DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
                if (color_pal)
                    colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);
                Draw_Text_Frame(pOrtho2D, txt, xOffset + (float)win_current_x - (txt_width / 2), yOffset + (float)win_current_y - (hotspot_height / 2) - txt_height - 2, colour, 0xFF000000, TextEffects::dropShadow);
                //Fallout_Debug_Info("is_mouse_over_hotspot_now %s", txt);
            }
        }
    }
    else {
        if (*p_wm_encounter_flag == 1)
            pFrm = pfrm_encounter_icons[*p_wm_encounter_icon_type];
        else
            pFrm = pfrm_wmaploc;
        if (pFrm) {
            pFrame = pFrm->GetFrame(0, 0);
            if (pFrame)
                pFrame->DrawFrame(pOrtho2D, xOffset + (float)win_current_x - (pFrame->GetWidth() / 2), yOffset + (float)win_current_y - (pFrame->GetHeight() / 2));
        }

        pFrm = pfrm_wmaptarg;
        if (pFrm) {
            pFrame = pFrm->GetFrame(0, 0);
            if (pFrame)
                pFrame->DrawFrame(pOrtho2D, xOffset + (float)(*p_wm_selected_pos_x - *p_wm_view_pos_x) - (pFrame->GetWidth() / 2), yOffset + (float)(*p_wm_selected_pos_y - *p_wm_view_pos_y) - (pFrame->GetHeight() / 2));
        }
    }
}

/*
//________________________________________________
void __declspec(naked) h_wm_update_map_icons(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push 0
        call WM_Update_Map_Icons
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
*/


//_________________________________________________
void OnDisplay_Instead_WM_Map(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    D3D11_RECT* pRect_clip = pWin_This->GetClippingRect();
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    UINT numScissorRects_Current = 0;
    D3D11_RECT* pScissorRects_Current = nullptr;
    if (pRect_clip) {
        pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, nullptr);
        if (numScissorRects_Current > 0) {
            pScissorRects_Current = new D3D11_RECT[numScissorRects_Current];
            pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, pScissorRects_Current);
        }
        pD3DDevContext->RSSetScissorRects(1, pRect_clip);
    }

    if (p_RT_WorldMap)
        p_RT_WorldMap->Display(nullptr, nullptr);
    if (p_RT_WorldMap_Fog)
        p_RT_WorldMap_Fog->Display();


    float win_x = 0;
    float win_y = 0;
    pWin_This->GetPosition(&win_x, &win_y);
    WinStructDx* pWin = pWin_This->Get_FalloutParent();

    XMMATRIX Ortho2D_XM = XMMatrixOrthographicOffCenterLH(0.0f, (float)SCR_WIDTH / wm_scale_RO, (float)SCR_HEIGHT / wm_scale_RO, 0.0f, -1.0f, 1000.0f);
    WM_Update_Map_Icons(&Ortho2D_XM, (pWin->rect.left + win_x) / wm_scale_RO, (pWin->rect.top + win_y) / wm_scale_RO);


    if (pScissorRects_Current != nullptr) {//restore the old clipping rect if the overlays clipping rect was enabled.
        pD3DDevContext->RSSetScissorRects(numScissorRects_Current, pScissorRects_Current);
        delete[] pScissorRects_Current;
        pScissorRects_Current = nullptr;
        numScissorRects_Current = 0;
    }

}


//_________________________
void WM_FogMap_Update_All() {
    if (p_RT_WorldMap_Fog) {
        ID3D11Device* pD3DDev = GetD3dDevice();
        ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
        if (pD3DDev && pD3DDevContext) {
            p_RT_WorldMap_Fog->ClearRenderTarget(nullptr);
            unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
            p_RT_WorldMap_Fog->SetRenderTarget(nullptr);

            XMMATRIX Ortho2D;
            p_RT_WorldMap_Fog->GetOrthoMatrix(&Ortho2D);

            //set vertex stuff
            UINT stride = sizeof(VERTEX_BASE);
            UINT offset = 0;
            pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB_pixel_quad, &stride, &offset);
            //set pixel shader stuff
            pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

            MATRIX_DATA posData;

            pD3DDevContext->OMSetBlendState(pBlendState_Zero, nullptr, -1);
            LONG tile_y_subs = 0;
            for (LONG tile_y = 0; tile_y < *p_wm_num_tiles; tile_y += *p_wm_num_horizontal_tiles) {
                LONG tile_x_subs = 0;
                for (LONG tile_x = 0; tile_x < *p_wm_num_horizontal_tiles; tile_x++) {
                    TILE_DATA* pTile = &(*pp_wm_tile_data)[tile_y + tile_x];
                    LONG sub_y_offset = 0;
                    for (int sub_y = 0; sub_y < WM_NUM_SUB_TILES_Y; sub_y++) {
                        for (int sub_x = 0; sub_x < WM_NUM_SUB_TILES_X; sub_x++) {
                            LONG fog_level = pTile->cell[sub_y_offset + sub_x].fog_level;
                            float sub_tile_fog = 0.0f;
                            if (fog_level == 0)
                                sub_tile_fog = 1.0f;
                            else if (fog_level == 1)
                                sub_tile_fog = 0.5f;

                            posData.World = DirectX::XMMatrixTranslation((float)tile_x_subs + sub_x, (float)tile_y_subs + sub_y, (float)0);
                            posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, Ortho2D);
                            pPS_BuffersFallout->UpdatePositionBuff(&posData);
                            pPS_BuffersFallout->SetPositionRender();

                            GEN_SURFACE_BUFF_DATA genSurfaceData;
                            XMFLOAT4 colour = { 0.0f,0.0f,0.0f,sub_tile_fog };
                            genSurfaceData.genData4_1 = colour;
                            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

                            pD3DDevContext->DrawIndexed(4, 0, 0);
                        }
                        sub_y_offset += WM_NUM_SUB_TILES_X;
                    }
                    tile_x_subs += WM_NUM_SUB_TILES_X;
                }
                tile_y_subs += WM_NUM_SUB_TILES_Y;
            }
            pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
        }
    }
    //Fallout_Debug_Info("WM_FogMap_Update_All");
}


//__________________________________________________________________________
void WM_FogMap_Update(LONG tile_num, LONG sub_x, LONG sub_y, LONG fog_level) {

    //Not checking if sub tile position is valid as this is already done in "WM_FogMap_Set_Tile".
    //if (sub_x < 0 || sub_x >= WM_NUM_SUB_TILES_X) 
    //    return;
    //if (sub_y < 0 || sub_y >= WM_NUM_SUB_TILES_Y) 
    //    return;
    if (tile_num < 0 || tile_num >= *p_wm_num_tiles)
        return;

    LONG tile_y = (tile_num / *p_wm_num_horizontal_tiles) * WM_NUM_SUB_TILES_Y;
    LONG tile_x = (tile_num % *p_wm_num_horizontal_tiles) * WM_NUM_SUB_TILES_X;

    if (p_RT_WorldMap_Fog) {
        ID3D11Device* pD3DDev = GetD3dDevice();
        ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
        if (pD3DDev && pD3DDevContext) {

            unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
            p_RT_WorldMap_Fog->SetRenderTarget(nullptr);

            XMMATRIX Ortho2D;
            p_RT_WorldMap_Fog->GetOrthoMatrix(&Ortho2D);

            //set vertex stuff
            UINT stride = sizeof(VERTEX_BASE);
            UINT offset = 0;
            pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB_pixel_quad, &stride, &offset);
            //set pixel shader stuff
            pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

            pD3DDevContext->OMSetBlendState(pBlendState_Zero, nullptr, -1);

            TILE_DATA* pTile = &(*pp_wm_tile_data)[tile_num];

            float sub_tile_fog = 0.0f;
            if (fog_level == 0)
                sub_tile_fog = 1.0f;
            else if (fog_level == 1)
                sub_tile_fog = 0.5f;

            MATRIX_DATA posData;
            posData.World = DirectX::XMMatrixTranslation((float)tile_x + sub_x, (float)tile_y + sub_y, (float)0);
            posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, Ortho2D);
            pPS_BuffersFallout->UpdatePositionBuff(&posData);
            pPS_BuffersFallout->SetPositionRender();

            GEN_SURFACE_BUFF_DATA genSurfaceData;
            XMFLOAT4 colour = { 0.0f,0.0f,0.0f,sub_tile_fog };
            genSurfaceData.genData4_1 = colour;
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

            pD3DDevContext->DrawIndexed(4, 0, 0);

            pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
        }
    }
    //Fallout_Debug_Info("WM_FogMap_Update tile %d x %d y %d fog %d", tile_num, sub_x, sub_y, fog_level);
}


//__________________________________________________________
bool WM_Get_World_ViewPort_Mouse_Pos(LONG* xPtr, LONG* yPtr) {
    if (*pWinRef_WorldMap == -1)
        return false;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return false;
    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_WM_Map);
    if (!subwin)
        return false;
    fall_Mouse_GetPos(xPtr, yPtr);
    *xPtr -= pWin->rect.left;
    *yPtr -= pWin->rect.top;
    float sub_x = 0;
    float sub_y = 0;
    subwin->GetPosition(&sub_x, &sub_y);
    *xPtr -= (LONG)sub_x;
    *yPtr -= (LONG)sub_y;
    if (*xPtr < 0 || *yPtr < 0 || *xPtr >= (LONG)subwin->GetWidth() / scaleLevel_GUI || *yPtr >= (LONG)subwin->GetHeight() / scaleLevel_GUI)
        return false;
    *xPtr = (LONG)(*xPtr / wm_scale_RO);
    *yPtr = (LONG)(*yPtr / wm_scale_RO);
    return true;
}


//___________________________________________________________
bool WM_Get_World_ViewPort_Centre_Pos(LONG* xPtr, LONG* yPtr) {
    if (*pWinRef_WorldMap == -1)
        return false;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return false;
    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_WM_Map);
    if (!subwin)
        return false;

    *xPtr = (LONG)subwin->GetWidth() / 2;
    *yPtr = (LONG)subwin->GetHeight() / 2;
    *xPtr = (LONG)(*xPtr / wm_scale_RO / scaleLevel_GUI);
    *yPtr = (LONG)(*yPtr / wm_scale_RO / scaleLevel_GUI);
    return true;
}


//__________________________
LONG WM_Map_Display_Update() {
    if (*p_wm_setup_flag != TRUE)
        return 0;

    if (*pWinRef_WorldMap == -1)
        return -1;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return -1;


    if (p_RT_WorldMap && SFALL_WorldMapTravelMarkers && *pp_wm_current_tile_cell && (*p_wm_selected_pos_x > 1 || *p_wm_selected_pos_y > 1)) {

        if (wm_last_pos_x == -1 || wm_last_pos_y == -1) {//these are set to -1 on world map start, set to current position to prevent drawing dots back to last saved position between loads.
            wm_last_pos_x = *p_wm_current_pos_x;
            wm_last_pos_y = *p_wm_current_pos_y;
        }
        static LONG dot_count = 0;
        static int dot_type = 0;
        if (wm_last_pos_x != *p_wm_current_pos_x || wm_last_pos_y != *p_wm_current_pos_y) {

            LONG pos_x = wm_last_pos_x;
            LONG pos_y = wm_last_pos_y;
            wm_last_pos_x = *p_wm_current_pos_x;
            wm_last_pos_y = *p_wm_current_pos_y;
            LONG terrain_type = (*pp_wm_current_tile_cell)->terrain;

            ID3D11Device* pD3DDev = GetD3dDevice();
            ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();

            //set vertex stuff
            UINT stride = sizeof(VERTEX_BASE);
            UINT offset = 0;
            pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB_pixel_quad, &stride, &offset);
            //set pixel shader stuff
            pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

            GEN_SURFACE_BUFF_DATA genSurfaceData;
            genSurfaceData.genData4_1 = wm_travel_dot_colour;
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
            pD3DDevContext->OMSetBlendState(pBlendState_Zero, nullptr, -1);

            while (pos_x != *p_wm_current_pos_x || pos_y != *p_wm_current_pos_y) {
                if (pos_x < *p_wm_current_pos_x)
                    pos_x++;
                else if (pos_x > *p_wm_current_pos_x)
                    pos_x--;
                if (pos_y < *p_wm_current_pos_y)
                    pos_y++;
                else if (pos_y > *p_wm_current_pos_y)
                    pos_y--;

                if (dot_type == 0)
                    p_RT_WorldMap->DrawIndexed(4, 0, 0, nullptr, (float)pos_x, (float)pos_y, 1, 1, nullptr);

                if (pterrain_types_markers[dot_type]) {
                    if (dot_count < pterrain_types_markers[dot_type][terrain_type])
                        dot_count++;
                    else {
                        dot_count = 0;
                        dot_type = 1 - dot_type;
                    }
                }
            }
            pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
        }
    }


    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_WM_Map);
    if (subwin && p_RT_WorldMap) {
        float win_x = 0;
        float win_y = 0;
        subwin->GetPosition(&win_x, &win_y);
        p_RT_WorldMap->SetPosition(((float)pWin->rect.left + win_x) + (float)-*p_wm_view_pos_x * wm_scale_RO, (pWin->rect.top + win_y) + (float)-*p_wm_view_pos_y * wm_scale_RO);
        p_RT_WorldMap_Fog->SetPosition(((float)pWin->rect.left + win_x) + (float)-*p_wm_view_pos_x * wm_scale_RO, (pWin->rect.top + win_y) + (float)-*p_wm_view_pos_y * wm_scale_RO);
    }


    WM_Update_Interface();
    return 0;
}


//______________________________________________
void __declspec(naked) h_wm_update_display(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_Map_Display_Update

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
LONG WM_Map_Display_Setup() {
    ID3D11Device* pD3DDev = GetD3dDevice();
    CreateQuadVB(pD3DDev, 1, 1, &pd3dVB_pixel_quad);

    BYTE colour_offset = SfallReadInt(L"Interface", L"TravelMarkerColor", 0);
    DWORD colour = 0xFFFFFFFF;
    if (color_pal)
        colour = color_pal->GetColour(colour & 0x000000FF);
    wm_travel_dot_colour = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, ((colour & 0xFF000000) >> 24) / 256.0f };


    if (*pWinRef_WorldMap == -1)
        return -1;
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return -1;


    if (winRef_WM_Map == -1) {
        D3D11_RECT win_rect{ 22, 21, (LONG)pWin->width - (640 - (22 + 450)), (LONG)pWin->height - (480 - (21 + 442)) };
        Window_DX* subwin = new Window_DX((float)win_rect.left, (float)win_rect.top, (win_rect.right - win_rect.left) * scaleLevel_GUI, (win_rect.bottom - win_rect.top) * scaleLevel_GUI, 0x00000000, pWin->winDx, &winRef_WM_Map);
        subwin->Set_OnDisplayFunctions(nullptr, nullptr, OnDisplay_Instead_WM_Map);
        subwin->SetScale(1.0f / scaleLevel_GUI, 1.0f / scaleLevel_GUI);
        subwin->SetClippingRect(&win_rect);
        subwin->ArrangeWin(2);
    }


    if (!p_RT_WorldMap) {
        DWORD frmID = 0;
        FRMCached* pfrm = nullptr;
        FRMframeDx* pFrame = nullptr;
        frmID = (*pp_wm_tile_data)[0].frmID;
        pfrm = new FRMCached(frmID);
        pFrame = pfrm->GetFrame(0, 0);
        DWORD frame_width = 350;
        DWORD frame_height = 300;
        if (pFrame) {
            frame_width = pFrame->GetWidth();
            frame_height = pFrame->GetHeight();
            pFrame = nullptr;
        }
        delete pfrm;
        pfrm = nullptr;

        LONG wm_width = *p_wm_num_horizontal_tiles * frame_width;
        LONG wm_num_vertical_tiles = *p_wm_num_tiles / *p_wm_num_horizontal_tiles;
        LONG wm_height = wm_num_vertical_tiles * frame_height;

        p_RT_WorldMap = new RenderTarget2(0, 0, wm_width, wm_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);

        Window_DX* subwin = pWin->winDx->GetSubWin(winRef_WM_Map);
        if (subwin && p_RT_WorldMap) {

            if (wm_scale_level < scaleLevel_GUI)
                wm_scale_RO = scaleSubUnit * (float)wm_scale_level;
            else
                wm_scale_RO = (float)wm_scale_level - scaleLevel_GUI + 1;

            p_RT_WorldMap->SetScale(wm_scale_RO, wm_scale_RO);

            float view_RO = (float)subwin->GetWidth() / subwin->GetHeight();
            float map_RO = (float)wm_width / wm_height;

            if (view_RO < map_RO)
                wm_scale_RO_max_out = (float)subwin->GetWidth() / scaleLevel_GUI / wm_width;
            else
                wm_scale_RO_max_out = (float)subwin->GetHeight() / scaleLevel_GUI / wm_height;

            float subtile_width = (float)frame_width / WM_NUM_SUB_TILES_X;
            float subtile_height = (float)frame_height / WM_NUM_SUB_TILES_Y;

            float subTile_R0 = subtile_width / subtile_height;
            if (view_RO < subTile_R0)
                wm_scale_RO_max_in = (float)subwin->GetWidth() / (subtile_width * 2);
            else
                wm_scale_RO_max_in = (float)subwin->GetHeight() / (subtile_height * 2);

            //*p_wm_view_right_limit = p_RT_WorldMap->GetWidth() - subwin->GetWidth() / scaleLevel_GUI;
            //*p_wm_view_bottom_limit = p_RT_WorldMap->GetHeight() - subwin->GetHeight() / scaleLevel_GUI;

            float view_width = subwin->GetWidth() / wm_scale_RO;
            float view_height = subwin->GetHeight() / wm_scale_RO;
            XMMATRIX Ortho2D_XM = XMMatrixOrthographicOffCenterLH(0.0f, view_width, view_height, 0.0f, -1.0f, 1000.0f);
            // p_RT_WorldMap->SetProjectionMatrix(&Ortho2D_XM);
            *p_wm_view_right_limit = (LONG)(p_RT_WorldMap->GetWidth() - view_width / scaleLevel_GUI);
            *p_wm_view_bottom_limit = (LONG)(p_RT_WorldMap->GetHeight() - view_height / scaleLevel_GUI);
            if (*p_wm_view_right_limit < 0)
                *p_wm_view_right_limit = 0;
            if (*p_wm_view_bottom_limit < 0)
                *p_wm_view_bottom_limit = 0;

            if (*p_wm_view_pos_x < 0)
                *p_wm_view_pos_x = 0;
            else if (*p_wm_view_pos_x > *p_wm_view_right_limit)
                *p_wm_view_pos_x = *p_wm_view_right_limit;
            if (*p_wm_view_pos_y < 0)
                *p_wm_view_pos_y = 0;
            else if (*p_wm_view_pos_y > *p_wm_view_bottom_limit)
                *p_wm_view_pos_y = *p_wm_view_bottom_limit;

        }

        XMMATRIX Ortho2D;
        p_RT_WorldMap->GetOrthoMatrix(&Ortho2D);
        MATRIX_DATA posData;
        ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
        float tile_y = 0;

        for (int y = 0; y < *p_wm_num_tiles; y += *p_wm_num_horizontal_tiles) {
            float tile_x = 0;
            for (int x = 0; x < *p_wm_num_horizontal_tiles; x++) {
                frmID = (*pp_wm_tile_data)[y + x].frmID;
                pfrm = new FRMCached(frmID);
                pFrame = pfrm->GetFrame(0, 0);
                if (pFrame) {
                    posData.World = XMMatrixTranslation(tile_x, tile_y, (float)0.0f);
                    posData.WorldViewProjection = XMMatrixMultiply(posData.World, Ortho2D);
                    pPS_BuffersFallout->UpdatePositionBuff(&posData);
                    pPS_BuffersFallout->SetPositionRender();
                    //set vertex stuff
                    UINT stride = sizeof(VERTEX_BASE);
                    UINT offset = 0;
                    ID3D11Buffer* frame_pd3dVB = nullptr;
                    pFrame->GetVertexBuffer(&frame_pd3dVB);// , nullptr);
                    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

                    //set texture stuff
                    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
                    pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

                    //set pixel shader stuff
                    if (pFrame->GetPixelWidth() == 1) {
                        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
                    }
                    else
                        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

                    p_RT_WorldMap->DrawIndexed(4, 0, 0, nullptr, tile_x, tile_y, pFrame->GetWidth(), pFrame->GetHeight(), nullptr);
                    pFrame = nullptr;
                }
                delete pfrm;
                pfrm = nullptr;

                tile_x += (float)frame_width;
            }
            tile_y += (float)frame_height;
        }

        //draw cities
        if (wm_draw_cities) {
            DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"
            if (color_pal)
                colour = color_pal->GetColour(*pPalColour_ConsoleGreen & 0x000000FF);

            GEN_SURFACE_BUFF_DATA genSurfaceData;
            // values in "WRLDSPR#.FRM"s range from 0 to 15 with 0 being the darkest, setting a max of 17 in colour alpha for shader returns results close to original.
            genSurfaceData.genData4_1 = { ((colour & 0x00FF0000) >> 16) / 256.0f, ((colour & 0x0000FF00) >> 8) / 256.0f, ((colour & 0x000000FF)) / 256.0f, 17.0f / 256.0f };
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);


            CITY_DATA* pCity = nullptr;
            float xPos = 0;
            float yPos = 0;

            UINT stride = sizeof(VERTEX_BASE);
            UINT offset = 0;
            ID3D11Buffer* frame_pd3dVB = nullptr;
            ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = nullptr;
            FONT_FUNC_STRUCT* font = GetCurrentFont();
            char* txt = nullptr;

            for (int i = 0; i < *p_wm_num_cities; i++) {
                if ((*pp_wm_city_data)[i].start_state != 0) {
                    pCity = &(*pp_wm_city_data)[i];
                    pFrame = p_worldSphereImage[pCity->size].pFrame;
                    if (pFrame) {
                        xPos = (float)pCity->world_pos_x - 22;
                        yPos = (float)pCity->world_pos_y - 21;

                        //set vertex stuff
                        pFrame->GetVertexBuffer(&frame_pd3dVB);
                        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
                        //set pixel shader stuff
                        pD3DDevContext->PSSetShader(pd3d_PS_Colour_32_Alpha, nullptr, 0);

                        //set texture stuff
                        pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
                        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

                        p_RT_WorldMap->DrawIndexed(4, 0, 0, nullptr, xPos, yPos, p_worldSphereImage[pCity->size].width, p_worldSphereImage[pCity->size].height, nullptr);
                        pFrame = nullptr;

                        if (fall_WM_IsAreaVisible(pCity->area_num) != FALSE)
                            txt = GetMsg(pMsgList_Map, pCity->area_num + WM_MSG_OFFSET_SHORT_CITY_NAMES, 2);
                        else
                            txt = GetMsg(pMsgList_WorldMap, WM_MSG_OFFSET_TERRAIN_UNKNOWN, 2);

                        DWORD txt_width = font->GetTextWidth(txt);
                        xPos += ((LONG)p_worldSphereImage[pCity->size].width - (LONG)txt_width) / 2;
                        yPos += (p_worldSphereImage[pCity->size].height + 1);
                        Draw_Text_Frame(&Ortho2D, txt, xPos, yPos, colour, 0xFF000000, TextEffects::none);
                    }



                }
            }
        }

        if (!p_RT_WorldMap_Fog) {
            p_RT_WorldMap_Fog = new RenderTarget(0, 0, *p_wm_num_horizontal_tiles * WM_NUM_SUB_TILES_X, wm_num_vertical_tiles * WM_NUM_SUB_TILES_Y, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
            if (wm_scale_level < scaleLevel_GUI)
                wm_scale_RO = scaleSubUnit * (float)wm_scale_level;
            else
                wm_scale_RO = (float)wm_scale_level - scaleLevel_GUI + 1;

            p_RT_WorldMap_Fog->SetScale(50 * wm_scale_RO, 50 * wm_scale_RO);
            WM_FogMap_Update_All();
        }
    }

    return 0;
}


//__________________________________________________________________________________________________________________
void WM_FogMap_Set_Tile(LONG tile_num, LONG sub_x, LONG sub_y, LONG sub_offset_x, LONG sub_offset_y, LONG fog_level) {

    TILE_DATA* pTile = &(*pp_wm_tile_data)[tile_num];
    sub_x += sub_offset_x;
    if (sub_x < 0 || sub_x >= WM_NUM_SUB_TILES_X) {
        //Fallout_Debug_Info("WM_FogMap_Set_Tile sub_x out of range");
        return;
    }
    sub_y += sub_offset_y;
    if (sub_y < 0 || sub_y >= WM_NUM_SUB_TILES_Y) {
        //Fallout_Debug_Info("WM_FogMap_Set_Tile sub_y out of range");
        return;
    }
    int sub_y_offset = sub_y * WM_NUM_SUB_TILES_X;

    if (fog_level == 1 && pTile->cell[sub_y_offset + sub_x].fog_level != 0)
        return;
    if (pTile->cell[sub_y_offset + sub_x].fog_level != fog_level)
        WM_FogMap_Update(tile_num, sub_x, sub_y, fog_level);

}


//_________________________________________________
void __declspec(naked) h_wm_fog_map_mark_tile(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push eax

        push dword ptr ss : [esp + 0x28]
        push dword ptr ss : [esp + 0x28]
        push ecx
        push ebx
        push edx
        push eax
        call WM_FogMap_Set_Tile
        add esp, 0x18

        pop eax
        pop edx
        pop ecx
        pop ebx

        mov ebp, p_wm_num_horizontal_tiles
        mov ebp, dword ptr ds:[ebp]
        ret
    }
}


//____________________________________________________
void __declspec(naked) h_wm_fog_map_mark_visited(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        mov eax, dword ptr ss:[esp + 0x1C + 0x8]
        push 2
        push 0
        push 0
        push edi
        push ebp
        push eax
        call WM_FogMap_Set_Tile
        add esp, 0x18

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        mov eax, pp_wm_tile_data
        mov eax, dword ptr ds : [eax]
        ret
    }
}


//______________________________________________
bool WorldMap_Zoom(int delta, bool focusOnMouse) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    if (!pWin || !pWin->winDx)
        return false;

    LONG x_pos = 0;
    LONG y_pos = 0;
    if (focusOnMouse) {
        if (!WM_Get_World_ViewPort_Mouse_Pos(&x_pos, &y_pos))
            return false;
    }
    else {
        if (!WM_Get_World_ViewPort_Centre_Pos(&x_pos, &y_pos))
            return false;
    }
    //mouse location on the map
    LONG mouse_map_pos_x = *p_wm_view_pos_x + x_pos;
    LONG mouse_map_pos_y = *p_wm_view_pos_y + y_pos;

    Window_DX* subwin = pWin->winDx->GetSubWin(winRef_WM_Map);
    if (!subwin || !p_RT_WorldMap)
        return false;

    if (delta > 0) {
        if (wm_scale_RO < wm_scale_RO_max_in) {//prevent zooming in to infinity, stops once you are at the border of a sub tile.
            wm_scale_level++;
            if (wm_scale_level < scaleLevel_GUI)
                wm_scale_RO = wm_scale_level * scaleSubUnit;
            else
                wm_scale_RO = (float)wm_scale_level - scaleLevel_GUI + 1;
        }
        else//already at max zoom-in level
            return true;

    }
    else if (delta < 0) {
        if (wm_scale_level > 1) {
            wm_scale_level--;
            if (wm_scale_level < scaleLevel_GUI)
                wm_scale_RO = wm_scale_level * scaleSubUnit;
            else
                wm_scale_RO = (float)wm_scale_level - scaleLevel_GUI + 1;
        }
        else if (wm_scale_level == 1) {//fit the map to the border of the window to view the whole map.
            wm_scale_RO = wm_scale_RO_max_out;
            wm_scale_level = 0;
        }
        else//already at max zoom-out level
            return true;
    }


    float view_width = subwin->GetWidth() / wm_scale_RO;
    float view_height = subwin->GetHeight() / wm_scale_RO;
    XMMATRIX Ortho2D_XM = XMMatrixOrthographicOffCenterLH(0.0f, view_width, view_height, 0.0f, -1.0f, 1000.0f);
    p_RT_WorldMap->SetScale(wm_scale_RO, wm_scale_RO);
    p_RT_WorldMap_Fog->SetScale(50 * wm_scale_RO, 50 * wm_scale_RO);

    view_width /= scaleLevel_GUI;
    view_height /= scaleLevel_GUI;
    *p_wm_view_right_limit = (LONG)(p_RT_WorldMap->GetWidth() - view_width);
    *p_wm_view_bottom_limit = (LONG)(p_RT_WorldMap->GetHeight() - view_height);
    if (*p_wm_view_right_limit < 0)
        *p_wm_view_right_limit = 0;
    if (*p_wm_view_bottom_limit < 0)
        *p_wm_view_bottom_limit = 0;

    if (wm_scale_RO == wm_scale_RO_max_out) {
        *p_wm_view_pos_x = -(LONG)((view_width - (float)p_RT_WorldMap->GetWidth()) / 2);
        *p_wm_view_pos_y = -(LONG)((view_height - (float)p_RT_WorldMap->GetHeight()) / 2);
        *p_wm_view_right_limit = *p_wm_view_pos_x;
        *p_wm_view_bottom_limit = *p_wm_view_pos_y;
    }
    else {
        x_pos -= (LONG)(view_width / 2);
        y_pos -= (LONG)(view_height / 2);

        *p_wm_view_pos_x += x_pos;
        *p_wm_view_pos_y += y_pos;

        //keep the view port inside the map borders
        if (*p_wm_view_pos_x < 0)
            *p_wm_view_pos_x = 0;
        else if (*p_wm_view_pos_x > *p_wm_view_right_limit)
            *p_wm_view_pos_x = *p_wm_view_right_limit;
        if (*p_wm_view_pos_y < 0)
            *p_wm_view_pos_y = 0;
        else if (*p_wm_view_pos_y > *p_wm_view_bottom_limit)
            *p_wm_view_pos_y = *p_wm_view_bottom_limit;
    }

    WM_Map_Display_Update();
    //adjust mouse position so that its over the same map location after scaling
    if (focusOnMouse) {
        float sub_x = 0;
        float sub_y = 0;
        subwin->GetPosition(&sub_x, &sub_y);
        x_pos = pWin->rect.left + (LONG)sub_x + (LONG)((mouse_map_pos_x - *p_wm_view_pos_x) * wm_scale_RO);
        y_pos = pWin->rect.top + (LONG)sub_y + (LONG)((mouse_map_pos_y - *p_wm_view_pos_y) * wm_scale_RO);
        fall_Mouse_SetPos(x_pos, y_pos);
    }
    return true;

}


//___________________________________
bool Mouse_Wheel_WorldMap(int zDelta) {

    if (zDelta == 0)
        return false;
    if (*pWinRef_WorldMap == -1)
        return false;
    return WorldMap_Zoom(zDelta, true);
}


//_________________________________
LONG WM_CheckControlKeys(LONG code) {
    if (code == 0x149) {//Page_Up - zoom in
        WorldMap_Zoom(1, false);
        return -1;
    }
    else if (code == 0x151) {//Page_Dn - zoom out
        WorldMap_Zoom(-1, false);
        return -1;
    }
    return code;
}


//____________________________
void WM_City_Scroll_List_End() {
    *p_wm_city_list_offset_unit = 0;
    for (int i = 0; i < num_menu_buttons; i++)
        fall_Button_Enable(p_wm_menu_buttRef[i]);
}


//____________________________________________________
void __declspec(naked) h_wm_city_scroll_list_end(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WM_City_Scroll_List_End

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//___________________________________
void WM_City_Scroll_List(LONG offset) {
    for (int i = 0; i < num_menu_buttons; i++)
        fall_Button_Disable(p_wm_menu_buttRef[i]);

    *p_wm_city_list_final_pos = *p_wm_city_list_y_pos;

    //LONG offset_y = offset * 7 + *p_wm_city_list_final_pos;
    LONG offset_y = offset * num_menu_buttons + *p_wm_city_list_final_pos;

    if (offset < 0) {
        if (*p_wm_city_list_final_pos > 0) {
            *p_wm_city_list_final_pos = offset_y;
            if (*p_wm_city_list_final_pos < 0)
                *p_wm_city_list_final_pos = 0;
            *p_wm_city_list_offset_unit = offset;
        }
    }
    else {
        LONG scroll_max = (wm_city_list_menu_size - num_menu_buttons) * 27;
        if (scroll_max > *p_wm_city_list_final_pos) {
            *p_wm_city_list_final_pos = offset_y;
            if (*p_wm_city_list_final_pos > scroll_max)
                *p_wm_city_list_final_pos = scroll_max;
            *p_wm_city_list_offset_unit = offset;
        }
    }
    if (*p_wm_city_list_offset_unit == 0) {
        WM_City_Scroll_List_End();//this isn't here in fallout2.exe, but is needed to reenable buttons if no scrolling is possible.
        return;
    }
    *p_wm_city_list_y_pos += offset;
    WM_Update_Interface();
    if (*p_wm_city_list_offset_unit < 0) {
        if (*p_wm_city_list_final_pos < *p_wm_city_list_y_pos)
            return;
    }
    else {
        if (*p_wm_city_list_final_pos > *p_wm_city_list_y_pos)
            return;
    }
    WM_City_Scroll_List_End();
}


//________________________________________________
void __declspec(naked) h_wm_city_scroll_list(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call WM_City_Scroll_List
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


//_________________________________________________
void OnScreenResize_World_Map(Window_DX* pWin_This) {
    if (!pWin_This)
        return;

    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    LONG oldX = pWin->rect.left;
    LONG oldY = pWin->rect.top;

    LONG winX = 0;
    LONG winY = 0;

    winX = ((LONG)SCR_WIDTH - (pWin->rect.right - pWin->rect.left)) / 2;
    winY = ((LONG)SCR_HEIGHT - (pWin->rect.bottom - pWin->rect.top)) / 2;

    MoveWindowX(pWin, winX, winY);

    //black background window
    Window_DX* subwin = pWin_This->GetSubWin(winRef_WM_Background);
    if (subwin) {
        subwin->SetPosition((float)-winX, (float)-winY);
        subwin->SetScale((float)SCR_WIDTH, (float)SCR_HEIGHT);
    }
    WM_Map_Display_Update();
}


//___________________
LONG WorldMap_Setup() {
    *p_wm_tick_count = GetTickCount();

    *p_wm_last_font = fall_GetFont();

    wm_scale_level = ConfigReadInt(L"WORLD_MAP", L"ZOOM_LEVEL", 1);
    if (wm_scale_level < 1)
        wm_scale_level = 1;

    SFALL_ExpandWorldMap = SfallReadInt(L"Interface", L"ExpandWorldMap", 0);
    SFALL_WorldMapTerrainInfo = SfallReadInt(L"Interface", L"WorldMapTerrainInfo", 0);
    SFALL_WorldMapTravelMarkers = SfallReadInt(L"Interface", L"WorldMapTravelMarkers", 0);
    SFALL_WorldMapSlots = SfallReadInt(L"Misc", L"WorldMapSlots", 17);

    if (SfallReadInt(L"Misc", L"WorldMapFontPatch", 0) != 0)
        fall_SetFont(101);
    else
        fall_SetFont(0);

    fall_Map_SaveInGame(1);

    if (*p_wm_has_vehicle == TRUE) {
        fall_Set_Background_Sound(p_wm_world_music_car, 12);//("20car", 12);
    }
    else
        fall_Set_Background_Sound(p_wm_world_music, 12);// ("23world", 12);

    fall_NotifyBar_Disable();
    fall_DisableGameEvents();

    fall_Mouse_SetImage(1);

    //for fallout et tu - dont draw city circles and names.  check if the exe mem has been altered to remove cities, alternatively set "DRAW_CITIES" to 0 in the config.
    if (ConfigReadInt(L"WORLD_MAP", L"DRAW_CITIES", 1) == 0 || *p_CHECK_VOODOO_remove_circle_name == 0x90)
        wm_draw_cities = false;
    else
        wm_draw_cities = true;

    wm_map_mouse_scrolling_enabled = false;

    wm_last_pos_x = -1;
    wm_last_pos_y = -1;

    DWORD frmID = 0;
    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    if (SFALL_ExpandWorldMap)
        frmID = fall_GetFrmID(ART_INTRFACE, 0x87, 0, 0, 0);//unused worldmap.frm
    else
        frmID = fall_GetFrmID(ART_INTRFACE, 0x88, 0, 0, 0);//wmapbox.frm    ; World map dialog box

    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);

    if (SFALL_ExpandWorldMap) {//if expanded world map art not found or graphic dimensions are greater the screen dimensions, disable SFALL_ExpandWorldMap and load normal background frm.
        if (!pFrame || pFrame->GetWidth() > SCR_WIDTH || pFrame->GetHeight() > SCR_HEIGHT) {
            SFALL_ExpandWorldMap = 0;
            frmID = fall_GetFrmID(ART_INTRFACE, 0x88, 0, 0, 0);//wmapbox.frm    ; World map dialog box
            delete pfrm;
            pfrm = new FRMCached(frmID);
            pFrame = pfrm->GetFrame(0, 0);
        }
    }

    if (!pFrame) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    DWORD win_width = pFrame->GetWidth();
    DWORD win_height = pFrame->GetHeight();

    LONG xPos = ((LONG)SCR_WIDTH - (LONG)win_width) / 2;
    LONG yPos = ((LONG)SCR_HEIGHT - (LONG)win_height) / 2;

    *pWinRef_WorldMap = fall_Win_Create(xPos, yPos, win_width, win_height, 0x100, FLG_WinToFront);
    if (*pWinRef_WorldMap == -1) {
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
        return -1;
    }
    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(*pWinRef_WorldMap);
    pWin->winDx->SetBackGroungColour(0x00000000);
    pWin->winDx->SetDrawFlag(false);
    pWin->winDx->ClearRenderTarget(nullptr);
    pWin->winDx->RenderTargetDrawFrame(0, 0, pFrame, nullptr, nullptr);
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_World_Map);

    //used by sfall for drawing dots
    *pp_wm_win_buff = pWin->buff;
    memset(*pp_wm_win_buff, 0, pWin->width * pWin->height);

    //create black background window
    Window_DX* subwin = new Window_DX((float)-xPos, (float)-yPos, 1, 1, 0x000000FF, pWin->winDx, &winRef_WM_Background);
    if (subwin) {
        subwin->ClearRenderTarget(nullptr);
        subwin->SetScale((float)SCR_WIDTH, (float)SCR_HEIGHT);
        subwin->ArrangeWin(TRUE);
    }


    for (int i = 0; i < 3; i++) {
        p_worldSphereImage[i].frmID = fall_GetFrmID(ART_INTRFACE, 0x150 + i, 0, 0, 0);//WRLDSPR0.FRM   ; World Sphere Overlay 0
        pfrm = new FRMCached(p_worldSphereImage[i].frmID);
        //pfrm->
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame) {
            p_worldSphereImage[i].width = pFrame->GetWidth();
            p_worldSphereImage[i].height = pFrame->GetHeight();

            p_worldSphereImage[i].pFrame = new FRMframeDx(nullptr, p_worldSphereImage[i].width, p_worldSphereImage[i].height, nullptr, nullptr, false, false);

            //frame values range from 0-15, values as alpha are reversed with 1 being the most opaque and 15 being the most transparent, 0 is masked.
            //Here we reverse and alter these values to cover the full range from 0(transparent) to 255(opaque).
            BYTE* pbuff = nullptr;
            UINT pitchBytes = 0;
            float pixel = 0;
            BYTE currentByte = 0;
            //bit of mucking about here to achieve results close to original.
            float multi = 255 / 16;
            if (SUCCEEDED(pFrame->Lock((void**)&pbuff, &pitchBytes, D3D11_MAP_READ))) {
                BYTE* pbuff_copy = nullptr;
                UINT pitchBytes_copy = 0;
                if (SUCCEEDED(p_worldSphereImage[i].pFrame->Lock((void**)&pbuff_copy, &pitchBytes_copy, D3D11_MAP_WRITE))) {
                    if (pitchBytes_copy == pitchBytes) {
                        DWORD size = p_worldSphereImage[i].height * pitchBytes;
                        DWORD offset = 0;
                        while (offset < size) {
                            currentByte = pbuff[offset];
                            if (currentByte == 0)
                                currentByte = 17;
                            pixel = 255 - ((float)(currentByte - 1) * multi);
                            pbuff_copy[offset] = (BYTE)pixel;
                            offset++;
                        }
                    }
                    p_worldSphereImage[i].pFrame->Unlock(nullptr);
                }
                pFrame->Unlock(nullptr);
            }
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }

    //Map Icons-------
    frmID = fall_GetFrmID(ART_INTRFACE, 0xA8, 0, 0, 0);//hotspot1.frm   ; Town map selctor shape #1
    pfrm_hotspot1 = new FRMCached(frmID);
    frmID = fall_GetFrmID(ART_INTRFACE, 0xDF, 0, 0, 0);//HOTSPOT2.FRM   ; Town map selector shape #2
    pfrm_hotspot2 = new FRMCached(frmID);

    pFrame = pfrm_hotspot1->GetFrame(0, 0);

    if (pFrame) {
        *p_hotspot_width = pFrame->GetWidth();
        *p_hotspot_height = pFrame->GetHeight();
    }
    pFrame = nullptr;


    frmID = fall_GetFrmID(ART_INTRFACE, 0x8B, 0, 0, 0);//wmaptarg.frm   ; World map move target maker #1
    pfrm_wmaptarg = new FRMCached(frmID);

    frmID = fall_GetFrmID(ART_INTRFACE, 0x8A, 0, 0, 0);//wmaploc.frm    ; World map location maker
    pfrm_wmaploc = new FRMCached(frmID);

    //
    for (int i = 0; i < num_encounter_icons; i++) {
        frmID = fall_GetFrmID(ART_INTRFACE, p_list_num_encounters[i], 0, 0, 0);//4 count - wmapfgt0.frm, wmapfgt1.frm, WMRNDEN2.FRM, WMRNDEN3.FRM
        pfrm_encounter_icons[i] = new FRMCached(frmID);
    }


    for (int i = 0; i < *p_wm_num_tiles; i++) //this wont be needed ------------------------------------
        (*pp_wm_tile_data)[i].frmObj = -1;

    //
    frmID = fall_GetFrmID(ART_INTRFACE, 0x16C, 0, 0, 0);//WMTABS.FRM     ; Worldmap Town Tabs Underlay
    pfrm = new FRMCached(frmID);

    pFrame = pfrm->GetFrame(0, 0);
    if (pFrame)
        *p_wm_city_menu_scroll_limit = (LONG)pFrame->GetHeight();
    pFrame = nullptr;
    delete pfrm;

    //frmID = fall_GetFrmID(ART_INTRFACE, 0x16F, 0, 0, 0);//WMTBEDGE.FRM   ; Worldmap Town Tabs Edging overlay
    //pfrm_wmtbedge = new FRMCached(frmID);


    frmID = fall_GetFrmID(ART_INTRFACE, 0x16D, 0, 0, 0);//WMDIAL.FRM; Worldmap Night / Day Dial
    pfrm_wmdial = new FRMCached(frmID);
    *ppfrmdx_wmdial = pfrm_wmdial->GetFrm();
    pFrame = pfrm_wmdial->GetFrame(0, 0);
    if (pFrame) {
        buttRef_WM_Dial = CreateButtonX_Overlay(*pWinRef_WorldMap, win_width - (640 - 532), 48, pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, -1, 0, 0, 0, 0);
        pFrame = nullptr;
        ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_WM_Dial, nullptr);
        if (p_butt)
            p_butt->buttDx->Overlay_SetFrm(frmID, 0, 0, 0, 0);
    }

    frmID = fall_GetFrmID(ART_INTRFACE, 0x81, 0, 0, 0);//months.frm     ; month strings for pip boy
    pfrm_months = new FRMCached(frmID);
    frmID = fall_GetFrmID(ART_INTRFACE, 0x52, 0, 0, 0);//NUMBERS.FRM; numbers for the hit pointsand fatigue counters
    pfrm_numbers = new FRMCached(frmID);


    frmID = fall_GetFrmID(ART_INTRFACE, 0x8, 0, 0, 0);//lilredup.frm   ; little red button up
    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);
    DWORD butt_width = 0;
    DWORD butt_height = 0;
    if (pFrame) {
        butt_width = pFrame->GetWidth();
        butt_height = pFrame->GetHeight();
    }
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    //town world button
    //0x9  //lilreddn.frm   ; little red button down
    //0x8  //lilredup.frm   ; little red button up
    int buttRef = CreateButtonX(*pWinRef_WorldMap, win_width - (640 - 519), win_height - (480 - 439), butt_width, butt_height, -1, -1, -1, 0x54, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    //menu buttons
    LONG butt_code = *p_wm_city_button_code_first;// 0x15E;
    LONG butt_y = 0x8A;

    num_menu_buttons = 7;
    if (SFALL_ExpandWorldMap)
        num_menu_buttons = 15;
    if (p_wm_menu_buttRef)
        delete[]p_wm_menu_buttRef;
    p_wm_menu_buttRef = new LONG[num_menu_buttons];

    for (int i = 0; i < num_menu_buttons; i++) {
        p_wm_menu_buttRef[i] = CreateButtonX(*pWinRef_WorldMap, win_width - (640 - 508), butt_y, butt_width, butt_height, -1, -1, -1, butt_code, 0x06000008, 0x06000009, 0, FLG_ButtTrans);
        SetButtonSoundsX(p_wm_menu_buttRef[i], &button_sound_Dn_1, &button_sound_Up_1);
        butt_code++;
        butt_y += 0x1B;
    }

    //up arrow button
    frmID = fall_GetFrmID(ART_INTRFACE, 0xC8, 0, 0, 0);//UPARWON.FRM    ; Character editor
    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);
    if (pFrame) {
        butt_width = pFrame->GetWidth();
        butt_height = pFrame->GetHeight();
    }
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    //0xC7 UPARWOFF.FRM   ; Character editor
    //0xC8 UPARWON.FRM    ; Character editor
    buttRef = CreateButtonX(*pWinRef_WorldMap, win_width - (640 - 480), 0x89, butt_width, butt_height, -1, -1, -1, 0x18D, 0x060000C7, 0x060000C8, 0, FLG_ButtTrans);
    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    //down arrow button
    frmID = fall_GetFrmID(ART_INTRFACE, 0xB6, 0, 0, 0);//DNARWON.FRM; Character editor
    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);
    if (pFrame) {
        butt_width = pFrame->GetWidth();
        butt_height = pFrame->GetHeight();
    }
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    //0xB5 DNARWOFF.FRM; Character editor
    //0xB6 DNARWON.FRM; Character editor
    buttRef = CreateButtonX(*pWinRef_WorldMap, win_width - (640 - 480), 0x98, butt_width, butt_height, -1, -1, -1, 0x191, 0x060000B5, 0x060000B6, 0, FLG_ButtTrans);
    SetButtonSoundsX(buttRef, &button_sound_Dn_1, &button_sound_Up_1);

    if (*p_wm_has_vehicle == TRUE) {
        frmID = fall_GetFrmID(ART_INTRFACE, 0x1B1, 0, 0, 0);//WMCARMVE.FRM   ; WorldMap Car Movie
        pfrm_wmcarmve = new FRMCached(frmID);
        *ppfrmdx_wmcarmve = pfrm_wmcarmve->GetFrm();

        frmID = fall_GetFrmID(ART_INTRFACE, 0x16B, 0, 0, 0);//WMSCREEN.FRM   ; Worldmap Overlay Screen

        pfrm = new FRMCached(frmID);
        pFrame = pfrm->GetFrame(0, 0);

        if (pFrame) {
            buttRef_WM_Vehicle = CreateButtonX_Overlay(*pWinRef_WorldMap, win_width - (640 - 499), win_height - (480 - 330), pFrame->GetWidth(), pFrame->GetHeight(), -1, -1, -1, -1, 0, 0, 0, 0);
            ButtonStruct_DX* p_butt = (ButtonStruct_DX*)fall_Button_Get(buttRef_WM_Vehicle, nullptr);
            if (p_butt) {
                frmID = fall_GetFrmID(ART_INTRFACE, 0x1B1, 0, 0, 0);//WMCARMVE.FRM   ; WorldMap Car Movie
                p_butt->buttDx->Overlay_SetFrm(frmID, 15, 6, 0, 0);

                unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
                p_butt->buttDx->Overlay_CreateTexture(0, 0, 0, 0);
                p_butt->buttDx->SetOverlayRenderTarget();
                DirectX::XMMATRIX Ortho2D;
                p_butt->buttDx->GetOrthoMatrix(&Ortho2D);
                pFrame->DrawFrame(&Ortho2D, 0, 0);
            }
        }
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;

        WM_Fuel_Gauge_Setup();
    }
    else {
        frmID = fall_GetFrmID(ART_INTRFACE, 0x16E, 0, 0, 0);//WMGLOBE.FRM    ; Worldmap Globe Stamp overlay
        pfrm = new FRMCached(frmID);
        pFrame = pfrm->GetFrame(0, 0);
        if (pFrame)
            pWin->winDx->RenderTargetDrawFrame((float)win_width - (640 - 495), (float)win_height - (480 - 330), pFrame, nullptr, nullptr);
        delete pfrm;
        pfrm = nullptr;
        pFrame = nullptr;
    }


    fall_Event_Add(fall_WM_SetMouseImage);

    if (fall_WM_CreateAreaList(pp_wm_city_list_current, p_wm_city_list_current_size) == -1)
        return -1;


    wm_city_list_menu_size = *p_wm_city_list_current_size;
    if (num_menu_buttons > wm_city_list_menu_size)
        wm_city_list_menu_size = num_menu_buttons;



    ///for (int i = 0; i < *p_wm_city_list_current_size; i++) 
    //    Fallout_Debug_Info("Area %d %d", i, (*pp_wm_city_list_current)[i]);

    //terrain_types
    terrain_types = new char* [*p_wm_num_terrain_types];
    for (int i = 0; i < *p_wm_num_terrain_types; i++)
        terrain_types[i] = (char*)GetMsg(pMsgList_WorldMap, WM_MSG_OFFSET_TERRAIN_TYPES + i, 2);
    //for (int i = 0; i < *p_wm_num_terrain_types; i++) {
    //    Fallout_Debug_Info("terrain type %d %s", i, (*ppfall_wm_terrain_types + 128 * i));
    //    Fallout_Debug_Info("terrain type %d %s", i, terrain_types[i]);
    //}


    if (SFALL_WorldMapTravelMarkers) {
        pterrain_types_markers[0] = new int[*p_wm_num_terrain_types];
        pterrain_types_markers[1] = new int[*p_wm_num_terrain_types];
        memset(pterrain_types_markers[0], 0, sizeof(int) * *p_wm_num_terrain_types);
        memset(pterrain_types_markers[1], 0, sizeof(int) * *p_wm_num_terrain_types);

        wchar_t* travelmarker_styles_string = new wchar_t[MAX_PATH];
        wchar_t* pos_dot = travelmarker_styles_string;
        wchar_t* pos_space = nullptr;
        int val = 0;
        SfallReadString(L"Interface", L"TravelMarkerStyles", L"0:0,0:0,0:0,0:0", travelmarker_styles_string, MAX_PATH);

        for (int i = 0; i < *p_wm_num_terrain_types; i++) {
            _set_errno(0);
            val = _wtoi(pos_dot);
            if (errno)
                continue;

            pterrain_types_markers[0][i] = val;
            pos_space = wcschr(pos_dot, L':');//find colon to get spacing val.
            pos_dot = wcschr(pos_dot, L',');//find start of next terrain data.

            if (pos_space == nullptr)//exit if no colon found.
                continue;
            if (pos_dot != nullptr && pos_dot < pos_space)//exit if missing spacing data.
                continue;
            pos_space++;//move to next char after colon.

            _set_errno(0);
            val = _wtoi(pos_space);
            if (errno)
                continue;

            pterrain_types_markers[1][i] = val;
            if (pos_dot == nullptr) //no more data found.
                continue;
            pos_dot++;//move to next char after comma.
        }
        delete[] travelmarker_styles_string;
        travelmarker_styles_string = nullptr;

        //for (int i = 0; i < *p_wm_num_terrain_types; i++)
        //    Fallout_Debug_Info("terrain_types_travel_marker %d %d ", pterrain_types_markers[0][i], pterrain_types_markers[1][i]);
    }


    if (WM_Map_Display_Setup() == -1)
        return -1;
    *p_wm_setup_flag = TRUE;
    if (WM_Map_Display_Update() == -1)
        return -1;

    fall_script_proccessing_disable();
    fall_script_remove_all();
    return 0;
}


//_____________________________________
void __declspec(naked) h_wm_setup(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WorldMap_Setup

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
void WorldMap_Destructor() {

    fall_Event_Remove(fall_WM_SetMouseImage);

    if (wm_scale_level < 1)
        wm_scale_level = 1;
    ConfigWriteInt(L"WORLD_MAP", L"ZOOM_LEVEL", wm_scale_level);
    //Set screen to black before destroying world wap window.
    SetPalette_Fading(pBLACK_PAL);

    fall_Win_Destroy(*pWinRef_WorldMap);
    winRef_WM_Background = -1;
    winRef_WM_TownView = -1;
    winRef_WM_TownView_Text = -1;
    winRef_WM_Date_Time = -1;
    winRef_WM_Fuel = -1;
    winRef_WM_City_List = -1;
    winRef_WM_City_Edge = -1;
    winRef_WM_Map = -1;

    if (p_wm_menu_buttRef)
        delete[]p_wm_menu_buttRef;


    if (terrain_types)
        for (int i = 0; i < *p_wm_num_terrain_types; i++)
            terrain_types[i] = nullptr;
    delete[]terrain_types;
    terrain_types = nullptr;

    if (pterrain_types_markers[0])
        delete[]pterrain_types_markers[0];
    pterrain_types_markers[0] = nullptr;
    if (pterrain_types_markers[1])
        delete[]pterrain_types_markers[1];
    pterrain_types_markers[1] = nullptr;

    if (p_RT_WorldMap)
        delete p_RT_WorldMap;
    p_RT_WorldMap = nullptr;

    if (p_RT_WorldMap_Fog)
        delete p_RT_WorldMap_Fog;
    p_RT_WorldMap_Fog = nullptr;

    if (pd3dVB_pixel_quad)
        pd3dVB_pixel_quad->Release();
    pd3dVB_pixel_quad = nullptr;

    if (pfrm_hotspot1)
        delete pfrm_hotspot1;
    pfrm_hotspot1 = nullptr;

    if (pfrm_hotspot2)
        delete pfrm_hotspot2;
    pfrm_hotspot2 = nullptr;

    if (pfrm_wmaptarg)
        delete pfrm_wmaptarg;
    pfrm_wmaptarg = nullptr;

    for (int i = 0; i < num_encounter_icons; i++) {
        if (pfrm_encounter_icons[i])
            delete pfrm_encounter_icons[i];
        pfrm_encounter_icons[i] = nullptr;
    }

    for (int i = 0; i < 3; i++) {
        if (p_worldSphereImage[i].pFrame)
            delete p_worldSphereImage[i].pFrame;
        p_worldSphereImage[i].pFrame = nullptr;
    }
    //if (pfrm_wmtabs)
    //    delete pfrm_wmtabs;
    //pfrm_wmtabs = nullptr;
    //if (pfrm_wmtbedge)
    //    delete pfrm_wmtbedge;
    //pfrm_wmtbedge = nullptr;
    if (pfrm_wmdial)
        delete pfrm_wmdial;
    pfrm_wmdial = nullptr;
    //if (pfrm_wmscreen)
    //    delete pfrm_wmscreen;
    //pfrm_wmscreen = nullptr;
    //if (pfrm_wmglobe)
    //    delete pfrm_wmglobe;
    //pfrm_wmglobe = nullptr;
    if (pfrm_months)
        delete pfrm_months;
    pfrm_months = nullptr;
    if (pfrm_numbers)
        delete pfrm_numbers;
    pfrm_numbers = nullptr;
    if (pfrm_wmcarmve)
        delete pfrm_wmcarmve;
    pfrm_wmcarmve = nullptr;

    *ppfrmdx_wmdial = nullptr;
    *ppfrmdx_wmcarmve = nullptr;

    if (pframe_wm_fuel_empty)
        delete pframe_wm_fuel_empty;
    pframe_wm_fuel_empty = nullptr;
    if (pframe_wm_fuel_full)
        delete pframe_wm_fuel_full;
    pframe_wm_fuel_full = nullptr;

    buttRef_WM_Vehicle = -1;
    buttRef_WM_Dial = -1;

    *p_wm_encounter_flag = 0;
    *p_wm_encounter_mapID = -1;
    *p_wm_encounter_type = -1;
    *p_wm_encounter_num_in_table = -1;

    fall_NotifyBar_Enable();
    fall_EnableGameEvents();
    fall_SetFont(*p_wm_last_font);


    if (*pp_wm_city_list_current) {
        fall_Mem_Deallocate(*pp_wm_city_list_current);
        *pp_wm_city_list_current = nullptr;
    }
    *p_wm_city_list_current_size = 0;
    *p_wm_setup_flag = FALSE;
    fall_script_proccessing_enable();

    //update the game map then un-blacken the screen
    DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof | FLG_Hud);
    SetPalette_Fading(pLOADED_PAL);
}


//__________________________________________
void __declspec(naked) h_wm_destructor(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        call WorldMap_Destructor

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________________________
LONG WM_Fix_GetNumFrames(FRMdx* p_frm) {
    if (p_frm)
        return p_frm->GetNumFrames();
    return 0;
}


//__________________________________________________
void __declspec(naked) h_wm_fix_get_num_frames(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call WM_Fix_GetNumFrames
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


//from update display
//004C37EC / $  53            PUSH EBX WM_SET_TILE_BUFF(EAX tileNum)
//from update display
//004C3FA8 / $  56            PUSH ESI; fallout2.WM_DRAW_CITY(EAX city_data_struct, EDX* wrldspro_image_Struct, EBX* to buff, ECX buff_x)(guessed Arg1)
//from update display
//004C41EC / $  53            PUSH EBX; fallout2.void WM_DRAW_MAP_ICONS()(guessed void)


//004C4A6C / $  53            PUSH EBX int WM_CREATE_HOTSPOT_BUTTON(EAX)

//004C4BD0 / $  53            PUSH EBX; fallout2.void WM_DRAW_HOT_SPOT()(guessed void)


//from update interface
//004C5244 / $  53            PUSH EBX void WM_DRAW_CAR_FUEL_LEVEL(EAX displayFlag)
//from update interface
//004C52B0 / $  53            PUSH EBX int WM_DRAW_CITY_LIST()



//_______________________________________________________________________
DWORD CheckMouseInWorldRect(LONG left, LONG top, LONG right, LONG bottom) {

    WinStruct* worldWin = fall_Win_Get(*pWinRef_WorldMap);
    return fall_Mouse_IsInArea(worldWin->rect.left + left, worldWin->rect.top + top, worldWin->rect.left + worldWin->width - (640 - right), worldWin->rect.top + worldWin->height - (480 - bottom));
}


//____________________________________________________
void __declspec(naked) check_mouse_in_world_rect(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ecx
        push ebx
        push edx
        push eax
        call CheckMouseInWorldRect
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//PREVENTS DRAWING OF BLACK WORLDMAP RECTANGLE OVER HAKUNIN MOVIES
//The "QUEUE_PROC" function is passing -1 as the window ref number to the "check_for_game_events(arg1 *retVal, arg2 winRef)" function.
// This is the window that will be hidden if a movie is scheduled to play.
// 
//check to see if winRef is -1 and if so pass the world map winRef in its place.
//____________________________________________
void __declspec(naked) h_world_movie_fix(void) {

    __asm {
        cmp esi, -1
        jne exit_func
        mov esi, pWinRef_WorldMap
        mov esi, dword ptr ds : [esi]
        exit_func :
        mov edx, 0x0B
        ret
    }
}


//_______________________________________________________
void __declspec(naked) h_wm_hotspot_click_range_fix(void) {

    __asm {
        sub eax, edx
        push edx
        mov edx, 5
        imul edx, scaleLevel_GUI
        cmp eax, edx
        pop edx
        ret
    }
}


//______________________________________
void GetWorldMouse(LONG* p_x, LONG* p_y) {

    WinStruct* pWin = fall_Win_Get(*pWinRef_WorldMap);

    WM_Get_World_ViewPort_Mouse_Pos(p_x, p_y);

    if (SFALL_WorldMapTerrainInfo && *p_wm_is_pc_moving == 0) {
        LONG w_mouse_x = *p_x;
        LONG w_mouse_y = *p_y;

        LONG hotspot_diff_x = abs((*p_wm_current_pos_x - *p_wm_view_pos_x) - w_mouse_x);
        LONG hotspot_diff_y = abs((*p_wm_current_pos_y - *p_wm_view_pos_y) - w_mouse_y);
        bool is_mouse_over_hotspot_now = false;
        LONG hotspot_range = 5 * scaleLevel_GUI;

        if (hotspot_diff_x < hotspot_range && hotspot_diff_y < hotspot_range) 
            is_mouse_over_hotspot_now = true;
        
        if (is_mouse_over_hotspot_now != wm_is_mouse_over_hotspot) {
            wm_is_mouse_over_hotspot = is_mouse_over_hotspot_now;
            WM_Map_Display_Update();
        }
    }
    return;
}


//_______________________________________________________
void __declspec(naked) get_world_mouse_n_check_keys(void) {

    __asm {
        push ebx
        push ecx
        push esi

        push edx
        push eax
        call GetWorldMouse
        add esp, 0x8

        pop esi
        pop ecx
        pop ebx

        //push ebx
        push ecx

        push esi
        call WM_CheckControlKeys
        add esp, 0x4
        mov esi, eax
        mov ebx, eax

        pop ecx
        //pop ebx
        ret
    }
}


//__________________________________________________
void WM_Check_Scroll(LONG* p_x_move, LONG* p_y_move) {
    *p_x_move = 0;
    *p_y_move = 0;
    WinStruct* pWin = fall_Win_Get(*pWinRef_WorldMap);
    LONG x = 0;
    LONG y = 0;

    fall_Mouse_GetPos(&x, &y);
    if (pWin) {

        if (x <= pWin->rect.left)
            *p_x_move = -1;
        else if (x >= pWin->rect.right)
            *p_x_move = 1;
        if (y <= pWin->rect.top)
            *p_y_move = -1;
        else if (y >= pWin->rect.bottom)
            *p_y_move = 1;

        if (wm_map_mouse_scrolling_enabled == false) {//to prevent scrolling if mouse is outside the window boundary when world map is first opened.
            if (*p_x_move == 0 && *p_y_move == 0)
                wm_map_mouse_scrolling_enabled = true;
            else {
                *p_x_move = 0;
                *p_y_move = 0;
            }
        }
    }
}


//_____________________________________________
void __declspec(naked) h_world_map_scroll(void) {
    __asm {
        push edx
        push eax
        call WM_Check_Scroll
        add esp, 0x8
        mov ecx, dword ptr ss : [esp + 4]
        mov ebx, dword ptr ss : [esp + 8]
        ret
    }
}


//__________________________________
void Modifications_WorldMap_CH(void) {

    //To-Do p_wm_world_music = (char*)0x;
    //To-Do p_wm_world_music_car = (char*)0x;

    //To-Do LOTS TO DO HERE

   //004C1FB9  |.  E8 14A00000   CALL 004CBFD2                            ; [Fallout2.004CBFD2
 ///  FuncWrite32(0x4C1FBA, 0xA014,  (DWORD)&h_world_map_scroll);

   //if mouseX > world.left - don't scroll
   //004C1FC6  |. /75 07         JNE SHORT 004C1FCF
    MemWrite8(0x4C1FC6, 0x75, 0x7F);//jg
    //if mouseX < world.right - don't scroll
    //004C1FD5  |. /75 05         JNE SHORT 004C1FDC
    MemWrite8(0x4C1FD5, 0x75, 0x7C);//jl
    //if mouseY > world.top - don't scroll
    //004C1FE1  |. /75 07         JNE SHORT 004C1FEA
    MemWrite8(0x4C1FE1, 0x75, 0x7F);//jg
    //if mouseY < world.bottom - don't scroll
    //004C1FEF  |. /75 05         JNE SHORT 004C1FF6
    MemWrite8(0x4C1FEF, 0x75, 0x7C);//jl


    //004BEE17  |.  E8 78D00000   |CALL 004CBE94                           ; [Fallout2.004CBE94
    FuncWrite32(0x4BEE18, 0xD078, (DWORD)&check_mouse_in_world_rect);

    //004BEF7D  |.  E8 12CF0000   |CALL 004CBE94                           ; [Fallout2.004CBE94
    FuncWrite32(0x4BEF7E, 0xCF12, (DWORD)&check_mouse_in_world_rect);


    //004C105A  |. E8 5EB00100    CALL Fallout2.004DC0BD
    //FuncWrite32(0x4C105B, 0x01B05E,  (DWORD)&WorldMapWinSetup);
    //FuncWrite32(0x4C105B, 0x01B05E,  (DWORD)&worldmap_win_setup);

    //004C1B3A  |.  E8 81A80100   CALL 004DC3C0                            ; [Fallout2.004DC3C0
    //FuncWrite32(0x4C1B3B, 0x01A881,  (DWORD)&world_map_kill);

    //004BEB25  |.  E8 A8D40000   |CALL 004CBFD2                           ; [Fallout2.004CBFD2
    ///FuncWrite32(0x4BEB26, 0xD4A8,  (DWORD)&h_world_map_mouse);

    //004A239A  |. 89D6           MOV ESI,EDX
    //004A239C  |. BA 0B000000    MOV EDX,0B
    //004A23A1  |. A1 10C55200    MOV EAX,DWORD PTR DS:[52C510]
    MemWrite8(0x4A239C, 0xBA, 0xE8);
    FuncWrite32(0x4A239D, 0x0B, (DWORD)&h_world_movie_fix);


    //FuncWrite32(0x4C1FBA, 0xA014, (DWORD)&get_world_mouse);
    //FuncWrite32(0x4BEB26, 0xD4A8, (DWORD)&get_world_mouse);
}


//_____________________________________
void Modifications_WorldMap_MULTI(void) {

    p_wm_world_music = (char*)FixAddress(0x50F790);
    p_wm_world_music_car = (char*)FixAddress(0x50F798);


    FuncReplace32(0x4C0168, 0xA7C8, (DWORD)&check_mouse_in_world_rect);
    FuncReplace32(0x4C02CE, 0xA662, (DWORD)&check_mouse_in_world_rect);

    MemWrite8(0x4A369C, 0xBA, 0xE8);
    FuncWrite32(0x4A369D, 0x0B, (DWORD)&h_world_movie_fix);

    FuncReplace32(0x4C3306, 0x76D2, (DWORD)&h_world_map_scroll);
    MemWrite16(0x4C330A, 0x7C8B, 0x36EB);//JMP SHORT 004C3342
    MemWrite16(0x4C330C, 0x0424, 0x9090);//JMP SHORT 004C3342

    FuncReplace32(0x4BFE76, 0xAB62, (DWORD)&get_world_mouse_n_check_keys);

    //subwin position is taken care of in get_world_mouse, the below subtractions are nop-ed.
    MemWrite16(0x4BFE91, 0xEF83, 0x9090);
    MemWrite32(0x4BFE93, 0x15ED8316, 0x90909090);


    p_wm_num_tiles = (LONG*)FixAddress(0x51DDF0);
    p_wm_num_horizontal_tiles = (LONG*)FixAddress(0x51DDF4);
    pp_wm_tile_data = (TILE_DATA**)FixAddress(0x51DDEC);

    pp_wm_current_tile_cell = (TILE_DATA_CELL**)FixAddress(0x672E14);

    p_wm_num_cities = (LONG*)FixAddress(0x51DDFC);
    pp_wm_city_data = (CITY_DATA**)FixAddress(0x51DDF8);

    //used by sfall for drawing dots
    pp_wm_win_buff = (BYTE**)FixAddress(0x51DE24);

    p_wm_tick_count = (DWORD*)FixAddress(0x51DEA0);

    p_wm_last_font = (LONG*)FixAddress(0x672FAC);

    p_wm_has_vehicle = (BOOL*)FixAddress(0x672E64);

    p_worldSphereImage = (IMAGE_DETAILS*)FixAddress(0x672FF8);

    p_hotspot_width = (DWORD*)FixAddress(0x672E94);
    p_hotspot_height = (DWORD*)FixAddress(0x672E98);

    p_list_num_encounters = (DWORD*)FixAddress(0x51DE84);

    p_wm_city_menu_scroll_limit = (LONG*)FixAddress(0x672F0C);

    p_wm_city_list_final_pos = (LONG*)FixAddress(0x672F54);
    p_wm_city_list_offset_unit = (LONG*)FixAddress(0x672F58);

    fall_WM_SetMouseImage = (void (*)())FixAddress(0x4C32EC);


    pfall_wm_create_area_list = (void*)FixAddress(0x4C55D4);

    p_wm_city_list_current_size = (LONG*)FixAddress(0x51DE98);
    pp_wm_city_list_current = (DWORD**)FixAddress(0x51DE94);
    p_wm_city_list_y_pos = (LONG*)FixAddress(0x672F10);

    p_wm_setup_flag = (BOOL*)FixAddress(0x51DE38);

    ppfrmdx_wmdial = (FRMdx**)FixAddress(0x672F30);
    ppfrmdx_wmcarmve = (FRMdx**)FixAddress(0x672E74);

    p_wm_area_number_selected = (LONG*)FixAddress(0x672E08);
    p_wm_area_number_town_view = (LONG*)FixAddress(0x51DE9C);

    p_wm_town_view_button = (LONG*)FixAddress(0x672DD8);

    pMsgList_WorldMap = (MSGList*)FixAddress(0x672FB0);
    pMsgList_Map = (MSGList*)FixAddress(0x631D48);

    p_wm_dial_frame_num = (LONG*)FixAddress(0x672F2C);
    p_wm_vehicle_frame_num = (LONG*)FixAddress(0x672E80);

    p_wm_vehicle_fuel_level = (LONG*)FixAddress(0x672E6C);

    p_wm_view_pos_x = (LONG*)FixAddress(0x51DE2C);
    p_wm_view_pos_y = (LONG*)FixAddress(0x51DE30);
    p_wm_selected_pos_x = (LONG*)FixAddress(0x672E20);
    p_wm_selected_pos_y = (LONG*)FixAddress(0x672E24);
    p_wm_current_pos_x = (LONG*)FixAddress(0x672E0C);
    p_wm_current_pos_y = (LONG*)FixAddress(0x672E10);

    p_wm_encounter_flag = (LONG*)FixAddress(0x672E48);
    p_wm_encounter_mapID = (LONG*)FixAddress(0x672E4C);
    p_wm_encounter_type = (LONG*)FixAddress(0x672E50);
    p_wm_encounter_num_in_table = (LONG*)FixAddress(0x672E50);
    p_wm_encounter_icon_type = (LONG*)FixAddress(0x672E58);

    p_wm_hot_spot_selected = (LONG*)FixAddress(0x672E00);

    pfall_wm_is_area_visible = (void*)FixAddress(0x4C453C);

    p_wm_num_terrain_types = (LONG*)FixAddress(0x51DDE8);
    ppfall_wm_terrain_types = (char**)FixAddress(0x51DDE4);

    p_wm_is_pc_moving = (LONG*)FixAddress(0x672E1C);

    p_wm_view_right_limit = (LONG*)FixAddress(0x672EFC);
    p_wm_view_bottom_limit = (LONG*)FixAddress(0x672F00);


    //fallout et tu - VOODOO_remove_circle_name
    p_CHECK_VOODOO_remove_circle_name = (BYTE*)FixAddress(0x4C407A);


    p_wm_city_button_code_first = (LONG*)FixAddress(0x4C0340);

    MemWrite32(0x4C21A8, FixAddress(0x672FD8), (DWORD)&p_wm_menu_buttRef);
    MemWrite32(0x4C227B, FixAddress(0x672FD8), (DWORD)&p_wm_menu_buttRef);
    MemWrite32(0x4C22DD, FixAddress(0x672FD8), (DWORD)&p_wm_menu_buttRef);
    MemWrite32(0x4C230A, FixAddress(0x672FD8), (DWORD)&p_wm_menu_buttRef);

    MemWrite8(0x4C219C, 0x53, 0xE9);
    FuncWrite32(0x4C219D, 0x57565251, (DWORD)&h_wm_city_scroll_list);

    MemWrite8(0x4C2270, 0x52, 0xE9);
    FuncWrite32(0x4C2271, 0x1589D231, (DWORD)&h_wm_city_scroll_list_end);
    MemWrite32(0x4C2275, FixAddress(0x672F58), 0x90909090);

    MemWrite8(0x4C2324, 0x53, 0xE9);
    FuncWrite32(0x4C2325, 0x57565251, (DWORD)&h_wm_setup);

    MemWrite8(0x4C2E44, 0x53, 0xE9);
    FuncWrite32(0x4C2E45, 0x57565251, (DWORD)&h_wm_destructor);

    MemWrite8(0x4C3830, 0x53, 0xE9);
    FuncWrite32(0x4C3831, 0x57565251, (DWORD)&h_wm_update_display);

    //fix get num frames - wmcarmve
    FuncReplace32(0x4BFF56, 0xFFF59832, (DWORD)&h_wm_fix_get_num_frames);


    //fix get num frames - wmdial
    FuncReplace32(0x4C1C96, 0xFFF57AF2, (DWORD)&h_wm_fix_get_num_frames);
    FuncReplace32(0x4C57DA, 0xFFF53FAE, (DWORD)&h_wm_fix_get_num_frames);

    MemWrite8(0x4C3C9C, 0x53, 0xE9);
    FuncWrite32(0x4C3C9D, 0x57565251, (DWORD)&h_wm_update_date_time);

    MemWrite8(0x4C50F4, 0x53, 0xE9);
    FuncWrite32(0x4C50F5, 0x57565251, (DWORD)&h_wm_update_interface);

    MemWrite8(0x4C5734, 0x53, 0xE9);
    FuncWrite32(0x4C5735, 0x57565251, (DWORD)&h_wm_update_dial);

    MemWrite8(0x4C4A6C, 0x53, 0xE9);
    FuncWrite32(0x4C4A6D, 0x57565251, (DWORD)&h_wm_town_view_setup);

    MemWrite8(0x4C4BD0, 0x53, 0xE9);
    FuncWrite32(0x4C4BD1, 0x57565251, (DWORD)&h_wm_town_view_update);

    MemWrite8(0x4C4D00, 0x53, 0xE9);
    FuncWrite32(0x4C4D01, 0x8B555251, (DWORD)&h_wm_town_view_destructor);

    //in world map queued function - draw scene not needed
    MemWrite8(0x4A3814, 0xE8, 0x90);
    MemWrite32(0x4A3815, 0xDABF, 0x90909090);


    MemWrite8(0x4C0190, 0x29, 0xE8);
    FuncWrite32(0x4C0191, 0x05F883D0, (DWORD)&h_wm_hotspot_click_range_fix);

    MemWrite8(0x4C01A1, 0x29, 0xE8);
    FuncWrite32(0x4C01A2, 0x05F883D0, (DWORD)&h_wm_hotspot_click_range_fix);

    MemWrite8(0x4C01E6, 0x29, 0xE8);
    FuncWrite32(0x4C01E7, 0x05F883D0, (DWORD)&h_wm_hotspot_click_range_fix);

    MemWrite8(0x4C01FB, 0x29, 0xE8);
    FuncWrite32(0x4C01FC, 0x05F883D0, (DWORD)&h_wm_hotspot_click_range_fix);


    MemWrite16(0x4C3437, 0x2D8B, 0xE890);
    FuncWrite32(0x4C3439, FixAddress(0x51DDF4), (DWORD)&h_wm_fog_map_mark_tile);

    MemWrite8(0x4C367D, 0xA1, 0xE8);
    FuncWrite32(0x4C367E, FixAddress(0x51DDEC), (DWORD)&h_wm_fog_map_mark_visited);



    //004C31F7 | .E8 04000000   CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
    //MemWrite8(0x4C31F7, 0xE8, 0x90);
    //MemWrite32(0x4C31F8, 0x04, 0x9008C483);
    //004C33E7 | .E8 14FEFFFF   CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
    //MemWrite8(0x4C33E7, 0xE8, 0x90);
    //MemWrite32(0x4C33E8, 0xFFFFFE14, 0x9008C483);

    //004C216C | > \E8 8F100000   CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
    //MemWrite8(0x4C216C, 0xE8, 0x90);
    //MemWrite32(0x4C216D, 0x108F, 0x9008C483);


    //004C0456 | .E8 A52D0000 | CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
    //MemWrite8(0x4C0456, 0xE8, 0x90);
   // MemWrite32(0x4C0457, 0x2DA5, 0x9008C483);
    //004C0472 | .E8 892D0000 | CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
    //MemWrite8(0x4C0472, 0xE8, 0x90);
    //MemWrite32(0x4C0473, 0x2D89, 0x9008C483);
    //004C048E | .E8 6D2D0000 | CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
   // MemWrite8(0x4C048E, 0xE8, 0x90);
   // MemWrite32(0x4C048F, 0x2D6D, 0x9008C483);
    //004C04AA | .E8 512D0000 | CALL void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2); \fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)
   // MemWrite8(0x4C04AA, 0xE8, 0x90);
//MemWrite32(0x4C04AB, 0x2D51, 0x9008C483);



    //004C3200 / $  56            PUSH ESI; fallout2.void WM_SET_WORLD_POS(EAX, EDX, EBX, ECX, Arg1, Arg2)(guessed Arg1, Arg2)
    //004C3201 | .  57            PUSH EDI
    //004C3202 | .  55            PUSH EBP
    //MemWrite8(0x4C3200, 0x56, 0xC2);
    //MemWrite16(0x4C3201, 0x5557, 0x0008);

}



//___________________________
void Modifications_WorldMap() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_WorldMap_CH();
    else
        Modifications_WorldMap_MULTI();

}




