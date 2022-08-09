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

#define FRM_VER_ORIGINAL    0x00000004
#define FRM_VER_32BIT       0x00000005

//structure for loading unlisted frm frames
class UNLSTDframe {
public:
    WORD width;
    WORD height;
    DWORD size;
    INT16 x;
    INT16 y;
    BYTE* buff;
    UNLSTDframe() {
        width = 0;
        height = 0;
        size = 0;
        x = 0;
        y = 0;
        buff = nullptr;
    }
    ~UNLSTDframe() {
        if (buff != nullptr)
            delete[] buff;
        buff = nullptr;
    }
};

//structure for loading unlisted frms
class UNLSTDfrm {
public:
    DWORD version;
    WORD FPS;
    WORD actionFrame;
    WORD numFrames;
    INT16 xCentreShift[6];
    INT16 yCentreShift[6];
    DWORD oriOffset[6];
    DWORD frameAreaSize;
    DWORD* pPal;
    UNLSTDframe** frames[6];
    UNLSTDframe** frames_base[6];
    bool uniformly_lit;//true if the frm should be lit uniformly as originally intend. Don't apply the new lighting.
    UNLSTDfrm() {
        version = 0;
        FPS = 0;
        actionFrame = 0;
        numFrames = 0;
        pPal = nullptr;
        for (int i = 0; i < 6; i++) {
            xCentreShift[i] = 0;
            yCentreShift[i] = 0;
            oriOffset[i] = 0;
            frames[i] = nullptr;
            frames_base[i] = nullptr;
        }
        frameAreaSize = 0;
        uniformly_lit = false;
    }
    ~UNLSTDfrm() {
        for (int ori = 0; ori < 6; ori++) {
            if (frames[ori] != nullptr) {
                for (int fNum = 0; fNum < numFrames; fNum++) {
                    if (frames[ori][fNum] != nullptr)
                        delete frames[ori][fNum];
                    frames[ori][fNum] = nullptr;
                }
                delete[] frames[ori];
            }
            frames[ori] = nullptr;
            if (frames_base[ori] != nullptr) {
                for (int fNum = 0; fNum < numFrames; fNum++) {
                    if (frames_base[ori][fNum] != nullptr)
                        delete frames_base[ori][fNum];
                    frames_base[ori][fNum] = nullptr;
                }
                delete[] frames_base[ori];
            }
            frames_base[ori] = nullptr;
        }
        if (pPal != nullptr)
            delete[] pPal;
        pPal = nullptr;
    }
};


// Exclude file extention in "sFileName" if you want to check for and load a bmp instead of an frm.
// Set "type" to -1 if "sFileName" contains the full path.
// If both the file extention is excluded and type is negative, a rix file will also be checked for after bmp. 
// If file is an FRM and has its own palette, palOffset_Mask is the palette offset you want to be invisible, Set to -1 to ignore.
UNLSTDfrm* LoadUnlistedFrm(const char* FrmName, LONG type, LONG palOffset_Mask);
UNLSTDfrm* CreateUnlistedFrm(int width, int height, int numFrames, int numOri);
UNLSTDfrm* LoadFrmFromID(DWORD frmID, LONG palOffset_Mask);

UNLSTDfrm* LoadBmpAsFrm(const char* path);
UNLSTDfrm* LoadRixAsFrm(const char* path);


//using fallouts data path and file functions
bool SaveBMP_DATA(const char* fileName, DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData, BYTE* pPal, WORD numPalEntries, WORD palEntrySizeBits, bool save32as24bit);
//using local path and regular c file functions
bool SaveBMP(const char* fileName, DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData, BYTE* pPal, WORD numPalEntries, WORD palEntrySizeBits, bool save32as24bit);

bool SaveScreenShot(DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData);

bool Load_Palette(const char* path, DWORD* pPal, DWORD size);
