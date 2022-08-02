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

struct ButtonStruct {
	LONG ref;//0x00
	DWORD flags;//0x04
	RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	LONG refHvOn;//0x18
	LONG refHvOff;//0x1C
	LONG refDn;//0x20
	LONG refUp;//0x24
	LONG refDnRht;//-1//0x28 right mouse button
	LONG refUpRht;//-1//0x2C right mouse button
	BYTE* buffUp;//0x30 up image buff ptr
	BYTE* buffDn;//0x34 down image buff ptr
	BYTE* buffHv;//0//0x38 hover image buff ptr
	BYTE* buffUpDis;//0x3C upDisabled image buff ptr
	BYTE* buffDnDis;//0x40 downDisabled image buff ptr
	BYTE* buffHvDis;//0x44 hoverDisabled image buff ptr
	BYTE* buffCurrent;//0x48 current image buff ptr
	BYTE* buffDefault;//0x4C default image buff ptr
	void* funcHvOn;//0x50 hover on func ptr
	void* funcHvOff;//0x54 hover off func ptr
	void* funcDn;//0x58 down func ptr
	void* funcUp;//0x5C up func ptr
	void* funcDnRht;//0x60 right mouse button func ptr
	void* funcUpRht;//0x64 right mouse button func ptr
	void* funcDnSnd;//0x68 push sound func
	void* funcUpSnd;//0x6C lift sound func
	DWORD unknown70;//0x70  related to toggle array
	ButtonStruct* nextButton;//0x74
	ButtonStruct* prevButton;//0x78
};

#define FLG_ButtToggle      0x00000001
#define FLG_ButtHoldOn      0x00000002  //button pops up if not held.
#define FLG_ButtTglOnce     0x00000004  //this + toggle == when toggle set button remains down permanently once pressed;
#define FLG_ButtDisabled    0x00000008  //button disabled?
#define FLG_ButtDragWin     0x00000010  //drag owner window position
#define FLG_ButtTrans       0x00000020  //palette pos 0 is tansparent
#define FLG_ButtReturnMsg   0x00000040  //return button message even if custom fuctions used.
#define FLG_ButtPics        0x00010000  //custom pics, buffers must be destroyed manually - automaticaly set by create pic button func.
#define FLG_ButtTglDn       0x00020000  //Button starts on, for use with toggle, will be stuck down in normal mode.
#define FLG_ButtRgtClickOn  0x00080000  //one or both of the right mouse click ref's are something other than -1 or one or both of their associated function pointers are not nullptr.

class WinStruct {
public:
	LONG ref;//0x00
	DWORD flags;//0x04
	RECT rect;//left 0x08, top 0x0C, right 0x10, bottom 0x14
	LONG width;//0x18
	LONG height;//0x1C
	DWORD colour;//0x20 backgroung colour index
	DWORD unknown24;//0x24 x?
	DWORD unknown28;//0x28 y?
	BYTE* buff;//0x2C image buffer
	ButtonStruct* ButtonList;//0x30 buttons
	DWORD unknown34;//0x34
	DWORD unknown38;//0x38
	DWORD unknown3C;//0x3C
	void (*pBlit)(BYTE* fromBuff, LONG subWidth, LONG subHeight, LONG fromWidth, BYTE* toBuff, DWORD toWidth);//0x40 drawing func address
};



#define FLG_WinToBack		0x2
#define FLG_WinToFront		0x4
#define FLG_WinHidden		0x8
#define FLG_WinExclusive	0x10
#define FLG_WinTrans		0x20
#define FLG_WinScript		0x100



extern LONG* pWinRef_Char;
extern LONG* pWinRef_Bio;
extern LONG* pWinRef_DialogMain;
extern LONG* pWinRef_DialogNpcText;
extern LONG* pWinRef_DialogPcText;
extern LONG* pWinRef_DialogBaseSub;
extern LONG* pWinRef_Inventory;
extern LONG* pWinRef_EndSlides;
extern LONG* pWinRef_Iface;
extern LONG* pWinRef_NotifyBar;
extern LONG* pWinRef_Skills;
extern LONG* pWinRef_LoadSave;
extern LONG* pWinRef_MainMenu;
extern LONG* pWinRef_Movies;
extern LONG* pWinRef_Options;
extern LONG* pWinRef_Pipboy;
extern LONG* pWinRef_GameArea;
extern LONG* pWinRef_WorldMap;

extern int winRef_Splash;


extern LONG* p_draw_window_flag;
extern LONG* p_num_windows;
extern WinStruct** pWin_Array;
extern LONG* p_winRef_Index;

void Fallout_Functions_Setup_Windows();


LONG fall_Win_Create(LONG x, LONG y, DWORD width, DWORD height, DWORD BGColourIndex, DWORD flags);
void fall_Win_Destroy(LONG WinRef);
WinStruct* fall_Win_Get(LONG WinRef);
BYTE* fall_Win_Get_Buff(LONG WinRef);
void fall_Win_Show(LONG WinRef);
void fall_Win_Hide(LONG WinRef);
void fall_Win_ReDraw(LONG WinRef);
void fall_Win_PrintText(LONG WinRef, const char* txtBuff, LONG txtWidth, LONG x, LONG y, DWORD palColourFlags);
void fall_Windows_Draw_Rect(RECT* rect);
void fall_Windows_Draw_Win(WinStruct* win, RECT* rect, BYTE* buff);

LONG fall_Button_Create(LONG winRef, LONG x, LONG y, LONG width, LONG height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, BYTE* upPicBuff, BYTE* downPicBuff, BYTE* hoverPicBuff, DWORD flags);
LONG fall_Button_Destroy(LONG buttRef);
ButtonStruct* fall_Button_Get(LONG buttRef, WinStruct** retWin);
LONG fall_Button_SetFunctions(LONG buttonRef, void* hoverOnfunc, void* hoverOffFunc, void* pushfunc, void* liftfunc);
LONG fall_Button_SetSounds(LONG buttonRef, void* soundFunc1, void* soundFunc2);
LONG fall_Button_SetToggleArray(LONG numButtons, LONG* pButtonRefs);
//state 0/1 = up/down, flags & 1 send key, flags & 2 = don't draw button.
LONG fall_Button_SetToggleState(LONG buttRef, LONG state, DWORD flags);
LONG fall_Button_Enable(LONG buttonRef);
LONG fall_Button_Disable(LONG buttonRef);
void fall_Button_Draw(WinStruct* p_win, RECT* p_rect);

LONG fall_Win_GetWinAtPos(LONG x, LONG y);




