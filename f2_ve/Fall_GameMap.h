/*
The MIT License (MIT)
Copyright � 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the �Software�), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Fall_Objects.h"


class OBJInfo {
   public:
   DWORD flags;
   OBJStruct *obj;
};


struct FloatingTextObj {
   DWORD flags;    //0x00 //flags
   OBJStruct *pObj; //0x04 //associated map obj
   LONG time;      //0x08 //creation time
   LONG numLines;  //0x0C //num lines of text
   LONG xOffset;   //0x10
   LONG yOffset;   //0x14
   LONG hexPos;    //0x18
   LONG scrnX;     //0x1C
   LONG scrnY;     //0x20
   LONG imgWidth;  //0x24
   LONG imgHeight; //0x28
   BYTE *pImgBuff; //0x2C
};


struct MAP_HEADER {
	DWORD version;
	char name[16];
	LONG pcHexPos;
	LONG pcLevel;
	LONG pcOri;
	LONG numLocalVars;
	LONG scriptIndex;
	DWORD levelFlags;
	DWORD unknown0x2C;
	LONG numGlobalVars;
	DWORD mapID;
	DWORD ticksEpoch;
	DWORD unknown0x3C[44];
};


//floating text vars
extern LONG *pFloatingText_IsInitiated;
extern LONG *pFloatingText_NumTotal;
extern FloatingTextObj **lpFloatingText_Obj;



extern LONG *pMAP_LEVEL;
extern LONG *pNUM_TILE_Y;
extern LONG *pNUM_TILE_X;
extern LONG *pNUM_TILES;
extern LONG *pNUM_HEXES;
extern LONG* pNUM_HEX_X;
extern LONG* pNUM_HEX_Y;

extern LONG* pVIEW_HEXPOS;
extern LONG* pVIEW_HEX_X;
extern LONG* pVIEW_HEX_Y;
extern LONG* pVIEW_SQU_HEX_X;
extern LONG* pVIEW_SQU_HEX_Y;

extern LONG* pVIEW_TILE_X;
extern LONG* pVIEW_TILE_Y;
extern LONG* pVIEW_SQU_TILE_X;
extern LONG* pVIEW_SQU_TILE_Y;

extern LONG *pAreObjectsInitiated;

extern DWORD *pSCROLL_BLOCK_FLAG;
extern DWORD* pDRAW_VIEW_FLAG;


extern LONG *pIsGameMouse;
extern LONG *pMousePicID;
extern LONG *pMouseToggleFlag;


extern OBJNode** pMapObjNodeArray;
extern DWORD*** pMapTileLevelOffset;

extern LONG* pLightHexArray;


extern BOOL* pAreRoovesVisible;
extern BOOL* pAreHexesVisible;
extern BOOL* pIsMapperMouse;

void Fallout_Functions_Setup_GameMap();

void fall_Map_RefreshLights();
// add_remove_flag [0 = add light, 1 = remove light]	p_rc_area [optional draw area rect]
LONG fall_Map_UpdateLightMap(OBJStruct* p_obj, LONG add_remove_flag, RECT* p_rc_area);


LONG GetAmbientLightIntensity();
void fall_Map_SetAmbientLightIntensity(LONG intensity, DWORD flag);

char* fall_AUTOMAP_GetCityElevationName(LONG mapIndex, LONG elevation);
char* fall_AUTOMAP_GetShortCityName(LONG mapIndex);

LONG fall_Map_SaveInGame(DWORD flag);

OBJStruct* fall_Map_GetBlockingObjAtPos(OBJStruct* obj, LONG hexNum, LONG level);

//replace associated functions at some point
void UpdateFloatingTextAreaWH(LONG width, LONG height);


LONG fall_Fallout_Initiate(const char* title, BOOL isMapper, LONG font, DWORD flags, DWORD pathLength, const char* path);

LONG fall_Map_Load(const char* map_file_name);
LONG fall_Map_Save(const char* map_file_name);

LONG fall_Map_SetLevel(LONG level);

MAP_HEADER* Get_Map_Header();

extern void(*fall_game_reset)();
extern void(*fall_game_exit)();

extern void(*fall_map_init)();
extern void (*fall_map_exit)();
extern void(*fall_map_reset)();
