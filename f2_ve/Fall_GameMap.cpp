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
#include "Fall_GameMap.h"
#include "memwrite.h"
#include "Fall_Objects.h"
#include "modifications.h"

char* pCurrentMapName = nullptr;

//floating text vars
LONG *pFloatingText_IsInitiated = nullptr;
LONG *pFloatingText_NumTotal = nullptr;
FloatingTextObj **lpFloatingText_Obj = nullptr;


LONG *pAreRoovesVisible = nullptr;
LONG *pAreMapHexesVisible = nullptr;


LONG* pMAP_LEVEL = nullptr;

LONG* pNUM_HEX_X = nullptr;
LONG* pNUM_HEX_Y = nullptr;
LONG* pNUM_HEXES = nullptr;

LONG* pVIEW_HEXPOS = nullptr;
LONG* pVIEW_HEX_X = nullptr;
LONG* pVIEW_HEX_Y = nullptr;
LONG* pVIEW_SQU_HEX_X = nullptr;
LONG* pVIEW_SQU_HEX_Y = nullptr;


LONG* pNUM_TILE_Y = nullptr;
LONG* pNUM_TILE_X = nullptr;
LONG* pNUM_TILES = nullptr;

LONG* pVIEW_TILE_X = nullptr;
LONG* pVIEW_TILE_Y = nullptr;
LONG* pVIEW_SQU_TILE_X = nullptr;
LONG* pVIEW_SQU_TILE_Y = nullptr;

DWORD* pDRAW_VIEW_FLAG = nullptr;
DWORD* pSCROLL_BLOCK_FLAG = nullptr;

LONG* pAmbientLightIntensity = nullptr;
LONG* pLightHexArray = nullptr;

OBJNode** pMapObjNodeArray = 0;
OBJNode* upperMapObjNodeArray[40000];

DWORD*** pMapTileLevelOffset = nullptr;

LONG* pObjViewTextWidth = nullptr;
LONG* pObjViewTextHeight = nullptr;

LONG* pIsGameMouse = nullptr;
LONG* pMousePicID = nullptr;
LONG* pMouseToggleFlag = nullptr;

LONG* pAreObjectsInitiated = nullptr;



void *pfall_FALLOUT2_SETUP = nullptr;

void *pfall_map_init = nullptr;
void *pfall_map_load = nullptr;
void *pfall_map_save = nullptr;

void* pfall_map_set_mapper_mouse = nullptr;

void* pfall_map_set_level = nullptr;

void* pfall_map_select_hex_pos = nullptr;

void* pfall_map_get_blocking_obj_at_pos = nullptr;

void* pfall_map_save_data = nullptr;

void* pfall_map_set_ambient_light = nullptr;


//004824CC / $  53            PUSH EBX; fallout2.char* GET_CITY_ELEVATION_NAME(EAX mapIndex, EDX level)(guessed void)
void* pfall_automap_get_city_elevation_name = nullptr;
//0048261C / $  53            PUSH EBX; fallout2.char* GET_CITY_SHORT_CITY_NAME(EAX mapIndex)(guessed void)
void* pfall_automap_get_short_city_name = nullptr;

void* pfall_map_save_in_game = nullptr;

void* pfall_map_refresh_lights =nullptr;


//________________________________________________________________
void fall_Map_SetAmbientLightIntensity(LONG intensity, DWORD flag) {
    __asm {
        mov edx, flag
        mov eax, intensity
        call pfall_map_set_ambient_light
    }
}


//_____________________________
LONG GetAmbientLightIntensity() {
    return *pAmbientLightIntensity;
}


//____________________________________________________________________
char* fall_AUTOMAP_GetCityElevationName(LONG mapIndex, LONG elevation) {
    char* retVal = 0;
    __asm {
        mov edx, elevation
        mov eax, mapIndex
        call pfall_automap_get_city_elevation_name
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________
char* fall_AUTOMAP_GetShortCityName(LONG mapIndex) {
    char* retVal = 0;
    __asm {
        mov eax, mapIndex
        call pfall_automap_get_short_city_name
        mov retVal, eax
    }
    return retVal;
}


//___________________________________
LONG fall_Map_SaveInGame(DWORD flag) {
    LONG retVal = 0;
    __asm {
        mov eax, flag
        call pfall_map_save_in_game
        mov retVal, eax
    }
    return retVal;
}


//___________________________
void fall_Map_RefreshLights() {
    __asm {
        call pfall_map_refresh_lights
    }
}


//______________________________________________________________________________
OBJStruct* fall_Map_GetBlockingObjAtPos(OBJStruct* obj, LONG hexNum, LONG level) {
    OBJStruct* pObj = nullptr;
    __asm {
        mov ebx, level
        mov edx, hexNum
        mov eax, obj
        call pfall_map_get_blocking_obj_at_pos
        mov pObj, eax
    }
    return pObj;
}


void UpdateFloatingTextAreaWH(LONG width, LONG height) {
    *pObjViewTextWidth = width;
    *pObjViewTextHeight = height;
}


//_______________________________________
void Fallout_Functions_Setup_GameMap_CH() {


    pMapTileLevelOffset = (DWORD***)0x67C388;

    pAmbientLightIntensity = (LONG*)0x52902C;
    pLightHexArray = (LONG*)0x5AEF14;

    pMapObjNodeArray = (OBJNode**)0x64A320;

    pMAP_LEVEL = (LONG*)0x529368;
    pVIEW_HEXPOS = (LONG*)0x67C3B4;

    pNUM_HEX_X = (LONG*)0x67C384;
    pNUM_HEX_Y = (LONG*)0x67C390;
    pNUM_HEXES = (LONG*)0x67C3A0;

    pNUM_TILE_Y = (LONG*)0x67C3A4;
    pNUM_TILE_X = (LONG*)0x67C3AC;
    pNUM_TILES = (LONG*)0x67C380;

    pVIEW_SQU_HEX_X = (LONG*)0x67C37C;
    pVIEW_SQU_HEX_Y = (LONG*)0x67C378;

    pVIEW_SQU_TILE_X = (LONG*)0x67C36C;
    pVIEW_SQU_TILE_Y = (LONG*)0x67C370;

    pVIEW_TILE_X = (LONG*)0x67C368;
    pVIEW_TILE_Y = (LONG*)0x67C364;

    pVIEW_HEX_X = (LONG*)0x67C398;
    pVIEW_HEX_Y = (LONG*)0x67C39C;


    pDRAW_VIEW_FLAG = (DWORD*)0x52D758;
    pSCROLL_BLOCK_FLAG = (DWORD*)0x52D744;

    //Map text vars
    pObjViewTextWidth = (LONG*)0x678790;
    pObjViewTextHeight = (LONG*)0x678794;


    pfall_FALLOUT2_SETUP = (void*)0x480140;
    pfall_map_init = (void*)0x4813E4;

    pfall_map_load = (void*)0x481E98;
    pfall_map_save = (void*)0x48309C;

    pfall_map_set_mapper_mouse = (void*)0x44C140;

    pAreMapHexesVisible = (LONG*)0x52D750;
    pAreRoovesVisible = (LONG*)0x52D74C;

    pfall_map_set_level = (void*)0x481588;

    pfall_map_get_blocking_obj_at_pos = (void*)0x48AC48;
    pfall_map_select_hex_pos = (void*)0x412A54;
    pCurrentMapName = (char*)0x6422D8;
    pfall_map_save_data = (void*)0x482DB0;
/*To-Do
    pfall_map_refresh_lights = (void*)0x;

    pAreObjectsInitiated = (LONG*)0x;

    pFloatingText_IsInitiated = (LONG*)0x;
    pFloatingText_NumTotal = (LONG*)0x;
    lpFloatingText_Obj = (FloatingTextObj**)0x;

    pIsGameMouse = (LONG*)0x;
    pMousePicID = (LONG*)0x;
    pMouseToggleFlag = (LONG*)0x;

    pfall_map_set_ambient_light = (void*)0x;

    pCurrentMapName = (char*)0x;

    pfall_automap_get_city_elevation_name = (void*)0x;
    pfall_automap_get_short_city_name = (void*)0x;

    pfall_map_save_in_game = (void*)0x;
*/
}


//__________________________________________
void Fallout_Functions_Setup_GameMap_MULTI() {


    pMapTileLevelOffset = (DWORD***)FixAddress(0x66BE08);

    pAmbientLightIntensity = (LONG*)FixAddress(0x51923C);
    pLightHexArray = (LONG*)FixAddress(0x59E994);

    pMapObjNodeArray = (OBJNode**)FixAddress(0x639DA0);

    pMAP_LEVEL = (LONG*)FixAddress(0x519578);
    pVIEW_HEXPOS = (LONG*)FixAddress(0x66BE34);

    pNUM_HEX_X = (LONG*)FixAddress(0x66BE04);
    pNUM_HEX_Y = (LONG*)FixAddress(0x66BE10);
    pNUM_HEXES = (LONG*)FixAddress(0x66BE20);

    pNUM_TILE_Y = (LONG*)FixAddress(0x66BE24);
    pNUM_TILE_X = (LONG*)FixAddress(0x66BE2C);
    pNUM_TILES = (LONG*)FixAddress(0x66BE00);

    pVIEW_SQU_HEX_X = (LONG*)FixAddress(0x66BDFC);
    pVIEW_SQU_HEX_Y = (LONG*)FixAddress(0x66BDF8);

    pVIEW_SQU_TILE_X = (LONG*)FixAddress(0x66BDEC);
    pVIEW_SQU_TILE_Y = (LONG*)FixAddress(0x66BDF0);

    pVIEW_TILE_X = (LONG*)FixAddress(0x66BDE8);
    pVIEW_TILE_Y = (LONG*)FixAddress(0x66BDE4);

    pVIEW_HEX_X = (LONG*)FixAddress(0x66BE18);
    pVIEW_HEX_Y = (LONG*)FixAddress(0x66BE1C);

    pDRAW_VIEW_FLAG = (DWORD*)FixAddress(0x51D968);
    pSCROLL_BLOCK_FLAG = (DWORD*)FixAddress(0x51D954);


    //Map text vars
    pObjViewTextWidth = (LONG*)FixAddress(0x668210);
    pObjViewTextHeight = (LONG*)FixAddress(0x668214);

    pfall_FALLOUT2_SETUP = (void*)FixAddress(0x480CC0);
    pfall_map_init = (void*)FixAddress(0x481FB4);
    pfall_map_load = (void*)FixAddress(0x482A68);
    pfall_map_save = (void*)FixAddress(0x483C6C);
    pfall_map_set_mapper_mouse = (void*)FixAddress(0x44C9F0);

    pAreMapHexesVisible = (LONG*)FixAddress(0x51D960);
    pAreRoovesVisible = (LONG*)FixAddress(0x51D95C);

    pfall_map_set_level = (void*)FixAddress(0x482158);
    pfall_map_get_blocking_obj_at_pos = (void*)FixAddress(0x48B848);
    pfall_map_select_hex_pos = (void*)FixAddress(0x412A54);
    pfall_map_save_data = (void*)FixAddress(0x483980);

    pfall_map_refresh_lights = (void*)FixAddress(0x48AC54);

    pAreObjectsInitiated = (LONG*)FixAddress(0x5195F8);

    pFloatingText_IsInitiated = (LONG*)FixAddress(0x668220);
    pFloatingText_NumTotal = (LONG*)FixAddress(0x51D944);
    lpFloatingText_Obj = (FloatingTextObj**)FixAddress(0x6681C0);

    pIsGameMouse = (LONG*)FixAddress(0x518BFC);
    pMousePicID = (LONG*)FixAddress(0x518C0C);
    pMouseToggleFlag = (LONG*)FixAddress(0x518D38);

    pfall_map_set_ambient_light = (void*)FixAddress(0x47A908);

    pCurrentMapName = (char*)FixAddress(0x631D58);

    pfall_automap_get_city_elevation_name = (void*)FixAddress(0x4824CC);
    pfall_automap_get_short_city_name = (void*)FixAddress(0x48261C);

    pfall_map_save_in_game = (void*)FixAddress(0x483C98);
}


//____________________________________
void Fallout_Functions_Setup_GameMap() {
    if (fallout_exe_region == EXE_Region::Chinese)
        Fallout_Functions_Setup_GameMap_CH();
    else
        Fallout_Functions_Setup_GameMap_MULTI();
}

