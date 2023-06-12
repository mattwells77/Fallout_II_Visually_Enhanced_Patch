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

extern DWORD wintool_view_type[];
extern DWORD wintool_select_type[];


void Rebar_ResizeBand(HWND hwnd_rebar, UINT bandNum);

HWND Rebar_Create(HWND hwnd_parent, HINSTANCE hinstance);

BOOL Rebar_Load(HWND hRebar, HINSTANCE hinstance);
BOOL Rebar_Save(HWND hwnd_parent);

HWND ToolBar_Create(HWND hwnd_parent, HINSTANCE hinstance);
BOOL ToolBar_Delete(HWND hwnd_parent, HWND hTool);

BOOL ToolBar_EditButtons(UINT code, LPTBNOTIFY lpTbNotify);


BOOL Tool_SetState(HWND hwnd_parent, DWORD buttID, BOOL isEnabled, BOOL isChecked, BOOL redraw_menu);

BOOL Tool_SetState_Level(HWND hwnd_parent, int levelNum);
BOOL Tool_SetState_ObjectsSelected(HWND hwnd_parent, BOOL isEnabled, BOOL redraw_menu);

BOOL Tool_SetState_Edges_Visible(HWND hwnd_parent, BOOL isEnabled, BOOL redraw_menu);
BOOL Tool_SetState_Edges_SelectEdge(HWND hwnd_parent, BOOL redraw_menu);

void Tools_SetAllCurrentToolStates(HWND hwnd_parent);
