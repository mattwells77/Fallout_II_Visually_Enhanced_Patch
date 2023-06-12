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

#include "proto_cache.h"

#include "win_ProtoBrowser.h"

#include "win_ToolBar.h"

#include "win_Edit_Light_Colour.h"

#include "win_ObjectEditor.h"

#include "mapper_tools.h"

#include "configTools.h"

#include "../resource.h"
#include "../win_fall.h"
#include "../errors.h"

#include "../Fall_GameMap.h"
#include "../Fall_Graphics.h"
#include "../Fall_File.h"
#include "../Fall_General.h"

#include "../Fall_Msg.h"

#include "../Dx_General.h"
#include "../Dx_Graphics.h"
#include "../Dx_Game.h"

#include "../game_map.h"



HWND hWinObjBrowser = nullptr;
HWND hWinObjTabProto = nullptr;
HWND hWinObjTabObject = nullptr;

WNDPROC pOldProcProViewTab;
WNDPROC pOldProcObjViewTab;

#define DEFAULT_PROTO_TYPE  ART_CRITTERS
int proto_list_filter_type = -1;

BOOL is_object_browser_opened = FALSE;
WINDOW_STATE object_broser_state = WINDOW_STATE::Child_Bottom;



//________________________________________________________________________________________________
HBITMAP CreateIconFRM(HDC hdc, DWORD frmID, int ori, DWORD width, DWORD height, DWORD* p_bgColour) {

    FRMCached* pfrm = nullptr;
    FRMframeDx* pFrame = nullptr;

    pfrm = new FRMCached(frmID);
    pFrame = pfrm->GetFrame(0, 0);

    if (!pFrame) {
        if (pfrm)
            delete pfrm;
        pfrm = new FRMCached((frmID & 0x0F000000) + 1);
        pFrame = pfrm->GetFrame(0, 0);
        if (!pFrame) {
            if (pfrm)
                delete pfrm;
            pfrm = nullptr;
            Fallout_Debug_Error("CreateIconFRM pfrm->GetFrame failed");
            return nullptr;
        }
    }
    DWORD frmWidth = pFrame->GetWidth();
    DWORD frmHeight = pFrame->GetHeight();


    DWORD bgColour2 = 0;
    if (p_bgColour == nullptr) {
        bgColour2 = GetBkColor(hdc);
        p_bgColour = &bgColour2;
    }

    BITMAPINFO* pBMP_Info = new BITMAPINFO{ 0 };

    pBMP_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pBMP_Info->bmiHeader.biWidth = frmWidth;
    pBMP_Info->bmiHeader.biHeight = -(LONG)frmHeight;
    pBMP_Info->bmiHeader.biBitCount = 32;
    pBMP_Info->bmiHeader.biPlanes = 1;

    pBMP_Info->bmiHeader.biCompression = BI_RGB;
    pBMP_Info->bmiHeader.biXPelsPerMeter = 1000;
    pBMP_Info->bmiHeader.biYPelsPerMeter = 1000;
    pBMP_Info->bmiHeader.biClrUsed = 0;
    pBMP_Info->bmiHeader.biClrImportant = 0;


    BYTE* frmBuff = nullptr;
    UINT pitchBytes = 0;

    DWORD* buff32 = new DWORD[frmWidth * frmHeight];
    if (SUCCEEDED(pFrame->Lock((void**)&frmBuff, &pitchBytes, D3D11_MAP_READ))) {
        DWORD* pbuff32 = buff32;
        for (DWORD y = 0; y < frmHeight; y++) {
            for (DWORD x = 0; x < frmWidth; x++) {
                if (pFrame->Is32bitColour()) {
                    if ((((DWORD*)frmBuff)[x] & 0xFF000000) == 0)//check if alpha is 0
                        pbuff32[x] = *p_bgColour;
                    else
                        pbuff32[x] = ((DWORD*)frmBuff)[x];
                }
                else {
                    if (frmBuff[x] == 0)
                        pbuff32[x] = *p_bgColour;
                    else
                        pbuff32[x] = color_pal->GetColour(frmBuff[x]);
                }
            }
            pbuff32 += frmWidth;
            frmBuff += pitchBytes;
        }
        pbuff32 = nullptr;
        pFrame->Unlock(nullptr);
    }
    delete pfrm;
    pfrm = nullptr;
    pFrame = nullptr;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
    if (hBitmap) {
        HDC hDCBits = CreateCompatibleDC(hdc);
        BITMAP Bitmap{ 0 };
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
        HGDIOBJ hgdiobj_old = SelectObject(hDCBits, hBitmap);
        float frmRO = (float)frmWidth / frmHeight;
        float bitmapRO = (float)Bitmap.bmWidth / Bitmap.bmHeight;
        DWORD display_w = 0;
        DWORD display_h = 0;
        LONG x = 0;
        LONG y = 0;
        if (frmRO >= bitmapRO) {
            x = 0;
            display_w = Bitmap.bmWidth;
            display_h = (DWORD)(display_w / frmRO);
            y = (Bitmap.bmHeight - display_h) / 2;
        }
        else {
            y = 0;
            display_h = Bitmap.bmHeight;
            display_w = (DWORD)(display_h * frmRO);
            x = ((LONG)Bitmap.bmWidth - (LONG)display_w) / 2;
        }

        StretchDIBits(hDCBits, x, y, display_w, display_h, 0, 0, frmWidth, frmHeight, buff32, pBMP_Info, DIB_RGB_COLORS, SRCCOPY);
        SelectObject(hDCBits, hgdiobj_old);
        DeleteDC(hDCBits);
    }
    else {
        Fallout_Debug_Error("CreateIconFRM CreateCompatibleBitmap failed");
    }
    delete pBMP_Info;
    delete[] buff32;


    return hBitmap;

}


//__________________________________________________
bool CheckRectInView(RECT* viewRect, RECT* itemRect) {

    if (viewRect == nullptr || itemRect == nullptr)
        return false;

    if (itemRect->left > viewRect->right)
        return false;
    else if (itemRect->right < viewRect->left)
        return false;
    else if (itemRect->top > viewRect->bottom)
        return false;
    else if (itemRect->bottom < viewRect->top)
        return false;

    return true;
}


//_____________________________________
int ListView_GetTopIconIndex(HWND hwnd) {

    int item = -1;

    DWORD spacing = ListView_GetItemSpacing(hwnd, FALSE);

    RECT iRect{ 0 };
    ListView_GetItemRect(hwnd, 0, &iRect, LVIR_BOUNDS);
    LONG search_height = iRect.bottom - iRect.top + HIWORD(spacing);
    int yTest = search_height / 4;
    LONG search_width = iRect.right - iRect.left + LOWORD(spacing);
    LONG xTest = search_width / 4;

    LVHITTESTINFO info{ 0 };
    info.pt.x = 0;// viewWidth;//set x to right side of view area
    info.pt.y = 0;//set y to top of view area
    for (LONG y = 0; y < 4; y++) {//search for icon at 1/4 icon height increments
        for (LONG x = 0; x < 4; x++) {//search for icon at 1/4 icon height increments

            item = ListView_HitTest(hwnd, &info);
            if (item != -1) {
                y = 4;
                x = 4;
            }
            info.pt.x += xTest;
        }
        info.pt.y += yTest;
    }

    if (item == -1)
        item = 0;
    return item;
}


//_________________________________________________
int ProtoList_FindItemText(NMLVFINDITEM* pFindInfo) {

    if (pFindInfo->lvfi.flags & LVFI_STRING) {

        LONG item = -1;
        LONG count = 0;
        LONG selectedItem = 0;
        PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
        if (!p_pCache)
            return 0;
        while (item == -1 && count < p_pCache->Get_NumberOfItems()) {
            selectedItem = p_pCache->Get_SortedItem(count);
            if (!_wcsicmp(pFindInfo->lvfi.psz, p_pCache->Get_Text(selectedItem))) {
                item = count;
            }
            count++;
        }
        return item;
    }

    return 0;
}


//_________________________________________________
BOOL ProtoList_EditProto(LPNMITEMACTIVATE lpnmitem) {

    if (lpnmitem->hdr.hwndFrom != GetDlgItem(hWinObjTabProto, IDC_LIST1))
        return FALSE;

    int selectedItem = ListView_GetNextItem(lpnmitem->hdr.hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
    if (selectedItem == -1)
        return FALSE;
    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (!p_pCache)
        return FALSE;
    selectedItem = p_pCache->Get_SortedItem(selectedItem);
    DWORD proID = GetProID(proto_list_filter_type, selectedItem);
    return TRUE;
}


//_________________________________________________
BOOL ProtoList_ProtoMenu(LPNMITEMACTIVATE lpnmitem) {

    if (lpnmitem->hdr.hwndFrom != GetDlgItem(hWinObjTabProto, IDC_LIST1))
        return FALSE;

    int selectedItem = ListView_GetNextItem(lpnmitem->hdr.hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
    if (selectedItem == -1)
        return FALSE;
    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (!p_pCache)
        return FALSE;

    selectedItem = p_pCache->Get_SortedItem(selectedItem);

    POINT menu_pos{ lpnmitem->ptAction.x, lpnmitem->ptAction.y };
    //Convert to screen coordinates.
    MapWindowPoints(lpnmitem->hdr.hwndFrom, HWND_DESKTOP, &menu_pos, 1);

    HMENU hMenuLoaded;
    HMENU hMenuSub;
    //Get the menu.
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_OBJECT));
    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);

        // Get the submenu for the first menu item.
        HMENU hPopupMenu = hMenuSub;

        // Show the menu and wait for input.
        TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
            menu_pos.x, menu_pos.y, lpnmitem->hdr.hwndFrom, nullptr);

        DestroyMenu(hMenuLoaded);
    }
    return TRUE;
}


//_________________________________________________________
BOOL ProtoList_ProtoMenu(HWND hwnd_list, POINT* p_menu_pos) {

    if (hwnd_list != GetDlgItem(hWinObjTabProto, IDC_LIST1))
        return FALSE;

    int selectedItem = ListView_GetNextItem(hwnd_list, -1, LVNI_FOCUSED | LVNI_SELECTED);
    if (selectedItem == -1)
        return FALSE;

    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (!p_pCache)
        return FALSE;

    selectedItem = p_pCache->Get_SortedItem(selectedItem);

    DWORD proID = GetProID(proto_list_filter_type, selectedItem);
    PROTO* pPro = nullptr;

    if (fall_GetPro(proID, (PROTO**)&pPro) == -1)
        return FALSE;
   
    POINT menu_pos{ p_menu_pos->x, p_menu_pos->y };
    //Convert to screen coordinates.
    MapWindowPoints(hwnd_list, HWND_DESKTOP, &menu_pos, 1);

    HMENU hMenuLoaded;
    HMENU hMenuSub;
    //Get the menu.
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_PROTO));
    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);

        //Get the submenu for the first menu item.
        HMENU hPopupMenu = hMenuSub;

        //not yet implemented
        EnableMenuItem(hPopupMenu, ID_OBJECT_EDIT, MF_GRAYED);

        //Show the menu and wait for input.
        BOOL menu_item = TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_NONOTIFY | TPM_RETURNCMD,
            menu_pos.x, menu_pos.y, hwnd_list, nullptr);

        DestroyMenu(hMenuLoaded);

        switch (menu_item) {
        case ID_OBJECT_EDIT: 
                Fallout_Debug_Info("ProtoList_ProtoMenu - ID_OBJECT_EDIT");
                break;
        case ID_OBJECT_EDITLIGHTCOLOUR:  {
                DWORD* p_light_colour = VE_PROTO_Get_Light_Colour_Ptr(pPro);
                if (!p_light_colour)
                    return FALSE;
                DWORD colour = *p_light_colour;
                if (EditLightColour(*phWinMain, phinstDLL, &colour)) {
                    //set the new proto light colour
                    *p_light_colour = colour;
                    //and save to file
                    char* p_proto_path = new char[MAX_PATH];
                    if (Get_Proto_File_Path(proID, p_proto_path, MAX_PATH))
                        VE_PROTO_LightColour_Write(p_proto_path, pPro);
                    delete[] p_proto_path;

                    //update object lights on the current map which use this proto.
                    OBJNode* pObj_List = nullptr;
                    ListObjects_On_Map(-1, &pObj_List, OBJ_SEARCH::matching_proID, proID, 0, TRUE);
                    if (pObj_List != nullptr) {
                        OBJNode* objNode = nullptr;
                        RECT rc_obj{ 0,0,0,0 };
                        while (pObj_List) {
                            fall_Map_UpdateLightMap(pObj_List->pObj, 1, nullptr);
                            rc_obj = { 0,0,0,0 };
                            GetObjRectDx(pObj_List->pObj, &rc_obj);
                            fall_Map_UpdateLightMap(pObj_List->pObj, 0, &rc_obj);
                            if(pObj_List->pObj->level == *pMAP_LEVEL)
                                DrawMapChanges(&rc_obj, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof);
                            
                            objNode = pObj_List;
                            pObj_List = objNode->next;
                            objNode->pObj = nullptr;
                            objNode->next = nullptr;
                            delete objNode;
                        }
                        objNode = nullptr;
                    }
                }
                break;
            }
        }
    }
    return TRUE;
}


//______________________________________________________
BOOL ProtoList_SetItemData(NMLVDISPINFO* p_display_info) {

    if (proto_list_filter_type == -1)
        return FALSE;

    if (p_display_info->hdr.hwndFrom != GetDlgItem(hWinObjTabProto, IDC_LIST1))
        return FALSE;

    HWND h_ListView = p_display_info->hdr.hwndFrom;

    LVITEM* lplvI = &p_display_info->item;

    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (!p_pCache)
        return FALSE;

    if (lplvI->iItem >= p_pCache->Get_NumberOfItems()) {
        lplvI->mask = lplvI->mask | LVIF_DI_SETITEM;//set mask to not repeat message
        return FALSE;
    }

    int sortNum = p_pCache->Get_SortedItem(lplvI->iItem);

    if (lplvI->mask & LVIF_TEXT) {

        wchar_t* chars = p_pCache->Get_Text(sortNum);
        if (chars) 
            wcsncpy_s(lplvI->pszText, lplvI->cchTextMax, chars, _TRUNCATE);
        else
            lplvI->pszText[0] = '\0';
    }

    if (lplvI->mask & LVIF_IMAGE) {
        PROTO* proto = 0;
        DWORD bgColour = ListView_GetBkColor(h_ListView);
        HIMAGELIST hLarge = p_pCache->Get_ImageList();
        if (!hLarge)
            return FALSE;

        int iconW = 0;
        int iconH = 0;
        ImageList_GetIconSize(hLarge, &iconW, &iconH);

        proto = p_pCache->Get_Proto(sortNum);

        if (proto && p_pCache->Get_ImageRef(sortNum) == -1) {
            DWORD frmID = proto->frmID;
            Check_If_Blocker_Proto(proto->proID, &frmID);

            HDC hdc = GetDC(h_ListView);
            HBITMAP bitmap = CreateIconFRM(hdc, frmID, 0, iconW, iconH, &bgColour);

            if (bitmap) {

                lplvI->iImage = p_pCache->Set_Image(bitmap, sortNum);
                if (lplvI->iImage == -1)
                    Fallout_Debug_Error("ProtoList_SetItemData ImageList_Add failed");
                DeleteObject(bitmap);
            }
            ReleaseDC(h_ListView, hdc);
        }
        else
            lplvI->iImage = p_pCache->Get_ImageRef(sortNum);
    }
    return TRUE;
}


//_____________________________________
BOOL ProtoList_SortByListNum(LONG type) {

    if (type == -1)
        return FALSE;
    PROTO_cache_type* p_pCache = ProtCache_GetType(type);
    if (!p_pCache)
        return FALSE;
    for (LONG lpos = 0; lpos < p_pCache->Get_NumberOfItems(); lpos++)
        p_pCache->Set_SortedItem(lpos, lpos);

    return TRUE;
}


//__________________________________
BOOL ProtoList_SortByText(LONG type) {

    if (type == -1)
        return FALSE;
    PROTO_cache_type* p_pCache = ProtCache_GetType(type);
    if (!p_pCache)
        return FALSE;
    wchar_t* str1 = 0, * str2 = 0;

    LONG lposHold = 0, lposHoldVal = 0;
    LONG lposVar = 0;
    for (LONG lpos = 1; lpos < p_pCache->Get_NumberOfItems(); lpos++) {
        lposHold = lpos;
        lposHoldVal = p_pCache->Get_SortedItem(lposHold);
        lposVar = lpos - 1;

        str1 = p_pCache->Get_Text(p_pCache->Get_SortedItem(lposHold));
        str2 = p_pCache->Get_Text(p_pCache->Get_SortedItem(lposVar));

        if (str1 && str2) {
            while (lposVar >= 0 && _wcsicmp(str2, str1) > 0) {
                p_pCache->Set_SortedItem(lposVar + 1, p_pCache->Get_SortedItem(lposVar));
                lposVar--;
                if (lposVar >= 0)
                    str2 = p_pCache->Get_Text(p_pCache->Get_SortedItem(lposVar));
            }
        }
        p_pCache->Set_SortedItem(lposVar + 1, lposHoldVal);
    }
    return TRUE;
}


//___________________________
BOOL ProtoList_Sort(LONG num) {
    if (proto_list_filter_type == -1)
        return FALSE;

    HWND h_ListView = GetDlgItem(hWinObjTabProto, IDC_LIST1);

    ListView_RedrawItems(h_ListView, 0, ListView_GetItemCount(h_ListView));

    InvalidateRect(h_ListView, nullptr, TRUE);
    for (LONG type = 0; type < 6; type++) {
        if (num == 0)
            ProtoList_SortByListNum(type);
        else if (num == 1)
            ProtoList_SortByText(type);
    }
    return TRUE;
}


//____________________________________________
BOOL ProtoList_SetActiveProto(HWND h_ListView) {

    if (h_ListView != GetDlgItem(hWinObjTabProto, IDC_LIST1))
        return FALSE;

    int selectedItem = ListView_GetNextItem(h_ListView, -1, LVNI_FOCUSED | LVNI_SELECTED);
    if (selectedItem == -1)
        return FALSE;

    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (!p_pCache)
        return FALSE;
    selectedItem = p_pCache->Get_SortedItem(selectedItem);

    DWORD proID_Active = GetProID(proto_list_filter_type, selectedItem);

    if (Action_Initiate_PlaceNewObject(proID_Active)) {
        p_pCache->Set_Position(selectedItem);
        return TRUE;
    }
    return FALSE;
}


//______________________
void ProtoList_Refresh() {
    if (!isMapperInitiated)
        return;
    if (!hWinObjTabProto)
        return;

    if (proto_list_filter_type == -1) {
        proto_list_filter_type = DEFAULT_PROTO_TYPE;
        HWND combo = GetDlgItem(hWinObjTabProto, IDC_COMBO_TYPE);
        SendMessage(combo, CB_SETCURSEL, (WPARAM)proto_list_filter_type, (LPARAM)0);
    }
    //if the currently displayed list is being rebuilt, refresh the list view.
    PROTO_cache_type* p_pCache = ProtCache_GetType(proto_list_filter_type);
    if (p_pCache) {
        HWND h_ListView = GetDlgItem(hWinObjTabProto, IDC_LIST1);
        ListView_DeleteAllItems(h_ListView);
        ListView_SetItemCountEx(h_ListView, p_pCache->Get_NumberOfItems(), 0);
        ListView_SetImageList(h_ListView, p_pCache->Get_ImageList(), LVSIL_NORMAL);

        //scroll to end of list
        ListView_EnsureVisible(h_ListView, ListView_GetItemCount(h_ListView) - 1, FALSE);
        //scroll back to the last position item so that it is at the top left.
        ListView_EnsureVisible(h_ListView, p_pCache->Get_Position(), TRUE);
        ListView_SetItemState(h_ListView, p_pCache->Get_Position(), LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }

}


//__________________________________________
void ProtoList_Set(LONG type, LONG position) {
    if (!isMapperInitiated)
        return;
    if (!hWinObjTabProto)
        return;

    if (type == -1)
        type = proto_list_filter_type;

    HWND h_ListView = GetDlgItem(hWinObjTabProto, IDC_LIST1);

    PROTO_cache_type* p_pCache = nullptr;


    if (type != proto_list_filter_type) {
        p_pCache = ProtCache_GetType(proto_list_filter_type);
        if (p_pCache) {
            //save list positon of current type list 
            int lastPos = ListView_GetTopIconIndex(h_ListView);// -1;
            if (lastPos < 0 || lastPos >= p_pCache->Get_NumberOfItems())
                lastPos = 0;

            p_pCache->Set_Position(lastPos);
        }
        //switch to the selected type list
        proto_list_filter_type = type;
    }

    if (position > 0) {
        p_pCache = ProtCache_GetType(proto_list_filter_type);
        if (p_pCache)
            p_pCache->Set_Position(position);
    }
    ProtoList_Refresh();

    //proto_list_filter_type = type;
    HWND combo = GetDlgItem(hWinObjTabProto, IDC_COMBO_TYPE);
    SendMessage(combo, CB_SETCURSEL, (WPARAM)proto_list_filter_type, (LPARAM)0);
}


//___________________________________
void ProtoList_GoToProto(DWORD proID) {

    if (proID == -1)
        return;
    LONG type = (proID & 0x0F000000) >> 0x18;
    LONG pro_num = proID & 0x00FFFFFF;
    if (type < 0 || type >= 6)
        return;

    PROTO_cache_type* p_pCache = ProtCache_GetType(type);
    if (!p_pCache)
        return;

    for (int lpos = 0; lpos < p_pCache->Get_NumberOfItems(); lpos++) {
        if (pro_num - 1 == p_pCache->Get_SortedItem(lpos)) {
            ProtoList_Set(type, lpos);
            //switch to proto tab on list refresh
            HWND hwndTab = GetDlgItem(hWinObjBrowser, IDC_TAB1);
            TabCtrl_SetCurFocus(hwndTab, 0);
            return;
        }
    }
}


//__________________________________________________
void ProtoList_GoToTileProto_With_FrmID(DWORD frmID) {

    if (frmID == -1)
        return;
    LONG type = (frmID & 0x0F000000) >> 0x18;
    if (type != ART_TILES)
        return;

    PROTO_cache_type* p_pCache = ProtCache_GetType(type);
    if (!p_pCache)
        return;
    for (int lpos = 0; lpos < p_pCache->Get_NumberOfItems(); lpos++) {
        LONG sortNum = p_pCache->Get_SortedItem(lpos);
        PROTO* p_pro = p_pCache->Get_Proto(sortNum);
        if (p_pro && frmID == p_pro->frmID) {
            ProtoList_Set(type, lpos);
            //switch to proto tab on list refresh
            HWND hwndTab = GetDlgItem(hWinObjBrowser, IDC_TAB1);
            TabCtrl_SetCurFocus(hwndTab, 0);
            return;
        }
    }
}


//_____________________________________________
class OBJECT_LIST_CACHE : public LIST_VIEW_DATA {
public:
    OBJECT_LIST_CACHE(OBJNode* p_objects, TILE_DATA_node* p_tiles) :
        LIST_VIEW_DATA()
    {

        LONG object_list_count = 0;

        OBJNode* p_obj_node = p_objects;
        while (p_obj_node) {
            if (p_obj_node->pObj)
                object_list_count++;
            p_obj_node = p_obj_node->next;
        }
        TILE_DATA_node* p_tile_node = p_tiles;
        LONG roof_tile = 0;
        LONG floor_tile = 0;
        while (p_tile_node) {
            if ((p_tile_node->frmID & 0x00FFFFFF) != 1)
                object_list_count++;
            p_tile_node = p_tile_node->next;
        }
       
        LONG count = 0;
        if (object_list_count) {
            Setup_List(object_list_count, TRUE, TRUE, 80, 80);
            p_map_object = new MAP_OBJECT[Get_NumberOfItems()];

            OBJNode* p_obj_node = p_objects;
            while (p_obj_node && count < Get_NumberOfItems()) {
                if (p_obj_node->pObj) {
                    p_map_object[count].Set(p_obj_node->pObj);
                    count++;
                }
                p_obj_node = p_obj_node->next;

            }
            TILE_DATA_node* p_tile_node = p_tiles;
            while (p_tile_node && count < Get_NumberOfItems()) {
                if ((p_tile_node->frmID & 0x00FFFFFF) != 1) {
                    p_map_object[count].Set(p_tile_node->num, p_tile_node->isRoof, p_tile_node->level);
                    count++;
                }
                p_tile_node = p_tile_node->next;

            }
        }
    }
    ~OBJECT_LIST_CACHE() {
        if (p_map_object)
            delete p_map_object;
        p_map_object = nullptr;
    }
    MAP_OBJECT* Get_MapObject(LONG item) {
        if (item < 0 || item >= Get_NumberOfItems())
            return nullptr;
        return &p_map_object[item];
    }
    OBJStruct* GetObj(LONG item) {
        if (item < 0 || item >= Get_NumberOfItems())
            return nullptr;
        return p_map_object[item].GetObj();
    }
    LONG GetTileNumber(LONG item) {
        if (item < 0 || item >= Get_NumberOfItems())
            return -1;
        return p_map_object[item].GetTileNumber();
    }
    LONG GetLevel(LONG item) {
        if (item < 0 || item >= Get_NumberOfItems())
            return -1;
        return p_map_object[item].GetLevel();
    }
    BOOL IsRoof(LONG item) {
        if (item < 0 || item >= Get_NumberOfItems())
            return -1;
        return p_map_object[item].IsRoof();
    }
protected:
private:
    MAP_OBJECT* p_map_object;
};

OBJECT_LIST_CACHE* objectCache = nullptr;


//__________________________________________________________________________________
BOOL ObjectList_Init(HWND hWndListView, OBJNode* p_objects, TILE_DATA_node* p_tiles) {

    if (objectCache)
        delete objectCache;
    objectCache = new OBJECT_LIST_CACHE(p_objects, p_tiles);

    ListView_DeleteAllItems(hWndListView);
    ListView_SetItemCountEx(hWndListView, objectCache->Get_NumberOfItems(), LVSICF_NOINVALIDATEALL);
    
    ListView_SetImageList(hWndListView, objectCache->Get_ImageList(), LVSIL_NORMAL);
    return TRUE;
}


//_______________________________________________________
BOOL ObjectList_SetItemData(NMLVDISPINFO* p_display_info) {

    if (!objectCache)
        return FALSE;
    if (p_display_info->hdr.hwndFrom != GetDlgItem(hWinObjTabObject, IDC_LIST1))
        return FALSE;

    HWND h_ListView = p_display_info->hdr.hwndFrom;

    LVITEM* lplvI = &p_display_info->item;

    if (lplvI->iItem >= objectCache->Get_NumberOfFilteredItems()) {
        lplvI->mask |= LVIF_DI_SETITEM;//set mask to not repeat message
        return FALSE;
    }

    LONG sortNum = objectCache->Get_FilteredItem(lplvI->iItem);
    MAP_OBJECT* p_mapObject = objectCache->Get_MapObject(sortNum);
    if (!p_mapObject)
        return FALSE;

    DWORD proType = -1;
    DWORD proNum = -1;
    DWORD frmID = -1;
    PROTO_cache_type* p_pCache = nullptr;


    OBJStruct* pObj = p_mapObject->GetObj();
    if (pObj) {
        proType = (pObj->proID & 0x0F000000) >> 0x18;
        proNum = (pObj->proID & 0x00FFFFFF) - 1;
        p_pCache = ProtCache_GetType(proType);
        if (p_pCache) {
            PROTO* proto = p_pCache->Get_Proto(proNum);
            if (proto) {
                frmID = proto->frmID;
                Check_If_Blocker_Proto(proto->proID, &frmID);
            }
        }
    }
    else {
        frmID = GameMap_GetTileFrmID(p_mapObject->GetTileNumber(), p_mapObject->GetLevel(), p_mapObject->IsRoof(), nullptr);
        proType = (frmID & 0x0F000000) >> 0x18;
        p_pCache = ProtCache_GetType(proType);
        if (p_pCache) {
            PROTO* proto;
            for (LONG lpos = 0; lpos < p_pCache->Get_NumberOfItems(); lpos++) {
                LONG sortNum_pro = p_pCache->Get_SortedItem(lpos);
                proto = p_pCache->Get_Proto(sortNum_pro);
                if (proto && frmID == proto->frmID) {
                    proNum = sortNum_pro;
                    lpos = p_pCache->Get_NumberOfItems();
                }
            }
        }
    }

    if (lplvI->mask & LVIF_TEXT) {
        wchar_t* chars = nullptr;
        if(p_pCache)
            chars = p_pCache->Get_Text(proNum);

        if (chars) {
            wcsncpy_s(lplvI->pszText, lplvI->cchTextMax, chars, _TRUNCATE);
        }
        else
            lplvI->pszText[0] = '\0';
    }

    if (lplvI->mask & LVIF_IMAGE) {
        if (frmID == -1)
            return FALSE;
        DWORD bgColour = ListView_GetBkColor(h_ListView);
        HIMAGELIST hLarge = objectCache->Get_ImageList();
        if (!hLarge)
            return FALSE;
        int iconW = 0;
        int iconH = 0;
        ImageList_GetIconSize(hLarge, &iconW, &iconH);

        if (objectCache->Get_ImageRef(sortNum) == -1) {
            HDC hdc = GetDC(h_ListView);
            HBITMAP bitmap = CreateIconFRM(hdc, frmID, 0, iconW, iconH, &bgColour);

            if (bitmap) {
                lplvI->iImage = objectCache->Set_Image(bitmap, sortNum);
                if (lplvI->iImage == -1)
                    Fallout_Debug_Error("ObjectList_SetItemData ImageList_Add failed - tile, frmID:%X sortnum:%d", frmID, sortNum);
                DeleteObject(bitmap);
            }
            ReleaseDC(h_ListView, hdc);
            return TRUE;
        }
        else
            lplvI->iImage = objectCache->Get_ImageRef(sortNum);
    }
   
    return TRUE;
}


//___________________________________________________________
BOOL ObjectList_ObjectMenu(HWND hwnd_list, POINT* p_menu_pos) {
    if (!objectCache)
        return FALSE;
    if (hwnd_list != GetDlgItem(hWinObjTabObject, IDC_LIST1))
        return FALSE;

    int selectedItem = ListView_GetNextItem(hwnd_list, -1, LVNI_FOCUSED | LVNI_SELECTED);

    if (selectedItem < 0 || selectedItem >= objectCache->Get_NumberOfFilteredItems())
        return FALSE;
    
    int sortNum = objectCache->Get_FilteredItem(selectedItem);
    MAP_OBJECT* p_mapObject = objectCache->Get_MapObject(sortNum);
    if (!p_mapObject)
        return FALSE;

    OBJStruct* pObj = p_mapObject->GetObj();
    DWORD tileNum = p_mapObject->GetTileNumber();
    BOOL isRoof = p_mapObject->IsRoof();
    LONG level = p_mapObject->GetLevel();

    POINT menu_pos{ p_menu_pos->x, p_menu_pos->y };
    //Convert to screen coordinates.
    MapWindowPoints(hwnd_list, HWND_DESKTOP, &menu_pos, 1);

    HMENU hMenuLoaded;
    HMENU hMenuSub;
    //Get the menu.
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_OBJECT));
    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);

        //Get the submenu for the first menu item.
        HMENU hPopupMenu = hMenuSub;

        if (!pObj) {
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDIT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_OBJECT_EDITLIGHTCOLOUR, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_SELECTOBJECTSONLEVEL_WITHMATCHINGSCRIPT, MF_GRAYED);
            EnableMenuItem(hPopupMenu, ID_SELECTOBJECTSONMAP_WITHMATCHINGSCRIPT, MF_GRAYED);
            DrawMenuBar(*phWinMain);
        }

        //Show the menu and wait for input.
        BOOL menu_item = TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_NONOTIFY | TPM_RETURNCMD,
            menu_pos.x, menu_pos.y, hwnd_list, nullptr);

        DestroyMenu(hMenuLoaded);


        switch (menu_item) {
        case ID_OBJECT_EDIT:
            Dialog_ObjectEditor(*phWinMain, phinstDLL, pObj);
            break;
        case ID_OBJECT_EDITLIGHTCOLOUR: {
            DWORD colour = ((OBJStructDx*)pObj)->light_colour;
            //if objects light colour == 0"unset" then get the starting light colour from its proto. 
            if (colour == 0) {
                PROTO* pPro = nullptr;
                if (fall_GetPro(pObj->proID, (PROTO**)&pPro) == -1)
                    return FALSE;
                colour = VE_PROTO_Get_Light_Colour(pPro);
            }
            if (EditLightColour(*phWinMain, phinstDLL, &colour)) {
                ((OBJStructDx*)pObj)->light_colour = colour;
                fall_Map_UpdateLightMap(pObj, 1, nullptr);
                RECT rc_obj{ 0,0,0,0 };
                GetObjRectDx(pObj, &rc_obj);
                fall_Map_UpdateLightMap(pObj, 0, &rc_obj);
                DrawMapChanges(&rc_obj, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof);
            }
            break;
        }
        case ID_OBJECT_COPY:
            if (MapObject_Copy(pObj, tileNum, isRoof, level)) {
                Tool_SetState(*phWinMain, ID_EDIT_PASTE, TRUE, FALSE, TRUE);
                DrawMenuBar(*phWinMain);
            }
            break;
        case ID_OBJECT_DELETEFROMMAP:
            MapObject_Delete(pObj, tileNum, isRoof, level);
            break;
        case ID_OBJECT_REMOVEFROMLIST:
            SelectObject_Remove(pObj, TRUE);
            break;

        case ID_OBJECT_GOTOPROTOTYPE: {
            if (pObj)
                ProtoList_GoToProto(pObj->proID);
            else if (tileNum != -1)
                ProtoList_GoToTileProto_With_FrmID(GameMap_GetTileFrmID(tileNum, level, isRoof, nullptr));
            break;
        }
        case ID_SELECTOBJECTSONLEVEL_WITHMATCHINGPROTO:
            if (pObj)
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::matching_proID, pObj->proID);
            else if (tileNum != -1)
                SelectTiles_With_Matching_FRM(*pMAP_LEVEL, GameMap_GetTileFrmID(tileNum, level, isRoof, nullptr));
            break;
        case ID_SELECTOBJECTSONLEVEL_WITHMATCHINGSCRIPT:
            if (pObj)
                SelectObjects_On_Map(*pMAP_LEVEL, OBJ_SEARCH::matching_scriptIndex, pObj->scriptIndex);
            break;
        case ID_SELECTOBJECTSONMAP_WITHMATCHINGPROTO:
            if (pObj)
                SelectObjects_On_Map(-1, OBJ_SEARCH::matching_proID, pObj->proID);
            else if (tileNum != -1)
                SelectTiles_With_Matching_FRM(-1, GameMap_GetTileFrmID(tileNum, -1, isRoof, nullptr));
            break;
        case ID_SELECTOBJECTSONMAP_WITHMATCHINGSCRIPT:
            if (pObj)
                SelectObjects_On_Map(-1, OBJ_SEARCH::matching_scriptIndex, pObj->scriptIndex);
            break;
        default:
            break;
        }
    }
    return TRUE;
}


//____________________________________________
BOOL ObjectList_ItemChanged(LPNMLISTVIEW pnmv) {
    if (pnmv->iItem == -1)
        return TRUE;
    if (pnmv->uOldState & LVIS_FOCUSED) {//LVIS_SELECTED
        Selected_List_ClearFocus();
    }
    if (pnmv->uNewState& LVIS_FOCUSED) {
        
        if (pnmv->hdr.hwndFrom != GetDlgItem(hWinObjTabObject, IDC_LIST1))
            return FALSE;

        LONG selectedItem = ListView_GetNextItem(pnmv->hdr.hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
        if (!objectCache)
            return FALSE;
        if (selectedItem < 0 || selectedItem >= objectCache->Get_NumberOfFilteredItems())
            return FALSE;
        

        LONG sortNum = objectCache->Get_FilteredItem(selectedItem);
        MAP_OBJECT* p_mapObject = objectCache->Get_MapObject(sortNum);
        Selected_List_SetFocus(p_mapObject);
    }
    return TRUE;
}


//____________________________________
void ObjectList_Filter_Type(LONG type) {
    if (!objectCache)
        return;
    HWND h_ListView = GetDlgItem(hWinObjTabObject, IDC_LIST1);
    ListView_RedrawItems(h_ListView, 0, ListView_GetItemCount(h_ListView));
    InvalidateRect(h_ListView, nullptr, true);

    DWORD object_list_count_type_filtered = 0;
        DWORD pro_type = 0;
        OBJStruct* pObj = nullptr;
        LONG object_type = 0;
        //MAP_OBJECT* p_mapObject = nullptr;
        for (LONG i = 0; i < objectCache->Get_NumberOfItems(); i++) {
            pObj = objectCache->GetObj(objectCache->Get_SortedItem(i));
            if (pObj)
                object_type = (pObj->proID & 0x0F000000) >> 0x18;
            else
                object_type = ART_TILES;

                if (type < 0 || type == object_type) {
                    objectCache->Set_FilteredItem(object_list_count_type_filtered, objectCache->Get_SortedItem(i));
                    object_list_count_type_filtered++;
                } 
        }
        objectCache->Set_NumberOfFilteredItems(object_list_count_type_filtered);

        ListView_DeleteAllItems(h_ListView);
        ListView_SetItemCountEx(h_ListView, object_list_count_type_filtered, 0);
}


//_________________________________
void Object_List_SortBy_Selection() {
    if (!objectCache)
        return;
    for (LONG i = 0; i < objectCache->Get_NumberOfItems(); i++) {
        objectCache->Set_SortedItem(i,i);
    }
}


//___________________________________
void ObjectList_SortBy_Level_HexNum() {
    if (!objectCache)
        return;
    LONG lpos_StoredVal = 0;
    LONG lposVar = 0;
    LONG hex_stored = 0;
    LONG hex_var= 0;

    MAP_OBJECT* p_mapObject = nullptr;
    OBJStruct* pObj_Stored = nullptr;

    LONG level_Stored = -1;
    //go though the list dropping each object down the list until it is less than or equal to the level of the all previous objects and its hex position is less than that of all previous objects.
    for (LONG lpos = 1; lpos < objectCache->Get_NumberOfItems(); lpos++) {
        lposVar = lpos - 1;
        lpos_StoredVal = objectCache->Get_SortedItem(lpos);
        p_mapObject = objectCache->Get_MapObject(objectCache->Get_SortedItem(lpos_StoredVal));
        pObj_Stored = p_mapObject->GetObj();
        level_Stored = p_mapObject->GetLevel();

        if (pObj_Stored)
            hex_stored = pObj_Stored->hexNum;
        else
            hex_stored = TileNumToHexNum(p_mapObject->GetTileNumber());
        
        if (objectCache->GetObj(objectCache->Get_SortedItem(lposVar)))
            hex_var = objectCache->GetObj(objectCache->Get_SortedItem(lposVar))->hexNum;
        else
            hex_var = TileNumToHexNum(objectCache->GetTileNumber(objectCache->Get_SortedItem(lposVar)));

        while (lposVar >= 0 && (objectCache->GetLevel(objectCache->Get_SortedItem(lposVar)) > level_Stored || (objectCache->GetLevel(objectCache->Get_SortedItem(lposVar)) == level_Stored && hex_var > hex_stored))) {
            objectCache->Set_SortedItem(lposVar + 1, objectCache->Get_SortedItem(lposVar));
            lposVar--;
            if (lposVar >= 0) {
                if (objectCache->GetObj(objectCache->Get_SortedItem(lposVar)))
                    hex_var = objectCache->GetObj(objectCache->Get_SortedItem(lposVar))->hexNum;
                else
                    hex_var = TileNumToHexNum(objectCache->GetTileNumber(objectCache->Get_SortedItem(lposVar)));
            }
        }
        objectCache->Set_SortedItem(lposVar + 1, lpos_StoredVal);
    }
}


//____________________________
BOOL ObjectList_Sort(LONG num) {

    if (num == 0)
        Object_List_SortBy_Selection();
    else if (num == 1)
        ObjectList_SortBy_Level_HexNum();
    else
        return FALSE;

    return TRUE;
}


//_______________________
void ObjectList_Arrange() {

    HWND h_combo = GetDlgItem(hWinObjTabObject, IDC_COMBO_SORT);
    ObjectList_Sort(SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));

    h_combo = GetDlgItem(hWinObjTabObject, IDC_COMBO_TYPE);
    ObjectList_Filter_Type(SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) - 1);
}


//__________________________________________________________________
void ObjectList_Refresh(OBJNode* p_objects, TILE_DATA_node* p_tiles) {

    if (!hWinObjTabObject)
        return;
    HWND h_ListView = GetDlgItem(hWinObjTabObject, IDC_LIST1);

    ObjectList_Init(h_ListView, p_objects, p_tiles);
    ObjectList_Arrange();
    if (!objectCache)
        return;
    //scroll to end of list
    ListView_EnsureVisible(h_ListView, ListView_GetItemCount(h_ListView) - 1, FALSE);
    //scroll back to the last position item so that it is at the top left.
    ListView_EnsureVisible(h_ListView, objectCache->Get_Position(), TRUE);

    //if (object_list_count) {
     if(objectCache->Get_NumberOfFilteredItems()){
        //switch to object tab on list refresh
        HWND hwndTab = GetDlgItem(hWinObjBrowser, IDC_TAB1);
        TabCtrl_SetCurFocus(hwndTab, 1);
    }
}


//____________________________________________________________________________________________
LRESULT CALLBACK WinProcProViewTabSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

  LRESULT lres;
  switch (uMsg) {
  case WM_GETDLGCODE:
    lres = CallWindowProc(pOldProcProViewTab,hwnd, uMsg, wParam, lParam);
    lres &= ~DLGC_WANTTAB;
    if (lParam &&
        ((MSG *)lParam)->message == WM_KEYDOWN &&
        ((MSG *)lParam)->wParam == VK_TAB) {
      lres &= ~DLGC_WANTMESSAGE;
      PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
    }
    return lres;
  }
  return CallWindowProc(pOldProcProViewTab,hwnd, uMsg, wParam, lParam);
}


//______________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ObjBrowser_Proto(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {

    case WM_INITDIALOG: {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(phinstDLL, MAKEINTRESOURCE(IDI_ICON1)));

        RECT rcHwnd;
        GetClientRect(hwnd, &rcHwnd);
        HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
        pOldProcProViewTab = (WNDPROC)SetWindowLongPtr(h_ListView, GWLP_WNDPROC, (LONG_PTR)WinProcProViewTabSubClass);

        HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_TYPE);

        wchar_t* str = new wchar_t[16];
        for (int type = 0; type < 6; type++) {
            LoadString(phinstDLL, Get_Proto_Type_TextID(type), str, 16);
            SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
            //Fallout_Debug_Info("%S", str);
        }
        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)proto_list_filter_type, (LPARAM)0);

        //sort box
        hwndCombo = GetDlgItem(hwnd, IDC_COMBO_SORT);
        LoadString(phinstDLL, IDS_SORT_LIST_NUMBER, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_SORT_NAME, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);

        delete[] str;

        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        return TRUE;
    }
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case NM_KEYDOWN: {
            MessageBox(nullptr, L"NM_KEYDOWN", L"Hi-Res patch Error", MB_ICONERROR);
            return 0;
        }
        case LVN_ODFINDITEM: {
            SetWindowLongPtr(hwnd, DWL_MSGRESULT, ProtoList_FindItemText((NMLVFINDITEM*)lParam));
            return TRUE;
            break;
        }
        case NM_RCLICK: {
            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
            ProtoList_ProtoMenu(lpnmitem->hdr.hwndFrom, &lpnmitem->ptAction);
            return TRUE;
            break;
        }
        case NM_RETURN: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            int selectedItem = ListView_GetNextItem(lpnm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
            if (selectedItem != -1) {
                RECT rcItem{ 0,0,0,0 };
                ListView_GetItemRect(lpnm->hwndFrom, selectedItem, &rcItem, LVIR_BOUNDS);
                ProtoList_ProtoMenu(lpnm->hwndFrom, (POINT*)&rcItem);
            }
            return TRUE;
        }
        case LVN_ITEMACTIVATE: {
            HWND hwnd;
#if(_WIN32_IE >= 0x0400)
            LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
            hwnd = lpnmia->hdr.hwndFrom;
#else
            LPNMHDR lpnm = (LPNMHDR)lParam;
            hwnd = lpnm->hwndFrom;
#endif
            ProtoList_SetActiveProto(hwnd);
            PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
            return TRUE;
        }
        case LVN_GETDISPINFO: {
            ProtoList_SetItemData((NMLVDISPINFO*)lParam);
            return TRUE;
            break;
        }
        default:
            break;
        }
        break;
    case WM_SIZE: {
        int width = (LOWORD(lParam));
        int height = (HIWORD(lParam));

        HWND hView = GetDlgItem(hwnd, IDC_LIST1);
        RECT listRect;
        GetWindowRect(hView, &listRect);
        POINT pt = { listRect.left, listRect.top }; //new point object using rect x, y
        ScreenToClient(hwnd, &pt); //convert screen co-ords to client based points
        MoveWindow(hView, pt.x, pt.y, width - pt.x, height - pt.y, TRUE);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_COMBO_TYPE: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                LONG type = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                ProtoList_Set(type, -1);
            }
            return TRUE;
        }
        case IDC_COMBO_SORT: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                LONG num = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                ProtoList_Sort(num);

            }
            return TRUE;
        }
        default:
            break;
        }
        break;
    case WM_DESTROY: 
        return 0;
    case WM_NCDESTROY:
        //Fallout_Debug_Info("proto browser destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//____________________________________________________________________________________________
LRESULT CALLBACK WinProcObjViewTabSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    LRESULT lres;
    switch (uMsg) {
    case WM_GETDLGCODE:
        lres = CallWindowProc(pOldProcObjViewTab, hwnd, uMsg, wParam, lParam);
        lres &= ~DLGC_WANTTAB;
        if (lParam &&
            ((MSG*)lParam)->message == WM_KEYDOWN &&
            ((MSG*)lParam)->wParam == VK_TAB) {
            lres &= ~DLGC_WANTMESSAGE;
            PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
        }
        return lres;
    }
    return CallWindowProc(pOldProcObjViewTab, hwnd, uMsg, wParam, lParam);
}


//________________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ObjBrowser_Objects(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {

    case WM_INITDIALOG: {

        RECT rcHwnd;
        GetClientRect(hwnd, &rcHwnd);
        HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
        pOldProcObjViewTab = (WNDPROC)SetWindowLongPtr(h_ListView, GWLP_WNDPROC, (LONG_PTR)WinProcObjViewTabSubClass);
        
        
        wchar_t* str = new wchar_t[16];
        HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_TYPE);

        for (int i = 0; i < 7; i++) {
            LoadString(phinstDLL, IDS_TEXT_ALL + i, str, 16);
            SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
            //Fallout_Debug_Info("%S", str);
        }

        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        //sort box
        hwndCombo = GetDlgItem(hwnd, IDC_COMBO_SORT);
        LoadString(phinstDLL, IDS_SORT_BY_SELECTION, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_SORT_BY_LEVEL_HEXNUM, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);;

        delete[] str;

        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        return TRUE;
    }
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case NM_KEYDOWN: {
            MessageBox(nullptr, L"NM_KEYDOWN", L"Hi-Res patch Error", MB_ICONERROR);
            return 0;
        }
        case LVN_ODFINDITEM: {
            NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)lParam;
            return TRUE;
            break;
        }
        case NM_RCLICK: {
            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
            ObjectList_ObjectMenu(lpnmitem->hdr.hwndFrom, &lpnmitem->ptAction);
            return TRUE;
            break;
        }
        case NM_RETURN: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            int selectedItem = ListView_GetNextItem(lpnm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
            if (selectedItem != -1) {
                RECT rcItem{ 0,0,0,0 };
                ListView_GetItemRect(lpnm->hwndFrom, selectedItem, &rcItem, LVIR_BOUNDS);
                ObjectList_ObjectMenu(lpnm->hwndFrom, (POINT*)&rcItem);
            }
            return TRUE;
        }
        case LVN_ITEMACTIVATE: {
            HWND hwnd;
#if(_WIN32_IE >= 0x0400)
            LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
            hwnd = lpnmia->hdr.hwndFrom;
#else
            LPNMHDR lpnm = (LPNMHDR)lParam;
            hwnd = lpnm->hwndFrom;
#endif
            PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
            return TRUE;
        }
        case LVN_GETDISPINFO: {
            ObjectList_SetItemData((NMLVDISPINFO*)lParam);
            return TRUE;
            break;
        }
        case LVN_ITEMCHANGED: {
            ObjectList_ItemChanged((LPNMLISTVIEW)lParam);

            break;
        }                
        default:
            break;
        }
        break;
    case WM_SIZE: {
        int width = (LOWORD(lParam));
        int height = (HIWORD(lParam));

        HWND hView = GetDlgItem(hwnd, IDC_LIST1);
        RECT listRect;
        GetWindowRect(hView, &listRect);
        POINT pt = { listRect.left, listRect.top }; //new point object using rect x, y
        ScreenToClient(hwnd, &pt); //convert screen co-ords to client based points
        MoveWindow(hView, pt.x, pt.y, width - pt.x, height - pt.y, TRUE);
        return true;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_COMBO_TYPE: {
            if (HIWORD(wParam) == CBN_SELCHANGE)
                ObjectList_Arrange();
            return TRUE;
        }
        case IDC_COMBO_SORT: {
            if (HIWORD(wParam) == CBN_SELCHANGE)
                ObjectList_Arrange();
            return TRUE;
        }
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        //Fallout_Debug_Info("object browser being destroyed");
        if (objectCache)
            delete objectCache;
        objectCache = nullptr;
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("object browser destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//_______________________________________
//Disable resizing of the Object Browser except from the top edge for the Bottom view or left edge for the Right view.
UINT OnNcHitTest(HWND hwnd, int x, int y) {
    UINT ht = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);
    if (object_broser_state == WINDOW_STATE::Child_Bottom) {
        switch (ht) {
        case HTBOTTOMLEFT:  ht = HTBORDER; break;
        case HTBOTTOMRIGHT: ht = HTBORDER; break;
        case HTBOTTOM:      ht = HTBORDER; break;
        case HTLEFT:        ht = HTBORDER; break;
        case HTRIGHT:       ht = HTBORDER; break;

        case HTTOPLEFT:     ht = HTTOP;    break;
        case HTTOPRIGHT:    ht = HTTOP;    break;
        }
    }
    else if (object_broser_state == WINDOW_STATE::Child_Right) {
        switch (ht) {
        case HTTOPRIGHT:    ht = HTBORDER;  break;
        case HTBOTTOMRIGHT: ht = HTBORDER;  break;
        case HTRIGHT:       ht = HTBORDER;  break;
        case HTBOTTOM:      ht = HTBORDER;  break;
        case HTTOP:         ht = HTBORDER;  break;

        case HTTOPLEFT:     ht = HTLEFT;    break;
        case HTBOTTOMLEFT:  ht = HTLEFT;    break;
        }
    }
    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, ht);
    return TRUE;
}


//________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ObjBrowser(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static WINDOW_STATE win_state;

    switch (Message) {
        HANDLE_MSG(hwnd, WM_NCHITTEST, OnNcHitTest);

    case WM_SETFOCUS:
        hDlgCurrent = hwnd;
        return 0;
    case WM_MOUSEACTIVATE:
        hDlgCurrent = hwnd;
        return MA_ACTIVATE;
    case WM_INITDIALOG: {
        //Fallout_Debug_Info("main browser being initiated");
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(phinstDLL, MAKEINTRESOURCE(IDI_ICON1)));
        win_state = object_broser_state;

        INITCOMMONCONTROLSEX iccex{0};
        // Initialize common controls.
        iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        iccex.dwICC = ICC_TAB_CLASSES;
        InitCommonControlsEx(&iccex);

        TCITEM tie{ 0 };

        HWND hwndTab = GetDlgItem(hwnd, IDC_TAB1);
        // Add a tab for each of the three child dialog boxes.
        tie.mask = TCIF_TEXT | TCIF_IMAGE;
        tie.iImage = -1;
        
        wchar_t* text = new wchar_t[32]{0};
        
        LoadString(phinstDLL, IDS_PROTOTYPES, text, 32);
        //Fallout_Debug_Info("text %S", text);
        tie.pszText = text;
        TabCtrl_InsertItem(hwndTab, 0, &tie);

        LoadString(phinstDLL, IDS_SELECTED_OBJECTS, text, 32);
        //Fallout_Debug_Info("text %S", text);
        tie.pszText = text;
        TabCtrl_InsertItem(hwndTab, 1, &tie);

        delete[] text;
        text = nullptr;


        if (win_state == WINDOW_STATE::Child_Right) {
            hWinObjTabProto = CreateDialogParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_PRO_VERTICAL), hwnd, &DlgProc_ObjBrowser_Proto, 0);
            hWinObjTabObject = CreateDialogParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_PRO_VERTICAL), hwnd, &DlgProc_ObjBrowser_Objects, 0);
        }
        else {
            hWinObjTabProto = CreateDialogParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_PRO_HORIZONTAL), hwnd, &DlgProc_ObjBrowser_Proto, 0);
            hWinObjTabObject = CreateDialogParam(phinstDLL, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_PRO_HORIZONTAL), hwnd, &DlgProc_ObjBrowser_Objects, 0);
        }

        //use IDD_DIALOG_BROWSER_PRO_HORIZONTAL for undocked window but change alignment style to from left-right to top-down.
        if (win_state == WINDOW_STATE::Popup) {
            HWND hView = GetDlgItem(hWinObjTabProto, IDC_LIST1);
            LONG longstyle = GetWindowLong(hView, GWL_STYLE);
            longstyle &= ~LVS_ALIGNLEFT;
            longstyle |= LVS_ALIGNTOP;
            SetWindowLong(hView, GWL_STYLE, longstyle);
        }

        RECT rcTab;
        GetClientRect(hwnd, &rcTab);
        TabCtrl_AdjustRect(hwndTab, FALSE, &rcTab);

        SetWindowPos(hWinObjTabProto, nullptr, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, SWP_NOZORDER);
        SetWindowPos(hWinObjTabObject, nullptr, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, SWP_NOZORDER);

        TabCtrl_SetCurFocus(hwndTab, 0);
        ShowWindow(hWinObjTabProto, SW_SHOW);
        ShowWindow(hWinObjTabObject, SW_HIDE);

        return TRUE;
    }
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case TCN_SELCHANGE: {
            HWND hwndTab = GetDlgItem(hwnd, IDC_TAB1);
            int tabNum = TabCtrl_GetCurSel(hwndTab);
            if (tabNum == 1) {
                ShowWindow(hWinObjTabProto, SW_HIDE);
                ShowWindow(hWinObjTabObject, SW_SHOW);
            }
            else if (tabNum == 0) {
                ShowWindow(hWinObjTabProto, SW_SHOW);
                ShowWindow(hWinObjTabObject, SW_HIDE);
            }
        }
        default:
            break;
        }
        break;
    case WM_SIZE: {
        RECT rcHwnd;
        GetClientRect(hwnd, &rcHwnd);

        HWND hwndTab = GetDlgItem(hwnd, IDC_TAB1);
        RECT listRect;
        GetWindowRect(hwndTab, &listRect);
        POINT pt = { listRect.left, listRect.top }; //new point object using rect x, y
        ScreenToClient(hwnd, &pt); //convert screen co-ords to client based points
        MoveWindow(hwndTab, pt.x, pt.y, rcHwnd.right - rcHwnd.left - pt.x, rcHwnd.bottom - rcHwnd.top - pt.y, TRUE);

        // Calculate the display rectangle.
        RECT rcTab;
        GetClientRect(hwnd, &rcTab);
        TabCtrl_AdjustRect(hwndTab, FALSE, &rcTab);

        SetWindowPos(hWinObjTabProto, nullptr, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, SWP_NOZORDER);
        SetWindowPos(hWinObjTabObject, nullptr, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, SWP_NOZORDER);

        Mapper_Windows_Refresh_Size(GetParent(hwnd));

        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            //this is here because dialogs capture the "return" key for IDOK, preventing NM_RETURN message being set to list views.
            HWND hwndTab = GetDlgItem(hwnd, IDC_TAB1);
            HWND hwndSub = nullptr;
            int tabNum = TabCtrl_GetCurSel(hwndTab);
            if (tabNum == 1) 
                hwndSub = hWinObjTabObject;
            else if (tabNum == 0) 
                hwndSub = hWinObjTabProto;
            
            HWND hwndList = GetDlgItem(hwndSub, IDC_LIST1);
            if (GetFocus() == hwndList) {
                NMHDR nmhdr{ hwndList,IDC_LIST1, NM_RETURN };
                SendMessage(hwndSub, WM_NOTIFY, 0, (LPARAM)&nmhdr);
            }
            return FALSE;
        }
        case IDCANCEL:
            return FALSE;
            break;
        default:
            break;
        }
        break;
    case WM_CLOSE:
        ObjectBrowser_Close();
        return 0;
    case WM_DESTROY: {
        //Fallout_Debug_Info("main browser being destroyed");
        if (win_state == WINDOW_STATE::Popup) {
            wchar_t mapper_window_file[] = L"\\win_object_browser.dat";
            WinData_Save(hwnd, mapper_window_file, _countof(mapper_window_file));
            //Fallout_Debug_Info("saving obj brow data");
        }
        hWinObjBrowser = nullptr;
        Tool_SetState(*phWinMain, ID_OBJECTBROWSER_OPEN, TRUE, ObjectBrowser_Close(), TRUE);
        //Fallout_Debug_Info("main browser destroyed");
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("main browser destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//_____________________________________________________________________
HWND ObjectBrowser_Create(HWND hwnd, HINSTANCE hinstance, RECT* pRcWin) {
    if(hWinObjBrowser)
        return hWinObjBrowser;

    LONG browser_x = 0;
    LONG browser_y = 0;
    LONG browser_width = 640;
    LONG browser_height = 480;

    if (object_broser_state == WINDOW_STATE::Popup) {
        hWinObjBrowser = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_MAIN_POPUP), hwnd, (DLGPROC)DlgProc_ObjBrowser);
        wchar_t mapper_window_file[] = L"\\win_object_browser.dat";
        WinData_Load(hWinObjBrowser, mapper_window_file, _countof(mapper_window_file));
        
        if (!hWinObjBrowser)
            return hWinObjBrowser;
    }
    else {
        hWinObjBrowser = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_MAIN_CHILD), hwnd, (DLGPROC)DlgProc_ObjBrowser);
        if (!hWinObjBrowser)
            return hWinObjBrowser;
        
        SetWindowLongPtr(hWinObjBrowser, GWLP_ID, IDC_BROWSER);

        if (object_broser_state == WINDOW_STATE::Child_Bottom) {
            browser_width = pRcWin->right - pRcWin->left;
            browser_height = 160;
            browser_x = 0;
            browser_y = pRcWin->bottom - browser_height;
        }
        else if (object_broser_state == WINDOW_STATE::Child_Right) {
            browser_width = 176;
            browser_height = pRcWin->bottom - pRcWin->top;
            browser_x = pRcWin->right - browser_width;
            browser_y = 0;
        }
        MoveWindow(hWinObjBrowser, browser_x, browser_y, browser_width, browser_height, TRUE);
    }

    //display the proto list using the last selected type if the object browser was previously opened.
    ProtoList_Refresh();
    //display currently selected objects list.
    Selected_List_Update_State();
    ShowWindow(hWinObjBrowser, SW_SHOW);

    return hWinObjBrowser;
}


//_______________________
BOOL ObjectBrowser_Open() {

    if(!hWinObjBrowser){
        RECT rcClient{ 0 };
        GetClientRect(*phWinMain, &rcClient);
        HWND hStatus = GetDlgItem(*phWinMain, IDC_STATUS_BAR);
        if (hStatus) {
            RECT rcTemp{ 0 };
            GetWindowRect(hStatus, &rcTemp);
            rcClient.bottom -= (rcTemp.bottom - rcTemp.top);
        }
        ObjectBrowser_Create(*phWinMain, phinstDLL, &rcClient);
        Mapper_Windows_Refresh_Size(*phWinMain);
        Tool_SetState(*phWinMain, ID_OBJECTBROWSER_OPEN, TRUE, TRUE, TRUE);
        //Fallout_Debug_Info("ObjectBrowser_Open Opened");
    }
    //else
    //    Fallout_Debug_Info("ObjectBrowser_Open Already");

    return is_object_browser_opened = TRUE;
}


//________________________
BOOL ObjectBrowser_Close() {

    if(hWinObjBrowser){
        DestroyWindow(hWinObjBrowser);
        Mapper_Windows_Refresh_Size(*phWinMain);
        Tool_SetState(*phWinMain, ID_OBJECTBROWSER_OPEN, TRUE, FALSE, TRUE);
        //Fallout_Debug_Info("ObjectBrowser_Close Closed");
    }
    //else
    //    Fallout_Debug_Info("ObjectBrowser_Close Already");

    return is_object_browser_opened = FALSE;
}


//______________________________
BOOL ObjectBrowser_Toggle_Open() {

    if (is_object_browser_opened)
        ObjectBrowser_Close();
    else
        ObjectBrowser_Open();

    return is_object_browser_opened;
}


//____________________________
BOOL IsObjectBrowserOpened() {
    return is_object_browser_opened;
}


//_______________________________________________________________
WINDOW_STATE ObjectBrowser_SetView(WINDOW_STATE new_browser_view) {
    //Fallout_Debug_Info("Set_Object_Browser_View1 new:%d", new_browser_view);
    BOOL view[3]{ FALSE, FALSE, FALSE };
    if (new_browser_view == WINDOW_STATE::Child_Bottom)
        view[0] = TRUE;
    else if (new_browser_view == WINDOW_STATE::Child_Right)
        view[1] = TRUE;
    else if (new_browser_view == WINDOW_STATE::Popup)
        view[2] = TRUE;
    else
        return object_broser_state;
    //Fallout_Debug_Info("Set_Object_Browser_View2 new:%d", new_browser_view);

    //Fallout_Debug_Info("Set_Object_Browser_View3 new:%d", new_browser_view);
    Tool_SetState(*phWinMain, ID_OBJECTBROWSER_BOTTOM, TRUE, view[0], FALSE);
    Tool_SetState(*phWinMain, ID_OBJECTBROWSER_RIGHT, TRUE, view[1], FALSE);
    Tool_SetState(*phWinMain, ID_OBJECTBROWSER_UNDOCKED, TRUE, view[2], TRUE);

    if (object_broser_state == new_browser_view)
        return object_broser_state;
    object_broser_state = new_browser_view;

    if (IsObjectBrowserOpened()) {
        ObjectBrowser_Close();
        ObjectBrowser_Toggle_Open();
    }
    return object_broser_state;
}
