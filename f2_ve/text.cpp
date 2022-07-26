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
#include "text.h"
#include "memwrite.h"

#include "Fall_Text.h"


//For loading GVX's font library for the Chinese version of Fallout2.
GVX* pGvx = nullptr;
//______________________________
void GVX::LoadFontLib(HWND hWin) {

    __asm {
        pushad
        mov esi, 0x4CAF22
        mov eax, hWin
        push retAddr
        push ebx
        push ecx
        jmp esi
        retAddr :
        popad
    }

}


////////////////---FON FONTS---///////////////////


//_____________________________________________________
DWORD GetCurrentFont_TextWidth_Fon(const char* txt_msg) {
    FONT_FON* fon = GetCurrentFont_Fon();
    unsigned char* pChar = (unsigned char*)txt_msg;
    DWORD txtWidth = 0;
    DWORD currentCharWidth = 0;
    while (*pChar != '\0') {
        currentCharWidth = fon->glyph[*pChar].width;
        txtWidth += currentCharWidth;
        pChar++;
        if (*pChar != '\0')
            txtWidth += fon->hGap;

    }
    return txtWidth;
}


//_______________________________
DWORD GetCurrentFont_Height_Fon() {
    FONT_FON* fon = GetCurrentFont_Fon();
    return fon->height;
}


//______________________________________________
DWORD GetCurrentFont_CharWidth_Fon(char charVal) {
    FONT_FON* fon = GetCurrentFont_Fon();
    return fon->glyph[(unsigned char)charVal].width;
}


//_____________________________________
DWORD GetCurrentFont_CharGapWidth_Fon() {
    FONT_FON* fon = GetCurrentFont_Fon();
    return fon->hGap;
}


//______________________________________
DWORD GetCurrentFont_CharWidth_Max_Fon() {
    FONT_FON* fon = GetCurrentFont_Fon();
    DWORD count = 0;
    DWORD maxWidth = 0;
    while (count < fon->numGlyps) {
        if (maxWidth < fon->glyph[count].width)
            maxWidth = fon->glyph[count].width;
        count++;
    }
    return maxWidth + fon->hGap;
}


//________________________________________________________
DWORD GetCurrentFont_TextWidthMax_Fon(const char* txt_msg) {
    FONT_FON* fon = GetCurrentFont_Fon();
    DWORD count = 0;
    while (txt_msg[count] != '\0')
        count++;
    return count * GetCurrentFont_CharWidth_Max_Fon();
}


//__________________________________________________________
DWORD GetCurrentFont_TextBufferSize_Fon(const char* txt_msg) {
    FONT_FON* fon = GetCurrentFont_Fon();
    return GetCurrentFont_TextWidth_Fon(txt_msg) * fon->height;
}


//___________________________________________________________________________________
DWORD PrintTextFon_8(BYTE* toBuff, const char* txtMsg, DWORD toWidth, DWORD TxtWidth) {

    FONT_FON* fon = GetCurrentFont_Fon();
    DWORD charCount = 0;
    unsigned char* pChar = (unsigned char*)txtMsg;
    BYTE* gl_buff = 0;
    DWORD to_xStart = 0;//start x position in to-buffer to draw glyph

    while (*pChar != '\0' && *pChar != '\n') {
        //exit loop if no room for next char
        if (*pChar != '\0' && to_xStart + fon->glyph[*pChar].width > TxtWidth)
            return charCount;
        gl_buff = fon->buffer + fon->glyph[*pChar].offset;//set position of glyph buffer for current char
        BYTE* to_buff = toBuff + to_xStart;//set offset in to-buffer to draw next line of glyph data
        DWORD pitch_bytes = (fon->glyph[*pChar].width + 7) / 8;//pitch width in bytes - glyph pixel values are in bits but stored in bytes, pitch_bytes is the total width in bytes containing the glyph x data

        for (DWORD h = 0; h < fon->height; h++) {
            DWORD pbitPos = 0;//bit pos in current byte of pitch width
            DWORD xbitPos = 0;//x bit pos within glyph width
            for (DWORD pByte = 0; pByte < pitch_bytes; pByte++) {
                while (xbitPos < fon->glyph[*pChar].width && pbitPos < 8) {
                    to_buff[xbitPos] = -((gl_buff[pByte] >> (7 - pbitPos)) & 0x01);//shift to the next bit(pixel Value) in the current byte of line data and mask, set pixel to 255 if equal to 1 or 0 by setting result negative;
                    xbitPos++;
                    pbitPos++;
                }
                pbitPos = 0;
            }
            gl_buff += pitch_bytes;// go to next line in glyph buffer
            to_buff += toWidth;// go to next line in to-buffer
        }
        //move start x position in to-buffer for next char
        to_xStart += fon->glyph[*pChar].width + fon->hGap;
        charCount++;
        pChar++;

    }
    return charCount;//number of chars drawn
}


//__________________________________________________________________________________________
DWORD GetCurrentFont_TextWidth_Fon(const char* txt_msg, DWORD maxWidth, LONG* pRet_NumChars) {
    FONT_FON* fon = GetCurrentFont_Fon();
    unsigned char* pChar = (unsigned char*)txt_msg;
    DWORD txtWidth = 0;
    DWORD currentCharWidth = 0;

    LONG count = 0;
    LONG lastSpace = -1;
    DWORD txtWidth_lastSpace = 0;
    DWORD txtWidth_lastGood = 0;

    while (*pChar != '\0' && *pChar != '\n') {
        if (*pChar == ' ') {
            lastSpace = count;
            txtWidth_lastSpace = txtWidth;
        }
        currentCharWidth = fon->glyph[*pChar].width;
        txtWidth += currentCharWidth;

        if (txtWidth <= maxWidth) {
            count++;
            txtWidth_lastGood = txtWidth;
        }
        else {
            if (lastSpace != -1) {
                *pRet_NumChars = lastSpace;
                return txtWidth_lastSpace;
            }
            *pRet_NumChars = count;
            return txtWidth_lastGood;
        }
        pChar++;

        if (*pChar != '\0' && *pChar != '\n')
            txtWidth += fon->hGap;

    }
    *pRet_NumChars = count;
    return txtWidth;
}


//___________________________________________________________________________________________________________________________________
DWORD PrintTextFon_8_Formated(BYTE* dest_buff, const char* txt_msg, DWORD dest_width, DWORD dest_height, RECT* pMargins, DWORD flags) {
    if (!pMargins)
        return 0;
    if (pMargins->left < 0)
        pMargins->left = 0;
    if (pMargins->top < 0)
        pMargins->top = 0;
    if (pMargins->right > (LONG)dest_width)
        pMargins->right = (LONG)dest_width;
    if (pMargins->bottom > (LONG)dest_height)
        pMargins->bottom = (LONG)dest_height;

    BYTE slope = 0;
    if (flags & (FLG_TEXT_SLOPED_L | FLG_TEXT_SLOPED_R))
        slope = flags >> 24;

    LONG txt_width = pMargins->right - pMargins->left;

    FONT_FON* fon = GetCurrentFont_Fon();
    if (fon == nullptr)
        return 0;
    if (fon->height > dest_height - pMargins->top) {
        if (dest_buff)//Only return if dest_buff is valid, so the needed buffer height can be returned on pMargins->top.
            return 0;
    }
    int vGap = fon->height / 4;
    int charCount = 0;
    unsigned char* pChar = (unsigned char*)txt_msg;

    DWORD d_height = (DWORD)pMargins->top;

    BYTE* d_pos = dest_buff;// pixel position in destination-buffer


    DWORD lineWidth = 0;
    LONG lineNumChars = 0;
    LONG xPos = pMargins->left;

    LONG indent = 0;
    if (flags & FLG_TEXT_INDENT)
        indent = fon->glyph[' '].width;

    while (*pChar != '\0') {
        lineWidth = GetCurrentFont_TextWidth_Fon((char*)pChar, txt_width, &lineNumChars);
        if (dest_buff) {
            if (flags & FLG_TEXT_CENTRED) {
                xPos = pMargins->left + (txt_width - (LONG)lineWidth) / 2;
                if (xPos < pMargins->left)
                    xPos = pMargins->left;
            }
            d_pos = dest_buff + d_height * dest_width + xPos + indent;
            indent = 0;
            PrintTextFon_8(d_pos, (char*)pChar, dest_width, lineWidth);
        }
        charCount += lineNumChars;
        pChar += lineNumChars;

        if (flags & FLG_TEXT_ADD_LINE_BREAKS) {
            if (*pChar == ' ')
                *pChar = '\n';
        }

        if (*pChar == '\n') {//skip the newline char if it was the cause of the last line break.
            charCount++;
            pChar++;
        }
        while (*pChar == ' ') {//skip any space chars before drawing next line of text.
            charCount++;
            pChar++;
        }
        d_height += fon->height;

        //set pMargins->top to the height of the last processed line.
        pMargins->top = d_height;

        d_height += vGap;

        if (dest_buff && *pChar != '\0' && d_height + fon->height + vGap > (DWORD)pMargins->bottom)
            return charCount;

        if (flags & (FLG_TEXT_SLOPED_L | FLG_TEXT_SLOPED_R)) {
            txt_width -= slope;
            if (flags & FLG_TEXT_SLOPED_L) {
                xPos += slope;
                if (flags & FLG_TEXT_SLOPED_R)
                    txt_width -= slope;
            }
        }
    }

    return charCount;//number of chars processed
}


////////////////---AFF FONTS---///////////////////


//_____________________________________________________
DWORD GetCurrentFont_TextWidth_Aff(const char* txt_msg) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    unsigned char* pChar = (unsigned char*)txt_msg;
    DWORD txtWidth = 0;
    DWORD currentCharWidth = 0;
    while (*pChar != '\0') {
        if (*pChar == ' ')
            currentCharWidth = aff->spaceWidth;
        else
            currentCharWidth = aff->glyph[*pChar].width;
        txtWidth += currentCharWidth;
        pChar++;
        if (*pChar != '\0')
            txtWidth += aff->hGap;

    }
    return txtWidth;
}


//_______________________________
DWORD GetCurrentFont_Height_Aff() {
    FONT_AFF* aff = GetCurrentFont_Aff();
    return aff->maxGlyphHeight + aff->vGap;
}


//______________________________________________
DWORD GetCurrentFont_CharWidth_Aff(char charVal) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    if (charVal == ' ')
        return aff->spaceWidth;
    return aff->glyph[(unsigned char)charVal].width;
}


//_____________________________________
DWORD GetCurrentFont_CharGapWidth_Aff() {
    FONT_AFF* aff = GetCurrentFont_Aff();
    return aff->hGap;
}


//______________________________________
DWORD GetCurrentFont_CharWidth_Max_Aff() {
    FONT_AFF* aff = GetCurrentFont_Aff();
    DWORD maxWidth = aff->maxGlyphWidth;
    if (maxWidth < aff->spaceWidth)
        maxWidth = aff->spaceWidth;
    return maxWidth + aff->hGap;
}


//________________________________________________________
DWORD GetCurrentFont_TextWidthMax_Aff(const char* txt_msg) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    DWORD count = 0;
    while (txt_msg[count] != '\0')
        count++;
    return count * GetCurrentFont_CharWidth_Max_Aff();
}


//__________________________________________________________
DWORD GetCurrentFont_TextBufferSize_Aff(const char* txt_msg) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    return GetCurrentFont_TextWidth_Aff(txt_msg) * aff->maxGlyphHeight;
}


//a guess at the alpha levels in aff fonts - the scale doesn't seem to be linear.
BYTE alphaLevel[10] = { 0,28,57,122,190,218,232,243,250,255 };

//___________________________________________________________________________________________
DWORD PrintTextAff_8(BYTE* dest_buff, const char* txt_msg, DWORD dest_width, DWORD txt_width) {

    FONT_AFF* aff = GetCurrentFont_Aff();
    if (aff == nullptr)
        return 0;

    int charCount = 0;
    unsigned char* pChar = (unsigned char*)txt_msg;

    DWORD g_width = 0;// holds current glyph width - no glyph for space char in aff fonts
    BYTE* g_buff = 0;// glyph buffer for the current char
    DWORD g_pos = 0;// current position in glyph buffer

    BYTE* d_pos = dest_buff;// pixel position in destination-buffer
    DWORD d_width_rem = 0;// remaining distance to the start of the next line in destination buffer
    DWORD d_xOffset = 0;// start x position in destination buffer for current char

    BYTE alpha = 0;

    while (*pChar != '\0' && *pChar != '\n') {
        //exit loop if no room for next char
        if (*pChar != '\0' && d_xOffset + aff->glyph[*pChar].width > txt_width)
            return charCount;

        if (*pChar == ' ')
            g_width = aff->spaceWidth;
        else {
            g_width = aff->glyph[*pChar].width;

            //get glyph buffer for current char
            g_buff = aff->buffer + aff->glyph[*pChar].offset;
            g_pos = 0;

            //set the start position for drawing current char in destination buffer
            d_pos = dest_buff + (aff->maxGlyphHeight - aff->glyph[*pChar].height) * dest_width + d_xOffset;
            d_width_rem = dest_width - aff->glyph[*pChar].width;

            for (DWORD h = 0; h < aff->glyph[*pChar].height; h++) {
                for (DWORD w = 0; w < aff->glyph[*pChar].width; w++) {
                    alpha = alphaLevel[g_buff[g_pos]];
                    *d_pos = alpha;
                    g_pos++;
                    d_pos++;
                }
                d_pos += d_width_rem;//add remaining distance to start of next line
            }
        }
        d_xOffset += (g_width + aff->hGap);//move x position in destination buffer for next char

        charCount++;
        pChar++;
    }

    return charCount;//number of chars drawn
}


//__________________________________________________________________________________________
DWORD GetCurrentFont_TextWidth_Aff(const char* txt_msg, DWORD maxWidth, LONG* pRet_NumChars) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    unsigned char* pChar = (unsigned char*)txt_msg;
    DWORD txtWidth = 0;
    DWORD currentCharWidth = 0;

    LONG count = 0;
    LONG lastSpace = -1;
    DWORD txtWidth_lastSpace = 0;
    DWORD txtWidth_lastGood = 0;

    while (*pChar != '\0' && *pChar != '\n') {
        if (*pChar == ' ') {
            lastSpace = count;
            txtWidth_lastSpace = txtWidth;
            currentCharWidth = aff->spaceWidth;
        }
        else
            currentCharWidth = aff->glyph[*pChar].width;
        txtWidth += currentCharWidth;

        if (txtWidth <= maxWidth) {
            count++;
            txtWidth_lastGood = txtWidth;
        }
        else {
            if (lastSpace != -1) {
                *pRet_NumChars = lastSpace;
                return txtWidth_lastSpace;
            }
            *pRet_NumChars = count;
            return txtWidth_lastGood;
        }
        pChar++;

        if (*pChar != '\0' && *pChar != '\n')
            txtWidth += aff->hGap;

    }
    *pRet_NumChars = count;
    return txtWidth;
}


//____________________________________________________________________________________________________
DWORD GetCurrentFont_TextWidth_Aff_MultiByte(const char* txt_msg, DWORD maxWidth, LONG* pRet_NumChars) {
    FONT_AFF* aff = GetCurrentFont_Aff();
    unsigned char* pChar = (unsigned char*)txt_msg;
    DWORD txtWidth = 0;
    DWORD currentCharWidth = 0;

    LONG count = 0;
    LONG lastSpace = -1;
    DWORD txtWidth_lastSpace = 0;
    DWORD txtWidth_lastGood = 0;

    while (*pChar != '\0' && *pChar != '\n') {
        if (*pChar == ' ') {
            lastSpace = count;
            txtWidth_lastSpace = txtWidth;
            currentCharWidth = aff->spaceWidth;
        }
        else
            currentCharWidth = aff->glyph[*pChar].width;
        txtWidth += currentCharWidth;

        if (txtWidth <= maxWidth) {
            count++;
            txtWidth_lastGood = txtWidth;
        }
        else {
            if (lastSpace != -1) {
                *pRet_NumChars = lastSpace;
                return txtWidth_lastSpace;
            }
            *pRet_NumChars = count;
            return txtWidth_lastGood;
        }

        if (*pChar > 160)//if multibyte char, exclude next byte from count.
            pChar += 2;
        else
            pChar++;

        if (*pChar != '\0' && *pChar != '\n')
            txtWidth += aff->hGap;

    }
    *pRet_NumChars = count;
    return txtWidth;
}



//The height of the drawn text is returned to pMargins->top, setting dest_buff no NULL will allow you to get the height without drawing anything.
//___________________________________________________________________________________________________________________________________
DWORD PrintTextAff_8_Formated(BYTE* dest_buff, const char* txt_msg, DWORD dest_width, DWORD dest_height, RECT* pMargins, DWORD flags) {
    if (!pMargins)
        return 0;
    if (pMargins->left < 0)
        pMargins->left = 0;
    if (pMargins->top < 0)
        pMargins->top = 0;
    if (pMargins->right > (LONG)dest_width)
        pMargins->right = (LONG)dest_width;
    if (pMargins->bottom > (LONG)dest_height)
        pMargins->bottom = (LONG)dest_height;


    BYTE slope = 0;
    if (flags & (FLG_TEXT_SLOPED_L | FLG_TEXT_SLOPED_R))
        slope = flags >> 24;

    LONG txt_width = pMargins->right - pMargins->left;

    FONT_AFF* aff = GetCurrentFont_Aff();
    if (aff == nullptr)
        return 0;
    if (aff->maxGlyphHeight > dest_height - pMargins->top) {
        if (dest_buff)//Only return if dest_buff is valid, so the needed buffer height can be returned on pMargins->top.
            return 0;
    }
    DWORD maxGapWidth;
    if (aff->maxGlyphWidth >= aff->spaceWidth)
        maxGapWidth = aff->maxGlyphWidth + aff->hGap;
    else maxGapWidth = aff->spaceWidth + aff->hGap;

    int charCount = 0;
    unsigned char* pChar = (unsigned char*)txt_msg;

    DWORD d_height = (DWORD)pMargins->top;

    BYTE* d_pos = dest_buff;// pixel position in destination-buffer

    LONG xPos = pMargins->left;

    LONG indent = 0;
    if (flags & FLG_TEXT_INDENT)
        indent = maxGapWidth;

    DWORD lineWidth = 0;
    LONG lineNumChars = 0;
    while (*pChar != '\0') {
        lineWidth = GetCurrentFont_TextWidth_Aff((char*)pChar, txt_width - indent, &lineNumChars);
        if (dest_buff) {
            if (flags & FLG_TEXT_CENTRED) {
                xPos = pMargins->left + (txt_width - (LONG)lineWidth) / 2;
                if (xPos < pMargins->left)
                    xPos = pMargins->left;
            }
            d_pos = dest_buff + d_height * dest_width + xPos + indent;
            indent = 0;
            PrintTextAff_8(d_pos, (char*)pChar, dest_width, lineWidth);
        }
        charCount += lineNumChars;
        pChar += lineNumChars;

        if (flags & FLG_TEXT_ADD_LINE_BREAKS) {
            if (*pChar == ' ')
                *pChar = '\n';
        }
        if (*pChar == '\n') {//skip the newline char if it was the cause of the last line break.
            charCount++;
            pChar++;
        }
        while (*pChar == ' ') {//skip any space chars before drawing next line of text.
            charCount++;
            pChar++;
        }
        d_height += aff->maxGlyphHeight;

        //set pMargins->top to the height of the last processed line.
        pMargins->top = d_height;

        d_height += aff->vGap;
        if (dest_buff && *pChar != '\0' && d_height + aff->maxGlyphHeight > (DWORD)pMargins->bottom)
            return charCount;

        if (flags & (FLG_TEXT_SLOPED_L | FLG_TEXT_SLOPED_R)) {
            txt_width -= slope;
            if (flags & FLG_TEXT_SLOPED_L) {
                xPos += slope;
                if (flags & FLG_TEXT_SLOPED_R)
                    txt_width -= slope;
            }
        }
    }
    return charCount;//number of chars processed
}


FONT_FUNC_STRUCT FontFuncAff = { 100, 110, nullptr, &PrintTextAff_8, &GetCurrentFont_Height_Aff, &GetCurrentFont_TextWidth_Aff, &GetCurrentFont_CharWidth_Aff, &GetCurrentFont_TextWidthMax_Aff, &GetCurrentFont_CharGapWidth_Aff, &GetCurrentFont_TextBufferSize_Aff, &GetCurrentFont_CharWidth_Max_Aff, &GetCurrentFont_TextWidth_Aff, &PrintTextAff_8_Formated };
FONT_FUNC_STRUCT FontFuncFon = { 0, 9, nullptr, &PrintTextFon_8, &GetCurrentFont_Height_Fon, &GetCurrentFont_TextWidth_Fon, &GetCurrentFont_CharWidth_Fon, &GetCurrentFont_TextWidthMax_Fon, &GetCurrentFont_CharGapWidth_Fon, &GetCurrentFont_TextBufferSize_Fon, &GetCurrentFont_CharWidth_Max_Fon, &GetCurrentFont_TextWidth_Fon, &PrintTextFon_8_Formated };

FONT_FUNC_STRUCT* SetFontFunctions = nullptr;
//________________________________
FONT_FUNC_STRUCT* GetCurrentFont() {
    return SetFontFunctions;
}


//______________________________
void SetCurrentFont(int fontNum) {

    if (fontNum >= FontFuncFon.fontNum_Min && fontNum <= FontFuncFon.fontNum_Max)
        SetFontFunctions = &FontFuncFon;
    else if (fontNum >= FontFuncAff.fontNum_Min && fontNum <= FontFuncAff.fontNum_Max)
        SetFontFunctions = &FontFuncAff;
    else
        SetFontFunctions = nullptr;
}


//____________________________________________
void __declspec(naked) set_current_font(void) {
    //004D596D  |.  89D1          MOV ECX,EDX
    //004D596F  |.  C1E2 02       SHL EDX,2
    __asm {
        pushad
        push ebx
        call SetCurrentFont
        add esp, 0x4
        popad
        mov ecx, edx
        shl edx, 0x2
        ret
    }
}


//_______________________
void Modifications_Text() {
    //To-Do chinese aaf text funcs load fonts via GDI32 CreateFont so this functionality needs to be added. 
    if (fallout_exe_region == EXE_Region::Chinese) {
        FontFuncAff.GetTextWidth2 = &GetCurrentFont_TextWidth_Aff_MultiByte;
    
        //To-Do set_current_font
    }
    else {
        //To-Do Gvx stuff should probably be removed - doubt it works now.
        if (fallout_exe_region == EXE_Region::USA) {
            if (*(DWORD*)0x4CAF23 == 0x225559)
                pGvx = new GVX;
            FontFuncAff.GetTextWidth2 = &GetCurrentFont_TextWidth_Aff_MultiByte;
        }

        MemWrite8(0x4D596D, 0x89, 0xE8);
        FuncWrite32(0x4D596E, 0x02E2C1D1, (DWORD)&set_current_font);
    }
}


