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


#include "../resource.h"
#include "win_ObjectEditor.h"
#include "win_ProtoBrowser.h"

#include "proto_cache.h"
#include "mapper_tools.h"

#include "../Dx_Graphics.h"
#include "../Dx_Game.h"

#include "../errors.h"
#include "../game_map.h"
#include "../Fall_General.h"
#include "../Fall_Scripts.h"
#include "../Fall_File.h"
#include "../Fall_Objects.h"
#include "../Fall_Msg.h"

WNDPROC pOldProc_InvViewTab;
WNDPROC pOldProc_ItemsViewTab;

HBITMAP hBitmap_checkBoxes = nullptr;
HBITMAP hBitmap_wieldBoxes = nullptr;

enum class WIELD_TYPE : LONG {
    worn = 0,
    left = 1,
    right = 2
};


//_________________________________________________________________________________________________________
ITEMblock* Find_Inv_Item_With_Matching_Pro(OBJStruct* pObj_Owner, DWORD proID, OBJStruct** ppObj_ret_Owner) {

    ITEMblock* pItem_container = nullptr;
    for (LONG i = 0; i < pObj_Owner->pud.general.inv_size; i++) {
        if (pObj_Owner->pud.general.p_item[i].p_obj) {
            if (fall_Obj_Item_GetType(pObj_Owner->pud.general.p_item[i].p_obj) == ITEM_TYPE::container) {
                if (pItem_container = Find_Inv_Item_With_Matching_Pro(pObj_Owner->pud.general.p_item[i].p_obj, proID, ppObj_ret_Owner))
                    return pItem_container;
            }
            if (pObj_Owner->pud.general.p_item[i].p_obj->proID == proID) {
                if(ppObj_ret_Owner)
                    *ppObj_ret_Owner = pObj_Owner;
                return &pObj_Owner->pud.general.p_item[i];
            }
        }
    }
    return nullptr;
    *ppObj_ret_Owner = nullptr;
}


struct PROTO_NODE {
    DWORD proID;
    PROTO_NODE* next;
    PROTO_NODE() {
        proID = -1;
        next = nullptr;
    }
};

//___________________________________________
class ITEM_LIST_CACHE : public LIST_VIEW_DATA {
public:
    ITEM_LIST_CACHE(OBJStruct* p_in_Obj) :
        LIST_VIEW_DATA()
    {
        pObj_Owner = p_in_Obj;
        p_InvProtoList = nullptr;
        Compile_Item_List(pObj_Owner);
        PROTO_cache_type* pItemCache = ProtCache_GetType(ART_ITEMS);

        if(pItemCache)
        Setup_List(pItemCache->Get_NumberOfItems(), TRUE, TRUE, 70, 70);
    }
    ~ITEM_LIST_CACHE() {
        if (p_InvProtoList) {
            PROTO_NODE* node = p_InvProtoList;
            while (p_InvProtoList) {
                node = p_InvProtoList->next;
                delete p_InvProtoList;
                p_InvProtoList = node;
            }
        }
    }
    void Update() {
        PROTO_cache_type* pItemCache = ProtCache_GetType(ART_ITEMS);
        if (pItemCache)
        Setup_List(pItemCache->Get_NumberOfItems(), TRUE, TRUE, 70, 70);
    }
    BOOL Add_Item_To_Selected_List(DWORD proID) {

        PROTO_NODE* pNode = p_InvProtoList;
        
        if (!pNode) {
            p_InvProtoList = new PROTO_NODE;
            p_InvProtoList->proID = proID;
            //Fallout_Debug_Info("list created item added to list proid:%X", proID);
            return TRUE;
        }

        while (pNode) {
            if (pNode->proID == proID) {
                //Fallout_Debug_Info("item already added to list proid:%X", proID);
                return TRUE;
            }
            if (pNode->next == nullptr) {
                pNode->next = new PROTO_NODE;
                pNode->next->proID = proID;
                //Fallout_Debug_Info("item added to list proid:%X", proID);
                return TRUE;
            }
            pNode = pNode->next;
        }
        return TRUE;
    }
    BOOL Remove_Item_From_Selected_List(DWORD proID) {

        PROTO_NODE* pNode_last = nullptr;
        PROTO_NODE* pNode = p_InvProtoList;

        while (pNode) {
            if (pNode->proID == proID) {
                if (pNode_last)
                    pNode_last->next = pNode->next;
                else
                    p_InvProtoList = pNode->next;
                delete pNode;
                return TRUE;
            }
            pNode_last = pNode;
            pNode = pNode->next;
        }
        return TRUE;
    }
    BOOL Is_Item_Selected(DWORD proID) {

        PROTO_NODE* pNode = p_InvProtoList;
        while (pNode) {
            if (pNode->proID == proID)
                return TRUE;
            pNode = pNode->next;
        }
        return FALSE;
    }
    PROTO* Get_Proto(LONG item_num) {
        PROTO_cache_type* pItemCache = ProtCache_GetType(ART_ITEMS);
        if (!pItemCache)
            return nullptr;
        return pItemCache->Get_Proto(item_num);
    };
    wchar_t* Get_Text(LONG item_num) {
        PROTO_cache_type* pItemCache = ProtCache_GetType(ART_ITEMS);
        if (!pItemCache)
            return nullptr;
        return pItemCache->Get_Text(item_num);
    };
    void Update_Object_Inventory() {
        if (!pObj_Owner)
            return;

        PROTO_NODE* remove_list = new PROTO_NODE;
        PROTO_NODE* node = remove_list;

        //make a list of item in the objects inventory that are not selected.
        for (LONG i = 0; i < pObj_Owner->pud.general.inv_size; i++) {
            if (pObj_Owner->pud.general.p_item[i].p_obj) {
                if (!Is_Item_Selected(pObj_Owner->pud.general.p_item[i].p_obj->proID)) {
                    node->proID = pObj_Owner->pud.general.p_item[i].p_obj->proID;
                    node->next = new PROTO_NODE;
                    node = node->next;
                }
            }
        }

        //remove all unselected items from the objects inventory.
        ITEMblock* item = nullptr;
        OBJStruct* pObj_ret_Owner = nullptr;
        OBJStruct* pObj_item = nullptr;
        while (remove_list) {
            while ((item = Find_Inv_Item_With_Matching_Pro(pObj_Owner, remove_list->proID, &pObj_ret_Owner)) != nullptr) {
                pObj_item = item->p_obj;
                fall_Obj_Inventory_RemoveItems(pObj_ret_Owner, pObj_item, item->num);
                fall_Obj_Destroy(pObj_item, nullptr);
            }
            node = remove_list->next;
            delete remove_list;
            remove_list = node;
        }

        //add selected items not currently included in objects inventory.
        node = p_InvProtoList;
        while (node) {
            if (!Find_Inv_Item_With_Matching_Pro(pObj_Owner, node->proID, nullptr)) {

                PROTO* pPro_item = nullptr;
                //create a new item object using the selected items proID and add it to the inventory.
                if (fall_GetPro(node->proID, &pPro_item) == 0) {
                    OBJStruct* pObj_new = nullptr;
                    if (fall_Obj_Create(&pObj_new, pPro_item->frmID, node->proID) == 0) {
                        if (fall_Obj_Inventory_AddItems(pObj_Owner, pObj_new, 1) == 0)
                            fall_Obj_Disconnect_From_Map(pObj_new, nullptr);
                        else
                            fall_Obj_Destroy(pObj_new, nullptr);//destroy object if it cannot be added to the inventory
                    }
                }
            }

            node = node->next;
        }
    }
protected:
private:
    PROTO_NODE* p_InvProtoList;
    OBJStruct* pObj_Owner;

    void Compile_Item_List(OBJStruct* pObj) {
        if (!pObj)
            return;
        for (LONG i = 0; i < pObj->pud.general.inv_size; i++) {
            if (pObj->pud.general.p_item[i].p_obj) {
                if (fall_Obj_Item_GetType(pObj->pud.general.p_item[i].p_obj) == ITEM_TYPE::container)
                    Compile_Item_List(pObj->pud.general.p_item[i].p_obj);
                Add_Item_To_Selected_List(pObj->pud.general.p_item[i].p_obj->proID);
            }
        }
    };
};

ITEM_LIST_CACHE* p_item_cache = nullptr;


//________________________________________________________
BOOL Draw_CheckBox(HDC hdc, HBITMAP hBitmap, BOOL checked) {
    if (hBitmap) {
        
        HDC hDCBits_check = CreateCompatibleDC(hdc);
        BITMAP Bitmap_check{ 0 };
        GetObject(hBitmap_checkBoxes, sizeof(BITMAP), (LPSTR)&Bitmap_check);
        HGDIOBJ hgdiobj_check_old = SelectObject(hDCBits_check, hBitmap_checkBoxes);

        HDC hDCBits = CreateCompatibleDC(hdc);
        BITMAP Bitmap{ 0 };
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
        HGDIOBJ hgdiobj_old = SelectObject(hDCBits, hBitmap);

        BITMAPINFO* pBMP_Info = new BITMAPINFO{ 0 };
        pBMP_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pBMP_Info->bmiHeader.biWidth = Bitmap_check.bmWidth;
        pBMP_Info->bmiHeader.biHeight = Bitmap_check.bmHeight;
        pBMP_Info->bmiHeader.biBitCount = 32;
        pBMP_Info->bmiHeader.biPlanes = Bitmap_check.bmPlanes;

        pBMP_Info->bmiHeader.biCompression = BI_RGB;
        pBMP_Info->bmiHeader.biXPelsPerMeter = 1000;
        pBMP_Info->bmiHeader.biYPelsPerMeter = 1000;
        pBMP_Info->bmiHeader.biClrUsed = 0;
        pBMP_Info->bmiHeader.biClrImportant = 0;
 
        DWORD dwBmpSize = Bitmap_check.bmWidth  * 4 * Bitmap_check.bmHeight;
        BYTE* pbytes = new BYTE[dwBmpSize];
        GetDIBits(hDCBits_check, hBitmap_checkBoxes, 0, (UINT)Bitmap_check.bmHeight, pbytes, pBMP_Info, DIB_RGB_COLORS);
        //need to set the alpha channel so that the check box displays opaque.
        for (DWORD d = 0; d < dwBmpSize; d+=4)
            pbytes[d+3] = 0xFF;
        
        if (checked)
            SetDIBitsToDevice(hDCBits, 0, 0, Bitmap_check.bmWidth / 2, Bitmap_check.bmHeight, Bitmap_check.bmWidth / 2, 0, 0, Bitmap_check.bmHeight, pbytes, pBMP_Info, DIB_RGB_COLORS);
        else
            SetDIBitsToDevice(hDCBits, 0, 0, Bitmap_check.bmWidth / 2, Bitmap_check.bmHeight, 0, 0, 0, Bitmap_check.bmHeight, pbytes, pBMP_Info, DIB_RGB_COLORS);

        SelectObject(hDCBits, hgdiobj_old);
        DeleteDC(hDCBits);
        SelectObject(hDCBits_check, hgdiobj_check_old);
        DeleteDC(hDCBits_check);
        delete pBMP_Info;
        delete pbytes;
    }
    return TRUE;
}


//_____________________________________________________
BOOL ItemList_SetItemData(NMLVDISPINFO* p_display_info) {

    if (!p_item_cache)
        return FALSE;

    HWND h_ListView = p_display_info->hdr.hwndFrom;

    LVITEM* lplvI = &p_display_info->item;

    if (lplvI->iItem >= p_item_cache->Get_NumberOfFilteredItems()) {
        lplvI->mask |= LVIF_DI_SETITEM;//set mask to not repeat message
        return FALSE;
    }

    LONG itemNum = p_item_cache->Get_FilteredItem(lplvI->iItem);

    if (lplvI->mask & LVIF_TEXT) {

        wchar_t* chars = p_item_cache->Get_Text(itemNum);
        if (chars)
            wcsncpy_s(lplvI->pszText, lplvI->cchTextMax, chars, _TRUNCATE);
        else
            lplvI->pszText[0] = '\0';
    }

    if (lplvI->mask & LVIF_IMAGE) {
        DWORD frmID = -1;
        PROTO* proto = p_item_cache->Get_Proto(itemNum);
        if (proto)
            frmID = proto->item.invFrmID;
        if (frmID == -1)
            return FALSE;
        DWORD bgColour = ListView_GetBkColor(h_ListView);
        HIMAGELIST hLarge = p_item_cache->Get_ImageList();
        if (!hLarge)
            return FALSE;
        int iconW = 0;
        int iconH = 0;
        ImageList_GetIconSize(hLarge, &iconW, &iconH);

        if (p_item_cache->Get_ImageRef(itemNum) == -1) {
            HDC hdc = GetDC(h_ListView);
            HBITMAP bitmap = CreateIconFRM(hdc, frmID, 0, iconW, iconH, &bgColour);
            
            if (bitmap) {
                Draw_CheckBox(hdc, bitmap, p_item_cache->Is_Item_Selected(proto->proID));
                lplvI->iImage = p_item_cache->Set_Image(bitmap, itemNum);
                if (lplvI->iImage == -1)
                    Fallout_Debug_Error("ItemList_SetItemData ImageList_Add failed - tile, frmID:%X sortnum:%d", frmID, itemNum);
                DeleteObject(bitmap);
            }

            ReleaseDC(h_ListView, hdc);
            return TRUE;
        }
        else
            lplvI->iImage = p_item_cache->Get_ImageRef(itemNum);
    }

    return TRUE;
}


//____________________________________________________
void ItemList_Filter_Type(HWND hwnd_parent, LONG type) {
    if (!p_item_cache)
        return;
    HWND h_ListView = GetDlgItem(hwnd_parent, IDC_LIST1);
    ListView_RedrawItems(h_ListView, 0, ListView_GetItemCount(h_ListView));
    InvalidateRect(h_ListView, nullptr, true);

    DWORD list_count_type_filtered = 0;

    PROTO* pPro = nullptr;
    LONG item_type = 0;

    for (LONG i = 0; i < p_item_cache->Get_NumberOfItems(); i++) {
        pPro = p_item_cache->Get_Proto(p_item_cache->Get_SortedItem(i));
        if (pPro) {
            if (pPro->item.invFrmID == -1)//don't include items without an inventory frm. This isn't done in the Bis-Mapper but seems logical to me.
                item_type = -1;
            else
                item_type = pPro->item.itemType;
        }
        else
            item_type = -1;

        if (type < 0 || type == item_type) {
            p_item_cache->Set_FilteredItem(list_count_type_filtered, p_item_cache->Get_SortedItem(i));
            list_count_type_filtered++;
        }
    }
    p_item_cache->Set_NumberOfFilteredItems(list_count_type_filtered);

    ListView_DeleteAllItems(h_ListView);
    ListView_SetItemCountEx(h_ListView, list_count_type_filtered, 0);
}


//______________________________
void ItemList_SortBy_Selection() {
    if (!p_item_cache)
        return;
    for (LONG i = 0; i < p_item_cache->Get_NumberOfItems(); i++) {
        p_item_cache->Set_SortedItem(i, i);
    }
}


//________________________
BOOL ItemList_SortByText() {
    if (!p_item_cache)
        return FALSE;
    wchar_t* str1 = 0, * str2 = 0;

    LONG lposHold = 0, lposHoldVal = 0;
    LONG lposVar = 0;
    for (LONG lpos = 1; lpos < p_item_cache->Get_NumberOfItems(); lpos++) {
        lposHold = lpos;
        lposHoldVal = p_item_cache->Get_SortedItem(lposHold);
        lposVar = lpos - 1;

        str1 = p_item_cache->Get_Text(p_item_cache->Get_SortedItem(lposHold));
        str2 = p_item_cache->Get_Text(p_item_cache->Get_SortedItem(lposVar));

        if (str1 && str2) {
            while (lposVar >= 0 && _wcsicmp(str2, str1) > 0) {
                p_item_cache->Set_SortedItem(lposVar + 1, p_item_cache->Get_SortedItem(lposVar));
                lposVar--;
                if (lposVar >= 0)
                    str2 = p_item_cache->Get_Text(p_item_cache->Get_SortedItem(lposVar));
            }
        }
        p_item_cache->Set_SortedItem(lposVar + 1, lposHoldVal);
    }
    return TRUE;
}


//____________________________
BOOL ItemList_Sort(LONG num) {

    if (num == 0)
        ItemList_SortBy_Selection();
    else if (num == 1)
        ItemList_SortByText();
    else
        return FALSE;

    return TRUE;
}


//_____________________________________
void ItemList_Arrange(HWND hwnd_parent) {

    HWND h_combo = GetDlgItem(hwnd_parent, IDC_COMBO_SORT);
    ItemList_Sort(SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));

    h_combo = GetDlgItem(hwnd_parent, IDC_COMBO_TYPE);
    ItemList_Filter_Type(hwnd_parent, SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) - 1);
}


//___________________________________________
BOOL ItemList_ToggleSelection(HWND hwnd_list) {
    if (!p_item_cache)
        return FALSE;

    LONG selectedItem = ListView_GetNextItem(hwnd_list, -1, LVNI_FOCUSED | LVNI_SELECTED);

    if (selectedItem < 0 || selectedItem >= p_item_cache->Get_NumberOfFilteredItems())
        return FALSE;

    LONG item_num = p_item_cache->Get_FilteredItem(selectedItem);
    PROTO* pPro = p_item_cache->Get_Proto(item_num);

    if (pPro) {
        if (p_item_cache->Is_Item_Selected(pPro->proID))
            p_item_cache->Remove_Item_From_Selected_List(pPro->proID);
        else
            p_item_cache->Add_Item_To_Selected_List(pPro->proID);

        DWORD frmID = -1;

        if (pPro)
            frmID = pPro->item.invFrmID;
        if (frmID == -1)
            return FALSE;
        DWORD bgColour = ListView_GetBkColor(hwnd_list);
        HIMAGELIST hLarge = p_item_cache->Get_ImageList();
        if (!hLarge)
            return FALSE;
        int iconW = 0;
        int iconH = 0;
        ImageList_GetIconSize(hLarge, &iconW, &iconH);

        HDC hdc = GetDC(hwnd_list);
        HBITMAP bitmap = CreateIconFRM(hdc, frmID, 0, iconW, iconH, &bgColour);

        if (bitmap) {
            Draw_CheckBox(hdc, bitmap, p_item_cache->Is_Item_Selected(pPro->proID));
            p_item_cache->Set_Image(bitmap, item_num);
            DeleteObject(bitmap);
        }

        ReleaseDC(hwnd_list, hdc);
        ListView_Update(hwnd_list, selectedItem);
        return TRUE;
    }
    return FALSE;
}


//_______________________________________________________________________________________________
LRESULT CALLBACK WinProc_ITemsViewTabSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    LRESULT lres;
    switch (uMsg) {
    case WM_GETDLGCODE:
        lres = CallWindowProc(pOldProc_ItemsViewTab, hwnd, uMsg, wParam, lParam);
        lres &= ~DLGC_WANTTAB;
        if (lParam &&
            ((MSG*)lParam)->message == WM_KEYDOWN &&
            ((MSG*)lParam)->wParam == VK_TAB) {
            lres &= ~DLGC_WANTMESSAGE;
            PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
        }
        return lres;
    }
    return CallWindowProc(pOldProc_ItemsViewTab, hwnd, uMsg, wParam, lParam);
}


//_________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ItemBrowser(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {

    case WM_INITDIALOG: {
        if (p_item_cache)
            delete p_item_cache;

        p_item_cache = new ITEM_LIST_CACHE((OBJStruct*)lParam);

        hBitmap_checkBoxes = (HBITMAP)LoadImage(phinstDLL, (LPCWSTR)MAKEINTRESOURCE(IDB_BITMAP_CHECKBOX), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

        RECT rcHwnd;
        GetClientRect(hwnd, &rcHwnd);
        HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
        pOldProc_ItemsViewTab = (WNDPROC)SetWindowLongPtr(h_ListView, GWLP_WNDPROC, (LONG_PTR)WinProc_ITemsViewTabSubClass);
        ListView_DeleteAllItems(h_ListView);

        ListView_SetItemCountEx(h_ListView, p_item_cache->Get_NumberOfItems(), LVSICF_NOINVALIDATEALL);
        ListView_SetImageList(h_ListView, p_item_cache->Get_ImageList(), LVSIL_NORMAL);

        //item combo box setup - type filter
        HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_TYPE);
        wchar_t* str = new wchar_t[16];
        for (LONG type = -1; type <= 6; type++) {
            LoadString(phinstDLL, Get_Item_Type_TextID(type), str, 16);
            SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
            //Fallout_Debug_Info("%S", str);
        }
        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        //item combo box setup - sort
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
            return TRUE;
            break;
        }
        case NM_CLICK: {
            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
            ItemList_ToggleSelection(lpnmitem->hdr.hwndFrom);
            return TRUE;
            break;
        }

        case NM_RETURN: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            int selectedItem = ListView_GetNextItem(lpnm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
            if (selectedItem != -1) {
             ItemList_ToggleSelection(lpnm->hwndFrom);
            }
            return TRUE;
            break;
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
            ItemList_SetItemData((NMLVDISPINFO*)lParam);
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
            if (HIWORD(wParam) == CBN_SELCHANGE) 
                ItemList_Arrange(hwnd);
            return TRUE;
        }
        case IDC_COMBO_SORT: {
            if (HIWORD(wParam) == CBN_SELCHANGE) 
                ItemList_Arrange(hwnd);
            return TRUE;
        }
        case IDOK: {
            //this is here because dialogs capture the "return" key for IDOK, preventing NM_RETURN message being set to list views.
            HWND hwndList = GetDlgItem(hwnd, IDC_LIST1);
            if (GetFocus() == hwndList) {
                NMHDR nmhdr{ hwndList,IDC_LIST1, NM_RETURN };
                SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)&nmhdr);
                break;
            }
            if (p_item_cache)
                p_item_cache->Update_Object_Inventory();
            EndDialog(hwnd, 0);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        if (p_item_cache)
            delete p_item_cache;
        p_item_cache = nullptr;

        DeleteObject(hBitmap_checkBoxes);
        hBitmap_checkBoxes = nullptr;
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("item browser destroyed");
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//____________________________________________________________________________
int Dialog_ItemSelector(HWND hwndParent, HINSTANCE hinstance, OBJStruct* pObj) {

    return DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_ITEM_SELECTOR), hwndParent, DlgProc_ItemBrowser, (LPARAM)pObj);
}


//__________________________________________________
class INVENTORY_LIST_CACHE : public LIST_VIEW_DATA {
public:
    INVENTORY_LIST_CACHE(OBJStruct* p_in_Obj) :
        LIST_VIEW_DATA()
    {
        pObj = p_in_Obj;
        if(pObj)
            Setup_List(pObj->pud.general.inv_size, TRUE, TRUE, 70, 70);
    }
    ~INVENTORY_LIST_CACHE() {
        pObj = nullptr;
    }
    void Update_Inv() {
        if (pObj)
            Setup_List(pObj->pud.general.inv_size, TRUE, TRUE, 70, 70);
    }
    OBJStruct* GetOwnerObj() {
        return pObj;
    };

    ITEMblock* Get_Item(LONG item) {
        if (!pObj || item < 0 || item >= Get_NumberOfItems())
            return nullptr;
        return &pObj->pud.general.p_item[item];
    }

    PROTO* Get_Proto(LONG item) {
        //OBJStruct* pObj = GetInvObj(item);
        ITEMblock* item_block = Get_Item(item);
        LONG proType = (item_block->p_obj->proID & 0x0F000000) >> 0x18;
        LONG proNum = (item_block->p_obj->proID & 0x00FFFFFF) - 1;

        PROTO_cache_type* pItemCache = ProtCache_GetType(proType);
        if (!pItemCache)
            return nullptr;

            return pItemCache->Get_Proto(proNum);
        
    };
    wchar_t* Get_Text(LONG item) {
        //OBJStruct* pObj = GetInvObj(item);
        ITEMblock* item_block = Get_Item(item);
        LONG proType = (item_block->p_obj->proID & 0x0F000000) >> 0x18;
        LONG proNum = (item_block->p_obj->proID & 0x00FFFFFF) - 1;

        PROTO_cache_type* pItemCache = ProtCache_GetType(ART_ITEMS);
        if (!pItemCache)
            return nullptr;
        return pItemCache->Get_Text(proNum);
    };
protected:
private:
    OBJStruct* pObj;
};

INVENTORY_LIST_CACHE* inv_cache = nullptr;


//____________________________________________________
LONG Get_Item_Quantity(OBJStruct* pObj, LONG numItems) {

    LONG itemType = fall_Obj_Item_GetType(pObj);
    if (itemType == ITEM_TYPE::ammo) {
        numItems--;
        LONG maxAmmo = fall_Obj_Item_GetMaxAmmo(pObj);
        numItems *= maxAmmo;
        numItems += fall_Obj_Item_GetCurrentAmmo(pObj);
    }
    return numItems;
}


//________________________________________________________
wchar_t* Create_Item_Text(OBJStruct* pObj, LONG num_items) {
    LONG proType = (pObj->proID & 0x0F000000) >> 0x18;
    LONG proNum = (pObj->proID & 0x00FFFFFF) - 1;
    wchar_t* chars = nullptr;
    
    PROTO_cache_type* pItemCache = ProtCache_GetType(proType);
    if (pItemCache) {
        wchar_t* item_text = pItemCache->Get_Text(proNum);
        if (item_text) {
            DWORD num_chars = wcslen(item_text) + 1 + 13;
            chars = new wchar_t[num_chars] {0};
            swprintf_s(chars, num_chars, L"%d x %s", Get_Item_Quantity(pObj, num_items), item_text);
        }
    }
    return chars;
}


//________________________________________________________________
wchar_t* Create_Item_Text_From_ProId(DWORD proID, LONG num_rounds) {
    LONG proType = (proID & 0x0F000000) >> 0x18;
    LONG proNum = (proID & 0x00FFFFFF) - 1;
    wchar_t* chars = nullptr;

    PROTO_cache_type* pItemCache = ProtCache_GetType(proType);
    if (pItemCache) {
        wchar_t* item_text = pItemCache->Get_Text(proNum);
        if (item_text) {
            DWORD num_chars = wcslen(item_text) + 1 + 13;
            chars = new wchar_t[num_chars] {0};
            swprintf_s(chars, num_chars, L"%d x %s", num_rounds, item_text);
        }
    }
    return chars;
}


struct AMMO_NODE {
    DWORD proID;
    LONG num;
    AMMO_NODE* next;
    AMMO_NODE() {
        proID = -1;
        num = 0;
        next = nullptr;
    }
};


//__________________________________________________________________________________________________________________
LONG Find_Compatible_Ammo_In_Inv(OBJStruct* pObj_owner, OBJStruct* pObj_weapon, DWORD** proID_ammo, HWND hwnd_combo) {
    PROTO* pPro_item = nullptr;
    LONG num_ammo_types = 0;

    AMMO_NODE* pPro_list = new AMMO_NODE;
    AMMO_NODE* pPro_node_current = pPro_list;

    LONG weapon_current_ammo = fall_Obj_Item_GetCurrentAmmo(pObj_weapon);

    if (weapon_current_ammo > 0) {
        pPro_node_current->proID = fall_Obj_Item_Weapon_GetAmmoProID(pObj_weapon);
        pPro_node_current->num = weapon_current_ammo;
        pPro_node_current->next = new AMMO_NODE;
        pPro_node_current = pPro_node_current->next;
        num_ammo_types++;
    }


    bool pro_match = false;
    AMMO_NODE* pPro_node = nullptr;

    if (fall_GetPro(pObj_weapon->proID, &pPro_item) == 0) {
        DWORD weapon_ammo_type = pPro_item->item.itemTypeData.data14;
        OBJStruct* pObj_ammo = nullptr;

        for (LONG i = 0; i < pObj_owner->pud.general.inv_size; i++) {
            pObj_ammo = pObj_owner->pud.general.p_item[i].p_obj;
            if (pObj_ammo && fall_Obj_Item_GetType(pObj_ammo) == ITEM_TYPE::ammo) {
                if (fall_GetPro(pObj_ammo->proID, &pPro_item) == 0) {
                    if (pPro_item->item.itemTypeData.data01 == weapon_ammo_type) {
                        pro_match = false;
                        pPro_node = pPro_list;
                        while (pPro_node) {
                            if (pPro_node->proID == pObj_ammo->proID) {
                                pPro_node->num += Get_Item_Quantity(pObj_ammo, pObj_owner->pud.general.p_item[i].num);
                                pro_match = true;
                            }
                            pPro_node = pPro_node->next;
                        }
                        if (!pro_match) {
                            pPro_node_current->proID = pObj_ammo->proID;
                            pPro_node_current->num = Get_Item_Quantity(pObj_ammo, pObj_owner->pud.general.p_item[i].num);
                            pPro_node_current->next = new AMMO_NODE;
                            pPro_node_current = pPro_node_current->next;
                            num_ammo_types++;
                        }
                    }
                }
            }
        }

        if (num_ammo_types == 0) {
            num_ammo_types++;
        }

        if (proID_ammo) {
            *proID_ammo = new DWORD[num_ammo_types];
            DWORD* p_ammo_proID = *proID_ammo;
            LONG count = 0;
           
            while (pPro_list) {
                if (count < num_ammo_types) {
                    p_ammo_proID[count] = pPro_list->proID;
                    if (hwnd_combo) {
                        if (pPro_list->proID == -1) {
                            SendMessage(hwnd_combo, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                            SendMessage(hwnd_combo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"none");
                        }
                        else {
                            wchar_t* chars = Create_Item_Text_From_ProId(pPro_list->proID, pPro_list->num);
                            SendMessage(hwnd_combo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)chars);
                            //Fallout_Debug_Info("insert compatable ammo: %S", chars);
                            if (chars)
                                delete chars;
                        }
                    }

                }
                pPro_node = pPro_list->next;
                delete pPro_list;
                pPro_list = pPro_node;
                count++;
            }
        }
    }
    return num_ammo_types;
}


//_____________________________________________________________________________
ITEMblock* Get_Inv_Item_With_Matching_ProID(OBJStruct* pObj_owner, DWORD proID) {
    if (!pObj_owner)
        return nullptr;

    for (LONG i = 0; i < pObj_owner->pud.general.inv_size; i++) {
        if (pObj_owner->pud.general.p_item[i].p_obj && pObj_owner->pud.general.p_item[i].p_obj->proID == proID)
            return &pObj_owner->pud.general.p_item[i];
    }
    return nullptr;
}


//___________________________________
LONG Pro_Item_GetMaxAmmo(DWORD proID) {
    if ((proID >> 24) != ART_ITEMS)
        return 0;
    PROTO* pPro = nullptr;
    if (fall_GetPro(proID, &pPro) == 0) {
        if (pPro->item.itemType == ITEM_TYPE::ammo)
            return pPro->item.itemTypeData.data02;
        else
            return pPro->item.itemTypeData.data16;
    }
}


//_________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_SetItemAmmo(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static OBJStruct* pObj_weapon = nullptr;
    static DWORD* p_ammo_inv_proID = nullptr;

    switch (Message) {

    case WM_INITDIALOG: {

        pObj_weapon = (OBJStruct*)lParam;
        LONG ammo = fall_Obj_Item_GetCurrentAmmo(pObj_weapon);
        DWORD ammo_proID = fall_Obj_Item_Weapon_GetAmmoProID(pObj_weapon);


        HWND hwnd_sub = nullptr;

        //setup combo box - create a list of compatible ammo from the inventory
        hwnd_sub = GetDlgItem(hwnd, IDC_COMBO1);

        p_ammo_inv_proID = nullptr;
        LONG num_ammo_types = Find_Compatible_Ammo_In_Inv(inv_cache->GetOwnerObj(), pObj_weapon, &p_ammo_inv_proID, hwnd_sub);

        if (num_ammo_types == 1 && p_ammo_inv_proID[0] == -1)
            SendMessage(hwnd_sub, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        else {
            for (LONG i = 0; i < num_ammo_types; i++) {
                if (p_ammo_inv_proID[i] == ammo_proID) 
                    SendMessage(hwnd_sub, CB_SETCURSEL, (WPARAM)i, (LPARAM)0);
            }
        }

        //setup slider - for adjusting ammo quantity
        hwnd_sub = GetDlgItem(hwnd, IDC_SLIDER1);

        LONG ammo_lot_size = 0;
        if (ammo_proID != -1)
            ammo_lot_size = Pro_Item_GetMaxAmmo(ammo_proID);

        SendMessage(hwnd_sub, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)0);
        SendMessage(hwnd_sub, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)fall_Obj_Item_GetMaxAmmo(pObj_weapon));
        SendMessage(hwnd_sub, TBM_SETLINESIZE, 0, (LPARAM)ammo_lot_size);
        SendMessage(hwnd_sub, TBM_SETPAGESIZE, 0, (LPARAM)ammo_lot_size);

        SendMessage(hwnd_sub, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)ammo);

        //static text - display current ammo quantity below slider
        SetDlgItemInt(hwnd, IDC_STATIC_QUANTITY, ammo, TRUE);
        return TRUE;
    }

    case WM_NOTIFY:
        break;
    case WM_HSCROLL:

        if ((HWND)lParam != nullptr) {
            int idc_slider = GetDlgCtrlID((HWND)lParam);
            if (idc_slider == IDC_SLIDER1) {
                LONG ammo_current = fall_Obj_Item_GetCurrentAmmo(pObj_weapon);
                LONG ammo_new = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
                LONG ammo_diff = abs(ammo_new - ammo_current);


                if (ammo_new < ammo_current) {//moving ammo from weapon to inv
                    DWORD ammo_proID = fall_Obj_Item_Weapon_GetAmmoProID(pObj_weapon);
                    LONG ammo_lot_size = Pro_Item_GetMaxAmmo(ammo_proID);
                    PROTO* pPro_item = nullptr;
                    LONG ammo_lot_available = 0;
                    if (fall_GetPro(ammo_proID, &pPro_item) == 0) {
                        OBJStruct* pObj_new = nullptr;
                        while (ammo_diff > 0) {
                            if (ammo_diff >= ammo_lot_size)
                                ammo_lot_available = ammo_lot_size;
                            else
                                ammo_lot_available = ammo_diff;

                            if (fall_Obj_Create(&pObj_new, pPro_item->frmID, ammo_proID) == 0) {
                                fall_Obj_Item_SetCurrentAmmo(pObj_new, ammo_lot_available);
                                fall_Obj_Disconnect_From_Map(pObj_new, nullptr);
                                if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_new, 1) != 0)
                                    fall_Obj_Destroy(pObj_new, nullptr);//destroy the object if it cannot be added to the inventory
                            }
                            ammo_diff -= ammo_lot_available;
                            ammo_current -= ammo_lot_available;
                        }
                    }
                }
                else if (ammo_new > ammo_current) {//moving ammo from inv to weapon
                    //get selected ammo proID
                    DWORD ammo_proID = p_ammo_inv_proID[SendMessage(GetDlgItem(hwnd, IDC_COMBO1), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0)];
                    // make sure the current weapon ammo proID matches the loading ammo's proID.
                    if (pObj_weapon->pud.general.pud.weapon.cur_ammo_type_pid == ammo_proID) {

                        ITEMblock* pItem = Get_Inv_Item_With_Matching_ProID(inv_cache->GetOwnerObj(), ammo_proID);
                        if (pItem) {
                            LONG inv_ammo_lot_available = fall_Obj_Item_GetCurrentAmmo(pItem->p_obj);
                            bool inv_ammo_depleted = false;

                            while (ammo_diff > 0 && !inv_ammo_depleted) {
                                inv_ammo_lot_available = fall_Obj_Item_GetCurrentAmmo(pItem->p_obj);
                                if (inv_ammo_lot_available <= ammo_diff) {
                                    ammo_current += inv_ammo_lot_available;
                                    ammo_diff -= inv_ammo_lot_available;

                                    if (pItem->num == 1)//last ammo lot, pItem will become invalid after object destruction.
                                        inv_ammo_depleted = true;
                                    fall_Obj_Destroy_InvObj(pItem->p_obj);//destroy ammo block if empty.
                                }
                                else {
                                    inv_ammo_lot_available -= ammo_diff;
                                    fall_Obj_Item_SetCurrentAmmo(pItem->p_obj, inv_ammo_lot_available);
                                    ammo_current += ammo_diff;
                                    ammo_diff = 0;
                                }
                            }
                        }
                    }
                    //else
                    //    Fallout_Debug_Error("SetItemAmmo - Adding ammo - current weapon ammo pid does not match selected ammo pid");
                }
                //set the newly adjusted weapon ammo.
                fall_Obj_Item_SetCurrentAmmo(pObj_weapon, ammo_current);
                //keep the slider position in scope by setting it to the new weapon ammo value.
                SendMessage((HWND)lParam, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)ammo_current);
                //update the numerical display below the slider.
                SetDlgItemInt(hwnd, IDC_STATIC_QUANTITY, ammo_current, TRUE);
            }
            return 0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_COMBO1: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                //A different ammo has been selected to load from inventory.
                //set slider position to zero, unloading the currently loaded ammo back into the inventory.
                SendMessage(GetDlgItem(hwnd, IDC_SLIDER1), TBM_SETPOSNOTIFY, (WPARAM)0, (LPARAM)0);
                //set weapon current ammo proID to the newly selected ammo proID.
                pObj_weapon->pud.general.pud.weapon.cur_ammo_type_pid = p_ammo_inv_proID[SendMessage(GetDlgItem(hwnd, IDC_COMBO1), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0)];
            }
            return TRUE;
        }
        case IDOK: {
            EndDialog(hwnd, 1);
            return FALSE;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        if (p_ammo_inv_proID)
            delete p_ammo_inv_proID;
        p_ammo_inv_proID = nullptr;
        return 0;
    }
    case WM_NCDESTROY:
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//______________________________________________________________________________
BOOL Set_Item_Ammo(HWND hwndParent, HINSTANCE hinstance, OBJStruct* pObj_weapon) {

    if (!pObj_weapon)
        return FALSE;

    if (DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_ITEM_AMMO), hwndParent, DlgProc_SetItemAmmo, (LPARAM)pObj_weapon) == 1)
        return TRUE;
    return FALSE;
}


//________________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_AdjustItemQuantity(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

    static OBJStruct* pObj_item = nullptr;
    static LONG num_items_current = 0;
    switch (Message) {

    case WM_INITDIALOG: {
        ITEMblock* p_item = (ITEMblock*)lParam;
        pObj_item = p_item->p_obj;
        num_items_current = p_item->num;

        HWND hwnd_sub = GetDlgItem(hwnd, IDC_EDIT1);
        SendMessage(hwnd_sub, EM_LIMITTEXT, (WPARAM)9, (LPARAM)0);
        SetDlgItemInt(hwnd, IDC_EDIT1, num_items_current, FALSE);
        //set the keyboard focus to the edit contol
        PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwnd_sub, (LPARAM)TRUE);

        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            BOOL translated = false;
            LONG num_items = GetDlgItemInt(hwnd, IDC_EDIT1, &translated, FALSE);
            if (translated) {
                if (num_items < 0)
                    num_items = 0;

                if (num_items > num_items_current) {
                    PROTO* pPro_item = nullptr;
                    //create a new item object using the selected items proID and add it to the inventory.
                    if (fall_GetPro(pObj_item->proID, &pPro_item) == 0) {
                        OBJStruct* pObj_new = nullptr;
                        if (fall_Obj_Create(&pObj_new, pPro_item->frmID, pObj_item->proID) == 0) {
                            if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_new, num_items - num_items_current) == 0)
                                fall_Obj_Disconnect_From_Map(pObj_new, nullptr);
                            else
                                fall_Obj_Destroy(pObj_new, nullptr);//destroy object if it cannot be added to the inventory
                        }
                    }
                }
                else if (num_items < num_items_current) {
                    fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, num_items_current - num_items);
                    if (num_items == 0)
                        fall_Obj_Destroy_InvObj(pObj_item);
                }
            }
            EndDialog(hwnd, 1);
            return FALSE;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            return FALSE;
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        return 0;
    }
    case WM_NCDESTROY:
        return 0;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//____________________________________________________________________________________
BOOL AdjustItemQuantity(HWND hwndParent, HINSTANCE hinstance, ITEMblock* p_item_block) {

    if (!p_item_block)
        return FALSE;

    if (DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_INV_QUANTITY), hwndParent, DlgProc_AdjustItemQuantity, (LPARAM)p_item_block) == 1)
        return TRUE;
    return FALSE;
}


//___________________________________________________
void InvList_Filter_Type(HWND hwnd_parent, LONG type) {
    if (!inv_cache)
        return;
    HWND h_ListView = GetDlgItem(hwnd_parent, IDC_LIST1);
    ListView_RedrawItems(h_ListView, 0, ListView_GetItemCount(h_ListView));
    InvalidateRect(h_ListView, nullptr, true);

    DWORD list_count_type_filtered = 0;

    PROTO* pPro = nullptr;
    LONG item_type = 0;

    for (LONG i = 0; i < inv_cache->Get_NumberOfItems(); i++) {
        pPro = inv_cache->Get_Proto(inv_cache->Get_SortedItem(i));
        if (pPro) {
            if (pPro->item.invFrmID == -1)//don't include items without an inventory frm. This isn't done in the Bis-Mapper but seems logical to me.
                item_type = -1;
            else
                item_type = pPro->item.itemType;
        }
        else
            item_type = -1;

        if (type < 0 || type == item_type) {
            inv_cache->Set_FilteredItem(list_count_type_filtered, inv_cache->Get_SortedItem(i));
            list_count_type_filtered++;
        }
    }
    inv_cache->Set_NumberOfFilteredItems(list_count_type_filtered);

    ListView_DeleteAllItems(h_ListView);
    ListView_SetItemCountEx(h_ListView, list_count_type_filtered, 0);
}


//_____________________________
void InvList_SortBy_Selection() {
    if (!inv_cache)
        return;
    for (LONG i = 0; i < inv_cache->Get_NumberOfItems(); i++) {
        inv_cache->Set_SortedItem(i, i);
    }
}


//_______________________
BOOL InvList_SortByText() {
    if (!inv_cache)
        return FALSE;
    wchar_t* str1 = 0, * str2 = 0;

    LONG lposHold = 0, lposHoldVal = 0;
    LONG lposVar = 0;
    for (LONG lpos = 1; lpos < inv_cache->Get_NumberOfItems(); lpos++) {
        lposHold = lpos;
        lposHoldVal = inv_cache->Get_SortedItem(lposHold);
        lposVar = lpos - 1;

        str1 = inv_cache->Get_Text(inv_cache->Get_SortedItem(lposHold));
        str2 = inv_cache->Get_Text(inv_cache->Get_SortedItem(lposVar));

        if (str1 && str2) {
            while (lposVar >= 0 && _wcsicmp(str2, str1) > 0) {
                inv_cache->Set_SortedItem(lposVar + 1, inv_cache->Get_SortedItem(lposVar));
                lposVar--;
                if (lposVar >= 0)
                    str2 = inv_cache->Get_Text(inv_cache->Get_SortedItem(lposVar));
            }
        }
        inv_cache->Set_SortedItem(lposVar + 1, lposHoldVal);
    }
    return TRUE;
}


//_________________________
BOOL InvList_Sort(LONG num) {

    if (num == 0)
        InvList_SortBy_Selection();
    else if (num == 1)
        InvList_SortByText();
    else
        return FALSE;

    return TRUE;
}


//____________________________________
void InvList_Arrange(HWND hwnd_parent) {

    HWND h_combo = GetDlgItem(hwnd_parent, IDC_COMBO_SORT);
    InvList_Sort(SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));

    h_combo = GetDlgItem(hwnd_parent, IDC_COMBO_TYPE);
    InvList_Filter_Type(hwnd_parent, SendMessage(h_combo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) - 1);
}


//__________________________________________________________________________
BOOL SetMenuItem(HMENU hMenu, UINT uID_Item, BOOL isChecked, BOOL isEnabled) {
    MENUITEMINFO menuInfo{ 0 };
    menuInfo.cbSize = sizeof(MENUITEMINFO);
    menuInfo.fMask = MIIM_STATE;
    if (!GetMenuItemInfo(hMenu, uID_Item, false, &menuInfo))
        return FALSE;

    if (!isEnabled)
        menuInfo.fState |= MFS_DISABLED;
    else
        menuInfo.fState &= ~MFS_DISABLED;
    if (isChecked)
        menuInfo.fState |= MFS_CHECKED;
    else
        menuInfo.fState &= ~MFS_CHECKED;

    return SetMenuItemInfo(hMenu, uID_Item, FALSE, &menuInfo);
}


//__________________________________________________
BOOL InvList_Menu(HWND hwnd_list, POINT* p_menu_pos) {
    if (!inv_cache)
        return FALSE;

    int selectedItem = ListView_GetNextItem(hwnd_list, -1, LVNI_FOCUSED | LVNI_SELECTED);

    if (selectedItem < 0 || selectedItem >= inv_cache->Get_NumberOfFilteredItems())
        return FALSE;


    //find equipped items if any
    LONG itemNum = 0;
    ITEMblock* pItem = inv_cache->GetOwnerObj()->pud.general.p_item;

    OBJStruct* pObj_armor = nullptr;
    OBJStruct* pObj_left = nullptr;
    OBJStruct* pObj_right = nullptr;

    while (itemNum < inv_cache->GetOwnerObj()->pud.general.inv_size) {
        pItem = &inv_cache->GetOwnerObj()->pud.general.p_item[itemNum];

        if (pItem->p_obj && pItem->num > 0) {
            if (pItem->p_obj->flags & FLG_IsHeldSlot1) {
                if (pItem->p_obj->flags & FLG_IsHeldSlot2)//if item is two handed - Leaving this in as fallout does it this way - from when two-handed weapons took up both slots.
                    pObj_right = pItem->p_obj;
                pObj_left = pItem->p_obj;

            }
            else if (pItem->p_obj->flags & FLG_IsHeldSlot2)
                pObj_right = pItem->p_obj;
            else if (pItem->p_obj->flags & FLG_IsWornArmor)
                pObj_armor = pItem->p_obj;
        }
        itemNum++;
    }

    //get the selected item
    itemNum = inv_cache->Get_FilteredItem(selectedItem);
    pItem = inv_cache->Get_Item(itemNum);
    if (!pItem || !pItem->p_obj)
        return FALSE;
    OBJStruct* pObj_item = pItem->p_obj;
    LONG num_items = pItem->num;

    LONG itemType = fall_Obj_Item_GetType(pObj_item);



    POINT menu_pos{ p_menu_pos->x, p_menu_pos->y };
    //Convert to screen coordinates.
    MapWindowPoints(hwnd_list, HWND_DESKTOP, &menu_pos, 1);

    HMENU hMenuLoaded;
    HMENU hMenuSub;
    //Get the menu.
    hMenuLoaded = LoadMenu(phinstDLL, MAKEINTRESOURCE(IDR_MENU_INV));

    if (hMenuLoaded) {
        hMenuSub = GetSubMenu(hMenuLoaded, 0);

        //Get the submenu for the first menu item.
        HMENU hPopupMenu = hMenuSub;

        if (itemType != ITEM_TYPE::weapon) {
            SetMenuItem(hPopupMenu, ID_INVENTORY_WEAPONLOAD, FALSE, FALSE);
            SetMenuItem(hPopupMenu, ID_INVENTORY_WEAPONUNLOAD, FALSE, FALSE);
        }
        else {
            if (!fall_Obj_Weapon_Can_Unload(pObj_item))
                SetMenuItem(hPopupMenu, ID_INVENTORY_WEAPONUNLOAD, FALSE, FALSE);
        }

        if (itemType != ITEM_TYPE::armor)
            SetMenuItem(hPopupMenu, ID_INVENTORY_WEARITEM, FALSE, FALSE);
        else if (pObj_item->flags & FLG_IsWornArmor)
            SetMenuItem(hPopupMenu, ID_INVENTORY_WEARITEM, TRUE, TRUE);

        if (pObj_item->flags & FLG_IsHeldSlot1)
            SetMenuItem(hPopupMenu, ID_INVENTORY_HOLDITEMINLEFT, TRUE, TRUE);
        if (pObj_item->flags & FLG_IsHeldSlot2)
            SetMenuItem(hPopupMenu, ID_INVENTORY_HOLDITEMINRIGHT, TRUE, TRUE);

        //Show the menu and wait for input.
        BOOL menu_item = TrackPopupMenuEx(hPopupMenu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL | TPM_NONOTIFY | TPM_RETURNCMD,
            menu_pos.x, menu_pos.y, hwnd_list, nullptr);

        DestroyMenu(hMenuLoaded);

        switch (menu_item) {
        case ID_INVENTORY_PLUS_1: {
            PROTO* pPro_item = nullptr;
            //create a new item object using the selected items proID and add it to the inventory.
            if (fall_GetPro(pObj_item->proID, &pPro_item) == 0) {
                OBJStruct* pObj_new = nullptr;
                if (fall_Obj_Create(&pObj_new, pPro_item->frmID, pObj_item->proID) == 0) {
                    if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_new, 1) == 0)
                        fall_Obj_Disconnect_From_Map(pObj_new, nullptr);
                    else
                        fall_Obj_Destroy(pObj_new, nullptr);//destroy object if it cannot be added to the inventory
                }
            }
            break;
        }
        case ID_INVENTORY_MINUS_1: {
            fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1);
            if (num_items <= 1)
                fall_Obj_Destroy_InvObj(pObj_item);
            break;
        }
        case ID_INVENTORY_ADJUSTQUANTITY: {
            AdjustItemQuantity(GetParent(hwnd_list), phinstDLL, pItem);
            break;
        }
        case ID_INVENTORY_REMOVEALL:
            fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, num_items);
            fall_Obj_Destroy_InvObj(pObj_item);
            break;
        case ID_INVENTORY_WEARITEM:
            if (pObj_item->flags & FLG_IsWornArmor) {//if wearing selected item - disrobe
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~FLG_IsWornArmor;
                    if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0)
                        fall_Obj_Critter_Adjust_AC(inv_cache->GetOwnerObj(), pObj_item, nullptr);
                }
            }
            else {
                if (pObj_armor) {//disrobe the currently worn item
                    if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_armor, 1) == 0) {
                        pObj_armor->flags &= ~FLG_IsWornArmor;
                        if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_armor, 1) == 0)
                            fall_Obj_Critter_Adjust_AC(inv_cache->GetOwnerObj(), pObj_armor, nullptr);
                    }
                }
                //wear the selected item
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~(FLG_IsHeldSlot1 | FLG_IsHeldSlot2);//unset other flags if present
                    pObj_item->flags |= FLG_IsWornArmor;
                    if (fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0)
                        fall_Obj_Critter_Adjust_AC(inv_cache->GetOwnerObj(), nullptr, pObj_item);
                }
            }
            break;
        case ID_INVENTORY_HOLDITEMINLEFT:
            if (pObj_item->flags & FLG_IsHeldSlot1) {//if holding selected item - release
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~(FLG_IsHeldSlot1 | FLG_IsHeldSlot2);
                    fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1);
                }
            }
            else {
                if (pObj_left) {//return currently held item to inv
                    if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_left, 1) == 0) {
                        pObj_left->flags &= ~FLG_IsHeldSlot1;
                        if (pObj_left->flags & FLG_IsHeldSlot2) {
                            pObj_left->flags &= ~FLG_IsHeldSlot2;
                            pObj_right = nullptr;
                        }
                        fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_left, 1);
                    }
                }
                //hold the selected item left
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~(FLG_IsHeldSlot2 | FLG_IsWornArmor);//unset other flags if present
                    pObj_item->flags |= FLG_IsHeldSlot1;
                    fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1);
                }
            }
            break;
        case ID_INVENTORY_HOLDITEMINRIGHT:
            if (pObj_item->flags & FLG_IsHeldSlot2) {//if holding selected item - release
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~(FLG_IsHeldSlot1 | FLG_IsHeldSlot2);
                    fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1);
                }
            }
            else {
                if (pObj_right) {//return currently held item to inv
                    if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_right, 1) == 0) {
                        pObj_right->flags &= ~FLG_IsHeldSlot2;
                        if (pObj_right->flags & FLG_IsHeldSlot1) {
                            pObj_right->flags &= ~FLG_IsHeldSlot1;
                            pObj_left = nullptr;
                        }
                        fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_right, 1);
                    }
                }
                //hold the selected item right
                if (fall_Obj_Inventory_RemoveItems(inv_cache->GetOwnerObj(), pObj_item, 1) == 0) {
                    pObj_item->flags &= ~(FLG_IsHeldSlot1 | FLG_IsWornArmor);//unset other flags if present
                    pObj_item->flags |= FLG_IsHeldSlot2;
                    fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObj_item, 1);
                }
            }
            break;
            //
        case ID_INVENTORY_WEAPONLOAD:
            Set_Item_Ammo(GetParent(hwnd_list), phinstDLL, pObj_item);
            break;
        case ID_INVENTORY_WEAPONUNLOAD: {
            OBJStruct* pObjAmmo = nullptr;
            do {
                pObjAmmo = fall_Obj_Weapon_Unload(pObj_item);
                if (pObjAmmo) {
                    fall_Obj_Disconnect_From_Map(pObjAmmo, nullptr);
                    fall_Obj_Inventory_AddItems(inv_cache->GetOwnerObj(), pObjAmmo, 1);
                }
            } while (pObjAmmo != nullptr);

            break;
        }
        default:
            break;
        }

    }
    inv_cache->Update_Inv();
    InvList_Arrange(GetParent(hwnd_list));
    ListView_DeleteAllItems(hwnd_list);
    ListView_SetItemCountEx(hwnd_list, inv_cache->Get_NumberOfItems(), 0);
    ListView_SetImageList(hwnd_list, inv_cache->Get_ImageList(), LVSIL_NORMAL);
    return TRUE;
}


//_______________________________________________________
BOOL Draw_WieldBox(HDC hdc, HBITMAP hBitmap, DWORD flags) {
    if (!(flags & (FLG_IsWornArmor | FLG_IsHeldSlot1 | FLG_IsHeldSlot2)))
        return FALSE;
    if (hBitmap) {

        HDC hDCBits_check = CreateCompatibleDC(hdc);
        BITMAP Bitmap_check{ 0 };
        GetObject(hBitmap_wieldBoxes, sizeof(BITMAP), (LPSTR)&Bitmap_check);
        HGDIOBJ hgdiobj_check_old = SelectObject(hDCBits_check, hBitmap_wieldBoxes);

        HDC hDCBits = CreateCompatibleDC(hdc);
        BITMAP Bitmap{ 0 };
        GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
        HGDIOBJ hgdiobj_old = SelectObject(hDCBits, hBitmap);

        BITMAPINFO* pBMP_Info = new BITMAPINFO{ 0 };
        pBMP_Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pBMP_Info->bmiHeader.biWidth = Bitmap_check.bmWidth;
        pBMP_Info->bmiHeader.biHeight = Bitmap_check.bmHeight;
        pBMP_Info->bmiHeader.biBitCount = 32;
        pBMP_Info->bmiHeader.biPlanes = Bitmap_check.bmPlanes;

        pBMP_Info->bmiHeader.biCompression = BI_RGB;
        pBMP_Info->bmiHeader.biXPelsPerMeter = 1000;
        pBMP_Info->bmiHeader.biYPelsPerMeter = 1000;
        pBMP_Info->bmiHeader.biClrUsed = 0;
        pBMP_Info->bmiHeader.biClrImportant = 0;

        DWORD dwBmpSize = Bitmap_check.bmWidth * 4 * Bitmap_check.bmHeight;
        BYTE* pbytes = new BYTE[dwBmpSize];
        GetDIBits(hDCBits_check, hBitmap_wieldBoxes, 0, (UINT)Bitmap_check.bmHeight, pbytes, pBMP_Info, DIB_RGB_COLORS);
        //need to set the alpha channel so that the check box displays opaque.
        for (DWORD d = 0; d < dwBmpSize; d += 4)
            pbytes[d + 3] = 0xFF;

        LONG icon_width = Bitmap_check.bmWidth / 3;
        LONG icon_x = 0;
        if (flags & (FLG_IsWornArmor | FLG_IsHeldSlot1 | FLG_IsHeldSlot2)) {
            
            if (flags & FLG_IsHeldSlot1) {
                icon_x += icon_width;
                if (flags & FLG_IsHeldSlot2)//for 2 handed weapons, this is never used.
                    icon_width *= 2;
            }
            else if (flags & FLG_IsHeldSlot2)
                icon_x += icon_width * 2;
 
            SetDIBitsToDevice(hDCBits, 0, 0, icon_width, Bitmap_check.bmHeight, icon_x, 0, 0, Bitmap_check.bmHeight, pbytes, pBMP_Info, DIB_RGB_COLORS);
        }

        SelectObject(hDCBits, hgdiobj_old);
        DeleteDC(hDCBits);
        SelectObject(hDCBits_check, hgdiobj_check_old);
        DeleteDC(hDCBits_check);
        delete pBMP_Info;
        delete pbytes;
    }
    return TRUE;
}


//____________________________________________________
BOOL InvList_SetItemData(NMLVDISPINFO* p_display_info) {

    if (!inv_cache)
        return FALSE;

    HWND h_ListView = p_display_info->hdr.hwndFrom;

    LVITEM* lplvI = &p_display_info->item;

    if (lplvI->iItem >= inv_cache->Get_NumberOfFilteredItems()) {
        lplvI->mask |= LVIF_DI_SETITEM;//set mask to not repeat message
        return FALSE;
    }

    LONG itemNum = inv_cache->Get_FilteredItem(lplvI->iItem);
    ITEMblock* pItem = inv_cache->Get_Item(itemNum);

    if (!pItem || !pItem->p_obj)
        return FALSE;

    if (lplvI->mask & LVIF_TEXT) {
        wchar_t* chars = Create_Item_Text(pItem->p_obj, pItem->num);
        if (chars) {
            wcsncpy_s(lplvI->pszText, lplvI->cchTextMax, chars, _TRUNCATE);
            delete[] chars;
        }
        else
            lplvI->pszText[0] = '\0';
    }

    if (lplvI->mask & LVIF_IMAGE) {
        PROTO* proto = nullptr;
        fall_GetPro(pItem->p_obj->proID, &proto);

        DWORD frmID = pItem->p_obj->frmID;
        if (proto) {
            frmID = proto->item.invFrmID;
            Check_If_Blocker_Proto(proto->proID, &frmID);
        }

        if (frmID == -1)
            return FALSE;
        DWORD bgColour = ListView_GetBkColor(h_ListView);
        HIMAGELIST hLarge = inv_cache->Get_ImageList();
        if (!hLarge)
            return FALSE;
        int iconW = 0;
        int iconH = 0;
        ImageList_GetIconSize(hLarge, &iconW, &iconH);

        if (inv_cache->Get_ImageRef(itemNum) == -1) {
            HDC hdc = GetDC(h_ListView);
            HBITMAP bitmap = CreateIconFRM(hdc, frmID, 0, iconW, iconH, &bgColour);

            if (bitmap) {
                Draw_WieldBox(hdc, bitmap, pItem->p_obj->flags);
                lplvI->iImage = inv_cache->Set_Image(bitmap, itemNum);
                if (lplvI->iImage == -1)
                    Fallout_Debug_Error("InvList_SetItemData ImageList_Add failed - tile, frmID:%X item_num:%d", frmID, itemNum);
                DeleteObject(bitmap);
            }
            ReleaseDC(h_ListView, hdc);
            return TRUE;
        }
        else
            lplvI->iImage = inv_cache->Get_ImageRef(itemNum);
    }
    return TRUE;
}


//__________________________________________________
void Setup_ObjectName(HWND hwnd_Static, DWORD proID) {
    LONG type = proID >> 24;
    PROTO* p_pro_fall = nullptr;
    DWORD num_chars = 0;
    wchar_t* p_pro_text = nullptr;
    if (fall_GetPro(proID, &p_pro_fall) == 0) {

        char* msg = GetMsg(pMsgList_Proto_Type_0 + type, p_pro_fall->txtID, 2);
        if (msg && msg[0] != '\0' && strncmp(msg, "Error", 5) != 0) {
            num_chars = strlen(msg) + 1;
            p_pro_text = new wchar_t[num_chars] {0};
            swprintf_s(p_pro_text , num_chars, L"%S", msg);
        }
        else {
            num_chars = 17;
            p_pro_text  = new wchar_t[num_chars] {0};
            swprintf_s(p_pro_text , num_chars, L"proto:%u", p_pro_fall->proID & 0x00FFFFFF);
        }

        if (p_pro_text) {
            SendMessage(hwnd_Static, WM_SETTEXT, 0, (LPARAM)p_pro_text);
            delete[] p_pro_text;
        }
    }
}


//__________________________________________________________________________
BOOL Script_Get_Name(LONG scriptIndex, wchar_t *ret_name, DWORD name_length) {

    if (!ret_name || !name_length)
        return FALSE;
    char* path = new char[MAX_PATH] {0};
    fall_GetScriptPath(path);
    strcat_s(path, MAX_PATH, "scripts.lst");
    void* File = fall_fopen(path, "rt");
    delete[] path;

    bool EndOfFile = false;
    if (File) {
        DWORD buff_size = 128;
        char* buff = new char[buff_size] { 0 };

        char c = '\0';
        char* last_space = nullptr;

        DWORD buff_char_count = 0;
        DWORD line_count = 0;
        while (c != EOF) {
            while ((c = fall_fgetc(File)) != EOF && c != '\n') {
                if (line_count == scriptIndex) {
                    if (buff_char_count < buff_size - 2) {
                        buff[buff_char_count] = c;

                        if (buff[buff_char_count] == ' ') {
                            if (!last_space)
                                last_space = &buff[buff_char_count];
                        }
                        else if (buff[buff_char_count] == ';') {// char';' precedes description comment
                            //cull space charactors before comments
                            for (DWORD name_count = 0; name_count < buff_char_count - 4; name_count++) {
                                //find file extension from script name
                                if (buff[name_count] == '.' && buff[name_count + 1] == 'i' && buff[name_count + 2] == 'n' && buff[name_count + 3] == 't') {
                                    name_count += 4;
                                    buff_char_count = name_count + 1;
                                    buff[name_count] = ' ';
                                    buff[buff_char_count] = ';'; //reinsert the ';'char before comment
                                }
                            }
                            //reset the last space marker as a different char was read.
                            last_space = nullptr;
                        }
                        else if (buff[buff_char_count] == '#') {// char'#' precedes number of local_vars comment
                            if (last_space)//trim preceding spaces.
                                *last_space = '\0';
                            else
                                buff[buff_char_count] = '\0';
                            //set char count to max to prevent further proccessing.
                            buff_char_count = buff_size;
                        }
                        else//reset the last space marker if a different char is read.
                            last_space = nullptr;

                        buff_char_count++;
                    }
                }
            }
            
            //make sure the buffer is null terminated.
            buff[buff_size - 1] = '\0';
            buff_char_count = 0;
            last_space = nullptr;
            if (line_count == scriptIndex) {
                swprintf_s(ret_name, name_length, L"%S", buff);
                fall_fclose(File);
                delete[] buff;
                return TRUE;
            }
            line_count++;
        }
        fall_fclose(File);
        delete[] buff;
    }
    return FALSE;
}


//_____________________________________________________________
void SetUpComboBox_Scripts(HWND hWndComboBox, LONG scriptIndex) {

    SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"None");

    char* path = new char[MAX_PATH] {0};
    fall_GetScriptPath(path);
    strcat_s(path, MAX_PATH, "scripts.lst");
    void* File = fall_fopen(path, "rt");
    delete[] path;

    bool EndOfFile = false;
    if (File) {
        DWORD buff_size = 128;
        char* buff = new char[buff_size]{ 0 };
        wchar_t* buff_w = new wchar_t[buff_size] { 0 };

        char c = '\0';
        char* last_space = nullptr;

        DWORD buff_char_count = 0;
        while (c != EOF) {
            while ((c = fall_fgetc(File)) != EOF && c != '\n') {
                
                if (buff_char_count < buff_size - 2) {
                    buff[buff_char_count] = c;
                    
                    if (buff[buff_char_count] == ' ') {
                        if(!last_space)
                            last_space = &buff[buff_char_count];
                    }
                    else if (buff[buff_char_count] == ';') {// char';' precedes description comment
                        //cull space charactors before comments
                        for (DWORD name_count = 0; name_count < buff_char_count-4; name_count++) {
                            //find file extension from script name
                            if (buff[name_count] == '.' && buff[name_count + 1] == 'i' && buff[name_count + 2] == 'n' && buff[name_count + 3] == 't') {
                                name_count += 4;
                                buff_char_count = name_count + 1;
                                buff[name_count] = ' ';
                                buff[buff_char_count] = ';'; //reinsert the ';'char before comment
                            }
                        }
                        //reset the last space marker as a different char was read.
                        last_space = nullptr;
                    }
                    else if (buff[buff_char_count] == '#') {// char'#' precedes number of local_vars comment
                        if (last_space)//trim preceding spaces.
                            *last_space = '\0';
                        else
                            buff[buff_char_count] = '\0';
                        //set char count to max to prevent further proccessing.
                        buff_char_count = buff_size;
                    }
                    else//reset the last space marker if a different char is read.
                        last_space = nullptr;

                    buff_char_count++;
                }
            }
            //make sure the buffer is null terminated.
            buff[buff_size - 1] = '\0';
            buff_char_count = 0;
            last_space = nullptr;

            swprintf_s(buff_w, buff_size, L"%S", buff);
            SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)buff_w);
        }
        fall_fclose(File);

        SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)scriptIndex, (LPARAM)0);

        delete[] buff;
        delete[] buff_w;
    }
    
}


//____________________________________________________________________
void SetUpComboBox_AIPacket(HWND hWndComboBox, INT32 CurrentPacketNum) {
    wchar_t* buff_w = new wchar_t[64];
    for (LONG i = 0; i < fall_Obj_GetNumAIPackets(); i++) {
        swprintf_s(buff_w, 64, L"%S", fall_Obj_Critter_GetAIPacketName(i));
        SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)buff_w);
    }
    delete[] buff_w;
    SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)CurrentPacketNum, (LPARAM)0);
}


//___________________________________________________
void Set_Flag_CheckBox_States(HWND hwnd, DWORD flags) {
    if (flags & FLG_Flat) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK1) , (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_NoBlock) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK2), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_MultiHex) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK3), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransNone) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK4), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransWall) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK5), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransGlass) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK6), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransSteam) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK7), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransEnergy) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK8), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_TransRed) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK9), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_ShootThru) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK10), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
    if (flags & FLG_LightThru) 
        SendMessage(GetDlgItem(hwnd, IDC_CHECK11), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
}


//_________________________________________
DWORD Get_Flag_CheckBox_States(HWND parent) {
    DWORD flags = 0;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK1), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_Flat;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK2), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_NoBlock;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK3), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_MultiHex;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK4), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransNone;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK5), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransWall;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK6), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransGlass;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK7), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransSteam;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK8), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransEnergy;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK9), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_TransRed;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK10), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_ShootThru;
    if (SendMessage(GetDlgItem(parent, IDC_CHECK11), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        flags |= FLG_LightThru;

    return flags;
}


//__________________________________________________
void DlgItem_Enable(HWND hwnd_dlgItem, BOOL bEnable) {
    EnableWindow(hwnd_dlgItem, bEnable);
    UpdateWindow(hwnd_dlgItem);
}


//_______________________________________________
void Disable_Critter_Specific_Controls(HWND hwnd) {
    HWND hDlgItem = GetDlgItem(hwnd, IDC_COMBO_AI_PACKET);
    EnableWindow(hDlgItem, FALSE);
    UpdateWindow(hDlgItem);
    hDlgItem = GetDlgItem(hwnd, IDC_EDIT_TEAM_NUMBER);
    EnableWindow(hDlgItem, FALSE);
    UpdateWindow(hDlgItem);
}


//________________________________________
void Disable_Inventory_Controls(HWND hwnd) {
    Button_Enable(GetDlgItem(hwnd, IDC_BUTTON_INV_ADD), FALSE);
    Button_Enable(GetDlgItem(hwnd, IDC_BUTTON_INV_CLEAR), FALSE);

    HWND hDlgItem = GetDlgItem(hwnd, IDC_LIST1);
    EnableWindow(hDlgItem, FALSE);
    UpdateWindow(hDlgItem);
}


//_____________________________________________________________________________________________
LRESULT CALLBACK WinProc_InvViewTabSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    LRESULT lres;
    switch (uMsg) {
    case WM_GETDLGCODE:
        lres = CallWindowProc(pOldProc_InvViewTab, hwnd, uMsg, wParam, lParam);
        lres &= ~DLGC_WANTTAB;
        if (lParam &&
            ((MSG*)lParam)->message == WM_KEYDOWN &&
            ((MSG*)lParam)->wParam == VK_TAB) {
            lres &= ~DLGC_WANTMESSAGE;
            PostMessage(*phWinMain, WM_COMMAND, ID_SHIFT_WINDOW_FOCUS, 0);
        }
        return lres;
    }
    return CallWindowProc(pOldProc_InvViewTab, hwnd, uMsg, wParam, lParam);
}


//__________________________________________________________________________________________
INT_PTR CALLBACK DlgProc_ObjectEditor(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {


    static OBJStruct* pObj_modified = nullptr;
    static OBJStruct* pObj_original = nullptr;

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

        if (inv_cache)
            delete inv_cache;
        inv_cache = nullptr;

        hBitmap_wieldBoxes = (HBITMAP)LoadImage(phinstDLL, (LPCWSTR)MAKEINTRESOURCE(IDB_BITMAP_WIELDBOX), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);


        pObj_original = (OBJStruct*)lParam;

        if (Copy_MapObject(&pObj_modified, pObj_original) == FALSE) {
            Fallout_Debug_Error("DlgProc_ObjectEditor - error copying object");
            return TRUE;
        }

        HWND hDlgItem = nullptr;
        wchar_t* str = new wchar_t[64];

        //set obj image
        hDlgItem = GetDlgItem(hwnd, IDC_STATIC_OBJECT_IMAGE);
        DWORD bgColour = GetSysColor(COLOR_3DFACE);
        HDC hdc = GetDC(hDlgItem);
        HBITMAP bitmap = CreateIconFRM(hdc, pObj_modified->frmID, 0, 255, 200, &bgColour);
        if (bitmap) {
            SendMessage(hDlgItem, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmap);
            DeleteObject(bitmap);
        }

        //set obj name
        Setup_ObjectName(GetDlgItem(hwnd, IDC_STATIC_OBJECT_NAME), pObj_modified->proID);

        //setup flags
        Set_Flag_CheckBox_States(hwnd, pObj_modified->flags);

        //setup intensity edit box
        SendMessage(GetDlgItem(hwnd, IDC_EDIT_LIGHT_INTENSITY), EM_LIMITTEXT, (WPARAM)3, 0);
        SetDlgItemInt(hwnd, IDC_EDIT_LIGHT_INTENSITY, (UINT)(pObj_modified->light_intensity / 655.36f), FALSE);

        //setup light hex radius combo
        hDlgItem = GetDlgItem(hwnd, IDC_COMBO_LIGHT_DIST);
        for (int i = 0; i <= 8; i++) {
            swprintf_s(str, 64, L"%d", i);
            SendMessage(hDlgItem, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        }

        SendMessage(hDlgItem, CB_SETCURSEL, (WPARAM)pObj_modified->light_radius, (LPARAM)0);

        //get title text
        SendMessage(hwnd, WM_GETTEXT, (WPARAM)64, (LPARAM)str);
        wcsncat_s(str, 64, L" - ", 16);

        LONG type = pObj_modified->proID >> 24;
        wchar_t* str_title_type = new wchar_t[16];

        switch (type) {
        case ART_ITEMS: {
            LoadString(phinstDLL, IDS_TEXT_ITEMS, str_title_type, 16);
            SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), pObj_modified->scriptIndex + 1);
            Disable_Critter_Specific_Controls(hwnd);
            PROTO* pPro = nullptr;
            if (fall_GetPro(pObj_modified->proID, &pPro) == -1 || pPro->item.itemType != ITEM_TYPE::container)
                Disable_Inventory_Controls(hwnd);
            else
                inv_cache = new INVENTORY_LIST_CACHE(pObj_modified);

            break;
        }
        case ART_CRITTERS:
            LoadString(phinstDLL, IDS_TEXT_CRITTERS, str_title_type, 16);
            SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), pObj_modified->scriptIndex + 1);
            SetUpComboBox_AIPacket(GetDlgItem(hwnd, IDC_COMBO_AI_PACKET), pObj_modified->pud.critter.combat_data.aiPacket);
            SetDlgItemInt(hwnd, IDC_EDIT_TEAM_NUMBER, pObj_modified->pud.critter.combat_data.teamNum, FALSE);
            inv_cache = new INVENTORY_LIST_CACHE(pObj_modified);
            break;
        case ART_SCENERY:
            LoadString(phinstDLL, IDS_TEXT_SCENERY, str_title_type, 16);
            SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), pObj_modified->scriptIndex + 1);
            Disable_Critter_Specific_Controls(hwnd);
            Disable_Inventory_Controls(hwnd);
            break;
        case ART_WALLS:
            SetUpComboBox_Scripts(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), pObj_modified->scriptIndex + 1);
            LoadString(phinstDLL, IDS_TEXT_WALLS, str_title_type, 16);
            Disable_Critter_Specific_Controls(hwnd);
            Disable_Inventory_Controls(hwnd);
            break;
            //case ART_TILES:
            //    break;
        case ART_MISC:
            LoadString(phinstDLL, IDS_TEXT_MISC, str_title_type, 16);
            DlgItem_Enable(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), FALSE);
            Disable_Critter_Specific_Controls(hwnd);
            Disable_Inventory_Controls(hwnd);

            break;
        default:
            str_title_type[0] = '\0';
            break;
        }

        //add object type string to window title
        wcsncat_s(str, 64, str_title_type, 16);
        delete[] str_title_type;
        //set title text
        SendMessage(hwnd, WM_SETTEXT, 64, (LPARAM)(LPCWSTR)str);


        //inv combo box setup - type filter
        HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_TYPE);

        for (LONG type = -1; type <= 6; type++) {
            LoadString(phinstDLL, Get_Item_Type_TextID(type), str, 16);
            SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        }
        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        //inv combo box setup - sort
        hwndCombo = GetDlgItem(hwnd, IDC_COMBO_SORT);
        LoadString(phinstDLL, IDS_SORT_LIST_NUMBER, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
        LoadString(phinstDLL, IDS_SORT_NAME, str, 16);
        SendMessage(hwndCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);

        SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        delete[]str;

        if (inv_cache) {
            HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
            pOldProc_InvViewTab = (WNDPROC)SetWindowLongPtr(h_ListView, GWLP_WNDPROC, (LONG_PTR)WinProc_InvViewTabSubClass);
            ListView_DeleteAllItems(h_ListView);
            ListView_SetItemCountEx(h_ListView, inv_cache->Get_NumberOfItems(), LVSICF_NOINVALIDATEALL);
            ListView_SetImageList(h_ListView, inv_cache->Get_ImageList(), LVSIL_NORMAL);
        }

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
            InvList_Menu(lpnmitem->hdr.hwndFrom, &lpnmitem->ptAction);
            return TRUE;
            break;
        }
        case NM_RETURN: {
            LPNMHDR lpnm = (LPNMHDR)lParam;
            int selectedItem = ListView_GetNextItem(lpnm->hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
            if (selectedItem != -1) {
                RECT rcItem{ 0,0,0,0 };
                ListView_GetItemRect(lpnm->hwndFrom, selectedItem, &rcItem, LVIR_BOUNDS);
                InvList_Menu(lpnm->hwndFrom, (POINT*)&rcItem);
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
            InvList_SetItemData((NMLVDISPINFO*)lParam);
            return TRUE;
            break;
        }
        case LVN_ITEMCHANGED: {
            break;
        }
        default:
            break;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_COMBO_TYPE: {
            if (HIWORD(wParam) == CBN_SELCHANGE)
                InvList_Arrange(hwnd);
            return TRUE;
        }
        case IDC_COMBO_SORT: {
            if (HIWORD(wParam) == CBN_SELCHANGE)
                InvList_Arrange(hwnd);
            return TRUE;
        }
        case IDC_BUTTON_INV_CLEAR:
            if (inv_cache) {
                fall_Obj_Clear_Inventory(&pObj_modified->pud);
                inv_cache->Update_Inv();
                InvList_Arrange(hwnd);
                HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
                ListView_DeleteAllItems(h_ListView);
                ListView_SetItemCountEx(h_ListView, inv_cache->Get_NumberOfItems(), 0);
                ListView_SetImageList(h_ListView, inv_cache->Get_ImageList(), LVSIL_NORMAL);
            }
            break;
        case IDC_BUTTON_INV_ADD:
            if (Dialog_ItemSelector(hwnd, phinstDLL, pObj_modified) == 0) {
                inv_cache->Update_Inv();
                InvList_Arrange(hwnd);
                HWND h_ListView = GetDlgItem(hwnd, IDC_LIST1);
                ListView_DeleteAllItems(h_ListView);
                ListView_SetItemCountEx(h_ListView, inv_cache->Get_NumberOfItems(), 0);
                ListView_SetImageList(h_ListView, inv_cache->Get_ImageList(), LVSIL_NORMAL);
            }
            break;
        case IDC_EDIT_LIGHT_INTENSITY: {
            if (HIWORD(wParam) == EN_UPDATE) {
                BOOL translated = false;
                int idc_edit = LOWORD(wParam);
                if (idc_edit != IDC_EDIT_LIGHT_INTENSITY)
                    break;
                DWORD light_intensity_percentage = GetDlgItemInt(hwnd, idc_edit, &translated, FALSE);
                if (translated) {
                    //keep component value under max 100%.
                    if (light_intensity_percentage > 100) {
                        light_intensity_percentage = 100;
                        SetDlgItemInt(hwnd, idc_edit, light_intensity_percentage, FALSE);
                    }
                }
            }
            break;
        }
        case IDOK: {
            if (pObj_modified && pObj_original) {
                LONG type = pObj_modified->proID >> 24;
                //clear current obj flags
                pObj_modified->flags &= (~(FLG_Flat | FLG_NoBlock | FLG_MultiHex | FLG_TransNone | FLG_TransWall | FLG_TransGlass | FLG_TransSteam | FLG_TransEnergy | FLG_TransRed | FLG_ShootThru | FLG_LightThru));
                //set flags from checkbox states.
                pObj_modified->flags |= Get_Flag_CheckBox_States(hwnd);
                BOOL translated = false;

                //set light intensity
                DWORD d_var = GetDlgItemInt(hwnd, IDC_EDIT_LIGHT_INTENSITY, &translated, FALSE);
                if (translated)
                    pObj_modified->light_intensity = (DWORD)(d_var * 655.36f);
                //set light radius
                d_var = SendMessage(GetDlgItem(hwnd, IDC_COMBO_LIGHT_DIST), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                if (d_var != CB_ERR)
                    pObj_modified->light_radius = d_var;
                //set script
                if (type != ART_MISC) {
                    d_var = SendMessage(GetDlgItem(hwnd, IDC_COMBO_SCRIPT), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    if (d_var != CB_ERR) {
                        if (pObj_modified->scriptID != -1) {
                            fall_Script_Remove(pObj_modified->scriptID);
                            pObj_modified->scriptID = -1;
                        }
                        pObj_modified->scriptIndex = d_var - 1;
                        if (pObj_modified->scriptIndex != -1) 
                            //fall_Obj_New_ScriptID(pObj_modified, &pObj_modified->scriptID);
                            fall_Obj_New_Script_Instance(pObj_modified, pObj_modified->scriptIndex >> 24, pObj_modified->scriptIndex & 0x00FFFFFF);
                    }
                }
                if (type == ART_CRITTERS) {
                    //set ai packet
                    d_var = SendMessage(GetDlgItem(hwnd, IDC_COMBO_AI_PACKET), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    if (d_var != CB_ERR)
                        pObj_modified->pud.critter.combat_data.aiPacket = d_var;
                    //set team number
                    d_var = GetDlgItemInt(hwnd, IDC_EDIT_TEAM_NUMBER, &translated, FALSE);
                    if (translated)
                        pObj_modified->pud.critter.combat_data.teamNum = d_var;
                
                    //set frm
                    DWORD weapon_code = 0;
                    OBJStruct* pObj_weapon = fall_obj_getInvItem_HeldInSlot_2(pObj_modified);
                    if (pObj_weapon && fall_Obj_Item_GetType(pObj_weapon) == ITEM_TYPE::weapon) {
                        weapon_code = fall_Obj_Weapon_AnimationCode(pObj_weapon);
                    }
                    pObj_modified->frmID = fall_GetFrmID(ART_CRITTERS, pObj_modified->frmID & 0x00000FFF, 0, weapon_code, pObj_modified->ori + 1);
                }

                Set_BlockerObject(pObj_modified);

                DWORD hexNum = pObj_original->hexNum;
                DWORD level = pObj_original->level;
                RECT rect;
                if (fall_Obj_Destroy(pObj_original, &rect) == -1)
                    Fallout_Debug_Error("Object Editor original object destruction failure");
                DrawMapChanges(&rect, level, FLG_Obj | FLG_Hud_Outline);

                fall_Obj_Move(pObj_modified, hexNum, level, &rect);
                DrawMapChanges(&rect, level, FLG_Obj | FLG_Hud_Outline);
                pObj_modified = nullptr;// set to null to avoid destruction in WM_DESTROY
            }
            EndDialog(hwnd, 0);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, -1);
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY: {
        if (inv_cache)
            delete inv_cache;
        inv_cache = nullptr;

        DeleteObject(hBitmap_wieldBoxes);
        hBitmap_wieldBoxes = nullptr;
        
        if (pObj_modified) {
            if (fall_Obj_Destroy(pObj_modified, nullptr) == -1)
                Fallout_Debug_Error("Object Editor temp object destruction failure");
        }
        pObj_modified = nullptr;
        //Fallout_Debug_Info("Object Editor being destroyed");
        return 0;
    }
    case WM_NCDESTROY:
        //Fallout_Debug_Info("Object Editor destroyed");
        return 0;


    default:
        return FALSE;
        break;
    }
    return TRUE;
}


//____________________________________________________________________________
int Dialog_ObjectEditor(HWND hwndParent, HINSTANCE hinstance, OBJStruct* pObj) {

    return DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_DIALOG_EDITOR_OBJECT), hwndParent, DlgProc_ObjectEditor, (LPARAM)pObj);
}
