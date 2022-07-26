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
#include "Fall_Graphics.h"
#include "Fall_File.h"
#include "memwrite.h"
#include "modifications.h"


void* pfall_frm_frame_get_buff = nullptr;
void* pfall_frm_frame_get_height = nullptr;
void* pfall_frm_frame_get_width = nullptr;
void* pfall_frm_get_frm = nullptr;
void* pfall_frm_get_buff = nullptr;
void* pfall_frm_unload = nullptr;
void* pfall_get_frm_id = nullptr;

ARTtype* pArtTypeArray = nullptr;

void* pfall_check_frm_file_exists = nullptr;

void* pfall_get_frm_file_path = nullptr;



//__________________________________
LONG ToggleArtTypeEnabled(LONG type) {
    if (type < 0 || type>11)
        return 0;
    pArtTypeArray[type].enabled = 1 - pArtTypeArray[type].enabled;
    return 1 - pArtTypeArray[type].enabled;
}


//______________________________
LONG IsArtTypeEnabled(LONG type) {
    if (type < 0 || type>11)
        return 0;
    return 1 - pArtTypeArray[type].enabled;
}


//_____________________________
char* GetArtTypeName(LONG type) {
    if (type < 0 || type>11)
        return nullptr;
    return pArtTypeArray[type].name;
}


//______________________________
DWORD GetArtTypeSize(LONG type) {
    if (type < 0 || type>11)
        return 0;
    return pArtTypeArray[type].listSize;
}


//______________________________________
int fall_CheckFrmFileExists(DWORD frmID) {
    int retVal = 0;
    __asm {
        push edx
        mov eax, frmID
        call pfall_check_frm_file_exists
        mov retVal, eax
        pop edx
    }
    return retVal;
}


//____________________________________________________________________________
DWORD fall_GetFrmID(DWORD art_type, DWORD lstNum, DWORD id2, DWORD id1, DWORD id3) {
    DWORD frmID = 0;
    __asm {
        push id3
        mov ecx, id1
        mov ebx, id2
        mov edx, lstNum
        mov eax, art_type
        call pfall_get_frm_id
        mov frmID, eax
    }
    return frmID;
}


//________________________________
void fall_Frm_Unload(DWORD frmObj) {
    __asm {
        mov eax, frmObj
        call pfall_frm_unload
    }
}


//________________________________________________________________________________
BYTE* fall_Frm_GetIndexBuffer(DWORD FID, DWORD frameNum, DWORD ori, DWORD* frmObj) {
    BYTE* buff;
    __asm {
        mov ecx, frmObj
        mov ebx, ori
        mov edx, frameNum
        mov eax, FID
        call pfall_frm_get_buff
        mov buff, eax
    }
    return buff;
}


//____________________________________________
FRMhead* fall_GetFrm(DWORD FID, DWORD* frmObj) {
    FRMhead* pfrm;
    __asm {
        mov eax, FID
        mov edx, frmObj
        CALL pfall_frm_get_frm
        MOV pfrm, EAX
    }
    return pfrm;
}


//____________________________________________________________________
DWORD fall_FrmFrame_GetWidth(FRMhead* pfrm, DWORD frameNum, DWORD ori) {
    DWORD width;
    __asm {
        mov ebx, ori//0-5
        mov edx, frameNum
        mov eax, pfrm
        call pfall_frm_frame_get_width
        mov width, eax
    }
    return width;
}


//_____________________________________________________________________
DWORD fall_FrmFrame_GetHeight(FRMhead* pfrm, DWORD frameNum, DWORD ori) {
    DWORD height;
    __asm {
        mov ebx, ori//0-5
        mov edx, frameNum
        mov eax, pfrm
        call pfall_frm_frame_get_height
        mov height, eax
    }
    return height;
}


//_________________________________________________________________________
BYTE* fall_FrmFrame_GetIndexBuffer(FRMhead* pfrm, DWORD frameNum, DWORD ori) {
    BYTE* buff;
    __asm {
        mov ebx, ori//0-5
        mov edx, frameNum
        mov eax, pfrm
        call pfall_frm_frame_get_buff
        mov buff, eax
    }
    return buff;
}


//____________________________________
char* fall_GetFrmFilePath(DWORD frmID) {
    char* frmPath;
    __asm {
        mov eax, frmID
        call pfall_get_frm_file_path
        mov frmPath, eax
    }
    return frmPath;
}


//_____________________________________
void Fallout_Functions_Setup_Graphics() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        pfall_frm_frame_get_buff = (void*)0x419808;
        pfall_frm_frame_get_height = (void*)0x419750;
        pfall_frm_frame_get_width = (void*)0x419738;
        pfall_frm_get_frm = (void*)0x4190F8;
        pfall_frm_get_buff = (void*)0x419120;
        pfall_frm_unload = (void*)0x4191F8;
        pfall_get_frm_id = (void*)0x419C20;

        pArtTypeArray = (ARTtype*)0x520528;

        pfall_check_frm_file_exists = (void*)FixAddress(0x419860);

        //To-Do pfall_get_frm_file_path = (void*)0x;
    }
    else {
        pfall_frm_frame_get_buff = (void*)FixAddress(0x419870);
        pfall_frm_frame_get_height = (void*)FixAddress(0x4197B8);
        pfall_frm_frame_get_width = (void*)FixAddress(0x4197A0);
        pfall_frm_get_frm = (void*)FixAddress(0x419160);
        pfall_frm_get_buff = (void*)FixAddress(0x419188);
        pfall_frm_unload = (void*)FixAddress(0x419260);
        pfall_get_frm_id = (void*)FixAddress(0x419C88);

        pArtTypeArray = (ARTtype*)FixAddress(0x510738);

        pfall_check_frm_file_exists = (void*)FixAddress(0x4198C8);

        pfall_get_frm_file_path = (void*)FixAddress(0x419428);
    }
}






