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

#include "win_Mapper.h"
#include "win_ListBox.h"
#include "win_ToolBar.h"
#include "win_ProtoBrowser.h"
#include "win_Set_Ambient_Light.h"
#include "win_Edit_Light_Colour.h"
#include "win_ObjectEditor.h"

#include "configTools.h"

#include "mapper_tools.h"
#include "proto_cache.h"

#include "../resource.h"
#include "../win_fall.h"

#include "../modifications.h"
#include "../memwrite.h"

#include "../Fall_General.h"
#include "../Fall_GameMap.h"
#include "../Fall_File.h"
#include "../Fall_Scripts.h"

#include "../Dx_Mouse.h"
#include "../Dx_General.h"
#include "../Dx_Graphics.h"
#include "../Dx_Game.h"
#include "../Dx_Windows.h"

#include "../text.h"

#include "../game_map.h"

bool isRunAsMapper = false;
bool isMapperInitiated = false;

wchar_t winClassName_MapperMain[] = L"WCN MapperMain";
wchar_t winClassName_MapperGame[] = L"WCN MapperGame";

wchar_t windowName_Mapper[] = L"Fallout II Mapper";

HWND hDlgCurrent = nullptr;

char workingMapName[MAP_FILE_NAME_LENGTH_PLUS_EXT]{0};

SCROLLINFO mapperScrollInfoH = { 0 };  // mapper scroll info structure
SCROLLINFO mapperScrollInfoV = { 0 };  // mapper scroll info structure


//int status_bar_element_positions[] = { 150, 150 + 170, -1 };
//                                       m:  x   y  hex  mp: name start:hex  lev  ori scr: name
//int status_bar_element_positions[]={ 50, 40, 40,  70,  40,  110, 40,  70,  40,  40, 40, 130,-1 };

int status_bar_element_positions[] = { 50, 90, 130, 200, 240, 350, 390, 460, 500, 540, 580, -1 };

DWORD active_edge_version = 2;


//________________________________________
void Edges_SetActiveVersion(DWORD version) {
    active_edge_version = version;
}


//____________________________
DWORD Edges_GetActiveVersion() {
    return active_edge_version;
}



//___________________________________
LONG Get_Proto_Type_TextID(LONG type) {
    switch (type) {
    case ART_ITEMS:
        return IDS_TEXT_ITEMS;
        break;
    case ART_CRITTERS:
        return IDS_TEXT_CRITTERS;
        break;
    case ART_SCENERY:
        return IDS_TEXT_SCENERY;
        break;
    case ART_WALLS:
        return IDS_TEXT_WALLS;
        break;
    case ART_TILES:
        return IDS_TEXT_TILES;
        break;
    case ART_MISC:
        return IDS_TEXT_MISC;
        break;
    default:
        break;
    }
    return IDS_TEXT_ALL;
}

//__________________________________
LONG Get_Item_Type_TextID(LONG type) {
    switch (type) {
    case ITEM_TYPE::armor:
        return IDS_ITEM_TYPE_ARMOR;
        break;
    case ITEM_TYPE::container:
        return IDS_ITEM_TYPE_CONTAINER;
        break;
    case ITEM_TYPE::drug:
        return IDS_ITEM_TYPE_DRUG;
        break;
    case ITEM_TYPE::weapon:
        return IDS_ITEM_TYPE_WEAPON;
        break;
    case ITEM_TYPE::ammo:
        return IDS_ITEM_TYPE_AMMO;
        break;
    case ITEM_TYPE::misc:
        return IDS_ITEM_TYPE_MISC;
        break;
    case ITEM_TYPE::key:
        return IDS_ITEM_TYPE_KEY;
        break;
    default:
        break;
    }
    return IDS_TEXT_ALL;
}


//______________________________________________________________________________
bool WinData_Save(HWND hwnd_win, const wchar_t* _FileName, DWORD fileNameLength) {
    FILE* pFile = nullptr;
    
    if (!MapperData_OpenFile(&pFile, _FileName, fileNameLength, L"wb"))
        return false;
    
    WINDOWPLACEMENT winPlacement{ 0 };
    winPlacement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd_win, &winPlacement);
    fwrite(&winPlacement, sizeof(WINDOWPLACEMENT), 1, pFile);

    fclose(pFile);
    return true;
}


//______________________________________________________________________________
bool WinData_Load(HWND hwnd_win, const wchar_t* _FileName, DWORD fileNameLength) {
    FILE* pFile = nullptr;

    if (!MapperData_OpenFile(&pFile, _FileName, fileNameLength, L"rb"))
        return false;

    WINDOWPLACEMENT winPlacement{ 0 };
    winPlacement.length = sizeof(WINDOWPLACEMENT);

    if (fread(&winPlacement, sizeof(WINDOWPLACEMENT), 1, pFile) == 1) {
        if (winPlacement.showCmd != SW_MAXIMIZE)
            winPlacement.showCmd = SW_SHOWNORMAL;

        SetWindowPlacement(hwnd_win, &winPlacement);
    }

    fclose(pFile);
    return true;
}


//___________________
void Mapper_On_Exit() {
    wchar_t mapper_window_file[] = L"\\win_main.dat";
    WinData_Save(*phWinMain, mapper_window_file, _countof(mapper_window_file));
    
    
    Rebar_Save(*phWinMain);

    ConfigWriteInt(L"MAPPER", L"OBJECT_BROWSER_OPENED", IsObjectBrowserOpened());
    ConfigWriteInt(L"MAPPER", L"OBJECT_BROWSER_WIN_STATE", static_cast<int>(object_broser_state));

    ConfigWriteInt(L"MAPPER", L"AMBIENT_LIGHT_INTENSITY", Get_Mapper_Ambient_Light_Intensity());

    if(Edges_GetActiveVersion() == 2)
        ConfigWriteInt(L"MAPPER", L"HRP_EDGE_V2_COMPATABILITY_ENABLED", TRUE);
    else
        ConfigWriteInt(L"MAPPER", L"HRP_EDGE_V2_COMPATABILITY_ENABLED", FALSE);
        
    ProtoCache_Destroy();
}


//_____________________________
ATOM MapperMain_RegisterClass() {
    WNDCLASSEX WndClass{ 0 };
    WndClass.cbSize = sizeof(WNDCLASSEX);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;

    WndClass.lpfnWndProc = &WinProc_Mapper;
    WndClass.hInstance = phinstDLL;
    WndClass.hIcon = LoadIcon(phinstDLL, MAKEINTRESOURCE(IDI_ICON1));
    WndClass.hCursor = LoadCursor(phinstDLL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAIN);
    WndClass.hIconSm = LoadIcon(phinstDLL, MAKEINTRESOURCE(IDI_ICON1));

    WndClass.lpszClassName = winClassName_MapperMain;

    return RegisterClassEx(&WndClass);
}


//___________________________________________________________________________________________________________
bool MapperData_OpenFile(FILE** ppFile, const wchar_t* _FileName, DWORD fileNameLength, const wchar_t* _Mode) {

    wchar_t mapper_folder[] = L"\\mapper_data";

    DWORD pathLength = GetCurrentDirectory(0, nullptr) + _countof(mapper_folder) + fileNameLength;//get the path length
    wchar_t* path = new wchar_t[pathLength];
    GetCurrentDirectory(pathLength, path);
    wcsncat_s(path, pathLength, mapper_folder, _countof(mapper_folder));

    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectory(path, nullptr)) {
            path[0] = L'\0';
            return false;
        }
    }
    wcsncat_s(path, pathLength, _FileName, fileNameLength);
    //Fallout_Debug_Info("MapperData_OpenFile:%S length:%d", path, pathLength);
    errno_t error = 0;
    error = _wfopen_s(ppFile, path, _Mode);
        
    delete[] path;

    if(error)
        return false;

    return true;
}


//____________________________________________________________________________________________________________________________
bool MapperData_OpenFileStream(const wchar_t* _FileName, DWORD fileNameLength, DWORD grfMode, BOOL fCreate, IStream** pStream) {

    wchar_t mapper_folder[] = L"\\mapper_data";

    DWORD pathLength = GetCurrentDirectory(0, nullptr) + _countof(mapper_folder) + fileNameLength;//get the path length
    wchar_t* path = new wchar_t[pathLength];
    GetCurrentDirectory(pathLength, path);
    wcsncat_s(path, pathLength, mapper_folder, _countof(mapper_folder));

    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectory(path, nullptr)) {
            path[0] = L'\0';
            return false;
        }
    }
    wcsncat_s(path, pathLength, _FileName, fileNameLength);
    //Fallout_Debug_Info("MapperData_OpenFile:%S length:%d", path, pathLength);
    
    bool no_error = false;
    HRESULT result = SHCreateStreamOnFileEx(path, grfMode, FILE_ATTRIBUTE_NORMAL, fCreate, nullptr, pStream);
    if (result == S_OK)
        no_error = true;

    delete[] path;
    //Fallout_Debug_Info("MapperData_OpenFileStream HRESULT: %d", result);
    return no_error;
}


//____________________
bool LoadMapFromList() {

    bool retVal = false;
    char** maplist = nullptr;

    int numMaps = fall_FileList_Get("maps\\*.map", &maplist);

    //remove extension from map name
    char* pExt = nullptr;
    for (int num = 0; num < numMaps; num++) {
        pExt = strrchr(maplist[num], '.');
        if (pExt)
            *pExt = '\0';

    }

    int current_list_pos = ConfigReadInt(L"MAPPER", L"CURRENT_MAP_NUM", 0);
    if (current_list_pos < 0 || current_list_pos >= numMaps)
        current_list_pos = 0;

    current_list_pos = Dialog_ListSelect(*phWinMain, phinstDLL, 50, 50, "Select A Map:", maplist, numMaps, current_list_pos, MAP_FILE_NAME_LENGTH, nullptr, 0);

    if (current_list_pos >= 0 && current_list_pos < numMaps) {
        char* new_map_name = new char[MAP_FILE_NAME_LENGTH_PLUS_EXT]{0};
        sprintf_s(new_map_name, MAP_FILE_NAME_LENGTH_PLUS_EXT, "%s.map", maplist[current_list_pos]);
        if (fall_Map_Load(new_map_name) != -1) {
            strncpy_s(workingMapName, MAP_FILE_NAME_LENGTH_PLUS_EXT, new_map_name, MAP_FILE_NAME_LENGTH_PLUS_EXT);
            ConfigWriteInt(L"MAPPER", L"CURRENT_MAP_NUM", current_list_pos);
            //HWND hStatus = GetDlgItem(*phWinMain, IDC_STATUS_BAR);
            //if (hStatus)
            //    SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)workingMapName);
            //wchar_t w_msg[32];
            //swprintf_s(w_msg, L"%S", workingMapName);
            //StatusBar_SetText(5, w_msg);
            //StatusBar_Print_Map_Vars();
            retVal = true;
        }
        //stop the wind sound that was started while loading map data.
        fall_Background_Sound_Stop();
    }

    fall_FileList_Release(&maplist);
    maplist = nullptr;
    return retVal;
}


//______________
void SaveMapAs() {

    char** maplist = nullptr;

    int numMaps = fall_FileList_Get("maps\\*.map", &maplist);

    int listPos = -1;
    char* pExt = nullptr;
    for (int num = 0; num < numMaps; num++) {
        if (CompareCharArray_IgnoreCase(workingMapName, maplist[num], 8))
            listPos = num;
        pExt = strrchr(maplist[num], '.');
        if (pExt)
            *pExt = '\0';
    }

    char* retMapName = new char[MAP_FILE_NAME_LENGTH_PLUS_EXT] {0};
    listPos = Dialog_SaveAs(*phWinMain, phinstDLL, 50, 50, "Save As ... (No EXT)", maplist, numMaps, listPos, MAP_FILE_NAME_LENGTH, "\\/:*?<>|", retMapName, MAP_FILE_NAME_LENGTH_PLUS_EXT);


    if (retMapName[0] != '\0') {
        //check if an extension was included
        pExt = strrchr(retMapName, '.');
        if (pExt)
            *pExt = '\0';

        strncat_s(retMapName, MAP_FILE_NAME_LENGTH_PLUS_EXT, ".map", 4);

        //Fallout_Debug_Info("save as %s", retMapName);

        fall_Map_Save(retMapName);
        strncpy_s(workingMapName, retMapName, MAP_FILE_NAME_LENGTH_PLUS_EXT);
    }

    delete[] retMapName;

    fall_FileList_Release(&maplist);
    maplist = nullptr;
}

struct SPATIAL_SCRIPT_VARS {
    LONG scriptIndex;
    LONG hexPos;
    LONG level;
    LONG radius;
};

//________________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_Set_Spatial_Script(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static SPATIAL_SCRIPT_VARS* pVars = nullptr;
    switch (Message) {

    case WM_INITDIALOG: {
        //set position to centre of parent window.
        RECT rc_Win{ 0,0,0,0 };
        GetWindowRect(hwnd, &rc_Win);
        HWND hwnd_parent = GetParent(hwnd);
        RECT rcParent{ 0,0,0,0 };
        if (hwnd_parent)
            GetWindowRect(hwnd_parent, &rcParent);
        SetWindowPos(hwnd, nullptr, ((rcParent.right - rcParent.left) - (rc_Win.right - rc_Win.left)) / 2, ((rcParent.bottom - rcParent.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        pVars = (SPATIAL_SCRIPT_VARS*)lParam;

        SendMessage(GetDlgItem(hwnd, IDC_EDIT1), EM_LIMITTEXT, (WPARAM)2, (LPARAM)0);
        SetDlgItemInt(hwnd, IDC_EDIT1, pVars->radius, FALSE);

        SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), pVars->scriptIndex + 1);//+1 to offset none text at 0 position.

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_EDIT1: {
            if (HIWORD(wParam) == EN_UPDATE) {
                BOOL translated = false;
                int idc_edit = LOWORD(wParam);
                if (idc_edit != IDC_EDIT1)
                    break;
                LONG radius = GetDlgItemInt(hwnd, idc_edit, &translated, FALSE);
                if (translated) {
                    //keep radius from going over 99.
                    if (radius > 99) {
                        radius = 99;
                        SetDlgItemInt(hwnd, idc_edit, radius, FALSE);
                    }
                }
            }
            break;
        }
        case IDOK: {
            INT_PTR nResult = 1;
            pVars->scriptIndex = SendMessage(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            if (pVars->scriptIndex == CB_ERR)
                nResult = -1;
            pVars->scriptIndex -= 1;

            if (pVars->scriptIndex == -1)
                nResult = -1;

            BOOL translated = false;
            pVars->radius = GetDlgItemInt(hwnd, IDC_EDIT1, &translated, FALSE);
            if(!translated)
                nResult = -1;
            
            if (pVars->radius < 0)
                nResult = -1;


            EndDialog(hwnd, nResult);
            return FALSE;
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//_____________________________________________________________________________________
LONG Select_SpatialScript_From_List(HWND hwndParent, DWORD *pScriptID, LONG numScripts) {
    LONG selected_script = 0;
    if (numScripts > 1) {
        wchar_t** pw_scrList = new wchar_t* [numScripts];
        wchar_t* w_scrName = new wchar_t[128];
        SCRIPT_STRUCT* pScr = nullptr;
        for (int i = 0; i < numScripts; i++) {
            pw_scrList[i] = new wchar_t[138];
            if (fall_Script_Get(pScriptID[i], &pScr) == 0) {
                w_scrName[0] = '\0';
                Script_Get_Name(pScr->index, w_scrName, 128);
                swprintf_s(pw_scrList[i], 138, L"Radius:%d %s", pScr->udata.sp.radius, w_scrName);
            }
            else
                swprintf_s(pw_scrList[i], 138, L"Error - could not find active script.");
        }

        selected_script = Dialog_ListSelect_WC(hwndParent, phinstDLL, -1, -1, L"", pw_scrList, numScripts, 0, 138, nullptr, 0);

        for (int i = 0; i < numScripts; i++) {
            delete[] pw_scrList[i];
        }
        delete[] pw_scrList;
        delete[] w_scrName;
    }
    return selected_script;
}


//___________________________
BOOL Menu_Game_Map(HWND hwnd){


    OBJStruct* p_obj_under_mouse = nullptr;
    LONG tilenum_under_mouse = -1;
    BOOL isRoof = FALSE;
    MAP_OBJECT* p_mouse_object = ObjectUnderMouse_Get();

    if (p_mouse_object) {
        p_obj_under_mouse = p_mouse_object->GetObj();
        tilenum_under_mouse = p_mouse_object->GetTileNumber();
        isRoof = p_mouse_object->IsRoof();
    }

    POINT menu_pos{ 0,0 };
    GetCursorPos(&menu_pos);

    HMENU hMenuLoaded;
    HMENU hMenuSub;
    //Get the menu.
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_ON_MAP));
    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);


        //Get the submenu for the first menu item.
        HMENU hPopupMenu = hMenuSub;

        if (!SpatialScripts_List_At(nullptr, ((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL)) {
            EnableMenuItem(hPopupMenu, ID_SPATIALSCRIPT_EDITATHEX, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_SPATIALSCRIPT_DESTROYATHEX, MF_GRAYED);
        }
        

        if (p_mouse_object == nullptr) {
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDIT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDITLIGHTCOLOUR, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_CUT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_COPY, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_DELETEFROMMAP, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_GOTOPROTOTYPE, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_FINDOBJECTSWITHMATCHINGPROTO, MF_GRAYED);
        }
        else if (!p_obj_under_mouse) {
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDIT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDITLIGHTCOLOUR, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECTUNDER_SELECTOBJECTSWITHMATCHINGSCRIPT, MF_GRAYED);
            DrawMenuBar(hwnd);
        }

        if (Get_Selected_Objects() == nullptr && Get_Selected_Tiles() == nullptr) {
            EnableMenuItem(hPopupMenu, ID_SELECTED_CUT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_SELECTED_COPY, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_SELECTED_DELETE, MF_GRAYED);
        }
        if (Get_Copied_Objects() == nullptr && Get_Copied_Tiles() == nullptr) {
            EnableMenuItem(hPopupMenu, ID_COPIED_PASTE, MF_GRAYED);
        }

        //Show the menu and wait for input.
        BOOL menu_item = TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_NONOTIFY | TPM_RETURNCMD,
            menu_pos.x, menu_pos.y, hwnd, nullptr);

        DestroyMenu(hMenuLoaded);


        switch (menu_item) {
        case ID_OBJECT_EDIT:
            Dialog_ObjectEditor(*phWinMain, phinstDLL, p_obj_under_mouse);
            break;
        case ID_OBJECT_EDITLIGHTCOLOUR: {
            DWORD colour = ((OBJStructDx*)p_obj_under_mouse)->light_colour;
            //if objects light colour == 0"unset" then get the starting light colour from its proto. 
            if (colour == 0) {
                PROTO* pPro = nullptr;
                if (fall_GetPro(p_obj_under_mouse->proID, (PROTO**)&pPro) == -1)
                    return FALSE;
                colour = VE_PROTO_Get_Light_Colour(pPro);
            }
            if (EditLightColour(*phWinMain, phinstDLL, &colour)) {
                fall_Map_UpdateLightMap(p_obj_under_mouse, 1, nullptr);
                ((OBJStructDx*)p_obj_under_mouse)->light_colour = colour;
                RECT rc_obj{ 0,0,0,0 };
                GetObjRectDx(p_obj_under_mouse, &rc_obj);
                fall_Map_UpdateLightMap(p_obj_under_mouse, 0, &rc_obj);
                DrawMapChanges(&rc_obj, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof);
            }
            break;
        }
        case ID_OBJECT_CUT:
            if (ObjectUnderMouse_Copy()) {
                ObjectUnderMouse_Delete();
                Tool_SetState(*phWinMain, ID_EDIT_PASTE, TRUE, FALSE, TRUE);
                DrawMenuBar(*phWinMain);
            }
            break;
        case ID_OBJECT_COPY:
            if (ObjectUnderMouse_Copy()) {
                Tool_SetState(*phWinMain, ID_EDIT_PASTE, TRUE, FALSE, TRUE);
                DrawMenuBar(*phWinMain);
            }
            break;
        case ID_OBJECT_DELETEFROMMAP:
            ObjectUnderMouse_Delete();
            break;

        case ID_SELECTED_CUT:
            SendMessage(*phWinMain, WM_COMMAND, LOWORD(ID_EDIT_CUT), 0);
            break;
        case ID_SELECTED_COPY:
            SendMessage(*phWinMain, WM_COMMAND, LOWORD(ID_EDIT_COPY), 0);
            
            break;
        case ID_SELECTED_DELETE:
            SendMessage(*phWinMain, WM_COMMAND, LOWORD(ID_EDIT_DELETE), 0);
            break;
        case ID_COPIED_PASTE:
            SendMessage(*phWinMain, WM_COMMAND, LOWORD(ID_EDIT_PASTE), 0);
            break;

        case ID_OBJECT_GOTOPROTOTYPE: {
            if (p_obj_under_mouse)
                ProtoList_GoToProto(p_obj_under_mouse->proID);
            else if (tilenum_under_mouse != -1) 
                ProtoList_GoToTileProto_With_FrmID(GameMap_GetTileFrmID(tilenum_under_mouse, *pMAP_LEVEL, isRoof, nullptr));
            break;
        }
        case ID_SELECTOBJECTSONLEVEL_WITHMATCHINGPROTO:
            if (p_obj_under_mouse)
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::matching_proID, p_obj_under_mouse->proID);
            else if (tilenum_under_mouse != -1)
                SelectTiles_With_Matching_FRM(*pMAP_LEVEL, GameMap_GetTileFrmID(tilenum_under_mouse, *pMAP_LEVEL, isRoof, nullptr));
            break;
        case ID_SELECTOBJECTSONLEVEL_WITHMATCHINGSCRIPT:
            if (p_obj_under_mouse)
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::matching_scriptIndex, p_obj_under_mouse->scriptIndex);
            break;
        case ID_SELECTOBJECTSONMAP_WITHMATCHINGPROTO:
            if (p_obj_under_mouse)
                SelectObjects_On_Map(-1, OBJ_SEARCH::matching_proID, p_obj_under_mouse->proID);
            else if (tilenum_under_mouse != -1)
                SelectTiles_With_Matching_FRM(-1, GameMap_GetTileFrmID(tilenum_under_mouse, *pMAP_LEVEL, isRoof, nullptr));
            break;
        case ID_SELECTOBJECTSONMAP_WITHMATCHINGSCRIPT:
            if (p_obj_under_mouse)
                SelectObjects_On_Map(-1, OBJ_SEARCH::matching_scriptIndex, p_obj_under_mouse->scriptIndex);
            break;


        case ID_OBJECTSWITHSCRIPTS_ALL:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::none));
            break;
        case ID_OBJECTSWITHSCRIPTS_SYSTEM:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::system));
            break;
        case ID_OBJECTSWITHSCRIPTS_SPATIAL:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::spatial));
            break;
        case ID_OBJECTSWITHSCRIPTS_TIME:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::time));
            break;
        case ID_OBJECTSWITHSCRIPTS_ITEM:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::item));
            break;
        case ID_OBJECTSWITHSCRIPTS_CRITTER:
            SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::critter));
            break;

            //ID_OBJECTSWITHSCRIPTS_ALL
        case ID_SELECTALL_OBJECTSWITHINVENTORY:
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::has_inventory, 0);
            break;
        case ID_SELECTALL_OBJECTSEMITTINGLIGHT:
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::emits_light, 0);
            break;

        case ID_OBJECTSWITHSCRIPTS_ALL_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::none));
            break;
        case ID_OBJECTSWITHSCRIPTS_SYSTEM_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::system));
            break;
        case ID_OBJECTSWITHSCRIPTS_SPATIAL_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::spatial));
            break;
        case ID_OBJECTSWITHSCRIPTS_TIME_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::time));
            break;
        case ID_OBJECTSWITHSCRIPTS_ITEM_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::item));
            break;
        case ID_OBJECTSWITHSCRIPTS_CRITTER_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_scriptID, static_cast<LONG>(SCRIPT_TYPE::critter));
            break;

            //ID_OBJECTSWITHSCRIPTS_ALL
        case ID_SELECTALL_OBJECTSWITHINVENTORY_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::has_inventory, 0);
            break;
        case ID_SELECTALL_OBJECTSEMITTINGLIGHT_ONMAP:
            SelectObjects_On_Map(-1, OBJ_SEARCH::emits_light, 0);
            break;

        case ID_SPATIALSCRIPT_CREATEATHEX: {
            if (((OBJStruct*)*ppObj_Mouse)) {
                SPATIAL_SCRIPT_VARS vars{ -1,((OBJStruct*)*ppObj_Mouse)->hexNum,*pMAP_LEVEL,0 };
                if (DialogBoxParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_CREATE_SPATIAL_SCRIPT), hwnd, DlgProc_Set_Spatial_Script, (LPARAM)&vars) == 1) {
                    if (SpatialScript_Create(vars.scriptIndex, vars.hexPos, vars.level, vars.radius) == FALSE)
                        Fallout_Debug_Error("ID_SPATIALSCRIPT_CREATEATHEX SpatialScript_Create - failed");
                    else
                        Fallout_Debug_Info("ID_SPATIALSCRIPT_CREATEATHEX SpatialScript_Create num active scripts:%d", fall_Get_Number_Of_Active_Scripts(static_cast<LONG>(SCRIPT_TYPE::spatial)));
                }
            }
            break;
        }
        case ID_SPATIALSCRIPT_EDITATHEX:
            if (((OBJStruct*)*ppObj_Mouse)) {
                DWORD* pScriptID = nullptr;
                LONG numScripts = SpatialScripts_List_At(&pScriptID, ((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL);
                if (numScripts) {
                    LONG script_to_edit = Select_SpatialScript_From_List(hwnd, pScriptID, numScripts);

                    if (script_to_edit >= 0 && script_to_edit < numScripts) {
                        SCRIPT_STRUCT* pScr = nullptr;
                        if (fall_Script_Get(pScriptID[script_to_edit], &pScr) == 0) {
                            SPATIAL_SCRIPT_VARS vars{ pScr->index,pScr->udata.sp.build_tile & 0x03FFFFFF,(pScr->udata.sp.build_tile & 0xE0000000) >> 0x1D,pScr->udata.sp.radius };
                            if (DialogBoxParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_CREATE_SPATIAL_SCRIPT), hwnd, DlgProc_Set_Spatial_Script, (LPARAM)&vars) == 1) {
                                if (fall_Script_Remove(pScr->id) == 0)
                                    SpatialScripts_Destroy_Marker_Objects(((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL, TRUE);
                                if (SpatialScript_Create(vars.scriptIndex, vars.hexPos, vars.level, vars.radius) == FALSE)
                                    Fallout_Debug_Error("ID_SPATIALSCRIPT_EDITATHEX SpatialScript_Create - failed");
                            }
                        }
                    }
                    delete[] pScriptID;
                }
            }
            break;
        case ID_SPATIALSCRIPT_DESTROYATHEX:
            if (((OBJStruct*)*ppObj_Mouse)) {
                DWORD* pScriptID = nullptr;
                LONG numScripts = SpatialScripts_List_At(&pScriptID, ((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL);
                if (numScripts) {
                    LONG script_to_remove = Select_SpatialScript_From_List(hwnd, pScriptID, numScripts);
                    if (script_to_remove >= 0 && script_to_remove < numScripts) {
                        SCRIPT_STRUCT* pScr = nullptr;
                        if (fall_Script_Get(pScriptID[script_to_remove], &pScr) == 0) {
                            if (fall_Script_Remove(pScr->id) == 0)
                                SpatialScripts_Destroy_Marker_Objects(((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL, TRUE);
                        }
                    }
                    delete[] pScriptID;
                }
            }
        case ID_MAPMENU_SETSTARTHEX:
            Map_Set_Start_Vars(((OBJStruct*)*ppObj_Mouse)->hexNum, *pMAP_LEVEL, -1);
            StatusBar_Print_Map_Vars();
            break;
            //
            break;
        default:
            break;
        }
    }

    return TRUE;
}


//____________________________________
void Mapper_UpdateScrollBarPositions() {

    if (!isRunAsMapper)
        return;
    if (isGameMode)
        return;
    GAME_AREA* pArea_Mapper = GameAreas_GetCurrentArea();
    if (!pArea_Mapper)
        return;

    mapperScrollInfoH.nMax = pArea_Mapper->rect.right;// 3200;
    mapperScrollInfoH.nMin = pArea_Mapper->rect.left;//-4800;
    mapperScrollInfoV.nMax = pArea_Mapper->rect.bottom;//3600;
    mapperScrollInfoV.nMin = pArea_Mapper->rect.top;//0;

    LONG x, y;
    HexNumToSqr_Scroll(*pVIEW_HEXPOS, &x, &y);
    mapperScrollInfoH.nPos = x;
    mapperScrollInfoV.nPos = y;

    SetScrollInfo(hGameWnd, SB_HORZ, &mapperScrollInfoH, true);
    SetScrollInfo(hGameWnd, SB_VERT, &mapperScrollInfoV, true);
}


//_________________________________________________________________________________
LRESULT CALLBACK WinProcGame(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    switch (Message) {

    case WM_CREATE:
        hGameWnd = hwnd;
        SetWindowTitle(GetParent(hwnd), L"");
        return 0;

    case WM_LBUTTONDOWN:
        if (isMapperInitiated) {
            pointerState.flags = pointerState.flags | 0x1;
            SetCapture(hwnd);
            SelectedAction_Preform();
        }
        return 0;
    case WM_RBUTTONDOWN:
        if (isMapperInitiated) {
            SelectedAction_End(TRUE);
            pointerState.flags = pointerState.flags | 0x100;
            if (Get_PerformingAction() == PERFORMING_ACTION::none)
                Menu_Game_Map(hwnd);
        }
        return 0;
    case WM_LBUTTONUP:
        if (isMapperInitiated) {
            SelectedAction_End(FALSE);
            ReleaseCapture();
            pointerState.flags = pointerState.flags & 0xFFFFFFFE;
        }
        return 0;
    case WM_RBUTTONUP:
        if (isMapperInitiated)
            pointerState.flags = pointerState.flags & 0xFFFFFEFF;
        return 0;
    case WM_MOUSEMOVE:
        if (isMapperInitiated)
            SelectedAction_Update();
        return 0;

    case WM_HSCROLL:
    case WM_VSCROLL: {
        if (!isMapperInitiated)
            break;
        Mapper_UpdateScrollBarPositions();
        int xPos = mapperScrollInfoH.nPos;
        int yPos = mapperScrollInfoV.nPos;
        bool axis;
        if (Message == WM_HSCROLL) {
            axis = 0;
            switch (LOWORD(wParam)) {
                // User clicked the scroll bar shaft left of the scroll box.
            case SB_PAGEUP:
                xPos -= 32;
                break;
                // User clicked the scroll bar shaft right of the scroll box.
            case SB_PAGEDOWN:
                xPos += 32;
                break;
                // User clicked the left arrow.
            case SB_LINEUP:
                xPos -= 32;
                break;

                // User clicked the right arrow.
            case SB_LINEDOWN:
                xPos += 32;
                break;
                // User dragged the scroll box.
            case SB_THUMBPOSITION:
                xPos = (__int16)HIWORD(wParam);
                break;
                // User is dragging the scroll box.
            case SB_THUMBTRACK:
                xPos = (__int16)HIWORD(wParam);
                break;
            default:
                break;
            }
        }
        else if (Message == WM_VSCROLL) {
            axis = 1;
            switch (LOWORD(wParam)) {
                // User clicked the scroll bar shaft left of the scroll box.
            case SB_PAGEUP:
                yPos -= 24;
                break;
                // User clicked the scroll bar shaft right of the scroll box.
            case SB_PAGEDOWN:
                yPos += 24;
                break;
                // User clicked the left arrow.
            case SB_LINEUP:
                yPos -= 24;
                break;
                // User clicked the right arrow.
            case SB_LINEDOWN:
                yPos += 24;
                break;
                // User dragged the scroll box.
            case SB_THUMBPOSITION:
                yPos = (__int16)HIWORD(wParam);
                break;
                // User is dragging the scroll box.
            case SB_THUMBTRACK:
                yPos = (__int16)HIWORD(wParam);
                break;
            default:
                break;
            }
        }

        if (xPos < mapperScrollInfoH.nMin) xPos = mapperScrollInfoH.nMin;
        else if (xPos > mapperScrollInfoH.nMax) xPos = mapperScrollInfoH.nMax;

        if (yPos < mapperScrollInfoV.nMin) yPos = mapperScrollInfoV.nMin;
        else if (yPos > mapperScrollInfoV.nMax) yPos = mapperScrollInfoV.nMax;

        SetViewPosition_Hex(SqrToHexNum_Scroll(xPos, yPos), 0);
        break;
    }
    case WM_WINDOWPOSCHANGED: {
        HWND parent = GetParent(hwnd);

        if (IsIconic(parent))
            break;
        WINDOWPOS* winpos = (WINDOWPOS*)lParam;
        if (winpos->flags & (SWP_NOSIZE)) {
            SetWindowTitle(parent, L"");
            return 0;
        }
        //Fallout_Debug_Info("WM_WINDOWPOSCHANGED");
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);

        DWORD old_SCR_WIDTH = SCR_WIDTH;
        DWORD old_SCR_HEIGHT = SCR_HEIGHT;
        Set_Fallout_Screen_Dimensions((rcClient.right - rcClient.left) / scaleLevel_GUI, (rcClient.bottom - rcClient.top) / scaleLevel_GUI);

        bool isMapperSizing_temp = isMapperSizing;
        isMapperSizing = true;
        ReSizeDisplayEx();
        OnScreenResize_Windows(old_SCR_WIDTH, old_SCR_HEIGHT);
        isMapperSizing = isMapperSizing_temp;

        SetWindowTitle(parent, L"");

        return 0;
    }

    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        return 0;
    case WM_PAINT:
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, Message, wParam, lParam);
}



//______________________________________________________________________________________
INT_PTR CALLBACK DlgProc_AboutBox(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {


    switch (Message) {

    case WM_INITDIALOG: {
        //set position to centre of parent window.
        RECT rc_Win{ 0,0,0,0 };
        GetWindowRect(hwnd, &rc_Win);
        HWND hwnd_parent = GetParent(hwnd);
        RECT rcParent{ 0,0,0,0 };
        if (hwnd_parent)
            GetWindowRect(hwnd_parent, &rcParent);
        SetWindowPos(hwnd, nullptr, ((rcParent.right - rcParent.left) - (rc_Win.right - rc_Win.left)) / 2, ((rcParent.bottom - rcParent.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: 
            EndDialog(hwnd, 1);
            return FALSE;
            break;
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//__________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_SetMapScript(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    switch (Message) {

    case WM_INITDIALOG: {
        //set position to centre of parent window.
        RECT rc_Win{ 0,0,0,0 };
        GetWindowRect(hwnd, &rc_Win);
        HWND hwnd_parent = GetParent(hwnd);
        RECT rcParent{ 0,0,0,0 };
        if (hwnd_parent)
            GetWindowRect(hwnd_parent, &rcParent);
        SetWindowPos(hwnd, nullptr, ((rcParent.right - rcParent.left) - (rc_Win.right - rc_Win.left)) / 2, ((rcParent.bottom - rcParent.top) - (rc_Win.bottom - rc_Win.top)) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        MAP_HEADER* p_map = Get_Map_Header();
        //would normally add 1 to the scriptIndex to offset for the none option in the created list. But the scriptIndex stored in the map header is offset + 1 for some reason.
        SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), p_map->scriptIndex);

        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            LONG scriptIndex = SendMessage(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            //would normally subtract 1 to the scriptIndex to offset for the none option in the list. But the scriptIndex stored in the map header is offset + 1 for some reason.
            if (scriptIndex != CB_ERR)
                fall_Set_Map_Script(scriptIndex);

            EndDialog(hwnd, 1);
            return FALSE;
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//______________________________________
int CheckMapperkeys(HWND hwnd, WORD key) {
    if (!isMapperInitiated)
        return -1;
    if (isGameMode)
        return -1;
    bool redrawFlag = false;
    DWORD draw_flags = FLG_Floor | FLG_Obj | FLG_Roof | FLG_Hud | FLG_Hud_Outline;

    bool redraw_menu = false;
    int list_var = 0;

    switch (key) {

    case ID_FILE_NEW:
        SpatialScripts_Destroy_Marker_Objects(-1, -1, FALSE);
        Selected_List_Destroy(FALSE);
        ObjectUnderMouse_Clear();
        //fall_scripts_game_clear();
        //fall_script_clear_dude_script();
        fall_map_reset();  //*pCurrentMapName[0]= '\0', in F_ResetNewMap();
        //F_ResetIfaceBar();
        workingMapName[0] = '\0';
        GameAreas_Load_Default();
        GameAreas_Load_Mapper();
        MapLight_Release_RenderTargets();
        fall_Map_SetAmbientLightIntensity(Get_Mapper_Ambient_Light_Intensity(), 1);
        StatusBar_Print_Map_Vars();
        StatusBar_Print_Map_Script_Name();
        redrawFlag = true;
        break;
    case ID_FILE_OPEN: {
        //fall_scripts_game_clear();
        //fall_script_clear_dude_script();
        SpatialScripts_Destroy_Marker_Objects(-1, -1, FALSE);
        if (LoadMapFromList()) {
            SpatialScripts_Create_Marker_Objects();
            Selected_List_Destroy(FALSE);
            ObjectUnderMouse_Clear();
            if (AreBlockerObjectsVisible())
                Show_BlockerObjects();
            fall_Map_SetAmbientLightIntensity(Get_Mapper_Ambient_Light_Intensity(), 1);
            StatusBar_Print_Map_Vars();
            StatusBar_Print_Map_Script_Name();
            redrawFlag = true;
        }
        break;
    }
    case ID_FILE_SAVE: {
        if(AreBlockerObjectsVisible())
            Hide_BlockerObjects();
        if (workingMapName[0] != '\0')
            fall_Map_Save(workingMapName);
        else
            SaveMapAs();
        GameAreas_Save(workingMapName, Edges_GetActiveVersion());
        if (AreBlockerObjectsVisible())
            Show_BlockerObjects();
        StatusBar_Print_Map_Vars();
        break;
    }
    case ID_FILE_SAVEAS:
        if (AreBlockerObjectsVisible())
            Hide_BlockerObjects();
        SaveMapAs();
        GameAreas_Save(workingMapName, Edges_GetActiveVersion());
        if (AreBlockerObjectsVisible())
            Show_BlockerObjects();
        StatusBar_Print_Map_Vars();
        break;
    case ID_FILE_QUIT:
        Selected_List_Destroy(FALSE);
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;

    //edit menu
    case ID_EDIT_CUT:
        CopiedObjects_CopySelected();
        CopiedTiles_CopySelected();
        SelectedObjects_Delete();
        Tool_SetState(hwnd, ID_EDIT_PASTE, TRUE, FALSE, FALSE);
        Tool_SetState_ObjectsSelected(hwnd, FALSE, TRUE);
        redraw_menu = true;
        break;
    case ID_EDIT_COPY:
        CopiedObjects_CopySelected();
        CopiedTiles_CopySelected();
        Tool_SetState(hwnd, ID_EDIT_PASTE, TRUE, FALSE, TRUE);
        redraw_menu = true;
        break;
    case ID_EDIT_PASTE:
        Action_Initiate_CopiedObjects_Pasting();
        break;
    case ID_EDIT_DELETE:
        SelectedObjects_Delete();
        Tool_SetState_ObjectsSelected(hwnd, FALSE, TRUE);
        break;
    case ID_EDIT_COPYLEVEL:
        CopyLevel_Copy(*pMAP_LEVEL);
        Tool_SetState(hwnd, ID_EDIT_PASTELEVEL, TRUE, FALSE, TRUE);
        redraw_menu = true;
        break;
    case ID_EDIT_PASTELEVEL:
        CopyLevel_Paste(*pMAP_LEVEL);
        break;
    
    case ID_EDIT_ROTATECLOCKWISE:

        if (Get_PerformingAction() == PERFORMING_ACTION::create_new_object)
            PlaceNewObject_Rotate_Clockwise();
        else {
            OBJNode* p_selected_obj = Get_Selected_Objects();
            if (p_selected_obj != nullptr && p_selected_obj->next == nullptr) {
                if (p_selected_obj->pObj) {
                    LONG ori = p_selected_obj->pObj->ori;
                    if (ori < 5)
                        ori++;
                    else
                        ori = 0;

                    FRMCached* pfrm = new FRMCached(p_selected_obj->pObj->frmID);
                    FRMframeDx* pFrame = pfrm->GetFrame(ori, 0);
                    if (pFrame) {
                        RECT rc_obj1{ 0,0,0,0 };
                        fall_Obj_GetRect(p_selected_obj->pObj, &rc_obj1);
                        p_selected_obj->pObj->ori = ori;
                        RECT rc_obj2{ 0,0,0,0 };
                        fall_Obj_GetRect(p_selected_obj->pObj, &rc_obj2);
                        UnionRect(&rc_obj1, &rc_obj1, &rc_obj2);
                        DrawMapChanges(&rc_obj1, p_selected_obj->pObj->level, FLG_Floor | FLG_Obj);
                    }
                    if(pfrm)
                        delete pfrm;
                    pfrm = nullptr;

                }
            }
        }
        break;
    case ID_EDIT_ROTATEANTICLOCKWISE:
        if (Get_PerformingAction() == PERFORMING_ACTION::create_new_object)
            PlaceNewObject_Rotate_AntiClockwise();
        else {
            OBJNode* p_selected_obj = Get_Selected_Objects();
            if (p_selected_obj != nullptr && p_selected_obj->next == nullptr) {
                if (p_selected_obj->pObj) {
                    LONG ori = p_selected_obj->pObj->ori;
                    if (ori > 0)
                        ori--;
                    else
                        ori = 5;

                    FRMCached* pfrm = new FRMCached(p_selected_obj->pObj->frmID);
                    FRMframeDx* pFrame = pfrm->GetFrame(ori, 0);
                    if (pFrame) {
                        RECT rc_obj1{ 0,0,0,0 };
                        fall_Obj_GetRect(p_selected_obj->pObj, &rc_obj1);
                        p_selected_obj->pObj->ori = ori;
                        RECT rc_obj2{ 0,0,0,0 };
                        fall_Obj_GetRect(p_selected_obj->pObj, &rc_obj2);
                        UnionRect(&rc_obj1, &rc_obj1, &rc_obj2);
                        DrawMapChanges(&rc_obj1, p_selected_obj->pObj->level, FLG_Floor | FLG_Obj);
                    }
                    if (pfrm)
                        delete pfrm;
                    pfrm = nullptr;

                }
            }
        }
        break;

        //view menu
    case ID_VIEW_ROOVES:
        Tool_SetState(hwnd, ID_VIEW_ROOVES, TRUE, ToggleRooves(), FALSE);
        redraw_menu = true;
        break;
    case ID_VIEW_HEXES:
        Tool_SetState(hwnd, ID_VIEW_HEXES, TRUE, ToggleHexes(), FALSE);
        redraw_menu = true;
        redrawFlag = true;
        break;
    case ID_VIEW_BLOCKERS:
        Tool_SetState(hwnd, key, TRUE, Toggle_BlockerObjects(), FALSE);
        redraw_menu = true;
        redrawFlag = true;
        draw_flags = FLG_Floor | FLG_Obj;
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
        Tool_SetState(hwnd, key, TRUE, ToggleArtTypeEnabled(list_var), FALSE);
        redraw_menu = true;
        redrawFlag = true;
        break;

    case ID_LEVEL_3:
        list_var++;
    case ID_LEVEL_2:
        list_var++;
    case ID_LEVEL_1:
        if (fall_Map_SetLevel(list_var) != -1) {
            SetViewPosition_Hex(*pVIEW_HEXPOS, 0);
            redrawFlag = true;
        }
        break;
    case ID_LEVEL_UP: {
        if (*pMAP_LEVEL > 0) {
            if (fall_Map_SetLevel(*pMAP_LEVEL - 1) != -1) {
                SetViewPosition_Hex(*pVIEW_HEXPOS, 0);
                redrawFlag = true;
            }
        }
        break;
    }
    case ID_LEVEL_DOWN: {
        if (*pMAP_LEVEL < 2) {
            if (fall_Map_SetLevel(*pMAP_LEVEL + 1) != -1) {
                SetViewPosition_Hex(*pVIEW_HEXPOS, 0);
                redrawFlag = true;
            }
        }
        break;
    }
                      
    case ID_VIEW_SETMAPPERLIGHTINTENSITY: {
        
        DWORD intensity = Get_Mapper_Ambient_Light_Intensity();
        if (Set_Ambient_Light_Intensity(hwnd, phinstDLL, &intensity)) {
            Set_Mapper_Ambient_Light_Intensity(intensity);
            fall_Map_SetAmbientLightIntensity(Get_Mapper_Ambient_Light_Intensity(), 1);
            redrawFlag = true;
        }
        break;
    }

     //tools menu
   case ID_SELECTIONTOOLOPTIONS_WINDOWORIENTATEDRECT:
        Set_SelectionRectType(SEL_RC_TYPE::square);
        Tool_SetState(hwnd, ID_SELECTIONTOOLOPTIONS_WINDOWORIENTATEDRECT, TRUE, TRUE, FALSE);
        Tool_SetState(hwnd, ID_SELECTIONTOOLOPTIONS_PERSPECTIVEORIENTATEDRECT, TRUE, FALSE, FALSE);
        redraw_menu = true;
        break;
    case ID_SELECTIONTOOLOPTIONS_PERSPECTIVEORIENTATEDRECT:
        Set_SelectionRectType(SEL_RC_TYPE::isometric);
        Tool_SetState(hwnd, ID_SELECTIONTOOLOPTIONS_WINDOWORIENTATEDRECT, TRUE, FALSE, FALSE);
        Tool_SetState(hwnd, ID_SELECTIONTOOLOPTIONS_PERSPECTIVEORIENTATEDRECT, TRUE, TRUE, FALSE);
        redraw_menu = true;
        break;
    case ID_OBJECTTYPESSELECTABLE_ALLTYPESSELECTABLE: {
        ObjectTypeSelectable_ToggleAll();
        for (LONG type = 0; type < 6; type++)
            Tool_SetState(hwnd, wintool_select_type[type], TRUE, IsObjectTypeSelectable(type), FALSE);
        redraw_menu = true;
        break;
    }
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
        Tool_SetState(hwnd, key, TRUE, ObjectTypeSelectable_Toggle(list_var), FALSE);
        redraw_menu = true;
        break;

    case ID_HRP_TOGGLEVISIBILITYOFEDGES:
        Tool_SetState(hwnd, key, TRUE, ToggleDrawAllEdges(), FALSE);
        Tool_SetState_Edges_Visible(hwnd, AreAllEdgesVisible(), FALSE);
        redraw_menu = true;
        redrawFlag = true;
        draw_flags = FLG_Hud_Outline;
        break;
    case ID_HRP_CYCLESELECTEDEDGERECT:
        if (!AreAllEdgesVisible())
            break;
        ToggleSelectedEdges();
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_ADDNEWEDGERECT:
        if (!AreAllEdgesVisible())
            break;
        MapArea_AddNew();
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_DELETESELECTEDEDGERECT:
        if (!AreAllEdgesVisible())
            break;
        MapArea_DeleteSelected();
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_EDITLEFTEDGEOFSELECTEDRECT:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::square, EDGE_SELECT::left);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITRIGHTEDGEOFSELECTEDRECT:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::square, EDGE_SELECT::right);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITTOPEDGEOFSELECTEDRECT:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::square, EDGE_SELECT::top);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITBOTTOMEDGEOFSELECTEDRECT:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::square, EDGE_SELECT::bottom);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;

    case ID_HRP_EDITISOMETRICMAPEDGES_WEST:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::isometric, EDGE_SELECT::left);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITISOMETRICMAPEDGES_NORTH:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::isometric, EDGE_SELECT::top);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITISOMETRICMAPEDGES_EAST:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::isometric, EDGE_SELECT::right);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;
    case ID_HRP_EDITISOMETRICMAPEDGES_SOUTH:
        if (!AreAllEdgesVisible())
            break;
        Action_Initiate_AdjustMapEdge(SEL_RC_TYPE::isometric, EDGE_SELECT::bottom);
        Tool_SetState_Edges_SelectEdge(hwnd, FALSE);
        redraw_menu = true;
        break;

    case ID_HRP_ISOEDGE_FLAG_WEST:
        if (!AreAllEdgesVisible())
            break;
        Tool_SetState(hwnd, key, TRUE, ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT::left, *pMAP_LEVEL), FALSE);
        redraw_menu = true;
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_ISOEDGE_FLAG_NORTH:
        if (!AreAllEdgesVisible())
            break;
        Tool_SetState(hwnd, key, TRUE, ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT::top, *pMAP_LEVEL), FALSE);
        redraw_menu = true;
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_ISOEDGE_FLAG_EAST:
        if (!AreAllEdgesVisible())
            break;
        Tool_SetState(hwnd, key, TRUE, ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT::right, *pMAP_LEVEL), FALSE);
        redraw_menu = true;
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_HRP_ISOEDGE_FLAG_SOUTH:
        if (!AreAllEdgesVisible())
            break;
        Tool_SetState(hwnd, key, TRUE, ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT::bottom, *pMAP_LEVEL), FALSE);
        redraw_menu = true;
        draw_flags = FLG_Hud_Outline;
        redrawFlag = true;
        break;
    case ID_REBUILDPROTOCACHE_ALL:
        ProtoCache_Rebuild(-1);
        break;
    case ID_REBUILDPROTOCACHE_MISC:
        list_var++;
    case ID_REBUILDPROTOCACHE_TILES:
        list_var++;
    case ID_REBUILDPROTOCACHE_WALLS:
        list_var++;
    case ID_REBUILDPROTOCACHE_SCENERY:
        list_var++;
    case ID_REBUILDPROTOCACHE_CRITTERS:
        list_var++;
    case ID_REBUILDPROTOCACHE_ITEMS:
        ProtoCache_Rebuild(list_var);
        break;
    //tool scripts
        //
    case ID_SCRIPTS_SETMAPSCRIPT:
        DialogBoxParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_SET_MAP_SCRIPT), hwnd, DlgProc_SetMapScript, (LPARAM)nullptr);
        StatusBar_Print_Map_Script_Name();
        break;

    //windows
    case ID_OBJECTBROWSER_OPEN: {
        //Fallout_Debug_Info("ID_OBJECTBROWSER_OPEN");
        ObjectBrowser_Toggle_Open();
        break;
    }
    case ID_OBJECTBROWSER_UNDOCKED:
        list_var++;
    case ID_OBJECTBROWSER_RIGHT:
        list_var++;
    case ID_OBJECTBROWSER_BOTTOM:
        ObjectBrowser_SetView(static_cast<WINDOW_STATE>(list_var));
        break;




    case ID_HELP_ABOUT: {
        DialogBoxParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hwnd, DlgProc_AboutBox, (LPARAM)nullptr);
        break;
    }
    default:
        break;
    }


    if(redraw_menu)
        DrawMenuBar(hwnd);

    if (redrawFlag)
        DrawMapChanges(nullptr, *pMAP_LEVEL, draw_flags);
    
    return 0;
}


//________________________________________________________
BOOL StatusBar_SetText(WORD position, const wchar_t* _msg) {
    HWND hStatus = GetDlgItem(*phWinMain, IDC_STATUS_BAR);
    if (hStatus) {
        return SendMessage(hStatus, SB_SETTEXT, position, (LPARAM)_msg);
    }
    return FALSE;
}


//_____________________________
void StatusBar_Print_Map_Vars() {
    MAP_HEADER* pMap = Get_Map_Header();
    if (!pMap)
        return;
    wchar_t w_msg[16];
    //wchar_t w_msg[32];
    swprintf_s(w_msg, L"%S", pMap->name);
    StatusBar_SetText(5, w_msg);
    swprintf_s(w_msg, L"hex:%d", pMap->pcHexPos);
    StatusBar_SetText(7, w_msg);
    swprintf_s(w_msg, L"lev:%d", pMap->pcLevel + 1);
    StatusBar_SetText(8, w_msg);
    swprintf_s(w_msg, L"ori:%d", pMap->pcOri);
    StatusBar_SetText(9, w_msg);
}

//____________________________________
void StatusBar_Print_Map_Script_Name() {
    MAP_HEADER* pMap = Get_Map_Header();
    if (!pMap)
        return;
    wchar_t* p_w_msg = new wchar_t[128];
    //need to subtract 1 here, as the scriptIndex stored in the map header is offset + 1 for some reason.
    if (Script_Get_Name(pMap->scriptIndex - 1, p_w_msg, 128))
        StatusBar_SetText(11, p_w_msg);
    delete[] p_w_msg;
}


//___________________________________________________
HWND Create_StatusBar(HWND hwnd, HINSTANCE hinstance) {

    HWND hStatus = CreateWindowEx(0, STATUSCLASSNAME, nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
        hwnd, (HMENU)IDC_STATUS_BAR, hinstance, nullptr);

    if (hStatus == nullptr) {
        MessageBox(nullptr, L"Status Bar Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return nullptr;
    }

    SendMessage(hStatus, SB_SETPARTS, sizeof(status_bar_element_positions) / sizeof(int), (LPARAM)status_bar_element_positions);


    StatusBar_SetText(0, L"Mouse:");
    StatusBar_SetText(4, L"Map:");
    StatusBar_SetText(6, L"Start:");
    StatusBar_SetText(10, L"Script:");
    //StatusBar_SetText(5, L"*.Map");
    return hStatus;
}


//________________________________________________________________________________
HWND Create_GameWindow_Mapper(HWND hwnd_parent, HINSTANCE hinstance, RECT* pRcWin) {

    WNDCLASSEX cWndClass{ 0 };

    cWndClass.cbSize = sizeof(WNDCLASSEX);
    cWndClass.style = 0;

    cWndClass.lpfnWndProc = &WinProcGame;
    cWndClass.cbClsExtra = 0;
    cWndClass.cbWndExtra = 0;
    cWndClass.hInstance = hinstance;
    cWndClass.hIcon = nullptr;
    cWndClass.hCursor = LoadCursor(hinstance, IDC_ARROW);
    cWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cWndClass.lpszMenuName = nullptr;// MAKEINTRESOURCEA(IDC_WIN_GAME);
    cWndClass.lpszClassName = winClassName_MapperGame;
    cWndClass.hIconSm = nullptr;

    if (!RegisterClassEx(&cWndClass)) {
        DWORD lastError = GetLastError();
        wchar_t msg[32];
        swprintf_s(msg, L"last error %d", lastError);
        
        MessageBox(nullptr, msg, L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return nullptr;
    }


    HWND hwnd = CreateWindowEx(0, winClassName_MapperGame, 0,
        WS_TABSTOP | WS_CLIPSIBLINGS | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
        pRcWin->left, pRcWin->top,
        pRcWin->right - pRcWin->left, pRcWin->bottom - pRcWin->top, hwnd_parent, (HMENU)IDC_WIN_GAME, hinstance, nullptr);


    if (hwnd == nullptr) {
        MessageBox(nullptr, L"Game Window Creation Failed!", L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return nullptr;
    }

    ShowWindow(hwnd, SW_SHOW);

    mapperScrollInfoH.cbSize = sizeof(SCROLLINFO);
    mapperScrollInfoH.fMask = SIF_ALL;
    mapperScrollInfoH.nMax = 8000;
    mapperScrollInfoH.nMin = 0;
    mapperScrollInfoH.nPage = 32;
    mapperScrollInfoH.nPos = 4000;
    mapperScrollInfoH.nTrackPos = 0;
    SetScrollInfo(hwnd, SB_HORZ, &mapperScrollInfoH, true);

    mapperScrollInfoV.cbSize = sizeof(SCROLLINFO);
    mapperScrollInfoV.fMask = SIF_ALL;
    mapperScrollInfoV.nMax = 3600;
    mapperScrollInfoV.nMin = 0;
    mapperScrollInfoV.nPage = 24;
    mapperScrollInfoV.nPos = 1800;
    mapperScrollInfoV.nTrackPos = 0;
    SetScrollInfo(hwnd, SB_VERT, &mapperScrollInfoV, true);

    return hwnd;
}


//__________________________________________________________________
BOOL Menu_Dropdown_LevelSelect(HWND hwnd_parent, LPNMTOOLBAR lpnmTB) {

    if (isGameMode)
        return FALSE;

    if (lpnmTB->iItem != ID_LEVEL_SELECT)
        return FALSE;
    //Get button coordinates.
    RECT rc{ 0 };
    SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc);

    //Convert to screen coordinates.
    MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);

    //Get the menu.
    HMENU hMenuLoaded;
    HMENU hMenuSub;
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_LEVEL));
    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);

        HMENU hPopupMenu = hMenuSub;

        //Set up the pop-up menu.
        //In case the toolbar is too close to the bottom of the screen,
        //set rcExclude equal to the button rectangle and the menu will appear above
        //the button, and not below it.
        TPMPARAMS tpm{ 0 };
        tpm.cbSize = sizeof(TPMPARAMS);
        tpm.rcExclude = rc;

        //Show the menu and wait for input.
        //If the user selects an item, its WM_COMMAND is sent.
        TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
            rc.left, rc.bottom, hwnd_parent, &tpm);

        DestroyMenu(hMenuLoaded);
    }

    return TRUE;
}



//____________________________________________________________
BOOL Menu_Edit_Toolbars(HWND hwnd_parent, LPNMMOUSE lpnmmouse) {

    wchar_t* pclass_name = new wchar_t[MAX_PATH];
    int lenght = GetClassName(lpnmmouse->hdr.hwndFrom, pclass_name, MAX_PATH);

    BOOL is_tool_bar = FALSE;
    if (wcscmp(pclass_name, TOOLBARCLASSNAME) == 0) {
        //Fallout_Debug_Info("is a tool bar");
        is_tool_bar = TRUE;
    }
    //else if (wcscmp(pclass_name, STATUSCLASSNAME) == 0) {
        //Fallout_Debug_Info("is a status bar");
    //}
    //else {
        //Fallout_Debug_Info("is some other window type");
    //}
    delete pclass_name;
    pclass_name = nullptr;
    if (!is_tool_bar) {
        //Fallout_Debug_Info("Menu_Edit_Toolbars- not a tool bar");
        return FALSE;
    }
    HWND hwnd_FocusToolBar = lpnmmouse->hdr.hwndFrom;

    RECT rcMain;
    GetClientRect(hwnd_FocusToolBar, &rcMain);
    rcMain.left += lpnmmouse->pt.x;
    rcMain.top += lpnmmouse->pt.y;

    MapWindowPoints(hwnd_FocusToolBar, HWND_DESKTOP, (LPPOINT)&rcMain, 2);

    //Get the menu.
    HMENU hMenuLoaded;
    HMENU hPopupMenu;

    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_TOOLBAR));
    hPopupMenu = GetSubMenu(hMenuLoaded, 0);

    TPMPARAMS tpm{ 0 };

    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude = rcMain;

    BOOL menu_item = TrackPopupMenuEx(hPopupMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_RETURNCMD,
        rcMain.left, rcMain.top, hwnd_FocusToolBar, &tpm);

    DestroyMenu(hMenuLoaded);

    switch (menu_item) {
    case ID_TOOLBARMENU_EDITTOOLBAR:
        SendMessage(hwnd_FocusToolBar, TB_CUSTOMIZE, 0, 0);
        break;
    case ID_TOOLBARMENU_ADDTOOLBAR:
        ToolBar_Create(hwnd_parent, phinstDLL);
        break;
    case ID_TOOLBARMENU_DELETETOOLBAR:
        ToolBar_Delete(hwnd_parent, hwnd_FocusToolBar);
        break;
    default:
        break;
    }

    return TRUE;
}



//__________________________________________________
bool Set_Mapper_Window_Size_On_First_Show(HWND hwnd) {

    //run this function once on first WM_SHOWWINDOW.
    static bool win_first_show = true;
    if (!win_first_show)
        return false;
    win_first_show = false;

    if (hwnd == nullptr)
        return false;

    wchar_t mapper_window_file[] = L"\\win_main.dat";
    WinData_Load(hwnd, mapper_window_file, _countof(mapper_window_file));

    if (!ConfigReadInt(L"MAPPER", L"START_VISIBLE_ROOF", 0))
        HideRooves();
    

    RECT rcTemp;
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    
    HWND hRebar = Rebar_Create(hwnd, phinstDLL);
    if (hRebar) {
        Tools_SetAllCurrentToolStates(hwnd);
        GetWindowRect(hRebar, &rcTemp);
        rcClient.top = (rcTemp.bottom - rcTemp.top);
        rcClient.bottom -= rcClient.top;
    }
    else {
        Fallout_Debug_Error("Create Rebar Failed");
        return false;
    }

    HWND hStatus = Create_StatusBar(hwnd, phinstDLL);
    if (hStatus) {
        SendMessage(hStatus, WM_SIZE, 0, 0);

        GetWindowRect(hStatus, &rcTemp);
        rcClient.bottom -= (rcTemp.bottom - rcTemp.top);
    }
    else {
        Fallout_Debug_Error("Create Status Bar Failed");
        return false;
    }
    
    hGameWnd = Create_GameWindow_Mapper(hwnd, phinstDLL, &rcClient);


    ObjectBrowser_SetView(static_cast<WINDOW_STATE>(ConfigReadInt(L"MAPPER", L"OBJECT_BROWSER_WIN_STATE", 0)));
    if (ConfigReadInt(L"MAPPER", L"OBJECT_BROWSER_OPENED", FALSE))
        ObjectBrowser_Open();


    if (ConfigReadInt(L"MAPPER", L"HRP_EDGE_V2_COMPATABILITY_ENABLED", TRUE))
        Edges_SetActiveVersion(2);
    else
        Edges_SetActiveVersion(3);
    return true;
}


//_________________________________________
void Mapper_Windows_Refresh_Size(HWND hwnd) {
    RECT rcTemp{ 0,0,0,0 };
    RECT rcGameWin{ 0,0,0,0 };
    GetClientRect(hwnd, &rcGameWin);

    HWND hRebar = GetDlgItem(hwnd, IDC_REBAR);
    if (hRebar) {
        SendMessage(hRebar, WM_SIZE, 0, 0);
        GetWindowRect(hRebar, &rcTemp);
        rcGameWin.top += (rcTemp.bottom - rcTemp.top);
        //Fallout_Debug_Info("WM_WINDOWPOSCHANGED Rebar %dx%d", rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top);
    }

    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS_BAR);
    if (hStatus) {
        SendMessage(hStatus, WM_SIZE, 0, 0);

        GetWindowRect(hStatus, &rcTemp);
        rcGameWin.bottom -= (rcTemp.bottom - rcTemp.top);
    }

    HWND hBrowser = GetDlgItem(hwnd, IDC_BROWSER);
    if (hBrowser) {
        GetWindowRect(hBrowser, &rcTemp);
        LONG browser_height = rcTemp.bottom - rcTemp.top;
        LONG browser_width = rcTemp.right - rcTemp.left;
        LONG browser_x = 0;
        LONG browser_y = rcGameWin.top;

        if (object_broser_state == WINDOW_STATE::Child_Bottom) {
            browser_width = rcGameWin.right - rcGameWin.left;
            LONG scroll_height = GetSystemMetrics(SM_CYHSCROLL);
            if (rcGameWin.bottom - (browser_height + scroll_height + 24) <= rcGameWin.top)
                browser_height = rcGameWin.bottom - rcGameWin.top - (scroll_height + 24);
            rcGameWin.bottom -= browser_height;
            browser_y = rcGameWin.bottom;
        }
        else if (object_broser_state == WINDOW_STATE::Child_Right) {
            browser_height = rcGameWin.bottom - rcGameWin.top;
            LONG scroll_width = GetSystemMetrics(SM_CXVSCROLL);
            if (rcGameWin.right - (browser_width + scroll_width + 32) <= rcGameWin.left)
                browser_width = rcGameWin.right - rcGameWin.left - (scroll_width + 32);
            rcGameWin.right -= browser_width;
            browser_x = rcGameWin.right;

        }
        MoveWindow(hBrowser, browser_x, browser_y, browser_width, browser_height, TRUE);
    }

    HWND hGameWin = GetDlgItem(hwnd, IDC_WIN_GAME);
    if (hGameWin)
        MoveWindow(hGameWin, rcGameWin.left, rcGameWin.top, rcGameWin.right - rcGameWin.left, rcGameWin.bottom - rcGameWin.top, TRUE);
    
}


//____________________________________________________________________________________
LRESULT CALLBACK WinProc_Mapper(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static bool is_cursor_hidden = true;
    switch (Message) {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {

        case NM_RCLICK:
            return Menu_Edit_Toolbars(hwnd, (LPNMMOUSE)lParam);
            //case NM_DBLCLK: {
            //    LPNMMOUSE lpnmmouse = (LPNMMOUSE)lParam;
            //    Fallout_Debug_Info("NM_DBLCLK %d, %d", lpnmmouse->pt.x, lpnmmouse->pt.y);
            //    return TRUE;
            //}
        case TBN_QUERYINSERT:
        case TBN_QUERYDELETE: {
            return TRUE;
        }
        case TBN_TOOLBARCHANGE: {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            HWND hRebar = GetParent(lpnmhdr->hwndFrom);
            int bandNum = SendMessage(hRebar, RB_IDTOINDEX, (WPARAM)GetDlgCtrlID(lpnmhdr->hwndFrom), (LPARAM)0);
            Rebar_ResizeBand(hRebar, bandNum);
            //Fallout_Debug_Info("TBN_TOOLBARCHANGE");
            break;
        }
        case TBN_GETBUTTONINFO:
        case TBN_BEGINADJUST:
        case TBN_RESET:
        case TBN_ENDADJUST:
            return ToolBar_EditButtons(((LPNMHDR)lParam)->code, (LPTBNOTIFY)lParam);
        case TTN_GETDISPINFO: {
            LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;

            // Set the instance of the module that contains the resource.
            lpttt->hinst = phinstDLL;
            UINT_PTR idButton = lpttt->hdr.idFrom;
            lpttt->lpszText = MAKEINTRESOURCE(idButton);
            break;
        }
        case TBN_DROPDOWN:
            //WinToolMenus(hwnd, Message, wParam, lParam);
            if (Menu_Dropdown_LevelSelect(hwnd, (LPNMTOOLBAR)lParam))
                return TBDDRET_DEFAULT;
            else
                return TBDDRET_NODEFAULT;
            break;
        default:
            break;
        }
        break;

    case WM_MBUTTONDOWN: {
        if (!isAltMouseInput)
            break;
        if (Mouse_Wheel_Imonitor(0, false))//check if mouse over imonitor and return if so.
            return 0;
        if (SFALL_MiddleMouse != 0) {
            fall_SendKey(SFALL_MiddleMouse);//otherwise send sfalls middle mouse key.
            return 0;
        }
        break;
    }
    //case WM_KEYDOWN:
    //    if (wParam == VK_CONTROL)
    //        break;
    //    break;
    case WM_KEYUP: 
        switch (wParam) {
        case VK_CONTROL:
            break;
        //ignore these keys only if the control key is down
        case VK_SHIFT:
        case 'V'://+CTRL = pasting
        case VK_LEFT://+CTRL = rotate anticlockwise
        case VK_RIGHT://+CTRL = rotate clockwise
            if (!(GetAsyncKeyState(VK_CONTROL) & 0x80000000)) {
                SelectedAction_End(TRUE);
                return 0;
            }
            break;
        case VK_RETURN:
            SelectedAction_End(FALSE);
            return 0;
            break;
        default:
            SelectedAction_End(TRUE);
            return 0;
            break;
        }
        break;
    case WM_MOUSEWHEEL: {
        if (!isMapperInitiated)
            break;
        int fwKeys = GET_KEYSTATE_WPARAM(wParam);
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        bool setTrans = false;
        if (fwKeys & MK_CONTROL)
            setTrans = true;
        if (Mouse_Wheel_GameWindow(zDelta, setTrans))
            return 0;

        if (Mouse_Wheel_WorldMap(zDelta))
            return 0;

        if (!isAltMouseInput)//the functions below will conflict with sfall.
            break;
        bool scrollPage = false;
        if (fwKeys & MK_MBUTTON)
            scrollPage = true;
        if (Mouse_Wheel_Imonitor(zDelta, scrollPage))
            return 0;
        int keyCode = -1;
        int pageSize = 0;
        if (Mouse_Wheel_Inventory(zDelta, &keyCode, &pageSize)) {
            if (!(fwKeys & MK_MBUTTON))//if middle button NOT down, scroll one item only
                pageSize = 1;
            for (int i = 0; i < pageSize; i++)
                fall_SendKey(keyCode);
            return 0;
        }
        break;
    }
    case WM_MOUSEACTIVATE:
        hDlgCurrent = nullptr;
        return MA_ACTIVATE;
    case WM_SHOWWINDOW:
        if (Set_Mapper_Window_Size_On_First_Show(hwnd)) {
            //paint the window client black, otherwise it appears white during the rest of the fallout setup. 
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
    case WM_WINDOWPOSCHANGING: {
        WINDOWPOS* winpos = (WINDOWPOS*)lParam;
        //Fallout_Debug_Info("WM_WINDOWPOSCHANGING flags=%X", winpos->flags);
        if (winpos->flags & (SWP_NOACTIVATE | SWP_NOSIZE))
            return 0;
        RECT rcWindow = { winpos->x, winpos->y, winpos->x + winpos->cx, winpos->y + winpos->cy };
        //Check_Window_GUI_Scaling_Limits(&rcWindow, &scaleLevel_GUI, winStyle, 0);
        if (rcWindow.right - rcWindow.left < 640)
            rcWindow.right = rcWindow.left + 640;
        if (rcWindow.bottom - rcWindow.top < 480)
            rcWindow.bottom = rcWindow.top + 480;

        winpos->x = rcWindow.left;
        winpos->y = rcWindow.top;
        winpos->cx = rcWindow.right - rcWindow.left;
        winpos->cy = rcWindow.bottom - rcWindow.top;

        //Fallout_Debug_Info("WM_WINDOWPOSCHANGING scaleLevel_GUI %d", scaleLevel_GUI);

        return 0;
    }
    case WM_SIZING: {
        //Fallout_Debug_Info("WM_SIZING");
        RECT* p_rcWindow = (RECT*)lParam;
        if (p_rcWindow->right - p_rcWindow->left < 640)
            p_rcWindow->right = p_rcWindow->left + 640;
        if (p_rcWindow->bottom - p_rcWindow->top < 480)
            p_rcWindow->bottom = p_rcWindow->top + 480;
        //Check_Window_GUI_Scaling_Limits(rcWindow, &scaleLevel_GUI, winStyle, 0);
        return TRUE;
    }
    case WM_WINDOWPOSCHANGED: {
        if (IsIconic(hwnd))
            break;
        WINDOWPOS* winpos = (WINDOWPOS*)lParam;
        if (winpos->flags & (SWP_NOSIZE)) {
            SetWindowTitle(hwnd, L"");
            return 0;
        }
        Mapper_Windows_Refresh_Size(hwnd);
        return 0;
    }
    case WM_SIZE: {
        switch ((wParam)) {
        case SIZE_MINIMIZED:
            break;
        case SIZE_RESTORED:
            break;
        case SIZE_MAXIMIZED:
            break;
        default:
            break;
        }
        if (IsIconic(hwnd))
            break;
        Mapper_Windows_Refresh_Size(hwnd);
        return 0;
    }
                //case WM_CLOSE: {
                //    DestroyWindow(hwnd);
                //    isMapperExiting = true;
                //    return 0;
                 //   break;
                //}
    case WM_ENTERMENULOOP://allows system menu keys to fuction
        //*p_is_winActive = FALSE;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(FALSE);
        break;
    case WM_EXITMENULOOP:
        //*p_is_winActive = TRUE;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(TRUE);
        break;
    case WM_DISPLAYCHANGE:
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_SHIFT_WINDOW_FOCUS) {
            if (!hDlgCurrent) {
                SetFocus(hWinObjBrowser);
            }
            else {
                hDlgCurrent = nullptr;
                SetFocus(hwnd);
            }
            return 0;
        }
        return CheckMapperkeys(hwnd, LOWORD(wParam));
        break;
    case WM_SYSCOMMAND:
        switch ((wParam & 0xFFF0)) {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return 0;
        default:
            break;
        }
        break;
    case WM_SETCURSOR: {
        DWORD currentWinStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        if (!(currentWinStyle & (WIN_MODE_STYLE | WS_OVERLAPPED | WS_BORDER))) {
            ClipAltMouseCursor();
            break;
        }

        WORD ht = LOWORD(lParam);
        if (HTCLIENT == ht) {
            ClipCursor(nullptr);
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            if (!isGameMode)
                break;
            if (!CheckClientRect()) {
                if (!is_cursor_hidden) {
                    is_cursor_hidden = true;
                    ShowCursor(false);
                }
            }
            else {
                if (is_cursor_hidden) {
                    is_cursor_hidden = false;
                    ShowCursor(true);
                }
            }
        }
        else {
            if (is_cursor_hidden) {
                is_cursor_hidden = false;
                ShowCursor(true);
            }
        }
        break;
    }
    case WM_ACTIVATEAPP:
        //*p_is_winActive = wParam;
        //SetWindowActivation(*p_is_winActive);
        Set_WindowActive_State(wParam);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        Fallout_Debug_Info("main window being destroyed");
        //isMapperExiting = true;
        Mapper_On_Exit();
        Fallout_On_Exit();

        fall_map_exit();
        fall_Background_Sound_Stop();
        //MessageBox(hwnd, L"main window being destroyed", L"hello", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    case WM_NCDESTROY:
        //MessageBox(hwnd, L"main window being destroyed", L"hello", MB_ICONEXCLAMATION | MB_OK);

        Fallout_Debug_Info("main window destroyed");
        fall_game_exit();
        Fallout_Debug_Info("main window destroyed2");
        fall_WinExit(0);
        return 0;
    case WM_PAINT:
        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, Message, wParam, lParam);
}
