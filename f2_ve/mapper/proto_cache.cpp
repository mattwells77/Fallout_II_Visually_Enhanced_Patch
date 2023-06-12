
#include "pch.h"

#include "proto_cache.h"

#include "win_ProtoBrowser.h"
#include "win_Mapper.h"
#include "win_ProgressBar.h"

#include "../resource.h"
#include "../win_fall.h"

#include "../Fall_Msg.h"


BOOL LIST_VIEW_DATA::Load_ImageList() {
    if (!file_name)
        return FALSE;

    IStream* pStream = nullptr;
    if (!MapperData_OpenFileStream(file_name, file_name_size, STGM_READ, FALSE, &pStream))
        return FALSE;

    DWORD numBytesRead = 0;

    //read number of items in list.
    LONG listSize_read = 0;
    if (pStream->Read(&listSize_read, sizeof(LONG), &numBytesRead) != S_OK) {
        pStream->Release();
        return FALSE;
    }
    if (listSize_read != num_items) {//if number of items saved doesn't match the declared size.
        pStream->Release();
        return FALSE;
    }

    //read image list refs for each item.
    for (LONG i = 0; i < listSize_read; i++) {
        if (pStream->Read(&image_Ref[i], sizeof(LONG), &numBytesRead) != S_OK) {
            pStream->Release();
            return FALSE;
        }
    }
    //read image list data.
    if (imageList_Large)
        ImageList_Destroy(imageList_Large);
    imageList_Large = ImageList_Read(pStream);
    pStream->Release();

    if (imageList_Large == nullptr)
        return FALSE;
    return TRUE;
};
BOOL LIST_VIEW_DATA::Save_ImageList() {
    if (!imageList_modified)
        return FALSE;
    if (!file_name)
        return FALSE;
    if (!image_Ref)
        return FALSE;
    if (!imageList_Large)
        return FALSE;

    IStream* pStream = nullptr;
    if (!MapperData_OpenFileStream(file_name, file_name_size, STGM_WRITE | STGM_CREATE, TRUE, &pStream))
        return FALSE;

    DWORD numBytesWritten = 0;

    //write number of items in list.
    if (pStream->Write(&num_items, sizeof(LONG), &numBytesWritten) != S_OK) {
        pStream->Release();
        return FALSE;
    }
    //write current image list refs for each item.
    for (LONG i = 0; i < num_items; i++) {
        if (pStream->Write(&image_Ref[i], sizeof(LONG), &numBytesWritten) != S_OK) {
            pStream->Release();
            return FALSE;
        }
    }
    //write image list data.
    BOOL retVal = ImageList_Write(imageList_Large, pStream);
    pStream->Release();

    return retVal;
}



//_____________________________
void PROTO_cache_type::Create() {

    Fallout_Debug_Info("PROTO_cache_type::Create(), type:%d num items:%d", pro_type, Get_NumberOfItems());

    PROTO* p_pro_fall = nullptr;

    wchar_t* string_pro_cachenew = new wchar_t[21];
    LoadString(phinstDLL, IDS_BUIDING_PROTO_CACHE, string_pro_cachenew, 21);
    wchar_t* string_pro_type = new wchar_t[16];
    LoadString(phinstDLL, Get_Proto_Type_TextID(pro_type), string_pro_type, 16);
    wchar_t* progessbar_string = new wchar_t[32];
    swprintf_s(progessbar_string, 32, L" %s:%s", string_pro_cachenew, string_pro_type);
    delete[] string_pro_type;
    delete[] string_pro_cachenew;

    HWND hwnd_progressBar = ProgressBar_Create(*phWinMain, phinstDLL, progessbar_string, (WORD)Get_NumberOfItems());
    delete[] progessbar_string;

    DWORD proID = -1;
    char* msg = nullptr;
    DWORD num_chars = 0;

    for (LONG num = 0; num < Get_NumberOfItems(); num++) {
        ProgressBar_Update(hwnd_progressBar);

        proID = GetProID(pro_type, num);
        if (proID == -1) {
            Fallout_Debug_Error("PROTO_cache GetProID failed, type:%d num:%d", pro_type, num);
            continue;
        }
        if (fall_GetPro(proID, &p_pro_fall) == 0) {
            //copy proto data
            memcpy(&p_pro[num], p_pro_fall, pro_size);
            //copy proto title string for list display and searching. if this fails, create a string based on the proto list number.
            msg = GetMsg(pMsgList_Proto_Type_0 + pro_type, p_pro[num].txtID, 2);
            if (msg && msg[0] != '\0' && strncmp(msg, "Error", 5) != 0) {

                num_chars = strlen(msg) + 1;
                p_pro_text[num] = new wchar_t[num_chars] {0};
                swprintf_s(p_pro_text[num], num_chars, L"%S", msg);
            }
            else {
                num_chars = 17;
                p_pro_text[num] = new wchar_t[num_chars] {0};
                swprintf_s(p_pro_text[num], num_chars, L"proto:%u", p_pro[num].proID & 0x00FFFFFF);
            }

            //Fallout_Debug_Info("PROTO_cache add pro passed, type:%d num:%d", pro_type, num);
        }
        else
            Fallout_Debug_Error("PROTO_cache add prototype ""fall_GetPro"" failed, type: % d num : % d", pro_type, num);
    }
    DestroyWindow(hwnd_progressBar);

    Save();
}


//___________________________
BOOL PROTO_cache_type::Load() {

    //Fallout_Debug_Info("PROTO_cache_type::Load(), type:%d", pro_type);

    FILE* fileCache = nullptr;
    DWORD version = 0;

    wchar_t file_name[20];
    swprintf_s(file_name, _countof(file_name), L"\\proto_Cache_%d.dat", pro_type);
    //Fallout_Debug_Info("PROTO_cache_type::Load() file name, type:%S", file_name);
    if (MapperData_OpenFile(&fileCache, file_name, _countof(file_name), L"rb")) {
        fread(&version, sizeof(DWORD), 1, fileCache);
        if (version != PROTO_CACHE_VERSION) {
            fclose(fileCache);
            Fallout_Debug_Error("PROTO_cache_type::Load(), version mismatch, type:%d version:%d", pro_type, version);
            return FALSE;
        }
        //Fallout_Debug_Info("PROTO_cache_type::Load(), version:%d", version);

        fread(&pro_size, sizeof(DWORD), 1, fileCache);
        //Fallout_Debug_Info("PROTO_cache_type::Load(), pro_size:%d", pro_size);
        DWORD num_items = 0;
        fread(&num_items, sizeof(DWORD), 1, fileCache);
        if (num_items != Get_NumberOfItems()) {
            fclose(fileCache);
            Fallout_Debug_Error("PROTO_cache_type::Load(), num_items mismatch, type: %d", pro_type);
            return FALSE;
        }
        //Fallout_Debug_Info("PROTO_cache_type::Load(), num_items:%d", num_items);

        DWORD num_chars = 0;
        for (LONG num = 0; num < Get_NumberOfItems(); num++) {
            fread(&num_chars, sizeof(DWORD), 1, fileCache);
            p_pro_text[num] = new wchar_t[num_chars] {0};
            if (fread(p_pro_text[num], num_chars * sizeof(wchar_t), 1, fileCache) < 1) {
                fclose(fileCache);
                Fallout_Debug_Error("PROTO_cache_type::Load(), fread error text, type: %d", pro_type);
                return FALSE;
            }
            //Fallout_Debug_Info("PROTO_cache_type::Load(), text:%S", p_pro_text[num]);
            if (fread(&p_pro[num], pro_size, 1, fileCache) < 1) {
                fclose(fileCache);
                Fallout_Debug_Error("PROTO_cache_type::Load(), fread error proto data, type: %d", pro_type);
                return FALSE;
            }
            //Fallout_Debug_Info("PROTO_cache_type::Load(), proid:%d", p_pro[num].item.proID);
        }
        fclose(fileCache);
    }
    else {
        Fallout_Debug_Error("PROTO_cache_type::Load(), MapperData_OpenFile failed, type: %d", pro_type);
        return FALSE;
    }
    //Fallout_Debug_Info("PROTO_cache_type::Load() end good, type:%d d", pro_type);
    return TRUE;
}


//___________________________
BOOL PROTO_cache_type::Save() {
    FILE* fileCache = nullptr;
    DWORD version = PROTO_CACHE_VERSION;

    wchar_t file_name[20];
    swprintf_s(file_name, _countof(file_name), L"\\proto_Cache_%d.dat", pro_type);

    if (MapperData_OpenFile(&fileCache, file_name, _countof(file_name), L"wb")) {
        fwrite(&version, sizeof(DWORD), 1, fileCache);
        fwrite(&pro_size, sizeof(DWORD), 1, fileCache);

        DWORD num_items = Get_NumberOfItems();
        fwrite(&num_items, sizeof(DWORD), 1, fileCache);

        DWORD num_chars = 0;
        for (LONG num = 0; num < Get_NumberOfItems(); num++) {
            num_chars = wcslen(p_pro_text[num]) + 1;
            fwrite(&num_chars, sizeof(DWORD), 1, fileCache);
            fwrite(p_pro_text[num], num_chars * sizeof(wchar_t), 1, fileCache);
            fwrite(&p_pro[num], pro_size, 1, fileCache);
        }
        fclose(fileCache);
    }
    else {
        Fallout_Debug_Error("PROTO_cache_type::Save(), MapperData_OpenFile failed, type: %d", pro_type);
        return FALSE;
    }
    return TRUE;
}



class PROTO_cache_all {
public:
    PROTO_cache_all() {
        wchar_t* mapper_icon_file = new wchar_t[32] { 0 };
        for (LONG type = 0; type < 6; type++) {
            p_proto_cache[type] = new PROTO_cache_type(type);
            swprintf_s(mapper_icon_file, 32, L"\\proto_IconCache_%d.dat", type);
            p_proto_cache[type]->Set_FileName(mapper_icon_file);
        }
        delete mapper_icon_file;
    }
    ~PROTO_cache_all() {
        for (LONG type = 0; type < 6; type++) {
            delete p_proto_cache[type];
            p_proto_cache[type] = nullptr;
        }
    }
    PROTO_cache_type* Get_ProtoCache(LONG type) {
        if (type < 0 || type >= 6)
            return nullptr;
        return p_proto_cache[type];
    }
    void Rebuild(LONG type) {
        if (type == -1) {
            for (LONG i = 0; i < 6; i++)
                p_proto_cache[i]->Rebuild();
        }
        else if (type >= 0 && type < 6)
            p_proto_cache[type]->Rebuild();
    }
protected:
private:
    PROTO_cache_type* p_proto_cache[6];
};


PROTO_cache_all* protoCache = nullptr;


//______________________
void ProtoCache_Create() {
    if (!protoCache) {
        protoCache = new PROTO_cache_all();
        PROTO_cache_type* p_pCache = nullptr;
        wchar_t* mapper_icon_file = new wchar_t[32] { 0 };
        for (LONG type = 0; type < 6; type++) {
            p_pCache = protoCache->Get_ProtoCache(type);
            swprintf_s(mapper_icon_file, 32, L"\\proto_IconCache_%d.dat", type);
            p_pCache->Set_FileName(mapper_icon_file);
        }
        delete mapper_icon_file;
    }
}


//_______________________
void ProtoCache_Destroy() {

    if (protoCache) {
        delete protoCache;
        protoCache = nullptr;
    }
}


//________________________________
void ProtoCache_Rebuild(LONG type) {
    //if type is -1, rebuild all
    if (!protoCache)
        ProtoCache_Create();
    protoCache->Rebuild(type);

    ProtoList_Refresh();
}


//____________________________________________
PROTO_cache_type* ProtCache_GetType(LONG type) {

    if (!protoCache)
        ProtoCache_Create();
    return protoCache->Get_ProtoCache(type);
}
