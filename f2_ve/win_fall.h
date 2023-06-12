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


struct POINTERstate {
    LONG x;
    LONG y;
    WORD flags;
};
extern POINTERstate pointerState;

extern HWND* phWinMain;
extern HWND hGameWnd;

extern LONG scaleLevel_GUI;
extern LONG scaleLevel_Game;
extern float scaleGame_RO;

extern float scaleSubUnit;

//extern BOOL *p_is_winActive;
extern RECT *pFALL_RC;

extern bool isGameMode;
extern bool isAltMouseInput;
extern bool isMapperExiting;
//extern bool isMapperSelecting;
extern bool isMapperScrolling;

extern LONG SFALL_UseScrollWheel;

extern bool isMapperSizing;

extern bool isGrayScale;


extern LONG SFALL_UseScrollWheel;
extern LONG SFALL_MiddleMouse;

#define WIN_MODE_STYLE  WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
extern DWORD winStyle;


BOOL Is_WindowActive();
void Set_WindowActive_State(BOOL isActive);

void WindowVars_Save();
void WindowVars_Load();
void SetWindowTitle(HWND hwnd, const wchar_t *msg);

int CheckMessages();
int CheckMessagesNoWait();

void Wait_ms(DWORD delay_ms);

void GetMousePos(LONG *pXPos, LONG *pYPos);
LONG GetWinAtPos(LONG xPos, LONG yPos);
void SetMousePosGame(LONG xPos, LONG yPos);

BOOL IsMouseInRect(RECT* p_rc);
BOOL IsMouseInRect(LONG left, LONG top, LONG right, LONG bottom);
BOOL IsMouseInWindowRect2(LONG winRef, RECT* p_rc);
BOOL IsMouseInWindowRect(LONG winRef, LONG left, LONG top, LONG right, LONG bottom);
BOOL IsMouseInButtonRect(int buttRef);

bool IsMouseDoubleClick();

void Fallout_On_Exit();

void Set_Fallout_Screen_Dimensions(DWORD width, DWORD height);
void SetWindowActivation(BOOL isActive);
void ClipAltMouseCursor();
int CheckClientRect();

void fall_WinExit(LONG val);
