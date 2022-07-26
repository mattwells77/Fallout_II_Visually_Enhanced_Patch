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
#include "Fall_File.h"
#include "memwrite.h"

void* pfall_fopen = nullptr;
void* pfall_fclose = nullptr;
void* pfall_fseek = nullptr;

void* pfall_ftell = nullptr;

void* pfall_fread = nullptr;
void* pfall_fgetc = nullptr;
void* pfall_fgets = nullptr;

void* pfall_fwrite = nullptr;
void* pfall_fputc = nullptr;

void* pfall_cfg_read_int = nullptr;
void* pfall_cfg_read_string = nullptr;
void* pfall_cfg_write_int = nullptr;

void* pfall_set_data_path = nullptr;

void* pfall_delete_files = nullptr;


void* p_fallout_cfg_file_ptr = nullptr;
char* p_fallout_cfg_file_name = nullptr;

void* pfall_file_list_get = nullptr;
void* pfall_file_list_release = nullptr;


//_______________________
WORD ByteSwap16(WORD num) {
    return (((num >> 8)) | (num << 8));
}

//_________________________
DWORD ByteSwap32(DWORD num) {
    return (((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
        ((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
}



//__________________________________________________
int fall_FileList_Get(const char* path, char*** list) {
    int numItems = 0;
    __asm {
        push ecx
        push ebx
        mov edx, list
        mov eax, path
        call pfall_file_list_get
        mov numItems, eax
        pop ebx
        pop ecx
    }
    return numItems;
}


//____________________________________
int fall_FileList_Release(char*** list) {
    int retVal = 0;
    __asm {
        xor edx, edx
        mov eax, list
        call pfall_file_list_release
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________
int fall_Files_Delete(const char* path, const char* ext) {
    int retVal = 0;
    __asm {
        mov edx, ext
        mov eax, path
        call pfall_delete_files
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________________________
int fall_SetDataPath(const char* path1, int isFolder1, const char* path2, int isFolder2) {
    int retVal = 0;
    __asm {
        mov ecx, isFolder2
        mov ebx, path2
        mov edx, isFolder1
        mov eax, path1
        CALL pfall_set_data_path
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________________________________
int fall_Cfg_Read_Int(void* FileStream, const char* secName, const char* keyName, int* pIntVal) {
    int retVal;
    __asm {
        mov ecx, pIntVal
        mov ebx, keyName
        mov edx, secName
        mov eax, FileStream
        CALL pfall_cfg_read_int
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________________________________________________________________
int fall_Cfg_Read_String(void* FileStream, const char* secName, const char* keyName, char** ppString) {
    int retVal;
    __asm {
        mov ecx, ppString
        mov ebx, keyName
        mov edx, secName
        mov eax, FileStream
        CALL pfall_cfg_read_string
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________________________________
int fall_Cfg_Write_Int(void* FileStream, const char* secName, const char* keyName, int IntVal) {
    int retVal;
    __asm {
        mov ecx, IntVal
        mov ebx, keyName
        mov edx, secName
        mov eax, FileStream
        CALL pfall_cfg_write_int
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________
void* fall_fopen(const char* FileName, const char* flags) {
    void* retVal = nullptr;
    __asm {
        mov edx, flags
        mov eax, FileName
        CALL pfall_fopen
        mov retVal, eax
    }
    return retVal;
}


//_______________________________
int fall_fclose(void* FileStream) {
    int retVal;
    __asm {
        mov eax, FileStream
        CALL pfall_fclose
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________
int fall_fseek(void* FileStream, LONG fOffset, LONG origin) {
    int retVal;
    __asm {
        mov ebx, origin
        mov edx, fOffset
        mov eax, FileStream
        CALL pfall_fseek
        mov retVal, eax
    }
    return retVal;
}


//_______________________________
LONG fall_ftell(void* FileStream) {
    LONG retVal;
    __asm {

        mov eax, FileStream
        CALL pfall_ftell
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________
int fall_fread8(void* FileStream, BYTE* byte) {

    return *byte = fall_fgetc(FileStream);
}


//_______________________________________________
int fall_fread16_LE(void* FileStream, WORD* word) {

    return fall_fread(word, 2, 1, FileStream);
}


//_________________________________________________
int fall_fread32_LE(void* FileStream, DWORD* dword) {

    return fall_fread(dword, 4, 1, FileStream);
}


//_______________________________________________
int fall_fread16_BE(void* FileStream, WORD* word) {
    int retVal = fall_fread(word, 2, 1, FileStream);
    *word = ByteSwap16(*word);
    return retVal;
}


//_________________________________________________
int fall_fread32_BE(void* FileStream, DWORD* dword) {
    int retVal = fall_fread(dword, 4, 1, FileStream);
    *dword = ByteSwap32(*dword);
    return retVal;
}


//__________________________________________________________________
int fall_fread16_Array_BE(void* FileStream, WORD* words, size_t num) {
    int retVal = fall_fread(words, 2, num, FileStream);
    for (UINT i = 0; i < num; i++)
        words[i] = ByteSwap16(words[i]);
    return retVal;

}


//____________________________________________________________________
int fall_fread32_Array_BE(void* FileStream, DWORD* dwords, size_t num) {
    int retVal = fall_fread(dwords, 4, num, FileStream);
    for (UINT i = 0; i < num; i++)
        dwords[i] = ByteSwap32(dwords[i]);
    return retVal;

}


//___________________________________________
int fall_fwrite8(void* FileStream, BYTE byte) {
    return fall_fputc(byte, FileStream);
}


//_______________________________________________
int fall_fwrite16_LE(void* FileStream, WORD word) {
    return fall_fwrite(&word, 2, 1, FileStream);
}


//_________________________________________________
int fall_fwrite32_LE(void* FileStream, DWORD dword) {
    return fall_fwrite(&dword, 4, 1, FileStream);
}


//_______________________________________________
int fall_fwrite16_BE(void* FileStream, WORD word) {
    word = ByteSwap16(word);
    return fall_fwrite(&word, 2, 1, FileStream);
}


//_________________________________________________
int fall_fwrite32_BE(void* FileStream, DWORD dword) {
    dword = ByteSwap32(dword);
    return fall_fwrite(&dword, 4, 1, FileStream);
}


//___________________________________________________________________
int fall_fwrite16_Array_BE(void* FileStream, WORD* words, size_t num) {
    for (UINT i = 0; i < num; i++)
        words[i] = ByteSwap16(words[i]);
    return fall_fwrite(words, 2, num, FileStream);

}


//_____________________________________________________________________
int fall_fwrite32_Array_BE(void* FileStream, DWORD* dwords, size_t num) {
    for (UINT i = 0; i < num; i++)
        dwords[i] = ByteSwap32(dwords[i]);
    return fall_fwrite(dwords, 4, num, FileStream);
}


//______________________________
int fall_fgetc(void* FileStream) {
    int retVal;
    __asm {
        mov eax, FileStream
        CALL pfall_fgetc
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________
int fall_fread(void* buffer, size_t size_, size_t num, void* FileStream) {
    int retVal;
    __asm {
        mov ecx, FileStream
        mov ebx, num
        mov edx, size_
        mov eax, buffer
        CALL pfall_fread
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________
int fall_fputc(int ch_, void* FileStream) {
    int retVal;
    __asm {
        mov edx, FileStream
        mov eax, ch_
        CALL pfall_fputc
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________________
int fall_fwrite(void* buffer, size_t size_, size_t num, void* FileStream) {
    int retVal;
    __asm {
        mov ecx, FileStream
        mov ebx, num
        mov edx, size_
        mov eax, buffer
        CALL pfall_fwrite
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________________
int FalloutCfg_Read_Int(const char* secName, const char* keyName, int* pIntVal) {
    return  fall_Cfg_Read_Int(p_fallout_cfg_file_ptr, secName, keyName, pIntVal);
}


//____________________________________________________________________________
int FalloutCfg_Write_Int(const char* secName, const char* keyName, int IntVal) {
    return fall_Cfg_Write_Int(p_fallout_cfg_file_ptr, secName, keyName, IntVal);
}


//___________________________________________________________________________________
int FalloutCfg_Read_String(const char* secName, const char* keyName, char** ppString) {
    return fall_Cfg_Read_String(p_fallout_cfg_file_ptr, secName, keyName, ppString);
}


//________________________________________________
bool FalloutData_Does_File_Exist(const char* path) {
    void* frmStream = fall_fopen(path, "rb");
    if (!frmStream)
        return false;
    fall_fclose(frmStream);
    frmStream = nullptr;
    return true;
}


//_________________________________
void Fallout_Functions_Setup_File() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        pfall_fopen = (void*)0x4E7992;
        pfall_fclose = (void*)0x4E7818;
        pfall_fseek = (void*)0x4E87F0;
        pfall_ftell = (void*)0x4E8965;

        pfall_fread = (void*)0x4E8541;
        pfall_fgetc = (void*)0x4E80AA;
        pfall_fgets = (void*)0x4E815E;
        pfall_fwrite = (void*)0x4E8642;
        pfall_fputc = (void*)0x4E8282;

        p_fallout_cfg_file_ptr = (void*)0x59EED0;
        p_fallout_cfg_file_name = (char*)0x59EEF8;

        pfall_cfg_read_int = (void*)0x42BC9C;
        pfall_cfg_read_string = (void*)0x42BB88;
        pfall_cfg_write_int = (void*)0x42BDA0;

        pfall_set_data_path = (void*)0x4C4BB9;

        pfall_delete_files = (void*)0x47F4B4;

        pfall_file_list_get = (void*)0x4C5CCE;
        pfall_file_list_release = (void*)0x4C5F32;

    }
    else {
        pfall_fopen = (void*)FixAddress(0x4DEE2C);
        pfall_fclose = (void*)FixAddress(0x4DED6C);
        pfall_fseek = (void*)FixAddress(0x4DF5D8);
        pfall_ftell = (void*)FixAddress(0x4DF690);

        pfall_fread = (void*)FixAddress(0x4DF44C);
        pfall_fgetc = (void*)FixAddress(0x4DF22C);
        pfall_fgets = (void*)FixAddress(0x4DF280);
        pfall_fwrite = (void*)FixAddress(0x4DF4E8);
        pfall_fputc = (void*)FixAddress(0x4DF320);

        p_fallout_cfg_file_ptr = (void*)FixAddress(0x58E950);
        p_fallout_cfg_file_name = (char*)FixAddress(0x58E978);

        pfall_cfg_read_int = (void*)FixAddress(0x42C05C);
        pfall_cfg_read_string = (void*)FixAddress(0x42BF48);
        pfall_cfg_write_int = (void*)FixAddress(0x42C160);
 
        pfall_set_data_path = (void*)FixAddress(0x4C5D30);

        pfall_delete_files = (void*)FixAddress(0x480040);

        pfall_file_list_get = (void*)FixAddress(0x4C6628);
        pfall_file_list_release = (void*)FixAddress(0x4C6868);

    }

}








