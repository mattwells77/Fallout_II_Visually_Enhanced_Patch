#pragma once

#include "../Fall_Graphics.h"
#include "../Fall_GameMap.h"


#define PROTO_CACHE_VERSION 1

class LIST_VIEW_DATA {
public:
    LIST_VIEW_DATA() {
        position = 0;
        focus = 0;
        is_sorted = FALSE;
        is_filtered = FALSE;
        num_items = 0;
        num_filtered_items = 0;
        p_sorted = nullptr;
        p_filtered = nullptr;

        icon_width = 80;
        icon_height = 80;

        image_Ref = nullptr;
        imageList_Large = nullptr;

        file_name = nullptr;
        file_name_size = 0;
        imageList_modified = FALSE;
    }
    LIST_VIEW_DATA(LONG in_num_items, BOOL in_is_sorted, BOOL in_is_filtered, DWORD in_icon_width, DWORD in_icon_height) {
        position = 0;
        focus = 0;
        is_sorted = in_is_sorted;
        is_filtered = in_is_filtered;
        num_items = in_num_items;
        num_filtered_items = 0;
        p_sorted = nullptr;
        p_filtered = nullptr;
        if (is_sorted) {
            p_sorted = new LONG[num_items];
        }
        if (is_filtered) {
            p_filtered = new LONG[num_items];
            num_filtered_items = num_items;
        }
        if (is_sorted || is_filtered) {
            for (LONG i = 0; i < num_items; i++) {
                if (is_sorted)
                    p_sorted[i] = i;
                if (is_filtered)
                    p_filtered[i] = i;
            }
        }

        icon_width = in_icon_width;
        icon_height = in_icon_height;
        image_Ref = nullptr;
        imageList_Large = nullptr;

        file_name = nullptr;
        file_name_size = 0;
        Reset_ImageList();
    }
    ~LIST_VIEW_DATA() {
        Save_ImageList();
        if (p_sorted)
            delete[] p_sorted;
        p_sorted = nullptr;
        if (p_filtered)
            delete[]p_filtered;
        p_filtered = nullptr;
        if (image_Ref)
            delete[] image_Ref;
        image_Ref = nullptr;
        if (imageList_Large)
            ImageList_Destroy(imageList_Large);
        imageList_Large = nullptr;
    }
    void Setup_List(LONG in_num_items, BOOL in_is_sorted, BOOL in_is_filtered, DWORD in_icon_width, DWORD in_icon_height) {
        position = 0;
        focus = 0;
        is_sorted = in_is_sorted;
        is_filtered = in_is_filtered;
        num_items = in_num_items;
        num_filtered_items = 0;

        if (p_sorted)
            delete[] p_sorted;
        p_sorted = nullptr;
        if (p_filtered)
            delete[]p_filtered;
        p_filtered = nullptr;


        if (is_sorted) {
            p_sorted = new LONG[num_items];
        }
        if (is_filtered) {
            p_filtered = new LONG[num_items];
            num_filtered_items = num_items;
        }
        if (is_sorted || is_filtered) {
            for (LONG i = 0; i < num_items; i++) {
                if (is_sorted)
                    p_sorted[i] = i;
                if (is_filtered)
                    p_filtered[i] = i;
            }
        }

        icon_width = in_icon_width;
        icon_height = in_icon_height;

        if (image_Ref)
            delete[] image_Ref;
        image_Ref = nullptr;
        if (imageList_Large)
            ImageList_Destroy(imageList_Large);
        imageList_Large = nullptr;
        Reset_ImageList();
        imageList_modified = TRUE;
    }
    void Set_FileName(const wchar_t* in_file_name) {
        if (!in_file_name)
            return;
        file_name_size = wcslen(in_file_name) + 1;
        file_name = new wchar_t[file_name_size];
        wcsncpy_s(file_name, file_name_size, in_file_name, _TRUNCATE);
        if (!Load_ImageList()) {
            Reset_ImageList();
        }
    }
    void Set_Position(LONG in_position) { position = in_position; };
    LONG Get_Position() { return position; };
    void Set_Focus(LONG in_focus) { focus = in_focus; };
    LONG Get_Focus() { return focus; };

    LONG Get_NumberOfItems() {
        return num_items;
    }
    LONG Get_NumberOfFilteredItems() {
        return num_filtered_items;
    }

    BOOL Set_NumberOfFilteredItems(LONG in_num_filtered_items) {
        if (!is_filtered)
            return FALSE;
        if (in_num_filtered_items < 0 || in_num_filtered_items > num_items)
            return FALSE;
        num_filtered_items = in_num_filtered_items;
        return TRUE;
    };

    LONG Get_SortedItem(LONG list_num) {
        if (!is_sorted)
            return 0;
        if (list_num < 0 || list_num >= num_items)
            return 0;
        return p_sorted[list_num];
    }
    BOOL Set_SortedItem(LONG list_num, LONG item_num) {
        if (!is_sorted)
            return FALSE;
        if (item_num < 0 || item_num >= num_items)
            return FALSE;
        if (list_num < 0 || list_num >= num_items)
            return FALSE;
        p_sorted[list_num] = item_num;
        return TRUE;
    }

    LONG Get_FilteredItem(LONG list_num) {
        if (!is_filtered)
            return 0;
        if (list_num < 0 || list_num >= num_items)
            return 0;
        return p_filtered[list_num];
    }
    BOOL Set_FilteredItem(LONG list_num, LONG item_num) {
        if (!is_filtered)
            return FALSE;
        if (item_num < 0 || item_num >= num_items)
            return FALSE;
        if (list_num < 0 || list_num >= num_items)
            return FALSE;
        p_filtered[list_num] = item_num;
        return TRUE;
    }
    void Reset_ImageList() {
        if (!image_Ref)
            image_Ref = new LONG[num_items];
        for (LONG i = 0; i < num_items; i++)
            image_Ref[i] = -1;
        if (!imageList_Large)
            imageList_Large = ImageList_Create(icon_width, icon_height, ILC_COLOR32, num_items, 0);
    }
    HIMAGELIST Get_ImageList() { return imageList_Large; };
    LONG Get_ImageRef(LONG num) {
        if (!image_Ref || num < 0 || num >= num_items)
            return -1;
        else
            return image_Ref[num];
    };
    //BOOL Reset_ImageRef(LONG num) {
    //    if (!image_Ref || num < 0 || num >= num_items)
    //        return FALSE;
    //    image_Ref[num] = -1;
    //    return TRUE;
    //};
    /*LONG Set_Image(HBITMAP bitmap, LONG num) {
        if (!bitmap)
            return -1;
        if (!image_Ref)
            return -1;
        if (!imageList_Large)
            return -1;
        if (num < 0 || num >= num_items)
            return -1;

        if (image_Ref[num] == -1) {
            int iconW = 0;
            int iconH = 0;
            ImageList_GetIconSize(imageList_Large, &iconW, &iconH);

            image_Ref[num] = ImageList_Add(imageList_Large, bitmap, nullptr);
            if (image_Ref[num] == -1)
                Fallout_Debug_Error("ImageList_Add failed");
            else
                imageList_modified = TRUE;
        }
        return image_Ref[num];
    };*/
    LONG Set_Image(HBITMAP bitmap, LONG num) {
        if (!bitmap)
            return -1;
        if (!image_Ref)
            return -1;
        if (!imageList_Large)
            return -1;
        if (num < 0 || num >= num_items)
            return -1;
       
            int iconW = 0;
            int iconH = 0;
            ImageList_GetIconSize(imageList_Large, &iconW, &iconH);
            if (image_Ref[num] == -1)
                image_Ref[num] = ImageList_Add(imageList_Large, bitmap, nullptr);
            else
                ImageList_Replace(imageList_Large, image_Ref[num], bitmap, nullptr);
            if (image_Ref[num] == -1)
                Fallout_Debug_Error("ImageList_Add failed");
            else
                imageList_modified = TRUE;
        
        return image_Ref[num];
    };
protected:
private:
    LONG num_items;
    LONG num_filtered_items;
    LONG position;
    LONG focus;
    BOOL is_sorted;
    BOOL is_filtered;
    LONG* p_sorted;
    LONG* p_filtered;

    DWORD icon_width;
    DWORD icon_height;
    HIMAGELIST imageList_Large;
    LONG* image_Ref;
    wchar_t* file_name;
    size_t file_name_size;
    BOOL imageList_modified;

    BOOL Load_ImageList();
    BOOL Save_ImageList();
};


//______________________________________________
class PROTO_cache_type : public LIST_VIEW_DATA {
public:
    PROTO_cache_type(LONG in_type) :
        LIST_VIEW_DATA(GetProListSize(in_type) - 1, TRUE, FALSE, 80, 80)
    {

        p_pro_text = nullptr;
        p_pro = nullptr;
        pro_type = in_type;

        switch (pro_type) {
        case ART_ITEMS:
            pro_size = sizeof(PROTOitem);
            break;
        case ART_CRITTERS:
            pro_size = sizeof(PROTOcritter);
            break;
        case ART_SCENERY:
            pro_size = sizeof(PROTOscenery);
            break;
        case ART_WALLS:
            pro_size = sizeof(PROTOwall);
            break;
        case ART_TILES:
            pro_size = sizeof(PROTOtile);
            break;
        case ART_MISC:
            pro_size = sizeof(PROTOtile);
            break;
        default:
            break;
        }

        p_pro_text = new wchar_t* [Get_NumberOfItems()];
        p_pro = new PROTO[Get_NumberOfItems()];

        if (Load())
            return;
        Create();
        Reset_ImageList();
    }
    ~PROTO_cache_type() {
        if (p_pro)
            delete[] p_pro;
        p_pro = nullptr;
        if (p_pro_text) {
            for (LONG num = 0; num < Get_NumberOfItems(); num++) {
                if (p_pro_text[num])
                    delete[] p_pro_text[num];
                delete[] p_pro_text;
            }
            p_pro_text = nullptr;
        }
    }
    void Rebuild() {
        if (p_pro)
            delete[] p_pro;
        p_pro = nullptr;
        if (p_pro_text) {
            for (LONG num = 0; num < Get_NumberOfItems(); num++) {
                if (p_pro_text[num])
                    delete[] p_pro_text[num];
                delete[] p_pro_text;
            }
            p_pro_text = nullptr;
        }

        Setup_List(GetProListSize(pro_type) - 1, TRUE, TRUE, 80, 80);
        p_pro_text = new wchar_t* [Get_NumberOfItems()];
        p_pro = new PROTO[Get_NumberOfItems()];
        Create();
    }
    PROTO* Get_Proto(LONG item_num) {
        if (!p_pro || item_num < 0 || item_num >= Get_NumberOfItems())
            return nullptr;
        return &p_pro[item_num];
    };
    wchar_t* Get_Text(LONG item_num) {
        if (!p_pro_text || item_num < 0 || item_num >= Get_NumberOfItems())
            return nullptr;
        return p_pro_text[item_num];
    };

protected:
private:
    LONG pro_type;
    DWORD pro_size;
    PROTO* p_pro;
    wchar_t** p_pro_text;

    void Create();
    //proto cache saved file structure:
    // item              size
    //________________________
    //version           4bytes
    //proto_size        4bytes
    //proto_text_size   4bytes
    //num_of_protos     4bytes
    //--data block x num of protos--
    //pro_text_count    4bytes
    //pro_text          proto_text_count * sizeof(wchar_t)
    //proto_data        proto_size
    //------------------------------
    BOOL Load();
    BOOL Save();
};



void ProtoCache_Destroy();
//rebuild proto cache if type is -1, rebuild all
void ProtoCache_Rebuild(LONG type);
PROTO_cache_type* ProtCache_GetType(LONG type);