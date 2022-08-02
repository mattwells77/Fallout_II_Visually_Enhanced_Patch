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

//mouse button flags
#define FLG_MouseL_Press 0x1
#define FLG_MouseL_Hold  0x4
#define FLG_MouseR_Press 0x2
#define FLG_MouseR_Hold  0x8

extern DWORD* pMOUSE_FLAGS;
extern DWORD* pGAME_EXIT_FLAGS;

//palette ptrs
extern BYTE* pWHITE_PAL;
extern BYTE* pBLACK_PAL;
extern BYTE* pLOADED_PAL;
extern BYTE* pCURRENT_PAL;
extern BYTE* pACTIVE_PAL;

extern LONG* p_PC_party_size;

bool IsExitGame();

void Fallout_Functions_Setup_General();

LONG fall_PlayAcm(const char*sound_name);

LONG fall_MessageBox(const char*text1, const char*text2, LONG text2Flag, LONG xPos, LONG yPos, DWORD text1Colour, DWORD unknown, DWORD text2Colour, DWORD flags);


BOOL IsMouseHidden();
DWORD GetMouseFlags();
DWORD GetMouseFlags_BACK();

void fall_Mouse_GetPos(LONG *xPtr, LONG *yPtr);
void fall_Mouse_SetPos(LONG xPos, LONG yPos);
void fall_Mouse_GetRect(RECT* rcMouse);
LONG fall_Mouse_IsInArea(long left, long top, long right, long bottom);
LONG fall_Mouse_SetFrm(DWORD frmID);
LONG fall_Mouse_SetBuff(BYTE* fBuff, LONG subWidth, LONG subHeight, LONG fWidth, LONG fX, LONG fY, DWORD maskColour);
LONG fall_Mouse_SetImage(LONG imageNum);
void fall_Mouse_SetImageFromList(LONG listNum);


void* fall_Mem_Allocate(DWORD sizeBytes);
void* fall_Mem_Reallocate(void* mem, DWORD sizeBytes);
void fall_Mem_Deallocate( void* mem);

LONG fall_Palette_Load(const char *FileName);
void fall_Palette_Set(BYTE *pal);
void fall_Palette_FadeTo(BYTE *pal);
void fall_Palette_SetActive(BYTE* pal);

void fall_SendKey(DWORD keyCode);


void fall_Event_Add(void (*func)());
void fall_Event_Remove(void (*func)());

void fall_EventProcessing_Enable();
void fall_EventProcessing_Disable();

void fall_GameMouse_Disable(DWORD flag);

LONG fall_Speech_Load(char* pFileName_noEXT, DWORD flag1, DWORD flag2, DWORD flag3);
void fall_Speech_Set_Callback_Function(void(*pFunc)());

LONG fall_TEXT_Divide_By_Width(const char* txt, LONG widthInPixels, SHORT* pLineOffsets, SHORT* pNumLines);



LONG fall_Set_Background_Sound(const char* pFileName_noEXT, DWORD flag1);
LONG fall_Set_Background_Sound(const char* pFileName_noEXT, DWORD flag1, DWORD flag2, DWORD flag3);


void fall_GameTime_Get_Date(LONG* p_month, LONG* p_day, LONG* p_year);

void button_sound_Dn_1();
void button_sound_Up_1();

void button_sound_Dn_2();
void button_sound_Up_2();

void button_sound_Dn_3();
void button_sound_Up_3();


extern void(*fall_Mouse_Show)();
extern void(*fall_Mouse_Hide)();
extern void(*fall_Mouse_ToggleHex)();

extern void(*fall_Sound_Continue_ALL)();

//not sure what these functions do, but are called in original death screen function.
extern void(*fall_Input_Related_01)();
extern void(*fall_Input_Related_02)();

extern LONG(*fall_FreeArtCache)();

extern void(*fall_Speech_Close)();
extern void(*fall_Speech_Play)();

extern int (*fall_Debug_printf)(const char* format, ...);

extern LONG(*fall_ExitMessageBox)();


extern void (*fall_EnableGameEvents)();
extern LONG(*fall_DisableGameEvents)();


extern LONG(*fall_Get_Input)();



extern LONG(*fall_GameTime_Get_Time)();



extern void(*fall_Process_Input)();
extern void(*fall_Process_Map_Mouse)();
extern void(*fall_Update_Mouse_State)();
