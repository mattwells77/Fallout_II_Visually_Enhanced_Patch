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

#pragma once

#include "Fall_GameMap.h"

#define FOG_QUAD_WIDTH  96
#define FOG_QUAD_HEIGHT  36


extern LONG EDGE_OFF_X;
extern LONG EDGE_OFF_Y;
extern LONG EDGE_X_DIFF;
extern LONG EDGE_Y_DIFF;

extern int FOG_OF_WAR;
extern LONG fogLight;
extern  RECT* p_rc_Fog;

class bitset {
public:
    bitset(DWORD nBits) {
        numBits = nBits;
        numDwords = ((numBits >> 5) + 1);
        dwords = new DWORD[numDwords];// (DWORD*)malloc(sizeof(DWORD) * numDwords);
        if (dwords != nullptr)
            memset(dwords, 0x0, sizeof(DWORD) * numDwords);
    }
    ~bitset() {
        //free(dwords);
        delete dwords;
        dwords = nullptr;
    }
    void set(DWORD bit) {
        DWORD bindex = bit >> 5;
        DWORD boffset = bit & 0x1F;
        if (bindex < numDwords)
            dwords[bindex] |= (1 << boffset);
    }
    void set() {
        memset(dwords, 0xFF, sizeof(DWORD) * numDwords);
    }
    void clear(DWORD bit) {
        DWORD bindex = bit >> 5;
        DWORD boffset = bit & 0x1F;
        if (bindex < numDwords)
            dwords[bindex] &= ~(1 << boffset);
    }
    void clear() {
        memset(dwords, 0x00, sizeof(DWORD) * numDwords);
    }
    DWORD get(DWORD bit) {
        DWORD bindex = bit >> 5;
        DWORD boffset = bit & 0x1F;
        if (bindex < numDwords)
            //return dwords[bindex] & (1 << boffset);
            return (dwords[bindex] & (1 << boffset)) >> boffset;
        else
            return 0;
    }
    DWORD NumBits() { return numBits; };
    DWORD NumDwords() { return numDwords; };
    DWORD GetDword(DWORD num) { 
        if (num > numDwords)
            return 0;
        return dwords[num];
    };
    bool SetDword(DWORD offset, DWORD dVal) {
        if (offset >= numDwords)
            return false;
        dwords[offset] = dVal;
        return true;
    };
protected:
private:
    DWORD numBits;
    DWORD* dwords;
    DWORD numDwords;
};


bool SetViewHexNum(LONG xSqu, LONG ySqu);
//Converts hex XY coordinates to scroll grid XY coordinates in pixels - to the nearest block of 32x24 pixels (1 hex wide, 2 hexes high)
void Hex2Sqr_Scroll(LONG x, LONG y, LONG* px, LONG* py);
//Converts square XY coordinates in pixels to scroll hex XY coordinates - to the nearest block of 32x24 pixels (1 hex wide, 2 hexes high)
void SqrToHex_Scroll(LONG x, LONG y, LONG* px, LONG* py);
//Converts hex number to rectangular scroll grid coordinates in pixels - to the nearest block of 32x24 pixels (1 hex wide, 2 hexes high)
void HexNumToSqr_Scroll(LONG hexNum, LONG* px, LONG* py);
//Converts rectangular scroll grid coordinates in pixels to hex number - to the nearest block of 32x24 pixels (1 hex wide, 2 hexes high)
LONG SqrToHexNum_Scroll(LONG x, LONG y);
//Converts rectangular XY coordinates in pixels to hex number - with xy offsets needed by game functions
LONG SqrToHexNum_GameOffset(LONG x, LONG y);
//Converts rectangular XY coordinates in pixels to hex xy position, returns hex number - with xy offsets needed by game functions
LONG SqrToHexPos_GameOffset(LONG x, LONG y, LONG* px, LONG* py);
//Converts hex XY to rectangular XY coordinates in pixels - with xy offsets needed by game functions
LONG HexToSqr_GameOffset(LONG x, LONG y, LONG* px, LONG* py);
//Converts hex number to rectangular XY coordinates in pixels - with xy offsets needed by game functions
LONG HexNumToSqr_GameOffset(LONG hexNum, LONG* px, LONG* py);
//Converts hex number to rectangular XY coordinates in units of 16x12
void HexNumToSqr_Grid16x12(LONG hexNum, LONG* px, LONG* py);
//Converts rectangular XY coordinates in pixels to hex XY coordinates
void SqrToHex(LONG x, LONG y, LONG* px, LONG* py);
//Converts hex XY coordinates to rectangular XY coordinates in pixels
void HexToSqr(LONG x, LONG y, LONG* px, LONG* py);
//Converts hex number to rectangular XY coordinates in pixels
void HexNumToSqr(LONG hexNum, LONG* px, LONG* py);
//Converts rectangular XY coordinates in pixels to hex number
LONG SqrToHexNum(LONG x, LONG y);
//Converts hex number to hex XY coordinates
bool HexNumToHexPos(LONG hexNum, LONG* x, LONG* y);
//Converts hex XY coordinates to hex number
bool HexPosToHexNum(LONG* p_hexPos, LONG x, LONG y);
//Converts hex number to floor tile number
LONG HexNumToTileNum(LONG hexNum);
//Converts floor tile number to hex number
LONG TileNumToHexNum(LONG tileNum);
//Converts floor tile number to rectangular XY coordinates in pixels
void TileToSqr(LONG tileNum, LONG* px, LONG* py);
//Converts floor tile XY to rectangular XY coordinates in pixels
void TileToSqr(LONG x, LONG y, LONG* px, LONG* py);
//Converts rectangular XY coordinates in pixels to floor tile XY
bool SqrToTile(LONG x, LONG y, LONG* pTileX, LONG* pTileY);
//Converts rectangular XY coordinates in pixels to floor tile number - with xy offsets needed by game functions
bool SqrToTile_GameOffset(LONG x, LONG y, LONG* pTileX, LONG* pTileY);
//Converts rectangular XY coordinates in pixels to roof tile number - with xy offsets needed by game functions
bool SqrToTile_Roof_GameOffset(LONG x, LONG y, LONG* pTileX, LONG* pTileY);
//Converts floor tile number to rectangular XY coordinates in pixels  - with xy offsets needed by game functions
void TileToSqr_GameOffsets(LONG tileNum, LONG* px, LONG* py);
//Converts roof tile number to rectangular XY coordinates in pixels  - with xy offsets needed by game functions
void TileToSqr_Roof_GameOffsets(LONG tileNum, LONG* px, LONG* py);
//Converts tile number to tile XY coordinates
bool TileNumToTilePos(LONG tileNum, LONG* px, LONG* py);
//Converts tile XY coordinates to tile number
bool TilePosToTileNum(LONG* p_tileNum, LONG x, LONG y);

//Converts rectangular XY coordinates in pixels to isometric XY coordinates
//x and y values are scaled out to the nearest product (96) of 32 x-step and 12 y-step, to avoid loss in the conversion.
void SqrToIso(LONG x, LONG y, LONG* px, LONG* py);
//Converts isometric XY coordinates to rectangular XY coordinates in pixels
//x and y values are scaled out to the nearest product (96) of 32 x-step and 12 y-step, to avoid loss in the conversion.
void IsoToSqr(LONG x, LONG y, LONG* px, LONG* py);

bool isHexWithinMapEdges(LONG hexPos);
bool isHexWithinMapEdges(LONG hex_x, LONG hex_y);
LONG GetNextHexPos(LONG hexPos, UINT direction, LONG distance);
LONG GetHexDistance(LONG hexStart, LONG hexEnd);

DWORD CheckAngledRoofTileEdge(LONG xPos, LONG yPos, DWORD tileLstNum);

LONG GetObjectsAtPos(LONG xPos, LONG yPos, LONG level, LONG type, OBJInfo** lpObjInfoArray);

bool IsInLineOfSightBlocked(LONG hexStart, LONG hexEnd, LONG level, DWORD flags, bool mark_fog_map);
//Scan the line of sight from origin hex to the hexes along the edge of map level.
void Scan_Line_Of_Sight(LONG hexNum_origin, LONG level);

int IsVisibleByPCDx(OBJStruct* obj);

LONG GetHexLight(LONG level, LONG hexNum, LONG ambientLight);


bool FogOfWarMap_CopyFiles(const char *pFromPath, const char *pToPath);
LONG FogOfWarMap_DeleteTmps(const char *path);
bool FogOfWarMap_Save(const char *MapName);
bool FogOfWarMap_Load(const char *MapName);

//LONG GetFloorHexLight(LONG elev, LONG hexNum, LONG globalLight);
LONG Get_Hex_FogLevel(LONG elev, LONG hexNum);
bool Is_Hex_Fogged(LONG elev, LONG hexNum);

bool VE_MAP_CopyFiles(const char* pFromPath, const char* pToPath);
LONG VE_MAP_DeleteTmps(const char* path);

bool VE_MAP_Open_WRITE(const char* path, void* FileStream_MAP);
void VE_MAP_Close_WRITE();
bool VE_MAP_Open_READ(const char* path, void* FileStream_MAP);
void VE_MAP_Close_READ();

bool VE_SAVE_DAT_Open_WRITE(const char* path, void* FileStream);
bool VE_SAVE_DAT_Open_READ(const char* path, void* FileStream);
LONG VE_SAVE_DAT_Close(void* FileStream);

bool LightColour_Read(void* FileStream, OBJStruct* pObj);
bool LightColour_Write(void* FileStream,  OBJStruct* pObj);


bool VE_PROTO_LightColour_Read(const char* path, PROTO* pPro);
bool VE_PROTO_LightColour_Write(const char* path, PROTO* pPro);

DWORD VE_PROTO_Get_Light_Colour(PROTO* pProDx);
DWORD* VE_PROTO_Get_Light_Colour_Ptr(PROTO* pPro);

DWORD GameMap_GetTileFrmID(LONG tileNum, LONG level, BOOL isRoof, DWORD* pRetTileFlags);
BOOL GameMap_SetTileFrmID(LONG tileNum, LONG level, BOOL isRoof, DWORD frmID);
