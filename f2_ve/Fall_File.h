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

void Fallout_Functions_Setup_File();

bool FalloutData_Does_File_Exist(const char* path);

int fall_SetDataPath(const char *path1, int isFolder1, const char *path2, int isFolder2);

int fall_Cfg_Read_Int(void *FileStream, const char *secName, const char *keyName, int *pIntVal);
int fall_Cfg_Read_String(void *FileStream, const char *secName, const char *keyName, char **ppString);
int fall_Cfg_Write_Int(void *FileStream, const char *secName, const char *keyName, int IntVal);

int FalloutCfg_Read_Int(const char* secName, const char* keyName, int* pIntVal);
int FalloutCfg_Write_Int(const char* secName, const char* keyName, int IntVal);
int FalloutCfg_Read_String(const char* secName, const char* keyName, char** ppString);


int fall_Files_Delete(const char* path, const char* ext);

int fall_FileList_Get(const char* path, char*** list);
int fall_FileList_Release(char*** list);



void* fall_fopen(const char *FileName, const char *flags);
int fall_fclose(void *FileStream);

int fall_fseek(void *FileStream, LONG fOffset, LONG origin);
LONG fall_ftell(void* FileStream);

int fall_fgetc(void* FileStream);
int fall_fread(void* buffer, size_t size, size_t num, void* FileStream);
int fall_fputc(int ch, void* FileStream);
int fall_fwrite(void* buffer, size_t size, size_t num, void* FileStream);

int fall_fread8(void* FileStream, BYTE* byte);
int fall_fwrite8(void* FileStream, BYTE byte);
//little endian
int fall_fread16_LE(void* FileStream, WORD* word);
int fall_fread32_LE(void* FileStream, DWORD* dword);

int fall_fwrite16_LE(void* FileStream, WORD word);
int fall_fwrite32_LE(void* FileStream, DWORD dword);
//big endian
int fall_fread16_BE(void* FileStream, WORD* word);
int fall_fread32_BE(void* FileStream, DWORD* dword);
int fall_fread16_Array_BE(void* FileStream, WORD* words, size_t num);
int fall_fread32_Array_BE(void* FileStream, DWORD* dwords, size_t num);

int fall_fwrite16_BE(void* FileStream, WORD word);
int fall_fwrite32_BE(void* FileStream, DWORD dword);
int fall_fwrite16_Array_BE(void* FileStream, WORD* words, size_t num);
int fall_fwrite32_Array_BE(void* FileStream, DWORD* dwords, size_t num);


