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
#include "graphics.h"

#include "Fall_General.h"
#include "Fall_File.h"
#include "Fall_Graphics.h"

//#include <stdio.h>


//_________________________________
void fwrite8(FILE* file, BYTE val) {
    fputc(val, file);
}


//___________________________________
void fwrite16(FILE* file, WORD val) {
    fputc(val, file);
    fputc(val >> 8, file);
}


//___________________________________
void fwrite32(FILE* file, DWORD val) {
    fputc(val, file);
    fputc(val >> 8, file);
    fputc(val >> 16, file);
    fputc(val >> 24, file);
}


//_________________________________________________________________________________________________________________________________________________________________________________________
bool SaveBMP(const char* fileName, DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData, BYTE* pPal, WORD numPalEntries, WORD palEntrySizeBits, bool save32as24bit) {
    //8bit saves require pPal, numPalEntries and palEntrySizeBits be set. This function does not convert other formats to 8bit, pData should be a list of palette offsets.

    if (pixelSizeBits != 8 && pixelSizeBits != 24 && pixelSizeBits != 32) {
        Fallout_Debug_Error("SaveBMP - input buffer must be 8, 24, or 32 bit");
        return false;
    }

    DWORD dibPaletteSize = 0;
    if (pixelSizeBits == 8) {
        dibPaletteSize = 1024;
        if (!pPal)
            return false;
        if (palEntrySizeBits != 24 && palEntrySizeBits != 32)//pal entrys must be 3 or 4 bytes in length.
            return false;
        if (numPalEntries > 256)//2^pixelSizeBits
            return false;
    }
    WORD pixelSizeBits_SaveAs = pixelSizeBits;
    if (pixelSizeBits == 32 && save32as24bit)//save a 32bit image as a 24bit bmp if "save32as24bit" is set;
        pixelSizeBits_SaveAs = 24;

    WORD pixelSizeBytes_SaveAs = pixelSizeBits_SaveAs >> 3;// / 8;//Bits to Bytes - pixel size to save to;

    WORD pixelSizeBytes = pixelSizeBits >> 3;// / 8;//Bits to Bytes - pixel size ifn buffer "pData";

    DWORD dibHeaderSize = 54;

    DWORD widthBytes = width * pixelSizeBytes_SaveAs;
    DWORD dstPitch = widthBytes;
    if (dstPitch & 3)
        dstPitch = (dstPitch & 0xFFFFFFFC) + 4;
    DWORD pitchDiff = dstPitch - widthBytes;
    DWORD dibBuffSize = dstPitch * height;

    DWORD buffSize = pitchBytes * height;
    DWORD buffOff = buffSize - pitchBytes;


    FILE* FileStream = nullptr;

    if (fopen_s(&FileStream, fileName, "wb") != 0) {
        Fallout_Debug_Error("SaveBMP Could Not Open File - %s", fileName);
        return false;
    }

    //write bmp header
    fwrite16(FileStream, 0x4D42);//"BM"

    fwrite32(FileStream, dibHeaderSize + dibPaletteSize + dibBuffSize);//size of BMP file in bytes (unreliable)

    fwrite16(FileStream, 0x0000);//reserved, must be zero
    fwrite16(FileStream, 0x0000);//reserved, must be zero

    fwrite32(FileStream, dibHeaderSize + dibPaletteSize);//data offset
    fwrite32(FileStream, 40);//headersize
    fwrite32(FileStream, width);//image width in pixels
    fwrite32(FileStream, height);//image height in pixels
    fwrite16(FileStream, 1);//number of planes in the image, must be 1
    fwrite16(FileStream, pixelSizeBits_SaveAs);//number of bits per pixel (1, 4, 8, or 24)
    fwrite32(FileStream, 0);//compression type (0=none, 1=RLE-8, 2=RLE-4)
    fwrite32(FileStream, dibBuffSize);//size of image data in bytes (including padding)
    fwrite32(FileStream, 0);//horizontal resolution in pixels per meter (unreliable)
    fwrite32(FileStream, 0);//vertical resolution in pixels per meter (unreliable)
    if (pixelSizeBits == 8)
        fwrite32(FileStream, 256);//2^pixelSizeBits//number of colors in image, or zero
    else
        fwrite32(FileStream, 0);//number of colors in image, or zero
    fwrite32(FileStream, 0);//number of important colors, or zero

    //save palette
    if (pixelSizeBits == 8) {
        DWORD palEntrySizeBytes = palEntrySizeBits >> 3;// / 8;
        DWORD palSizeBytes = numPalEntries * palEntrySizeBytes;
        for (DWORD i = 0; i < palSizeBytes; i += palEntrySizeBytes) {
            fwrite8(FileStream, pPal[i + palEntrySizeBytes - 1] << 2);//fallout palette values range from 0-64, need to be multiplied by 4 "<< 2" or will appear very dark.
            fwrite8(FileStream, pPal[i + palEntrySizeBytes - 2] << 2);
            fwrite8(FileStream, pPal[i + palEntrySizeBytes - 3] << 2);
            fwrite8(FileStream, 0);
        }
    }
    //save image buff
    for (DWORD yPos = 0; yPos < height; yPos++) {

        for (DWORD xPos = 0; xPos < width * pixelSizeBytes; xPos += pixelSizeBytes) {
            for (DWORD cVal = 0; cVal < pixelSizeBytes_SaveAs; cVal++)
                fwrite8(FileStream, pData[buffOff + xPos + cVal]);
        }
        for (DWORD p = 0; p < pitchDiff; p++)
            fwrite8(FileStream, 0);
        buffOff -= pitchBytes;
    }

    fclose(FileStream);
    return true;
}


//_______________________________________________________________________________________________
bool SaveScreenShot(DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData) {

    FILE* FileStream = nullptr;
    char scrPath[13];
    sprintf_s(scrPath, 13, "scr%.5d.bmp\0", 0);
    LONG scrNum = 0;
    while (fopen_s(&FileStream, scrPath, "rb") == 0) {
        if (FileStream)
            fclose(FileStream);
        FileStream = nullptr;
        scrNum++;
        sprintf_s(scrPath, 13, "scr%.5d.bmp\0", scrNum);
    }
    return SaveBMP(scrPath, width, height, pixelSizeBits, pitchBytes, pData, nullptr, 0, 0, true);
}


//______________________________________________________________________________________________________________________________________________________________________________________________
bool SaveBMP_DATA(const char* fileName, DWORD width, DWORD height, WORD pixelSizeBits, DWORD pitchBytes, BYTE* pData, BYTE* pPal, WORD numPalEntries, WORD palEntrySizeBits, bool save32as24bit) {
    //8bit saves require pPal, numPalEntries and palEntrySizeBits be set. This function does not convert other formats to 8bit, pData should be a list of palette offsets.

    if (pixelSizeBits != 8 && pixelSizeBits != 24 && pixelSizeBits != 32) {
        Fallout_Debug_Error("SaveBMP_DATA - input buffer must be 8, 24, or 32 bit");
        return false;
    }

    DWORD dibPaletteSize = 0;
    if (pixelSizeBits == 8) {
        dibPaletteSize = 1024;
        if (!pPal)
            return false;
        if (palEntrySizeBits != 24 && palEntrySizeBits != 32)//pal entrys must be 3 or 4 bytes in length.
            return false;
        if (numPalEntries > 256)//2^pixelSizeBits
            return false;
    }
    WORD pixelSizeBits_SaveAs = pixelSizeBits;
    if (pixelSizeBits == 32 && save32as24bit)//save a 32bit image as a 24bit bmp if "save32as24bit" is set;
        pixelSizeBits_SaveAs = 24;

    WORD pixelSizeBytes_SaveAs = pixelSizeBits_SaveAs >> 3;// / 8;//Bits to Bytes - pixel size to save to;

    WORD pixelSizeBytes = pixelSizeBits >> 3;// / 8;//Bits to Bytes - pixel size ifn buffer "pData";

    DWORD dibHeaderSize = 54;

    DWORD widthBytes = width * pixelSizeBytes_SaveAs;
    DWORD dstPitch = widthBytes;
    if (dstPitch & 3)
        dstPitch = (dstPitch & 0xFFFFFFFC) + 4;
    DWORD pitchDiff = dstPitch - widthBytes;
    DWORD dibBuffSize = dstPitch * height;

    DWORD buffSize = pitchBytes * height;
    DWORD buffOff = buffSize - pitchBytes;


    //void* FileStream = nullptr;
    void* FileStream = fall_fopen(fileName, "wb");
    if (!FileStream) {
        Fallout_Debug_Error("SaveBMP_DATA Could Not Open File - %s", fileName);
        return false;
    }

    //write bmp header
    fall_fwrite16_LE(FileStream, 0x4D42);//"BM"

    fall_fwrite32_LE(FileStream, dibHeaderSize + dibPaletteSize + dibBuffSize);//size of BMP file in bytes (unreliable)

    fall_fwrite16_LE(FileStream, 0x0000);//reserved, must be zero
    fall_fwrite16_LE(FileStream, 0x0000);//reserved, must be zero

    fall_fwrite32_LE(FileStream, dibHeaderSize + dibPaletteSize);//data offset
    fall_fwrite32_LE(FileStream, 40);//headersize
    fall_fwrite32_LE(FileStream, width);//image width in pixels
    fall_fwrite32_LE(FileStream, height);//image height in pixels
    fall_fwrite16_LE(FileStream, 1);//number of planes in the image, must be 1
    fall_fwrite16_LE(FileStream, pixelSizeBits_SaveAs);//number of bits per pixel (1, 4, 8, or 24)
    fall_fwrite32_LE(FileStream, 0);//compression type (0=none, 1=RLE-8, 2=RLE-4)
    fall_fwrite32_LE(FileStream, dibBuffSize);//size of image data in bytes (including padding)
    fall_fwrite32_LE(FileStream, 0);//horizontal resolution in pixels per meter (unreliable)
    fall_fwrite32_LE(FileStream, 0);//vertical resolution in pixels per meter (unreliable)
    if (pixelSizeBits == 8)
        fall_fwrite32_LE(FileStream, 256);//2^pixelSizeBits//number of colors in image, or zero
    else
        fall_fwrite32_LE(FileStream, 0);//number of colors in image, or zero
    fall_fwrite32_LE(FileStream, 0);//number of important colors, or zero

    //save palette
    if (pixelSizeBits == 8) {
        DWORD palEntrySizeBytes = palEntrySizeBits >> 3;// / 8;
        DWORD palSizeBytes = numPalEntries * palEntrySizeBytes;
        for (DWORD i = 0; i < palSizeBytes; i += palEntrySizeBytes) {
            fall_fwrite8(FileStream, pPal[i + palEntrySizeBytes - 1] << 2);//fallout palette values range from 0-64, need to be multiplied by 4 "<< 2" or will appear very dark.
            fall_fwrite8(FileStream, pPal[i + palEntrySizeBytes - 2] << 2);
            fall_fwrite8(FileStream, pPal[i + palEntrySizeBytes - 3] << 2);
            fall_fwrite8(FileStream, 0);
        }
    }
    //save image buff
    for (DWORD yPos = 0; yPos < height; yPos++) {

        for (DWORD xPos = 0; xPos < width * pixelSizeBytes; xPos += pixelSizeBytes) {
            for (DWORD cVal = 0; cVal < pixelSizeBytes_SaveAs; cVal++)
                fall_fwrite8(FileStream, pData[buffOff + xPos + cVal]);
        }
        for (DWORD p = 0; p < pitchDiff; p++)
            fall_fwrite8(FileStream, 0);
        buffOff -= pitchBytes;
    }

    fall_fclose(FileStream);
    return true;
}


//_______________________________________________________
bool LoadFrmHeader(UNLSTDfrm* frmHeader, void* frmStream) {

    if (fall_fread32_BE(frmStream, &frmHeader->version) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, &frmHeader->FPS) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, &frmHeader->actionFrame) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, &frmHeader->numFrames) == -1)
        return false;

    else if (fall_fread16_Array_BE(frmStream, (WORD*)frmHeader->xCentreShift, 6) == -1)
        return false;
    else if (fall_fread16_Array_BE(frmStream, (WORD*)frmHeader->yCentreShift, 6) == -1)
        return false;
    else if (fall_fread32_Array_BE(frmStream, frmHeader->oriOffset, 6) == -1)
        return false;
    else if (fall_fread32_BE(frmStream, &frmHeader->frameAreaSize) == -1)
        return false;

    return true;
}


//____________________________________________________
bool LoadFrmFrame(UNLSTDframe* frame, void* frmStream) {

    if (fall_fread16_BE(frmStream, &frame->width) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, &frame->height) == -1)
        return false;
    else if (fall_fread32_BE(frmStream, &frame->size) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, (WORD*)&frame->x) == -1)
        return false;
    else if (fall_fread16_BE(frmStream, (WORD*)&frame->y) == -1)
        return false;
    frame->buff = new BYTE[frame->size];
    if (fall_fread(frame->buff, frame->size, 1, frmStream) != 1)
        return false;

    return true;
}


//________________________________________________________
UNLSTDfrm* LoadFrmFromID(DWORD frmID, LONG palOffset_Mask) {

    DWORD type = (frmID & 0x0F000000) >> 24;
    if (type > 10) return nullptr;

    char FrmPath[MAX_PATH];

    sprintf_s(FrmPath, MAX_PATH, "%s\0", fall_GetFrmFilePath(frmID));

    //convert path to lower case - to make it easier to search for file extension when checking for matching pal files
    char* pCH = FrmPath;
    while (*pCH != '\0') {
        *pCH = (char)tolower(*pCH);
        pCH++;
    }

    UNLSTDfrm* frm = new UNLSTDfrm;

    void* frmStream = fall_fopen(FrmPath, "rb");
    
    int numOri = 6;
    if (frmStream) {
        if (!LoadFrmHeader(frm, frmStream)) {
            fall_fclose(frmStream);
            delete frm;
            return nullptr;
        }
        if (frm->oriOffset[0] == frm->oriOffset[1])
            numOri = 1;
        for (int ori = 0; ori < numOri; ori++) {
            frm->frames[ori] = new UNLSTDframe * [frm->numFrames];
            for (int fNum = 0; fNum < frm->numFrames; fNum++) {
                frm->frames[ori][fNum] = new UNLSTDframe;
                if (!LoadFrmFrame(frm->frames[ori][fNum], frmStream)) {
                    fall_fclose(frmStream);
                    delete frm;
                    return nullptr;
                }
            }
        }

        fall_fclose(frmStream);
    }
    else {
        delete frm;
        return nullptr;
    }

    char* pExt = strstr(FrmPath, ".fr");
    if (pExt) {

        char ext_ori = *(pExt + 3);//copy the last char from the extension
        strcpy_s(pExt, 5, ".bas");
        if(ext_ori!='m')//if extension doesn't end with 'm', than the frm is divided into separate files by orientation. Put this back to find the matching bas file. ".ba#" where # is the ori.
            *(pExt + 3) = ext_ori;
        //check if the frm has matching base frames for shadow casting.
        void* baseStream = fall_fopen(FrmPath, "rb");
        if (baseStream) {
            UNLSTDfrm* frm_base = new UNLSTDfrm;
            if (LoadFrmHeader(frm_base, baseStream)) {
                if (frm_base && frm_base->numFrames == frm->numFrames) {
                    int numOri_base = 6;
                    if (frm_base->oriOffset[0] == frm_base->oriOffset[1]) {
                        numOri_base = 1;
                    }
                    if (numOri_base == numOri) {
                        for (int ori = 0; ori < numOri; ori++) {
                            frm->frames_base[ori] = new UNLSTDframe * [frm->numFrames];
                            for (int fNum = 0; fNum < frm->numFrames; fNum++) {
                                frm->frames_base[ori][fNum] = new UNLSTDframe;
                                if (!LoadFrmFrame(frm->frames_base[ori][fNum], baseStream) || frm->frames_base[ori][fNum]->width != frm->frames[ori][fNum]->width || frm->frames_base[ori][fNum]->height != frm->frames[ori][fNum]->height) {
                                    Fallout_Debug_Error("LoadFrmFromID - Loading the frm bas file frame ori%d num%d %s", ori, fNum, FrmPath);
                                    delete frm->frames_base[ori][fNum];
                                    frm->frames_base[ori][fNum] = nullptr;
                                    fNum = frm->numFrames;
                                    ori = numOri;
                                }
                            }
                        }
                    }
                }
            }
            else
                Fallout_Debug_Error("LoadFrmFromID - Loading the frm bas file header %s", FrmPath);
            delete frm_base;
            fall_fclose(baseStream);
        }

        strcpy_s(pExt, 5, ".pal");
        //check if frm has a pal file and load
        void* palStream = fall_fopen(FrmPath, "rb");
        if (palStream) {
            frm->pPal = new DWORD[256];
            BYTE r, g, b;
            for (int i = 0; i < 256; i++) {
                fall_fread8(palStream, &r);
                fall_fread8(palStream, &g);
                fall_fread8(palStream, &b);
                r = r << 2;
                g = g << 2;
                b = b << 2;
                frm->pPal[i] = (0xFF << 24) | (r << 16) | (g << 8) | (b);
                if (i == palOffset_Mask)//set alpha of mask colour to 0, pal[0] is mask
                    frm->pPal[i] &= 0x00FFFFFF;
            }
            fall_fclose(palStream);
            palStream = nullptr;
        }
        //check if frm has a matching ".exl" file, if so this means that the frm should be lit uniformly.
        strcpy_s(pExt, 5, ".exl");
        void* isUnifomlyLit_Stream = fall_fopen(FrmPath, "rb");
        if (isUnifomlyLit_Stream) {
            frm->uniformly_lit = true;
            fall_fclose(isUnifomlyLit_Stream);
            isUnifomlyLit_Stream = nullptr;

        }
    }
    return frm;
}


//__________________________________________________________
bool Load_Palette(const char* path, DWORD* pPal, DWORD size) {
    if (pPal == nullptr)
        return false;
    void* palStream = fall_fopen(path, "rb");
    if (palStream) {
        BYTE r, g, b;
        for (DWORD i = 0; i < size; i++) {
            fall_fread8(palStream, &r);
            fall_fread8(palStream, &g);
            fall_fread8(palStream, &b);
            r = r << 2;
            g = g << 2;
            b = b << 2;
            pPal[i] = (0xFF << 24) | (r << 16) | (g << 8) | (b);
            if (i == 0)//set alpha of mask colour to 0, pal[0] is mask
                pPal[i] &= 0x00FFFFFF;
        }
        fall_fclose(palStream);
        palStream = nullptr;
    }

    return true;
}


#define BMP_HEADER 14
#define BITMAPINFOHEADER 40
#define BITMAPV2INFOHEADER 52
#define BITMAPV3INFOHEADER 56
#define BITMAPV4HEADER 108
#define BITMAPV5HEADER 124


//_______________________________________
UNLSTDfrm* LoadBmpAsFrm(const char* path) {

    void* FileStream = fall_fopen(path, "rb");
    if (FileStream == nullptr)
        return nullptr;

    BYTE byteVal = 0;
    WORD wordVal = 0;
    DWORD dwordVal = 0;

    //read bmp header
    fall_fread16_LE(FileStream, &wordVal);
    if (wordVal != 0x4D42) {//"BM"
        fall_fclose(FileStream);
        Fallout_Debug_Error("LoadBmpAsFrm - BMP header invalid %s", path);
        return nullptr;
    }
    DWORD buffSize = 0;
    fall_fread32_LE(FileStream, &buffSize);

    fall_fseek(FileStream, 10, 0);
    DWORD buffOffset = 0;
    fall_fread32_LE(FileStream, &buffOffset);

    buffSize -= buffOffset;

    //read dib header

    //read dib core------------------------------------
    DWORD dibHeaderSize = 0;
    fall_fread32_LE(FileStream, &dibHeaderSize);

    if (dibHeaderSize < BITMAPINFOHEADER) {
        fall_fclose(FileStream);
        Fallout_Debug_Error("LoadBmpAsFrm - BITMAPINFOHEADER must be this version 1 or higher %s", path);
        return nullptr;
    }


    DWORD width = 0;
    DWORD height = 0;
    fall_fread32_LE(FileStream, &width);
    fall_fread32_LE(FileStream, &height);

    DWORD imageSize = width * height;

    UNLSTDfrm* pFrm = new UNLSTDfrm;

    pFrm->version = FRM_VER_32BIT;
    pFrm->FPS = 0;
    pFrm->actionFrame = 0;
    pFrm->numFrames = 1;
    for (int i = 0; i < 6; i++) {
        pFrm->xCentreShift[i] = 0;
        pFrm->yCentreShift[i] = 0;
        pFrm->oriOffset[i] = 0;
    }
    pFrm->frameAreaSize = imageSize * 4;

    pFrm->frames[0] = new UNLSTDframe * [pFrm->numFrames];
    pFrm->frames[0][0] = new UNLSTDframe;
    pFrm->frames[0][0]->width = (WORD)width;
    pFrm->frames[0][0]->height = (WORD)height;
    pFrm->frames[0][0]->size = pFrm->frameAreaSize;
    pFrm->frames[0][0]->x = 0;
    pFrm->frames[0][0]->y = 0;
    pFrm->frames[0][0]->buff = new BYTE[pFrm->frameAreaSize];
    DWORD* buff_32 = (DWORD*)pFrm->frames[0][0]->buff;


    fall_fread16_LE(FileStream, &wordVal);//colour planes
    WORD pixelSize = 0;
    fall_fread16_LE(FileStream, &pixelSize);
    //end dib core
    //read dib extended
    DWORD compression = 0;

    fall_fread32_LE(FileStream, &compression);
    if (compression != 0 && compression != 3 && compression != 6) {
        fall_fclose(FileStream);
        Fallout_Debug_Error("LoadBmpAsFrm - does not support compression %s", path);
        if (pFrm)
            delete pFrm;
        pFrm = nullptr;
        return pFrm;
    }

    LONG bitfield_size = 0;//needed for BITMAPINFOHEADER v1 with optional bit mask
    if (dibHeaderSize == BITMAPINFOHEADER) {
        if (compression == 0)
            bitfield_size = 0;
        else if (compression == 3)
            bitfield_size = 12;
        else if (compression == 6)
            bitfield_size = 16;
    }

    LONG num_colours_palette = 0;
    fall_fseek(FileStream, (LONG)(BMP_HEADER + 32), 0);//32 num colours offset
    fall_fread32_LE(FileStream, (DWORD*)&num_colours_palette);
    if (num_colours_palette == 0)
        num_colours_palette = 256;

    DWORD bitField = 0;
    if (compression) {
        fall_fseek(FileStream, (LONG)(BMP_HEADER + BITMAPINFOHEADER), 0);

        if (compression == 3) {
            fall_fread32_LE(FileStream, &dwordVal);//red
            bitField |= dwordVal;
            fall_fread32_LE(FileStream, &dwordVal);//green
            bitField |= dwordVal;
            fall_fread32_LE(FileStream, &dwordVal);//blue
            bitField |= dwordVal;
        }
        if (compression == 6 && dibHeaderSize != BITMAPV2INFOHEADER) {//BITMAPV2INFOHEADER doesn't contain alpha channel
            fall_fread32_LE(FileStream, &dwordVal);//alpha
            bitField |= dwordVal;
        }

    }
    else
        bitField = 0xFFFFFFFF;

    if (pixelSize == 8) {
        //load palette
        fall_fseek(FileStream, (LONG)(BMP_HEADER + dibHeaderSize + bitfield_size), 0);
        DWORD* pal_32 = new DWORD[num_colours_palette];
        for (int i = 0; i < num_colours_palette; i++) {
            fall_fread32_LE(FileStream, &pal_32[i]);
            pal_32[i] |= 0xFF000000;//set alpha to opaque
        }
        //load image buff
        fall_fseek(FileStream, (long)buffOffset, 0);

        DWORD pitch = width;
        if (pitch & 3)
            pitch = (pitch & 0xFFFFFFFC) + 4;
        DWORD padding = pitch - width;

        DWORD y_Offset = imageSize - width;
        for (DWORD y = 0; y < height; y++) {
            for (DWORD x = 0; x < width; x++) {
                fall_fread8(FileStream, &byteVal);
                buff_32[y_Offset + x] = pal_32[byteVal];
            }
            for (DWORD p = 0; p < padding; p++)
                fall_fread8(FileStream, &byteVal);

            y_Offset -= width;
        }
        delete[] pal_32;
        pal_32 = nullptr;
    }
    else if (pixelSize == 24) {
        //load image buff
        fall_fseek(FileStream, (long)buffOffset, 0);

        DWORD padding = buffSize / height - width * 3;
        DWORD y_Offset = imageSize - width;
        for (DWORD y = 0; y < height; y++) {
            for (DWORD x = 0; x < width; x++) {
                buff_32[y_Offset + x] = 0xFFFFFFFF;
                fall_fread(&buff_32[y_Offset + x], 3, 1, FileStream);
                buff_32[y_Offset + x] &= bitField;
            }
            for (DWORD p = 0; p < padding; p++)
                fall_fread8(FileStream, &byteVal);
            y_Offset -= width;
        }

    }
    else if (pixelSize == 32) {
        //load image buff
        fall_fseek(FileStream, (long)buffOffset, 0);
        DWORD y_Offset = imageSize - width;
        for (DWORD y = 0; y < height; y++) {
            for (DWORD x = 0; x < width; x++) {
                fall_fread32_LE(FileStream, &buff_32[y_Offset + x]);
                buff_32[y_Offset + x] &= bitField;
                if (compression == 3)
                    buff_32[y_Offset + x] |= 0xFF000000;//set alpha to opaque
            }
            y_Offset -= width;
        }

    }
    else {
        fall_fclose(FileStream);
        Fallout_Debug_Error("LoadBmpAsFrm - only supports 8, 24 or 32 bit bmps %s", path);
        if (pFrm)
            delete pFrm;
        pFrm = nullptr;
        return pFrm;
    }
    fall_fclose(FileStream);

    return pFrm;
}


//_______________________________________
UNLSTDfrm* LoadRixAsFrm(const char* path) {

    void* FileStream = fall_fopen(path, "rb");
    if (FileStream == nullptr)
        return nullptr;

    DWORD RixType = 0;
    fall_fread32_LE(FileStream, &RixType);
    if (RixType != 0x33584952) {//"RIX3"
        fall_fclose(FileStream);
        Fallout_Debug_Error("LoadRixAsFrm - Rix header invalid %s", path);
        return nullptr;
    }
    UNLSTDfrm* pFrm = new UNLSTDfrm;

    pFrm->version = FRM_VER_32BIT;
    pFrm->FPS = 0;
    pFrm->actionFrame = 0;
    pFrm->numFrames = 1;
    for (int i = 0; i < 6; i++) {
        pFrm->xCentreShift[i] = 0;
        pFrm->yCentreShift[i] = 0;
        pFrm->oriOffset[i] = 0;
    }


    WORD width = 0;
    WORD height = 0;
    fall_fread16_LE(FileStream, &width);
    fall_fread16_LE(FileStream, &height);
    DWORD imageSize = width * height;

    pFrm->frameAreaSize = imageSize * 4;

    pFrm->frames[0] = new UNLSTDframe * [pFrm->numFrames];
    pFrm->frames[0][0] = new UNLSTDframe;
    pFrm->frames[0][0]->width = width;
    pFrm->frames[0][0]->height = height;
    pFrm->frames[0][0]->size = pFrm->frameAreaSize;
    pFrm->frames[0][0]->x = 0;
    pFrm->frames[0][0]->y = 0;
    pFrm->frames[0][0]->buff = new BYTE[imageSize * 4];
    DWORD* buff_32 = (DWORD*)pFrm->frames[0][0]->buff;

    WORD wordVal = 0;
    fall_fread16_LE(FileStream, &wordVal);//unknown

    DWORD* pal_32 = new DWORD[256];
    BYTE* pal_8 = nullptr;

    for (int i = 0; i < 256; i++) {
        pal_8 = (BYTE*)&pal_32[i];
        pal_8[3] = 0xFF;//set alpha set to opaque
        fall_fread8(FileStream, &pal_8[2]);
        fall_fread8(FileStream, &pal_8[1]);
        fall_fread8(FileStream, &pal_8[0]);
        //multiply colour values by 4 - colours were darkened for fallout
        pal_8[2] <<= 2;
        pal_8[1] <<= 2;
        pal_8[0] <<= 2;

    }
    pal_8 = nullptr;

    BYTE byteVal = 0;

    for (DWORD uVal = 0; uVal < imageSize; uVal++) {
        fall_fread8(FileStream, &byteVal);
        buff_32[uVal] = pal_32[byteVal];
    }
    fall_fclose(FileStream);

    delete[] pal_32;
    pal_32 = nullptr;

    return pFrm;
}


//_______________________________________________________________________________
UNLSTDfrm* LoadUnlistedFrm(const char* sFileName, LONG type, LONG palOffset_Mask) {
    // Exclude file extention in "sFileName" if you want to check for and load a bmp instead of an frm.
    // Set "type" to -1 if "sFileName" contains the full path.
    // If both the file extention is excluded and type is negative, a rix file will also be checked for after bmp. 
    // If file is an FRM and has its own palette, palOffset_Mask is the palette offset you want to be invisible, Set to -1 to ignore.

    if (type > 10)
        return nullptr;

    char FrmPath[MAX_PATH];
    if (type < 0) {//if type is negative assume FrmName is the full path
        strcpy_s(FrmPath, MAX_PATH, sFileName);
    }
    else {
        char* artfolder = pArtTypeArray[type].name;//address of art type name
        sprintf_s(FrmPath, MAX_PATH, "art\\%s\\%s\0", artfolder, sFileName);
    }

    //convert path to lower case - to make it easier to search for file extension when checking for matching pal files
    char* pCH = FrmPath;
    while (*pCH != '\0') {
        *pCH = (char)tolower(*pCH);
        pCH++;
    }

    char* pExt = strstr(FrmPath, ".fr");

    if (!pExt) {//if no frm extension is found - check other image formats first - bmp , rix then frm. 
        strcat_s(FrmPath, _countof(FrmPath), ".bmp");
        UNLSTDfrm* frm = LoadBmpAsFrm(FrmPath);
        if (frm)
            return frm;

        pExt = strstr(FrmPath, ".bmp");//get a pointer to the extension for other file types

        if (pExt && type < 0) {//Only check for rix if type is negative - this is only used for splash screens where full path is used.
            strcpy_s(pExt, 5, ".rix");
            frm = LoadRixAsFrm(FrmPath);
            if (frm)
                return frm;
        }
        if (pExt)
            strcpy_s(pExt, 5, ".frm");

    }

    void* frmStream = fall_fopen(FrmPath, "rb");
    if (!frmStream)
        return nullptr;

    UNLSTDfrm* frm = new UNLSTDfrm;

    if (!LoadFrmHeader(frm, frmStream)) {
        fall_fclose(frmStream);
        delete frm;
        return nullptr;
    }

    int numOri = 6;
    if (frm->oriOffset[0] == frm->oriOffset[1])
        numOri = 1;
    for (int ori = 0; ori < numOri; ori++) {
        frm->frames[ori] = new UNLSTDframe * [frm->numFrames];
        for (int fNum = 0; fNum < frm->numFrames; fNum++) {
            frm->frames[ori][fNum] = new UNLSTDframe;
            if (!LoadFrmFrame(frm->frames[ori][fNum], frmStream)) {
                fall_fclose(frmStream);
                delete frm;
                return nullptr;
            }
        }
    }
    fall_fclose(frmStream);

    //check if frm has a pal file and load
    if (pExt) {
        strcpy_s(pExt, 5, ".pal");

        void* palStream = fall_fopen(FrmPath, "rb");
        if (palStream) {
            frm->pPal = new DWORD[256];
            BYTE r, g, b;
            for (int i = 0; i < 256; i++) {
                fall_fread8(palStream, &r);
                fall_fread8(palStream, &g);
                fall_fread8(palStream, &b);
                r = r << 2;
                g = g << 2;
                b = b << 2;
                frm->pPal[i] = (0xFF << 24) | (r << 16) | (g << 8) | (b);
                if (i == palOffset_Mask) //set alpha of mask colour to 0.
                    frm->pPal[i] &= 0x00FFFFFF;

            }
            fall_fclose(palStream);
            palStream = nullptr;
        }
        //check if frm has a matching ".exl" file, if so this means that the frm should be lit uniformly.
        strcpy_s(pExt, 5, ".exl");
        void* isUnifomlyLit_Stream = fall_fopen(FrmPath, "rb");
        if (isUnifomlyLit_Stream) {
            frm->uniformly_lit = true;
            fall_fclose(isUnifomlyLit_Stream);
            isUnifomlyLit_Stream = nullptr;
        }
    }
    return frm;
}





//____________________________________________________________________________
UNLSTDfrm* CreateUnlistedFrm(int width, int height, int numFrames, int numOri) {
    if (!width || !height || !numOri || !numFrames) {
        Fallout_Debug_Error("CreateUnlistedFrm failed. All frm vars must be greater than 0.");
        return nullptr;
    }
    UNLSTDfrm* frm = new UNLSTDfrm;
    frm->version = FRM_VER_ORIGINAL;
    frm->numFrames = numFrames;
    frm->frameAreaSize = (width * height + 12) * 6;
 
    for (int ori = 0; ori < 6; ori++) {
        frm->frames[ori] = new UNLSTDframe * [numFrames];
        for (int frame = 0; frame < numFrames; frame++) {
            frm->frames[ori][frame] = new UNLSTDframe;
            frm->frames[ori][frame]->width = width;
            frm->frames[ori][frame]->height = height;
            frm->frames[ori][frame]->size = width * height;
            frm->frames[ori][frame]->buff = new BYTE[frm->frames[ori][frame]->size];
            memset(frm->frames[ori][frame]->buff, '\0', frm->frames[ori][frame]->size);
        }
    }
    return frm;
}


