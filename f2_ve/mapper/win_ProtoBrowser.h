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

#include "mapper_tools.h"
#include "../Fall_Objects.h"

#include "win_Mapper.h"





enum class WINDOW_STATE {
    Child_Bottom,
    Child_Right,
    Popup
};

extern WINDOW_STATE object_broser_state;

//extern OBJNode *pObjs_Selected;

extern HWND hWinObjBrowser;

BOOL ObjectBrowser_Toggle_Open();
BOOL ObjectBrowser_Open();
BOOL ObjectBrowser_Close();
BOOL IsObjectBrowserOpened();
WINDOW_STATE ObjectBrowser_SetView(WINDOW_STATE new_browser_view);

HBITMAP CreateIconFRM(HDC hdc, DWORD frmID, int ori, DWORD width, DWORD height, DWORD* p_bgColour);

void ObjectList_Refresh(OBJNode* p_objects, TILE_DATA_node* p_tiles);

void ProtoList_Refresh();
void ProtoList_Set(LONG type, LONG position);
void ProtoList_GoToProto(DWORD proID);
void ProtoList_GoToTileProto_With_FrmID(DWORD frmID);

//BOOL ProtoList_Rebuild_Cache(LONG type);




