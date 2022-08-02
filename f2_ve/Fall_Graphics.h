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

#pragma pack(2)
struct FRMframe {
      WORD width;
      WORD height;
      DWORD size;
      INT16 x;
      INT16 y;
      //BYTE index[];
};


struct FRMhead {
      DWORD version;//version num
      WORD FPS;//frames per sec
      WORD actionFrame;
      WORD numFrames;//number of frames per direction
      INT16 xCentreShift[6];//offset from frm centre +=right -=left
      INT16 yCentreShift[6];//offset from frm centre +=down -=up
      DWORD oriOffset[6];//frame area offset for diff orientations
      DWORD frameAreaSize;
};
#pragma pack()



struct ARTtype {
      LONG enabled;
      char name[8];
      DWORD unkVal01;
      DWORD unkVal02;
      DWORD unkVal03;//frm name list ? char [13][listSize] //file names 8 ch for name + 4 ch ext + null
      DWORD unkVal04;
      DWORD listSize;
};

extern ARTtype* pArtTypeArray;


#define ART_ITEMS 0
#define ART_CRITTERS 1
#define ART_SCENERY 2
#define ART_WALLS 3
#define ART_TILES 4
#define ART_MISC 5
#define ART_INTRFACE 6
#define ART_INVEN 7
#define ART_HEADS 8
#define ART_BACKGRND 9
#define ART_SKILLDEX 10

LONG IsArtTypeEnabled(LONG type);
char* GetArtTypeName(LONG type);
DWORD GetArtTypeSize(LONG type);
LONG ToggleArtTypeEnabled(LONG type);


DWORD fall_GetFrmID(DWORD art_type, DWORD lstNum, DWORD id2, DWORD id1, DWORD id3);
void fall_Frm_Unload(DWORD frmObj);
BYTE* fall_Frm_GetIndexBuffer(DWORD frmID, DWORD frameNum, DWORD ori, DWORD *frmObj);
FRMhead* fall_GetFrm(DWORD FID, DWORD *frmObj);
DWORD fall_FrmFrame_GetWidth(FRMhead *frm, DWORD frameNum, DWORD ori);
DWORD fall_FrmFrame_GetHeight(FRMhead *frm, DWORD frameNum, DWORD ori);
BYTE* fall_FrmFrame_GetIndexBuffer(FRMhead *frm, DWORD frameNum, DWORD ori);

int fall_CheckFrmFileExists(DWORD frmID);
char* fall_GetFrmFilePath(DWORD frmID);

void Fallout_Functions_Setup_Graphics();



