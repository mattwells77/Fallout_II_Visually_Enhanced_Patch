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

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define IDC_WIN_GAME            (WM_APP+1)
#define IDC_STATUS_BAR          (WM_APP+2)
#define IDC_REBAR               (WM_APP+3)
#define IDC_BROWSER			    (WM_APP+4)
#define ID_RB_BAND_001          (WM_APP+5)//put this last here, unknown number of toolbars toolbars


#define MAP_FILE_NAME_LENGTH 8
#define MAP_FILE_NAME_LENGTH_PLUS_EXT   (MAP_FILE_NAME_LENGTH + 4 + 1)



extern HWND hDlgCurrent;

extern wchar_t winClassName_MapperMain[];
extern wchar_t windowName_Mapper[];

extern bool isRunAsMapper;
extern bool isMapperInitiated;

ATOM MapperMain_RegisterClass();
void Mapper_On_Exit();

LRESULT CALLBACK WinProc_Mapper(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void Mapper_UpdateScrollBarPositions();

bool MapperData_OpenFile(FILE** ppFile, const wchar_t* _FileName, DWORD fileNameLength, const wchar_t* _Mode);
bool MapperData_OpenFileStream(const wchar_t* _FileName, DWORD fileNameLength, DWORD grfMode, BOOL fCreate, IStream** pStream);


void Mapper_Windows_Refresh_Size(HWND hwnd);

bool WinData_Save(HWND hwnd_win, const wchar_t* _FileName, DWORD fileNameLength);
bool WinData_Load(HWND hwnd_win, const wchar_t* _FileName, DWORD fileNameLength);

void Edges_SetActiveVersion(DWORD version);
DWORD Edges_GetActiveVersion();

BOOL StatusBar_SetText(WORD position, const wchar_t* _msg);
void StatusBar_Print_Map_Vars();
void StatusBar_Print_Map_Script_Name();

LONG Get_Proto_Type_TextID(LONG type);
LONG Get_Item_Type_TextID(LONG type);
