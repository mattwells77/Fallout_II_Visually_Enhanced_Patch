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

#include "pch.h"

struct GLYPH_FON {
    DWORD width;		// glyph width in points - 1 point is 1 bit in size
    DWORD offset;		// offset in buffer for glyph image data
};

struct FONT_FON {
    DWORD numGlyps;         // number of Glyphs
    DWORD height; 		// glyph height in points
    DWORD hGap; 		// horizontal gap distance between glyphs
    GLYPH_FON* glyph;    // glyph array - size of  FONGlyph * num
    BYTE* buffer;		// glyph image data    each glyph size is   (width+7)/8*height  or   ceil(width/8)*height
};

#pragma pack(2)
struct GLYPH_AFF {
    WORD width;
    WORD height;
    DWORD offset;
};

struct FONT_AFF {
    WORD maxGlyphHeight;
    WORD hGap;
    WORD spaceWidth;
    WORD vGap;
    WORD maxGlyphWidth;
    WORD unused;
    GLYPH_AFF glyph[256];
    BYTE* buffer;
};
#pragma pack()


void fall_SetFont(LONG ref);
LONG fall_GetFont();

void fall_PrintText(BYTE *toBuff, const char *txtBuff, DWORD txtWidth, DWORD toWidth, BYTE palColour);
DWORD fall_GetTextHeight();
DWORD fall_GetTextWidth(const char *TextMsg);
DWORD fall_GetCharWidth(const char CharVal);
DWORD fall_GetMaxTextWidth(const char *TextMsg);
DWORD fall_GetCharGapWidth();
DWORD fall_GetTextBoxSize(const char *TextMsg);
DWORD fall_GetMaxCharWidth();

void fall_DrawTextOutline(BYTE *txtBackBuff, LONG width1, LONG height, LONG width2, DWORD Colour);


FONT_FON* GetCurrentFont_Fon();
FONT_AFF* GetCurrentFont_Aff();

void Fallout_Functions_Setup_Text();



