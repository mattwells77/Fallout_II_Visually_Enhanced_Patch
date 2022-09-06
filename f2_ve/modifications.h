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

#define IFACE_BAR_HEIGHT 99

extern DWORD SCR_WIDTH;
extern DWORD SCR_HEIGHT;

extern UINT IFACE_BAR_MODE;

extern LONG* p_winRef_barter_for_inv;

extern DWORD* pPalColour_ConsoleGreen;
extern DWORD* pPalColour_ConsoleDarkGreen;
extern DWORD* pPalColour_ConsoleRed;
extern DWORD* pPalColour_ConsoleGrey;
extern DWORD* pPalColour_ConsoleDarkGrey;
extern DWORD* pPalColour_ConsoleYellow;
extern DWORD* pPalColour_DarkYellow;
extern DWORD* pPalColour_LightGrey;
extern DWORD* pPalColour_Mustard;

void Modifications_Dx_Game();
void Modifications_Dx_General();
void Modifications_Dx_Window();
void Modifications_Dx_Movies();
void Modifications_Dx_Mouse();

bool IsSplash();

bool IsMainMenu();
void MainMenu_SetToExit();

void imonitorInsertText(const char *msg);

bool ResizeGameWin();

void ReSizeCredits();
void ResizePauseWin();

//Save the load/save map picture to file.
void LS_Save_Picture_To_File(const char* p_save_dat_path);

void GetMousePosOnGamePortal(LONG *pXPos, LONG *pYPos);
void GetMousePosOnGameMap(LONG* pXPos, LONG* pYPos);

bool Mouse_Wheel_Inventory(int zDelta, int* p_keyCode, int* p_pageSize);
bool Mouse_Wheel_Imonitor(int zDelta, bool scrollPage);
bool Mouse_Wheel_GameWindow(int zDelta, bool setTrans);
bool Mouse_Wheel_WorldMap(int zDelta);

//resets the zoom level to within map boundaries when changing levels or resizing the game window.
void ResetZoomLevel();

void Modifications_Character();
void Modifications_Credits();
void Modifications_Dialogue();
void Modifications_Inventory();
void Modifications_Options();
void Modifications_EndSlides();
void Modifications_WorldMap();
void Modifications_Death();
void Modifications_Movie();
void Modifications_LoadSave();
void Modifications_MainMenu();
void Modifications_Interface_Bar();
void Modifications_Pipboy();
void Modifications_Other();
void Modifications_Splash();
void Modifications_Game_Map();
void Modifications_Help();

void Modifications_Win();
void Modifications_Pause();

extern LONG(*fall_NotifyBar_Enable)();
extern LONG(*fall_NotifyBar_Disable)();
