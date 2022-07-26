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
#include "memwrite.h"


EXE_Region fallout_exe_region = EXE_Region::UNKNOWN;


struct OFFSETdiff {
   int num;
   DWORD *address;
   DWORD*offset;
   OFFSETdiff() {
      num=0;
      address=nullptr;
      offset=nullptr;
   }
   ~OFFSETdiff() {
      num=0;
      delete[]address;
      delete[]offset;
   }

};


OFFSETdiff* offsetDiff = nullptr;


//________________________________
DWORD FixAddress(DWORD memAddress) {
    if (!offsetDiff)
        return memAddress;

    DWORD tmpMemAddess = memAddress;

    if (memAddress < 0x410000)
        tmpMemAddess += 0x410000;

    int next = offsetDiff->num - 1;
    while (tmpMemAddess < offsetDiff->address[next] && next>0) {
        next--;
    }
    memAddress = memAddress + offsetDiff->offset[next];

    return memAddress;
}


//______________________
void SetAddressDiffsUK() {

    offsetDiff = new OFFSETdiff;
    offsetDiff->num = 6;
    offsetDiff->address = new DWORD[offsetDiff->num];
    offsetDiff->offset = new DWORD[offsetDiff->num];

    offsetDiff->address[0] = 0;
    offsetDiff->offset[0] = 0;
    offsetDiff->address[1] = 0x442B84;
    offsetDiff->offset[1] = +432;
    offsetDiff->address[2] = 0x500000;
    offsetDiff->offset[2] = +80;

    offsetDiff->address[3] = 0x50C3A8;
    offsetDiff->offset[3] = +92;
    offsetDiff->address[4] = 0x50C3AC;
    offsetDiff->offset[4] = +80;

    offsetDiff->address[5] = 0x56DBAC;
    offsetDiff->offset[5] = 0;

    //offsetDiff->address[3] = 0x570000;
    //offsetDiff->offset[3] = 0;

    //offsetDiff->address[3] = 0x58E000;//600000;
    //offsetDiff->offset[3] = 0;
}


//_________________________
void SetAddressDiffsFR_GR() {

    offsetDiff = new OFFSETdiff;
    offsetDiff->num = 8;
    offsetDiff->address = new DWORD[offsetDiff->num];
    offsetDiff->offset = new DWORD[offsetDiff->num];

    offsetDiff->address[0] = 0;
    offsetDiff->offset[0] = 0;
    offsetDiff->address[1] = 0x442B84;
    offsetDiff->offset[1] = +432;
    offsetDiff->address[2] = 0x4551E1;
    offsetDiff->offset[2] = +500;
    offsetDiff->address[3] = 0x45D880;
    offsetDiff->offset[3] = +496;
    offsetDiff->address[4] = 0x483188;
    offsetDiff->offset[4] = +728;
    offsetDiff->address[5] = 0x4845B0;
    offsetDiff->offset[5] = +720;
    offsetDiff->address[6] = 0x500000;
    offsetDiff->offset[6] = +128;
    offsetDiff->address[7] = 0x56DBAC;
    offsetDiff->offset[7] = 0;
    //offsetDiff->address[7] = 0x570000;
    //offsetDiff->offset[7] = 0;
    //offsetDiff->address[7] = 0x58E000;//6A0000;
    //offsetDiff->offset[7] = 0;
}


//_________________________
void SetAddressDiffsRU_LC() {

    offsetDiff = new OFFSETdiff;
    offsetDiff->num = 22;
    offsetDiff->address = new DWORD[offsetDiff->num];
    offsetDiff->offset = new DWORD[offsetDiff->num];

    offsetDiff->address[0] = 0;
    offsetDiff->offset[0] = 0;
    offsetDiff->address[1] = 0x452804;
    offsetDiff->offset[1] = -196;
    offsetDiff->address[2] = 0x452970;
    offsetDiff->offset[2] = -192;
    offsetDiff->address[3] = 0x4541C8;
    offsetDiff->offset[3] = -188;
    offsetDiff->address[4] = 0x45967D;
    offsetDiff->offset[4] = -190;
    offsetDiff->address[5] = 0x4598BC;
    offsetDiff->offset[5] = -192;
    offsetDiff->address[6] = 0x4836DC;
    offsetDiff->offset[6] = -199;
    offsetDiff->address[7] = 0x483784;
    offsetDiff->offset[7] = -200;
    offsetDiff->address[8] = 0x4845B0;
    offsetDiff->offset[8] = -208;
    offsetDiff->address[9] = 0x488DAA;
    offsetDiff->offset[9] = -226;
    offsetDiff->address[10] = 0x48909C;
    offsetDiff->offset[10] = -224;
    offsetDiff->address[11] = 0x494504;
    offsetDiff->offset[11] = -234;
    offsetDiff->address[12] = 0x49460C;
    offsetDiff->offset[12] = -232;
    offsetDiff->address[13] = 0x494D7C;
    offsetDiff->offset[13] = -296;
    offsetDiff->address[14] = 0x4961B0;
    offsetDiff->offset[14] = -304;
    offsetDiff->address[15] = 0x4B1BA2;
    offsetDiff->offset[15] = -311;
    offsetDiff->address[16] = 0x4B1D20;
    offsetDiff->offset[16] = -312;
    offsetDiff->address[17] = 0x4B39F0;
    offsetDiff->offset[17] = -304;
    offsetDiff->address[18] = 0x500000;
    offsetDiff->offset[18] = -16;
    offsetDiff->address[19] = 0x50C3A8;
    offsetDiff->offset[19] = 0;
    offsetDiff->address[20] = 0x50C804;
    offsetDiff->offset[20] = -16;
    offsetDiff->address[21] = 0x56DBAC;
    offsetDiff->offset[21] = 0;
    //offsetDiff->address[19] = 0x570000;
    //offsetDiff->offset[19] = 0;
    //offsetDiff->address[19] = 0x58E000;//6A0000;
    //offsetDiff->offset[19] = 0;
}


//________________________
void MemWrite_DestroyMem() {
    if (offsetDiff)
        delete offsetDiff;
    offsetDiff = nullptr;
}


//______________________________________________________________________________________________
void FuncWrite32__(const char* file, int line, DWORD address, DWORD expected, DWORD funcAddress) {
    address = FixAddress(address);
    funcAddress = funcAddress - (address + 4);

    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (CompareMem_DWORD(file, line, (DWORD*)address, expected))
        *(DWORD*)address = funcAddress;
    VirtualProtect((LPVOID)address, 4, oldProtect, &oldProtect);
}


//________________________________________________________________________________________________
void FuncReplace32__(const char* file, int line, DWORD address, DWORD expected, DWORD funcAddress) {
    expected = expected + (address + 4);
    address = FixAddress(address);
    funcAddress = FixAddress(funcAddress) - (address + 4);
    expected = FixAddress(expected) - (address + 4);

    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (CompareMem_DWORD(file, line, (DWORD*)address, expected))
        *(DWORD*)address = funcAddress;
    VirtualProtect((LPVOID)address, 4, oldProtect, &oldProtect);
}


//________________________________________________________________________________________
void MemWrite8__(const char* file, int line, DWORD address, BYTE expected, BYTE change_to) {
    address = FixAddress(address);

    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (CompareMem_BYTE(file, line, (BYTE*)address, expected))
        *(BYTE*)address = change_to;
    VirtualProtect((LPVOID)address, 1, oldProtect, &oldProtect);
}


//_________________________________________________________________________________________
void MemWrite16__(const char* file, int line, DWORD address, WORD expected, WORD change_to) {
    address = FixAddress(address);

    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 2, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (CompareMem_WORD(file, line, (WORD*)address, expected))
        *(WORD*)address = change_to;
    VirtualProtect((LPVOID)address, 2, oldProtect, &oldProtect);
}


//___________________________________________________________________________________________
void MemWrite32__(const char* file, int line, DWORD address, DWORD expected, DWORD change_to) {
    address = FixAddress(address);

    DWORD oldProtect;
    VirtualProtect((LPVOID)address, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
    if (CompareMem_DWORD(file, line, (DWORD*)address, expected))
        *(DWORD*)address = change_to;
    VirtualProtect((LPVOID)address, 4, oldProtect, &oldProtect);
}


//_______________________________________________________________________________________________________________________________
void MemWriteString__(const char* file, int line, DWORD address, unsigned char* expected, unsigned char* change_to, DWORD length) {
    address = FixAddress(address);

    if (memcmp((void*)address, expected, length) != 0)
        Error_RecordMemMisMatch(file, line, address, expected, (unsigned char*)address, length);
    else {
        DWORD oldProtect;
        VirtualProtect((LPVOID)address, length + 1, PAGE_EXECUTE_READWRITE, &oldProtect);
        memcpy((void*)address, change_to, length);
        VirtualProtect((LPVOID)address, length + 1, oldProtect, &oldProtect);
    }
}


//_________________________________________________________________________________________________
void MemBlt8(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

    for (int h = 0; h < subHeight; h++) {
        memcpy(toBuff, fromBuff, subWidth);
        fromBuff += fromWidth;
        toBuff += toWidth;
    }
}


//_______________________________________________________________________________________________________
void MemBltMasked8(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth) {

    for (int h = 0; h < subHeight; h++) {
        for (int w = 0; w < subWidth; w++) {
            if (fromBuff[w] != 0)
                toBuff[w] = fromBuff[w];
        }
        fromBuff += fromWidth;
        toBuff += toWidth;
    }
}


//_________________________________________________________________________________________________________________________________________________
void MemBlt8Stretch(BYTE* fromBuff, int subWidth, int subHeight, int fromWidth, BYTE* toBuff, int toWidth, int toHeight, bool ARatio, bool centred) {

    float toWidthAR = (float)toWidth, toHeightAR = (float)toHeight;

    if (ARatio) {
        float imageRO = (float)subWidth / subHeight;
        float winRO = (float)toWidth / toHeight;

        if (winRO > imageRO) {
            toWidthAR = toHeightAR / subHeight * subWidth;
            if (centred)
                toBuff += (toWidth - (int)toWidthAR) / 2;
        }
        else if (winRO < imageRO) {
            toHeightAR = toWidthAR / subWidth * subHeight;
            if (centred)
                toBuff += ((toHeight - (int)toHeightAR) / 2) * toWidth;
        }
    }

    float pWidth = subWidth / toWidthAR;
    float pHeight = subHeight / toHeightAR;

    float fx = 0, fy = 0;
    int fyMul = 0;

    for (int ty = 0; ty < (int)toHeightAR; ty++) {
        fx = 0;
        for (int tx = 0; tx < (int)toWidthAR; tx++) {//draw stretched line
            toBuff[tx] = fromBuff[fyMul + (int)fx];
            fx += pWidth;
            if (fx >= subWidth)
                fx = (float)subWidth - 1;
        }
        fy += pHeight;
        if ((int)fy >= subHeight)
            fy = (float)subHeight - 1;
        fyMul = (int)fy * fromWidth;
        toBuff += toWidth;
    }
}


//__________________________________________________________________________________
bool CompareCharArray_IgnoreCase(const char* msg1, const char* msg2, DWORD numChars) {
    if (msg1 == nullptr || msg2 == nullptr)
        return false;
    for (DWORD num = 0; num < numChars; num++) {
        if (msg1[num] == '\0' && msg2[num] != '\0')
            return false;
        if (msg2[num] == '\0' && msg1[num] != '\0')
            return false;
        if (tolower(msg1[num]) != tolower(msg2[num]))
            return false;
    }
    return true;
}


//___________________________
bool Set_Fallout_EXE_Region() {

    if (*(DWORD*)0x476B0D == 0x519078) {
        //US 1.02d
        fallout_exe_region = EXE_Region::USA;
    }
    else if (*(DWORD*)0x476CBD == 0x5190C8) {
        //UK 1.02e
        fallout_exe_region = EXE_Region::UK;
        SetAddressDiffsUK();
    }
    else if (*(DWORD*)0x476CFD == 0x5190F8) {
        //French or German 1.02d
        fallout_exe_region = EXE_Region::French_German;
        SetAddressDiffsFR_GR();
    }
    else if (*(DWORD*)0x476109 == 0x528E68) {
        //Chinese 1.02
        fallout_exe_region = EXE_Region::Chinese;
    }
    else if (*(DWORD*)0x476A4D == 0x519068) {
        //Russian Lev Corp
        fallout_exe_region = EXE_Region::Russian_Lev_Corp;
        SetAddressDiffsRU_LC();
    }
    else {
        //UNKNOWN
        fallout_exe_region = EXE_Region::UNKNOWN;
        return false;
    }
    return true;
}
