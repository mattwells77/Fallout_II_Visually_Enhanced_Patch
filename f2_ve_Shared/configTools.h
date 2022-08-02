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

UINT ConfigReadInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int nDefault);
BOOL ConfigWriteInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int intVal);

DWORD ConfigReadString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize);
BOOL ConfigWriteString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpString);

BOOL ConfigReadWinData(const wchar_t* lpAppName, const wchar_t* lpKeyName, WINDOWPLACEMENT* pWinData);
BOOL ConfigWriteWinData(const wchar_t* lpAppName, const wchar_t* lpKeyName, WINDOWPLACEMENT* pWinData);

BOOL ConfigReadStruct(const wchar_t* lpszSection, const wchar_t* lpszKey, LPVOID lpStruct, UINT uSizeStruct);
BOOL ConfigWriteStruct(const wchar_t* lpszSection, const wchar_t* lpszKey, LPVOID lpStruct, UINT uSizeStruct);

UINT SfallReadInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int nDefault);
BOOL SfallWriteInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int intVal);

DWORD SfallReadString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize);

bool GetSfall_VersionInfo(char* msg, DWORD maxChars);

wchar_t* ConfigGetPath();
bool ConfigDestroy();