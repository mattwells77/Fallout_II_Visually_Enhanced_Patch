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

#define FLG_TEXT_WORDWRAP   0x00000001
#define FLG_TEXT_CENTRED    0x00000002
#define FLG_TEXT_INDENT     0x00000004
#define FLG_TEXT_SLOPED_L   0x00000008 //slope in from left, slope value stored if flags 4th byte (<<24)
#define FLG_TEXT_SLOPED_R   0x00000010 //slope in fron right, slope value stored if flags 4th byte (<<24). both slopes can be used at the same time.
#define FLG_TEXT_ADD_LINE_BREAKS   0x00000020 //format the text by inserting new_line charaters at the end of lines.

//0x00000000
enum class TextEffects {
    none,
    outlined,
    dropShadow
    //dropShadow_LD,
    //dropShadow_RD,
    //dropShadow_LU,
    //dropShadow_RU
};






struct FONT_FUNC_STRUCT {
    LONG fontNum_Min;
    LONG fontNum_Max;
    void (*SetFontAddress)(int);
    DWORD(*PrintText)(BYTE*, const char*, DWORD, DWORD);
    DWORD(*GetFontHeight)();
    DWORD(*GetTextWidth)(const char*);
    DWORD(*GetCharWidth)(char);
    DWORD(*GetMaxTextWidth)(const char*);
    DWORD(*GetHorizontalGapWidth)();
    DWORD(*GetTextBufferSize)(const char*);
    DWORD(*GetMaxCharWidth)();
    DWORD(*GetTextWidth2)(const char*, DWORD maxWidth, LONG* pRet_NumChars);
    DWORD(*PrintText_Formated)(BYTE* dest_buff, const char* txt_msg, DWORD dest_width, DWORD dest_height, RECT* pMargins, DWORD flags);//Returns the number of characters processed, the height of the drawn text is returned to pMargins->top. Setting dest_buff no NULL will allow you to get the height without drawing anything.
};

//For loading GVX's font library for the Chinese version of Fallout2.
class GVX {
public:
    //For loading GVX's font library for the Chinese version of Fallout2.
    void LoadFontLib(HWND hWin);
};
//For loading GVX's font library for the Chinese version of Fallout2.
extern GVX* pGvx;


FONT_FUNC_STRUCT* GetCurrentFont();
void SetCurrentFont(int fontNum);

void Modifications_Text();