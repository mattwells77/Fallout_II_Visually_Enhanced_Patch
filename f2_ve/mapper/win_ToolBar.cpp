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

#include "win_ToolBar.h"
#include "win_Mapper.h"
#include "win_ProtoBrowser.h"

#include "mapper_tools.h"

#include "configTools.h"

#include "../resource.h"
#include "../win_fall.h"
#include "../errors.h"


#include "../Fall_GameMap.h"
#include "../Fall_Graphics.h"
#include "../Fall_File.h"

#include "../game_map.h"


DWORD wintool_view_type[]{ ID_VIEW_ITEMS, ID_VIEW_CRITTERS, ID_VIEW_SCENERY, ID_VIEW_WALLS, ID_VIEW_TILES, ID_VIEW_MISC };
DWORD wintool_select_type[]{ ID_OBJECTTYPESSELECTABLE_ITEMS, ID_OBJECTTYPESSELECTABLE_CRITTERS, ID_OBJECTTYPESSELECTABLE_SCENERY, ID_OBJECTTYPESSELECTABLE_WALLS, ID_OBJECTTYPESSELECTABLE_TILES, ID_OBJECTTYPESSELECTABLE_MISC };
DWORD wintool_select_edge[]{ ID_HRP_EDITLEFTEDGEOFSELECTEDRECT, ID_HRP_EDITTOPEDGEOFSELECTEDRECT, ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT, ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT };
DWORD wintool_select_iso_edge[]{ ID_HRP_EDITISOMETRICMAPEDGES_WEST, ID_HRP_EDITISOMETRICMAPEDGES_NORTH, ID_HRP_EDITISOMETRICMAPEDGES_EAST, ID_HRP_EDITISOMETRICMAPEDGES_SOUTH };


#define BMP_DEFAULT						0
#define BMP_FILE_NEW					1
#define BMP_FILE_OPEN					2
#define BMP_FILE_SAVE					3
#define BMP_FILE_SAVE_AS				4

#define BMP_SEL_CUT						5
#define BMP_SEL_COPY					6
#define BMP_SEL_PASTE					7
#define BMP_SEL_DELETE					8

#define BMP_VIEW_ROOF					9
#define BMP_VIEW_HEXES					10
#define BMP_VIEW_BLOCK					11

#define BMP_VIEW_ITEM					12
#define BMP_VIEW_CRITTER				13
#define BMP_VIEW_SCENERY				14
#define BMP_VIEW_WALL					15
#define BMP_VIEW_TILE					16
#define BMP_VIEW_MISC					17

#define BMP_LEVEL_UP					18
#define BMP_LEVEL_DOWN					19
#define BMP_LEVEL_1						20
#define BMP_LEVEL_2						21
#define BMP_LEVEL_3						22

#define BMP_SELECT_All_TYPES			23
#define BMP_SELECT_ITEM					24
#define BMP_SELECT_CRITTER				25
#define BMP_SELECT_SCENERY				26
#define BMP_SELECT_WALL					27
#define BMP_SELECT_TILE					28
#define BMP_SELECT_MISC					29
#define BMP_SELECT_TILE_FLOOR			30
#define BMP_SELECT_TILE_ROOF			31
#define BMP_SELECT_TILE_ALL				32

#define BMP_HR_EDGE_TOGGLE_VIS			33
#define BMP_HR_EDGE_CYCLE_SEL			34
#define BMP_HR_EDGE_ADD					35
#define BMP_HR_EDGE_DEL					36
#define BMP_HR_EDGE_LEFT				37
#define BMP_HR_EDGE_RIGHT				38
#define BMP_HR_EDGE_TOP					39
#define BMP_HR_EDGE_BOTTOM				40

#define BMP_HR_ISO_EDGE_WEST			41
#define BMP_HR_ISO_EDGE_NORTH			42
#define BMP_HR_ISO_EDGE_EAST			43
#define BMP_HR_ISO_EDGE_SOUTH			44

#define BMP_HR_ISO_EDGE_OBJ_VIS_WEST	45
#define BMP_HR_ISO_EDGE_OBJ_VIS_NORTH	46
#define BMP_HR_ISO_EDGE_OBJ_VIS_EAST	47
#define BMP_HR_ISO_EDGE_OBJ_VIS_SOUTH	48

#define BMP_COPY_LEVEL					49
#define BMP_PASTE_LEVEL					50


struct TOOLBARinfo {
	DWORD numButtons;
	REBARBANDINFO rbBandInfo;
	TBBUTTON button;
};


HIMAGELIST g_hImageList = nullptr;
const int ImageListID = 0;
const DWORD buttonStyles = BTNS_AUTOSIZE;

DWORD toolbarStyles = WS_CHILD | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE;// | TBSTYLE_WRAPABLE;
DWORD toolbarStylesEx = 0;//TBSTYLE_EX_DRAWDDARROWS;


DWORD buttArray[] = {
ID_FILE_NEW, ID_FILE_OPEN, ID_FILE_SAVE, ID_FILE_SAVEAS,/// IDM_QUIT_MAPPER,
ID_VIEW_ROOVES, ID_VIEW_HEXES, ID_VIEW_BLOCKERS, ID_VIEW_ITEMS, ID_VIEW_CRITTERS, ID_VIEW_SCENERY, ID_VIEW_WALLS,
ID_VIEW_TILES, ID_VIEW_MISC,
ID_EDIT_COPY, ID_EDIT_CUT, ID_EDIT_PASTE, ID_EDIT_DELETE, ID_EDIT_COPYLEVEL, ID_EDIT_PASTELEVEL,
ID_LEVEL_UP, ID_LEVEL_DOWN, ID_LEVEL_SELECT, ID_LEVEL_1, ID_LEVEL_2, ID_LEVEL_3,
ID_OBJECTTYPESSELECTABLE_ALLTYPESSELECTABLE, ID_OBJECTTYPESSELECTABLE_ITEMS, ID_OBJECTTYPESSELECTABLE_CRITTERS, ID_OBJECTTYPESSELECTABLE_SCENERY, ID_OBJECTTYPESSELECTABLE_WALLS, ID_OBJECTTYPESSELECTABLE_TILES, ID_OBJECTTYPESSELECTABLE_MISC,
ID_HRP_TOGGLEVISIBILITYOFEDGES, ID_HRP_CYCLESELECTEDEDGERECT, ID_HRP_ADDNEWEDGERECT, ID_HRP_DELETESELECTEDEDGERECT, ID_HRP_EDITLEFTEDGEOFSELECTEDRECT, ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT, ID_HRP_EDITTOPEDGEOFSELECTEDRECT, ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT,
ID_HRP_EDITISOMETRICMAPEDGES_WEST, ID_HRP_EDITISOMETRICMAPEDGES_NORTH, ID_HRP_EDITISOMETRICMAPEDGES_EAST, ID_HRP_EDITISOMETRICMAPEDGES_SOUTH,
ID_HRP_ISOEDGE_FLAG_WEST, ID_HRP_ISOEDGE_FLAG_NORTH, ID_HRP_ISOEDGE_FLAG_EAST, ID_HRP_ISOEDGE_FLAG_SOUTH
};


//__________________________________________________________
BOOL ToolBar_SetButtonInfo(int buttonID, TBBUTTON* tbbutton) {

	ZeroMemory(tbbutton, sizeof(TBBUTTON));
	tbbutton->idCommand = buttonID;
	tbbutton->fsState = TBSTATE_ENABLED;
	tbbutton->fsStyle = buttonStyles;

	int list_var = 0;

	switch (buttonID) {
	case ID_FILE_NEW:
		tbbutton->iBitmap = BMP_FILE_NEW;
		break;
	case ID_FILE_OPEN:
		tbbutton->iBitmap = BMP_FILE_OPEN;
		break;
	case ID_FILE_SAVE:
		tbbutton->iBitmap = BMP_FILE_SAVE;
		break;
	case ID_FILE_SAVEAS:
		tbbutton->iBitmap = BMP_FILE_SAVE_AS;
		break;

	case ID_EDIT_CUT:
		tbbutton->iBitmap = BMP_SEL_CUT;
		if (Get_Selected_Objects() == nullptr && Get_Selected_Tiles() == nullptr)
			tbbutton->fsState = 0;
		break;
	case ID_EDIT_COPY:
		tbbutton->iBitmap = BMP_SEL_COPY;
		if (Get_Selected_Objects() == nullptr && Get_Selected_Tiles() == nullptr)
			tbbutton->fsState = 0;
		break;
	case ID_EDIT_PASTE:
		tbbutton->iBitmap = BMP_SEL_PASTE;
		if (Get_Copied_Objects() == nullptr && Get_Copied_Tiles() == nullptr)
			tbbutton->fsState = 0;
		break;
	case ID_EDIT_DELETE:
		tbbutton->iBitmap = BMP_SEL_DELETE;
		if (Get_Selected_Objects() == nullptr && Get_Selected_Tiles() == nullptr)
			tbbutton->fsState = 0;
		break;
	case ID_EDIT_COPYLEVEL:
		tbbutton->iBitmap = BMP_COPY_LEVEL;
		break;
	case ID_EDIT_PASTELEVEL:
		tbbutton->iBitmap = BMP_PASTE_LEVEL;
		if (Get_Copied_Level_Objects() == nullptr && Get_Copied_Level_Tiles() == nullptr)
			tbbutton->fsState = 0;
		break;

	case ID_VIEW_HEXES:
		tbbutton->iBitmap = BMP_VIEW_HEXES;
		if (AreHexesVisible())
			tbbutton->fsState |= TBSTATE_PRESSED;
		tbbutton->fsStyle |= BTNS_CHECK;
		break;
	case ID_VIEW_BLOCKERS:
		tbbutton->iBitmap = BMP_VIEW_BLOCK;
		if (AreBlockerObjectsVisible())
			tbbutton->fsState |= TBSTATE_PRESSED;
		tbbutton->fsStyle |= BTNS_CHECK;
		break;
	case ID_VIEW_ROOVES:
		tbbutton->iBitmap = BMP_VIEW_ROOF;
		if (AreRoovesVisible())
			tbbutton->fsState |= TBSTATE_PRESSED;
		tbbutton->fsStyle |= BTNS_CHECK;
		break;

	case ID_VIEW_MISC:
		list_var++;
	case ID_VIEW_TILES:
		list_var++;
	case ID_VIEW_WALLS:
		list_var++;
	case ID_VIEW_SCENERY:
		list_var++;
	case ID_VIEW_CRITTERS:
		list_var++;
	case ID_VIEW_ITEMS:
		tbbutton->iBitmap = BMP_VIEW_ITEM + list_var;
		tbbutton->fsStyle |= BTNS_CHECK;
		if (IsArtTypeEnabled(list_var))
			tbbutton->fsState |= TBSTATE_PRESSED;
		break;

	case ID_LEVEL_SELECT:
		//swprintf_s(level_msg, L"j%i  ", *pMAP_LEVEL + 1);
		//tbbutton->iString = (INT_PTR)level_msg;
		tbbutton->iBitmap = BMP_LEVEL_1 + *pMAP_LEVEL;
		tbbutton->fsStyle = BTNS_WHOLEDROPDOWN | BTNS_SHOWTEXT;
		break;
	case ID_LEVEL_UP:
		tbbutton->iBitmap = BMP_LEVEL_UP;
		break;
	case ID_LEVEL_DOWN:
		tbbutton->iBitmap = BMP_LEVEL_DOWN;
		break;
	case ID_LEVEL_3:
		list_var++;
	case ID_LEVEL_2:
		list_var++;
	case ID_LEVEL_1:
		if(list_var == *pMAP_LEVEL)
			tbbutton->fsState |= TBSTATE_PRESSED;
		tbbutton->iBitmap = BMP_LEVEL_1 + list_var;
		tbbutton->fsStyle |= BTNS_CHECK;//BTNS_CHECKGROUP
		break;
	case ID_OBJECTTYPESSELECTABLE_ALLTYPESSELECTABLE:
		tbbutton->iBitmap = BMP_SELECT_All_TYPES;
		break;
	case ID_OBJECTTYPESSELECTABLE_MISC:
		list_var++;
	case ID_OBJECTTYPESSELECTABLE_TILES:
		list_var++;
	case ID_OBJECTTYPESSELECTABLE_WALLS:
		list_var++;
	case ID_OBJECTTYPESSELECTABLE_SCENERY:
		list_var++;
	case ID_OBJECTTYPESSELECTABLE_CRITTERS:
		list_var++;
	case ID_OBJECTTYPESSELECTABLE_ITEMS:
		tbbutton->iBitmap = BMP_SELECT_ITEM + list_var;
		tbbutton->fsStyle |= BTNS_CHECK;
		if (IsObjectTypeSelectable(list_var))
			tbbutton->fsState |= TBSTATE_PRESSED;
		break;

	case ID_HRP_TOGGLEVISIBILITYOFEDGES:
		tbbutton->iBitmap = BMP_HR_EDGE_TOGGLE_VIS;
		break;
	case ID_HRP_CYCLESELECTEDEDGERECT:
		tbbutton->iBitmap = BMP_HR_EDGE_CYCLE_SEL;
		break;
	case ID_HRP_ADDNEWEDGERECT:
		tbbutton->iBitmap = BMP_HR_EDGE_ADD;
		break;
	case ID_HRP_DELETESELECTEDEDGERECT:
		tbbutton->iBitmap = BMP_HR_EDGE_DEL;
		break;
	case ID_HRP_EDITLEFTEDGEOFSELECTEDRECT:
		tbbutton->iBitmap = BMP_HR_EDGE_LEFT;
		break;
	case ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT:
		tbbutton->iBitmap = BMP_HR_EDGE_RIGHT;
		break;
	case ID_HRP_EDITTOPEDGEOFSELECTEDRECT:
		tbbutton->iBitmap = BMP_HR_EDGE_TOP;
		break;
	case ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT:
		tbbutton->iBitmap = BMP_HR_EDGE_BOTTOM;
		break;

	case ID_HRP_EDITISOMETRICMAPEDGES_WEST:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_WEST;
		break;
	case ID_HRP_EDITISOMETRICMAPEDGES_NORTH:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_NORTH;
		break;
	case ID_HRP_EDITISOMETRICMAPEDGES_EAST:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_EAST;
		break;
	case ID_HRP_EDITISOMETRICMAPEDGES_SOUTH:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_SOUTH;
		break;

	case ID_HRP_ISOEDGE_FLAG_WEST:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_OBJ_VIS_WEST;
		break;
	case ID_HRP_ISOEDGE_FLAG_NORTH:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_OBJ_VIS_NORTH;
		break;
	case ID_HRP_ISOEDGE_FLAG_EAST:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_OBJ_VIS_EAST;
		break;
	case ID_HRP_ISOEDGE_FLAG_SOUTH:
		tbbutton->iBitmap = BMP_HR_ISO_EDGE_OBJ_VIS_SOUTH;
		break;

	default:
		tbbutton->iBitmap = BMP_DEFAULT;
		return FALSE;
		break;
	}
	return TRUE;
}


//________________________________________________________
HWND ToolBar_Create(HWND hwnd_parent, HINSTANCE hinstance) {

	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);
	UINT numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);

	HWND hTool = CreateWindowEx(toolbarStylesEx, TOOLBARCLASSNAME, nullptr,
		toolbarStyles,
		0, 0, 0, 0,
		hwnd_parent, (HMENU)ID_RB_BAND_001 + numBands, hinstance, nullptr);

	if (hTool == nullptr)
		return nullptr;

	SendMessage(hTool, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)g_hImageList);

	//The system uses the size to determine which version of the common control dynamic-link library (DLL) is being used.
	SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	//Displays the Customize Toolbar dialog box.
	SendMessage(hTool, TB_CUSTOMIZE, 0, 0);

	//Initialize band info.
	REBARBANDINFO rbBand{ 0 };
	rbBand.cbSize = sizeof(REBARBANDINFO);
	rbBand.fMask =
		RBBIM_STYLE			//fStyle is valid.
		| RBBIM_TEXT		//lpText is valid.
		| RBBIM_CHILD		//hwndChild is valid.
		| RBBIM_CHILDSIZE	//cxMinChild, cyMinChild, cyChild, cyMaxChild, and cyIntegral members are valid.
		| RBBIM_SIZE		//cx member is valid.
		| RBBIM_IDEALSIZE	//Version 4.71.The cxIdeal member is valid or must be set.
		| RBBIM_ID;			//wID member is valid. 
	rbBand.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;


	//Get the size of the toolbar
	RECT rcTool{ 0,0,0,0 }, rcItem{ 0,0,0,0 };
	int numButtons = SendMessage(hTool, (UINT)TB_BUTTONCOUNT, (WPARAM)0, (LPARAM)0);
	for (int num = 0; num < numButtons; num++) {
		SendMessage(hTool, TB_GETITEMRECT, num, (LPARAM)&rcItem);
		UnionRect(&rcTool, &rcTool, &rcItem);
	}
	//Set values to the band with the toolbar.
	rbBand.lpText = (wchar_t*)TEXT("");
	rbBand.hwndChild = hTool;
	rbBand.wID = GetDlgCtrlID(hTool);
	//rbBand.cyChild = rcTool.bottom - rcTool.top;
	rbBand.cxMinChild = rcTool.right - rcTool.left;
	rbBand.cyMinChild = rcTool.bottom - rcTool.top;
	rbBand.cx = rbBand.cxMinChild;
	rbBand.cxIdeal = rbBand.cxMinChild;

	//Add the band that has the toolbar.
	SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

	return hTool;
}


//___________________________________________________________________________________________
HWND ToolBar_Create(HWND hwnd_parent, HINSTANCE hinstance, DWORD *pButtonID, int num_buttons) {
	if (!pButtonID || num_buttons < 1)
		return nullptr;


	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);
	UINT numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);

	HWND hTool = CreateWindowEx(toolbarStylesEx, TOOLBARCLASSNAME, nullptr,
		toolbarStyles,
		0, 0, 0, 0,
		hwnd_parent, (HMENU)ID_RB_BAND_001 + numBands, hinstance, nullptr);
	if (hTool == nullptr)
		return nullptr;
	SendMessage(hTool, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)g_hImageList);

	TBBUTTON* ptbb = new TBBUTTON[num_buttons]{ 0 };

	for(int i = 0; i< num_buttons; i++)
		ToolBar_SetButtonInfo(pButtonID[i], &ptbb[i]);

	//The system uses the size to determine which version of the common control dynamic-link library (DLL) is being used.
	SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	//Add buttons to toolbar.
	SendMessage(hTool, TB_ADDBUTTONS, num_buttons, (LPARAM)ptbb);

	delete[]ptbb;

	// Initialize band info.
	REBARBANDINFO rbBand{ 0 };
	rbBand.cbSize = sizeof(REBARBANDINFO);
	rbBand.fMask =
		RBBIM_STYLE			//fStyle is valid.
		| RBBIM_TEXT		//lpText is valid.
		| RBBIM_CHILD		//hwndChild is valid.
		| RBBIM_CHILDSIZE	//cxMinChild, cyMinChild, cyChild, cyMaxChild, and cyIntegral members are valid.
		| RBBIM_SIZE		//cx member is valid.
		| RBBIM_IDEALSIZE	//Version 4.71.The cxIdeal member is valid or must be set.
		| RBBIM_ID;			//wID member is valid. 
	rbBand.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;

	//Get the size of the toolbar
	RECT rcTool{ 0,0,0,0 }, rcItem{ 0,0,0,0 };
	int numButtons = SendMessage(hTool, (UINT)TB_BUTTONCOUNT, (WPARAM)0, (LPARAM)0);
	for (int num = 0; num < numButtons; num++) {
		SendMessage(hTool, TB_GETITEMRECT, num, (LPARAM)&rcItem);
		UnionRect(&rcTool, &rcTool, &rcItem);
	}
	//Set values to the band with the toolbar.
	rbBand.lpText = (wchar_t*)TEXT("");
	rbBand.hwndChild = hTool;
	rbBand.wID = GetDlgCtrlID(hTool);
	//rbBand.cyChild = rcTool.bottom - rcTool.top;
	rbBand.cxMinChild = rcTool.right - rcTool.left;
	rbBand.cyMinChild = rcTool.bottom - rcTool.top;
	rbBand.cx = rbBand.cxMinChild;
	rbBand.cxIdeal = rbBand.cxMinChild;

	//Add the band that has the toolbar.
	if (SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand))
		Fallout_Debug_Info("CreateFileBar %dx%d", rbBand.cxMinChild, rbBand.cyMinChild);

	return hTool;
}


//______________________________________________________
HWND Rebar_Create(HWND hwnd_parent, HINSTANCE hinstance) {
	//Initialize common controls.
	INITCOMMONCONTROLSEX icex{ 0 };
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);

	//Create the rebar.
	HWND hwnd_rebar = CreateWindowEx(WS_EX_TOOLWINDOW,
		REBARCLASSNAME,
		nullptr,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
		RBS_BANDBORDERS | RBS_DBLCLKTOGGLE | RBS_REGISTERDROP | RBS_VARHEIGHT | RBS_AUTOSIZE |
		CCS_NODIVIDER,
		0, 0, 0, 0,
		hwnd_parent,
		(HMENU)IDC_REBAR,
		hinstance,
		nullptr);

	if (!hwnd_rebar)
		return nullptr;

	g_hImageList = ImageList_LoadImage(hinstance, MAKEINTRESOURCE(IDB_BITMAP_TOOLS), 16, 26, 0x00FFFFFF, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);

	if (Rebar_Load(hwnd_rebar, hinstance))
		return hwnd_rebar;

	//If loading rebar from file fails, create initial bars. 
	DWORD* pButtons = new DWORD[35]{ ID_FILE_NEW, ID_FILE_OPEN, ID_FILE_SAVE, ID_FILE_SAVEAS,
		ID_EDIT_COPY, ID_EDIT_CUT, ID_EDIT_PASTE, ID_EDIT_DELETE, ID_EDIT_COPYLEVEL, ID_EDIT_PASTELEVEL,
		ID_VIEW_ROOVES, ID_VIEW_HEXES, ID_VIEW_BLOCKERS, ID_VIEW_ITEMS, ID_VIEW_CRITTERS, ID_VIEW_SCENERY, ID_VIEW_WALLS, ID_VIEW_TILES, ID_VIEW_MISC,
		ID_OBJECTTYPESSELECTABLE_ALLTYPESSELECTABLE, ID_OBJECTTYPESSELECTABLE_ITEMS, ID_OBJECTTYPESSELECTABLE_CRITTERS, ID_OBJECTTYPESSELECTABLE_SCENERY, ID_OBJECTTYPESSELECTABLE_WALLS, ID_OBJECTTYPESSELECTABLE_TILES, ID_OBJECTTYPESSELECTABLE_MISC,
		ID_HRP_TOGGLEVISIBILITYOFEDGES, ID_HRP_CYCLESELECTEDEDGERECT, ID_HRP_ADDNEWEDGERECT, ID_HRP_DELETESELECTEDEDGERECT, ID_HRP_EDITLEFTEDGEOFSELECTEDRECT, ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT, ID_HRP_EDITTOPEDGEOFSELECTEDRECT, ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT,
		ID_LEVEL_SELECT };

	ToolBar_Create(hwnd_parent, hinstance, pButtons, 4);
	ToolBar_Create(hwnd_parent, hinstance, &pButtons[4], 6);
	ToolBar_Create(hwnd_parent, hinstance, &pButtons[10], 9);
	ToolBar_Create(hwnd_parent, hinstance, &pButtons[19], 7);
	ToolBar_Create(hwnd_parent, hinstance, &pButtons[26], 8);
	ToolBar_Create(hwnd_parent, hinstance, &pButtons[34], 1);
	//maximize the second last band, pushing the last band(level selection) to the far right.
	SendMessage(hwnd_rebar, RB_MAXIMIZEBAND, (WPARAM)4, (LPARAM)0);
	delete pButtons;
	pButtons = nullptr;


	return (hwnd_rebar);
}


//__________________________________________________
void Rebar_ResizeBand(HWND hwnd_rebar, UINT bandNum) {

	REBARBANDINFO rbBandInfo{ 0 };

	int numButtons = 0;
	RECT rcTool{ 0,0,0,0 };
	RECT rcItem{ 0,0,0,0 };

	rbBandInfo.cbSize = sizeof(REBARBANDINFO);
	rbBandInfo.fMask =
		RBBIM_CHILD			//hwndChild is valid.
		| RBBIM_CHILDSIZE	//cxMinChild, cyMinChild, cyChild, cyMaxChild, and cyIntegral members are valid.
		| RBBIM_SIZE		//cx member is valid.
		| RBBIM_IDEALSIZE;	//Version 4.71.The cxIdeal member is valid or must be set.


	rbBandInfo.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_IDEALSIZE;

	SendMessage(hwnd_rebar, RB_GETBANDINFO, (WPARAM)bandNum, (LPARAM)&rbBandInfo);

	numButtons = SendMessage(rbBandInfo.hwndChild, (UINT)TB_BUTTONCOUNT, (WPARAM)0, (LPARAM)0);

	for (int numb = 0; numb < numButtons; numb++) {
		SendMessage(rbBandInfo.hwndChild, TB_GETITEMRECT, numb, (LPARAM)&rcItem);
		UnionRect(&rcTool, &rcTool, &rcItem);
	}
	rbBandInfo.cxMinChild = rcTool.right - rcTool.left;
	rbBandInfo.cx = rbBandInfo.cxMinChild;
	rbBandInfo.cxIdeal = rbBandInfo.cxMinChild;
	SendMessage(hwnd_rebar, RB_SETBANDINFO, (WPARAM)bandNum, (LPARAM)&rbBandInfo);
}


//______________________________________________________________________________________________________
BOOL MenuItem_SetState(HWND hwnd_parent, DWORD buttID, BOOL isEnabled, BOOL isChecked, BOOL redraw_menu) {

	HMENU hmenu = GetMenu(hwnd_parent);
	if (!hmenu)
		return FALSE;

	MENUITEMINFO menuInfo{ 0 };
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
	if (!GetMenuItemInfo(hmenu, buttID, false, &menuInfo))
		return FALSE;

	if (!isEnabled)
		menuInfo.fState |= MFS_DISABLED;
	else
		menuInfo.fState &= ~MFS_DISABLED;
	if (isChecked)
		menuInfo.fState |= MFS_CHECKED;
	else
		menuInfo.fState &= ~MFS_CHECKED;

	BOOL retVal = SetMenuItemInfo(hmenu, buttID, FALSE, &menuInfo);
	if(retVal && redraw_menu)
		DrawMenuBar(hwnd_parent);
	return retVal;
}


//___________________________________________________________________________________
BOOL ToolBar_SetState(HWND hwnd_parent, DWORD buttID, BOOL isEnabled, BOOL isChecked) {

	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);

	REBARBANDINFO rbBandInfo{ 0 };
	rbBandInfo.cbSize = sizeof(REBARBANDINFO);
	rbBandInfo.fMask = RBBIM_CHILD; //hwndChild is valid.

	UINT numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);

	WORD buttState = 0;
	if (isEnabled)
		buttState |= TBSTATE_ENABLED;
	if (isChecked)
		buttState |= TBSTATE_PRESSED;

	for (UINT num = 0; num < numBands; num++) {
		SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)num, (LPARAM)&rbBandInfo);
		SendMessage(rbBandInfo.hwndChild, TB_SETSTATE, (WPARAM)buttID, (LPARAM)LOWORD(buttState));
		if (buttID == ID_OBJECTTYPESSELECTABLE_TILES) {
			UINT32 bmpID = BMP_SELECT_TILE;
			if (isChecked == 1)
				bmpID = BMP_SELECT_TILE_FLOOR;
			else if (isChecked == 2)
				bmpID = BMP_SELECT_TILE_ROOF;
			else if (isChecked == (1 | 2))
				bmpID = BMP_SELECT_TILE_ALL;
			SendMessage(rbBandInfo.hwndChild, TB_CHANGEBITMAP, (WPARAM)ID_OBJECTTYPESSELECTABLE_TILES, (LPARAM)bmpID);
		}
	}
	return TRUE;
}


//__________________________________________________________________________________________________
BOOL Tool_SetState(HWND hwnd_parent, DWORD buttID, BOOL isEnabled, BOOL isChecked, BOOL redraw_menu) {
	if (ToolBar_SetState(hwnd_parent, buttID, isEnabled, isChecked) == TRUE &&
		MenuItem_SetState(hwnd_parent, buttID, isEnabled, isChecked, redraw_menu) == TRUE)
		return TRUE;

	return FALSE;
}


//____________________________________________________________________________________
BOOL Tool_SetState_ObjectsSelected(HWND hwnd_parent, BOOL isEnabled, BOOL redraw_menu) {
	if (Tool_SetState(hwnd_parent, ID_EDIT_CUT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_EDIT_COPY, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_EDIT_DELETE, isEnabled, FALSE, redraw_menu) == TRUE) {
		return TRUE;
	}
	return FALSE;
}


/*//_________________________________________________________
BOOL Tool_SetState_ObjectsCopied(HWND hwnd, BOOL isEnabled) {
	return Tool_SetState(hwnd, ID_EDIT_PASTE, isEnabled, FALSE);
}


//______________________________________________________________
BOOL Tool_SetState_LevelObjectsCopied(HWND hwnd, BOOL isEnabled) {
	return Tool_SetState(hwnd, ID_EDIT_PASTELEVEL, isEnabled, FALSE);
}*/

//
//__________________________________________________________________________________
BOOL Tool_SetState_Edges_Visible(HWND hwnd_parent, BOOL isEnabled, BOOL redraw_menu) {
	if (Tool_SetState(hwnd_parent, ID_HRP_CYCLESELECTEDEDGERECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_ADDNEWEDGERECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_DELETESELECTEDEDGERECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITLEFTEDGEOFSELECTEDRECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITTOPEDGEOFSELECTEDRECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT, isEnabled, FALSE, FALSE) == TRUE &&

		Tool_SetState(hwnd_parent, ID_HRP_EDITISOMETRICMAPEDGES_WEST, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITISOMETRICMAPEDGES_NORTH, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITISOMETRICMAPEDGES_EAST, isEnabled, FALSE, FALSE) == TRUE &&
		Tool_SetState(hwnd_parent, ID_HRP_EDITISOMETRICMAPEDGES_SOUTH, isEnabled, FALSE, FALSE) == TRUE &&

		Tool_SetState(hwnd_parent, ID_HRP_ISOEDGE_FLAG_WEST, isEnabled, IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT::left, *pMAP_LEVEL), FALSE) &&
		Tool_SetState(hwnd_parent, ID_HRP_ISOEDGE_FLAG_NORTH, isEnabled, IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT::top, *pMAP_LEVEL), FALSE) &&
		Tool_SetState(hwnd_parent, ID_HRP_ISOEDGE_FLAG_EAST, isEnabled, IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT::right, *pMAP_LEVEL), FALSE) &&
		Tool_SetState(hwnd_parent, ID_HRP_ISOEDGE_FLAG_SOUTH, isEnabled, IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT::bottom, *pMAP_LEVEL), redraw_menu))
	{
		return TRUE;
	}
	return FALSE;
}


//_____________________________________________________________________
BOOL Tool_SetState_Edges_SelectEdge(HWND hwnd_parent, BOOL redraw_menu) {
	if (!AreAllEdgesVisible())
		return FALSE;
	EDGE_SELECT edge = Get_SelectedEdge(SEL_RC_TYPE::square);

	int edge_int = static_cast<int>(edge);
	BOOL isSelected = FALSE;
	for (int i = 0; i < _countof(wintool_select_edge); i++) {
		if (edge_int == i)
			isSelected = TRUE;
		Tool_SetState(hwnd_parent, wintool_select_edge[i], TRUE, isSelected, FALSE);
		isSelected = FALSE;
	}

	edge = Get_SelectedEdge(SEL_RC_TYPE::isometric);
	edge_int = static_cast<int>(edge);

	for (int i = 0; i < _countof(wintool_select_iso_edge); i++) {
		if (edge_int == i)
			isSelected = TRUE;
		Tool_SetState(hwnd_parent, wintool_select_iso_edge[i], TRUE, isSelected, FALSE);
		isSelected = FALSE;
	}

	return FALSE;
}


//__________________________________________________
void Tools_SetAllCurrentToolStates(HWND hwnd_parent) {

	BOOL isEnabled = FALSE;
	if (Get_Selected_Objects() != nullptr || Get_Selected_Tiles() != nullptr)
		isEnabled = TRUE;
	Tool_SetState_ObjectsSelected(hwnd_parent, isEnabled, FALSE);

	isEnabled = FALSE;
	if (Get_Copied_Objects() != nullptr || Get_Copied_Tiles() != nullptr)
		isEnabled = TRUE;
	Tool_SetState(hwnd_parent, ID_EDIT_PASTE, isEnabled, FALSE, FALSE);


	isEnabled = FALSE;
	if (Get_Copied_Level_Objects() != nullptr || Get_Copied_Level_Tiles() != nullptr)
		isEnabled = TRUE;
	Tool_SetState(hwnd_parent, ID_EDIT_PASTELEVEL, isEnabled, FALSE, FALSE);

	isEnabled = TRUE;
	if (Get_SelectionRectType() == SEL_RC_TYPE::square) {
		Tool_SetState(hwnd_parent, ID_SELECTIONTOOLOPTIONS_WINDOWORIENTATEDRECT, isEnabled, TRUE, FALSE);
		Tool_SetState(hwnd_parent, ID_SELECTIONTOOLOPTIONS_PERSPECTIVEORIENTATEDRECT, isEnabled, FALSE, FALSE);
	}
	else {
		Tool_SetState(hwnd_parent, ID_SELECTIONTOOLOPTIONS_WINDOWORIENTATEDRECT, isEnabled, FALSE, FALSE);
		Tool_SetState(hwnd_parent, ID_SELECTIONTOOLOPTIONS_PERSPECTIVEORIENTATEDRECT, isEnabled, TRUE, FALSE);
	}


	Tool_SetState(hwnd_parent, ID_VIEW_ROOVES, isEnabled, AreRoovesVisible(), FALSE);
	Tool_SetState(hwnd_parent, ID_VIEW_HEXES, isEnabled, AreHexesVisible(), FALSE);
	Tool_SetState(hwnd_parent, ID_VIEW_BLOCKERS, isEnabled, AreBlockerObjectsVisible(), FALSE);

	for (int type = 0; type < 6; type++)
		Tool_SetState(hwnd_parent, wintool_view_type[type], isEnabled, IsArtTypeEnabled(type), FALSE);

	for (int type = 0; type < 6; type++)
		Tool_SetState(hwnd_parent, wintool_select_type[type], isEnabled, IsObjectTypeSelectable(type), FALSE);

	Tool_SetState(hwnd_parent, ID_HRP_TOGGLEVISIBILITYOFEDGES, isEnabled, AreAllEdgesVisible(), FALSE);

	Tool_SetState(hwnd_parent, ID_OBJECTBROWSER_OPEN, isEnabled, IsObjectBrowserOpened(), FALSE);


	Tool_SetState_Level(hwnd_parent, *pMAP_LEVEL);

	Tool_SetState_Edges_Visible(hwnd_parent, AreAllEdgesVisible(), FALSE);

	//not yet implemented
	Tool_SetState(hwnd_parent, ID_VIEW_HEXES, FALSE, FALSE, FALSE);

	DrawMenuBar(hwnd_parent);
}


//______________________________________________________
BOOL Tool_SetState_Level(HWND hwnd_parent, int levelNum) {
	if (levelNum < 0 || levelNum >= 3)
		return FALSE;
	
	wchar_t msg[12];
	swprintf_s(msg, L"%s \0", L"WW");

	SIZE msgSize;
	int maxWidth = 50;
	HDC hdc = GetDC(hwnd_parent);
	GetTextExtentPoint32(hdc, msg, 3, &msgSize);
	if (maxWidth < msgSize.cx)
		maxWidth = msgSize.cx + 32;

	swprintf_s(msg, L"%i  ", levelNum + 1);

	DWORD levelID = ID_LEVEL_1;
	if (levelNum == 1)
		levelID = ID_LEVEL_2;
	else if (levelNum == 2)
		levelID = ID_LEVEL_3;


	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);

	REBARBANDINFO rbBandInfo{ 0 };
	rbBandInfo.cbSize = sizeof(REBARBANDINFO);
	rbBandInfo.fMask = RBBIM_CHILD; //hwndChild is valid.

	TBBUTTONINFO tbButtoninfo{ 0 };
	tbButtoninfo.cbSize = sizeof(TBBUTTONINFO);
	tbButtoninfo.dwMask = TBIF_TEXT | TBIF_SIZE;

	UINT numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);

	for (UINT num = 0; num < numBands; num++) {
		SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)num, (LPARAM)&rbBandInfo);

		SendMessage(rbBandInfo.hwndChild, TB_SETSTATE, (WPARAM)ID_LEVEL_1, (LPARAM)LOWORD(TBSTATE_ENABLED));
		SendMessage(rbBandInfo.hwndChild, TB_SETSTATE, (WPARAM)ID_LEVEL_2, (LPARAM)LOWORD(TBSTATE_ENABLED));
		SendMessage(rbBandInfo.hwndChild, TB_SETSTATE, (WPARAM)ID_LEVEL_3, (LPARAM)LOWORD(TBSTATE_ENABLED));
		SendMessage(rbBandInfo.hwndChild, TB_SETSTATE, (WPARAM)levelID, (LPARAM)LOWORD(TBSTATE_ENABLED | TBSTATE_PRESSED));

		SendMessage(rbBandInfo.hwndChild, TB_CHANGEBITMAP, (WPARAM)ID_LEVEL_SELECT, (LPARAM)BMP_LEVEL_1 + levelNum);

		tbButtoninfo.pszText = msg;
		tbButtoninfo.cx = maxWidth;

		if (SendMessage(rbBandInfo.hwndChild, TB_SETBUTTONINFO, (WPARAM)ID_LEVEL_SELECT, (LPARAM)&tbButtoninfo))
			Rebar_ResizeBand(hRebar, num);

	}
	return TRUE;
}


//________________________________________________________
BOOL ToolBar_EditButtons(UINT code, LPTBNOTIFY lpTbNotify) {
	static int        nResetCount = 0;
	static LPTBBUTTON lpSaveButtons = 0;
	static int        buttonCount = 0;
	bool ismaximized = false;
	//LPTBNOTIFY lpTbNotify = (LPTBNOTIFY)lParam;

	switch (code) {
	case TBN_GETBUTTONINFO: {
		//Pass the next button from the array. There is no need to filter out buttons that are already used they will be ignored.
		int buttonCount = sizeof(buttArray) / sizeof(DWORD);
		int buttonNum = buttArray[lpTbNotify->iItem];

		TBBUTTON button{ 0 };
		ToolBar_SetButtonInfo(buttonNum, &button);

		if (lpTbNotify->iItem < buttonCount) {
			lpTbNotify->tbButton = button;
			LoadString(phinstDLL, button.idCommand, lpTbNotify->pszText, lpTbNotify->cchText);
			buttonCount++;
			return TRUE;
		}
		else {
			buttonCount;
			return FALSE;  // No more buttons.
		}
		break;
	}
	case TBN_BEGINADJUST: {
		//Save the current configuration so the original toolbar can be restored, if the user presses reset.
		nResetCount = SendMessage(lpTbNotify->hdr.hwndFrom, TB_BUTTONCOUNT, 0, 0);
		lpSaveButtons = new TBBUTTON[nResetCount];

		for (int i = 0; i < nResetCount; i++)
			SendMessage(lpTbNotify->hdr.hwndFrom, TB_GETBUTTON, i, (LPARAM)(lpSaveButtons + i));
		return TRUE;
	}
	case TBN_RESET: {
		//Remove all of the existing buttons, starting with the last one.
		int nCount = SendMessage(lpTbNotify->hdr.hwndFrom, TB_BUTTONCOUNT, 0, 0);
		for (int i = nCount - 1; i >= 0; i--)
			SendMessage(lpTbNotify->hdr.hwndFrom, TB_DELETEBUTTON, i, 0);
		//Restore the saved buttons.
		SendMessage(lpTbNotify->hdr.hwndFrom, TB_ADDBUTTONS, (WPARAM)nResetCount, (LPARAM)lpSaveButtons);

		HWND hRebar = GetParent(lpTbNotify->hdr.hwndFrom);
		int bandNum = SendMessage(hRebar, RB_IDTOINDEX, (WPARAM)GetDlgCtrlID(lpTbNotify->hdr.hwndFrom), (LPARAM)0);
		Rebar_ResizeBand(hRebar, bandNum);
		return TRUE;
	}
	case TBN_ENDADJUST:
		//Free up the memory allocated to store the original toolbar configuration.
		delete lpSaveButtons;
		lpSaveButtons = nullptr;

		TBBUTTONINFO tbButtoninfo{ 0 };
		tbButtoninfo.cbSize = sizeof(TBBUTTONINFO);
		tbButtoninfo.dwMask = TBIF_BYINDEX| TBIF_COMMAND| TBIF_IMAGE| TBIF_LPARAM| TBIF_STATE| TBIF_STYLE| TBIF_TEXT | TBIF_SIZE;

		TBBUTTON button{0};
		int nCount = SendMessage(lpTbNotify->hdr.hwndFrom, TB_BUTTONCOUNT, 0, 0);
		for (int i = nCount - 1; i >= 0; i--) {
			SendMessage(lpTbNotify->hdr.hwndFrom, TB_GETBUTTON, i, (LPARAM)&button);

			SendMessage(lpTbNotify->hdr.hwndFrom, TB_SETSTATE, (WPARAM)button.idCommand, (LPARAM)LOWORD(button.fsState));
		}

		//HWND hRebar = GetParent(lpTbNotify->hdr.hwndFrom);
		//int bandNum = SendMessage(hRebar, RB_IDTOINDEX, (WPARAM)GetDlgCtrlID(lpTbNotify->hdr.hwndFrom), (LPARAM)0);
		//Rebar_ResizeBand(hRebar, bandNum);
		return TRUE;
	}
	return FALSE;
}


//_______________________________________________
BOOL ToolBar_Delete(HWND hwnd_parent, HWND hTool) {

	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);
	UINT numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);
	
	//don't delete the toolbar if it's the last one.
	if (numBands <= 1)
		return FALSE;

	REBARBANDINFO rbBandInfo{ 0 };
	rbBandInfo.cbSize = sizeof(REBARBANDINFO);
	rbBandInfo.fMask = RBBIM_CHILD; //hwndChild is valid.
	for (UINT num = 0; num < numBands; num++) {
		SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)num, (LPARAM)&rbBandInfo);

		if (rbBandInfo.hwndChild == hTool) {
			DestroyWindow(hTool);
			SendMessage(hRebar, RB_DELETEBAND, (WPARAM)num, (LPARAM)0);
		}
	}

	return TRUE;
}


//_______________________________
BOOL Rebar_Save(HWND hwnd_parent) {

	HWND hRebar = GetDlgItem(hwnd_parent, IDC_REBAR);
	DWORD numBands = SendMessage(hRebar, RB_GETBANDCOUNT, (WPARAM)0, (LPARAM)0);

	REBARBANDINFO rbBandInfo{ 0 };
	rbBandInfo.cbSize = sizeof(REBARBANDINFO);
	rbBandInfo.fMask = RBBIM_BACKGROUND | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_HEADERSIZE | RBBIM_IDEALSIZE |
		RBBIM_ID | RBBIM_IMAGE | RBBIM_LPARAM | RBBIM_SIZE | RBBIM_STYLE | RBBIM_TEXT;

	DWORD numButtons = 0;
	LPTBBUTTON lpSaveButtons = 0;

	BYTE* toolData = nullptr;
	TOOLBARinfo* pToolbarInfo = nullptr;

	wchar_t mapper_toolbar_file[] = L"\\toolbar.dat";
	FILE* pFile = nullptr;
	if (!MapperData_OpenFile(&pFile, mapper_toolbar_file, _countof(mapper_toolbar_file), L"wb"))
		return FALSE;
	//write the number bands in the rebar
	fwrite(&numBands, sizeof(DWORD), 1, pFile);

	for (UINT num = 0; num < numBands; num++) {
		//write the current band info
		SendMessage(hRebar, RB_GETBANDINFO, (WPARAM)num, (LPARAM)&rbBandInfo);
		fwrite(&rbBandInfo, sizeof(REBARBANDINFO), 1, pFile);
		//write the number of toolbar buttons in the current band
		numButtons = SendMessage(rbBandInfo.hwndChild, TB_BUTTONCOUNT, 0, 0);
		fwrite(&numButtons, sizeof(DWORD), 1, pFile);

		lpSaveButtons = new TBBUTTON[numButtons];
		for (DWORD i = 0; i < numButtons; i++)
			SendMessage(rbBandInfo.hwndChild, TB_GETBUTTON, i, (LPARAM)(&lpSaveButtons[i]));
		//write the button data
		fwrite(lpSaveButtons, sizeof(TBBUTTON), numButtons, pFile);

		delete[] lpSaveButtons;
	}

	fclose(pFile);
	return TRUE;
}


//___________________________________________________
BOOL Rebar_Load(HWND hwnd_rebar, HINSTANCE hinstance) {

	REBARBANDINFO rbBandInfo{ 0 };
	rbBandInfo.cbSize = sizeof(REBARBANDINFO);

	DWORD numBands = 0;
	DWORD numButtons = 0;
	LPTBBUTTON lpSaveButtons = 0;

	HWND hTool;

	wchar_t mapper_toolbar_file[] = L"\\toolbar.dat";
	FILE* pFile = nullptr;
	if (!MapperData_OpenFile(&pFile, mapper_toolbar_file, _countof(mapper_toolbar_file), L"rb"))
		return FALSE;

	//read the number bands in the rebar
	fread(&numBands, sizeof(DWORD), 1, pFile);

	for (UINT num = 0; num < numBands; num++) {
		//read the current band info
		fread(&rbBandInfo, sizeof(REBARBANDINFO), 1, pFile);
		//read the number of toolbar buttons in the current band
		fread(&numButtons, sizeof(DWORD), 1, pFile);

		lpSaveButtons = new TBBUTTON[numButtons];
		//read the button data
		fread(lpSaveButtons, sizeof(TBBUTTON), numButtons, pFile);

		for (DWORD numb = 0; numb < numButtons; numb++)
			lpSaveButtons[numb].iString = 0;

		//create a toolbar and add the buttons
		hTool = CreateWindowEx(toolbarStylesEx, TOOLBARCLASSNAME, nullptr,
			toolbarStyles,
			0, 0, 0, 0,
			hwnd_rebar, (HMENU)(ID_RB_BAND_001 + num), hinstance, nullptr);

		if (hTool == nullptr)
			return FALSE;

		SendMessage(hTool, TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)g_hImageList);

		//The system uses the size to determine which version of the common control dynamic-link library (DLL) is being used.
		SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

		SendMessage(hTool, TB_ADDBUTTONS, numButtons, (LPARAM)lpSaveButtons);

		delete lpSaveButtons;

		//add the toolbar to the current band
		rbBandInfo.lpText = (wchar_t*)TEXT("");
		rbBandInfo.hwndChild = hTool;
		rbBandInfo.wID = GetDlgCtrlID(hTool);

		//add the current band to the rebar.
		SendMessage(hwnd_rebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBandInfo);
		//Fallout_Debug_Info("tool bar loaded %dx%d", rbBandInfo.cxMinChild, rbBandInfo.cyMinChild);
	}
	fclose(pFile);

	return TRUE;
}
