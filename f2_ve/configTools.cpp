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
#include "configTools.h"
#include "Fall_File.h"
#include "version.h"


wchar_t* pConfigPath = nullptr;
wchar_t* pSFallPath = nullptr;


//__________________________________________________
bool GetSfall_VersionInfo(char* msg, DWORD maxChars) {
    if (msg == nullptr)
        return false;
    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = nullptr;

    DWORD  verSize = GetFileVersionInfoSizeA("ddraw.dll", &verHandle);
    if (verSize == 0)
        return false;

    LPSTR verData = new char[verSize];
    verHandle = 0;//ignored in GetFileVersionInfoA - causes a compiler warning if not set 0. ?
    if (!GetFileVersionInfoA("ddraw.dll", verHandle, verSize, verData))
        return false;

    if (!VerQueryValueA(verData, "\\", (VOID FAR * FAR*) & lpBuffer, &size))
        return false;
    if (!size)
        return false;

    VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
    if (verInfo->dwSignature != 0xfeef04bd)
        return false;

    sprintf_s(msg, maxChars, "SFALL %d.%d.%d.%d\n", (verInfo->dwFileVersionMS >> 16) & 0xffff, (verInfo->dwFileVersionMS >> 0) & 0xffff, (verInfo->dwFileVersionLS >> 16) & 0xffff, (verInfo->dwFileVersionLS >> 0) & 0xffff);
    delete[] verData;
    return true;
}


// ______________________________________________
DWORD GetConfigPath_NonUAC(wchar_t* pReturnPath) {
    DWORD buffSize = GetCurrentDirectory(0, nullptr);
    buffSize += 12;
    if (!pReturnPath)
        return buffSize;

    if (GetCurrentDirectory(buffSize, pReturnPath))
        wcsncat_s(pReturnPath, buffSize, L"\\f2_ve.ini", 10);

    return buffSize;
}

/*
//____________________________________________
DWORD UAC_GetAppDataPath(wchar_t* appDataPath) {

    wchar_t* pRoamingAppData = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, SHGFP_TYPE_CURRENT, nullptr, &pRoamingAppData);

    DWORD appDatPathSize = 0;
    DWORD appPathSize = 0;

    while (pRoamingAppData[appPathSize] != '\0')
        appPathSize++;

    DWORD currentDirSize = GetCurrentDirectory(0, nullptr);//get the path size
    wchar_t* dirCurrent = new wchar_t[currentDirSize];
    GetCurrentDirectory(currentDirSize, dirCurrent);


    BYTE bHash[16];//convert the game path to hash data
    HashData((BYTE*)dirCurrent, currentDirSize, bHash, 16);
    wchar_t bHashString[33];

    for (int i = 0; i < 16; ++i)//convert the hash data to a string, this will be a unique folder name to store the config data in.
        swprintf_s(&bHashString[i * 2], 33 - i * 2, L"%02x", bHash[i]);

    appDatPathSize = 4 + appPathSize + 9 + 1 + 33;
    if (!appDataPath) {
        CoTaskMemFree(pRoamingAppData);
        return appDatPathSize;
    }

    ZeroMemory(appDataPath, appDatPathSize);
    wcsncat_s(appDataPath, appDatPathSize, L"\\\\?\\", appPathSize);//uniPrependSize // to allow for strings longer than MAX_PATH
    wcsncat_s(appDataPath, appDatPathSize, pRoamingAppData, appPathSize);//appDatPathSize
    CoTaskMemFree(pRoamingAppData);
    wcsncat_s(appDataPath, appDatPathSize, L"\\Fallout2", 9);

    if (GetFileAttributes(appDataPath) == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectory(appDataPath, nullptr)) {
            appDataPath[0] = L'\0';
            return 0;
        }
    }

    wcsncat_s(appDataPath, appDatPathSize, L"\\", 1);//BackSlashSize
    wcsncat_s(appDataPath, appDatPathSize, bHashString, 33);//hexFolderSize
    delete[] dirCurrent;
    if (GetFileAttributes(appDataPath) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(appDataPath, nullptr);

    return appDatPathSize;
}
*/

//_________________
void ConfigCreate() {

    //To-Do - Lets ditch the UAC stuff for now, not sure anyone was using it anyway.

    /*LONG UAC_AWARE = ConfigReadInt(L"MAIN", L"UAC_AWARE", 1);

    if (UAC_AWARE && IsWindowsVistaOrGreater()) {
        DWORD buffSize = GetCurrentDirectory(0, nullptr);
        wchar_t* pGameFolderPath = new wchar_t[buffSize];
        GetCurrentDirectory(buffSize, pGameFolderPath);
        //insert current path for visual identification.
        WritePrivateProfileString(L"LOCATION", L"path", pGameFolderPath, pConfigPath);
        delete[] pGameFolderPath;
        pGameFolderPath = nullptr;

    }*/


    ConfigWriteInt(L"MAIN", L"SCALE_LEVEL", 1);
    ConfigWriteInt(L"MAIN", L"WINDOWED", 0);
    ConfigWriteInt(L"MAIN", L"WIN_DATA", 0);
    ConfigWriteString(L"MAIN", L"f2_res_dat", L"f2_res.dat");
    ConfigWriteString(L"MAIN", L"f2_res_patches", L"data\\");

    ConfigWriteInt(L"INPUT", L"ALT_MOUSE_INPUT", 1);
    ConfigWriteInt(L"INPUT", L"SCROLLWHEEL_FOCUS_PRIMARY_MENU", 1);

    ConfigWriteInt(L"EFFECTS", L"IS_GRAY_SCALE", 1);

    ConfigWriteInt(L"MOVIES", L"MOVIE_SIZE", 1);


    ConfigWriteInt(L"MAPS", L"EDGE_CLIPPING_ON", 1);
    ConfigWriteInt(L"MAPS", L"IGNORE_MAP_EDGES", 0);
    ConfigWriteInt(L"MAPS", L"NumPathNodes", 1);
    ConfigWriteInt(L"MAPS", L"FOG_OF_WAR", 1);
    ConfigWriteInt(L"MAPS", L"ZOOM_LEVEL", 1);
    ConfigWriteInt(L"MAPS", L"IS_ZOOM_BOUND_BY_EDGES", 0);
    ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_FLOOR", 0);
    ConfigWriteInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_OBJECTS", 0);
    ConfigWriteInt(L"MAPS", L"PC_LIGHT_COLOUR", 0);
    ConfigWriteString(L"MAPS", L"PC_LIGHT_COLOUR", L"0xFFFFFF ;R00 G00 B00");//display as a hexadecimal number in the ini, so as to more intuitively modify colour values.

    ConfigWriteInt(L"WORLD_MAP", L"ZOOM_LEVEL", 1);
    ConfigWriteInt(L"WORLD_MAP", L"DRAW_CITIES", 1);


    ConfigWriteInt(L"IFACE", L"IFACE_BAR_SIDE_ART", 1);
    ConfigWriteInt(L"IFACE", L"IFACE_BAR_WIDTH", 800);
    ConfigWriteInt(L"IFACE", L"IFACE_BAR_LOCATION", 0);//0 = centre, 1 = left, 2 = right
    //ConfigWriteInt(L"IFACE", L"IFACE_BG_COLOUR", 0x0000007F);
    ConfigWriteString(L"IFACE", L"IFACE_BG_COLOUR", L"0x0000007F ;R00 G00 B00 A00");//display as a hexadecimal number in the ini, so as to more intuitively modify colour values.

    ConfigWriteInt(L"MAINMENU", L"MAIN_MENU_SIZE", 0);
    ConfigWriteInt(L"MAINMENU", L"USE_HIRES_IMAGES", 1);
    ConfigWriteInt(L"MAINMENU", L"MENU_BG_OFFSET_X", -14);
    ConfigWriteInt(L"MAINMENU", L"MENU_BG_OFFSET_Y", -4);

    ConfigWriteInt(L"STATIC_SCREENS", L"DEATH_SCRN_SIZE", 1);
    ConfigWriteInt(L"STATIC_SCREENS", L"END_SLIDE_SIZE", 1);
    ConfigWriteInt(L"STATIC_SCREENS", L"HELP_SCRN_SIZE", 1);
    ConfigWriteInt(L"STATIC_SCREENS", L"SPLASH_SCRN_SIZE", 1);


    ConfigWriteInt(L"OTHER_SETTINGS", L"CD_CHECK", 0);
    ConfigWriteInt(L"OTHER_SETTINGS", L"DIALOG_SCRN_BACKGROUND", 0);
    ConfigWriteInt(L"OTHER_SETTINGS", L"DIALOG_SCRN_ART_FIX", 1);
    ConfigWriteInt(L"OTHER_SETTINGS", L"SPLASH_SCRN_TIME", 0);
    ConfigWriteInt(L"OTHER_SETTINGS", L"CPU_USAGE_FIX", 0);

}


//____________________
void ConfigPathSetup() {
    if (pConfigPath)
        return;

    DWORD pathSize = 0;

    pathSize = GetConfigPath_NonUAC(nullptr);
    if (pathSize) {
        pConfigPath = new wchar_t[pathSize];
        GetConfigPath_NonUAC(pConfigPath);
    }

    //To-Do - Lets ditch the UAC stuff for now, not sure anyone was using it anyway.

    /*LONG UAC_AWARE = ConfigReadInt(L"MAIN", L"UAC_AWARE", 1);

    if (UAC_AWARE && IsWindowsVistaOrGreater()) {
        delete[] pConfigPath;
        pConfigPath = nullptr;
        DWORD appDatPathSize = UAC_GetAppDataPath(nullptr);
        wchar_t* pAppDatPath = nullptr;
        if (appDatPathSize) {
            pAppDatPath = new wchar_t[appDatPathSize];
            appDatPathSize = UAC_GetAppDataPath(pAppDatPath);
        }

        pathSize = appDatPathSize + 12;
        pConfigPath = new wchar_t[pathSize];
        ZeroMemory(pConfigPath, pathSize);
        if (pAppDatPath)
            wcsncat_s(pConfigPath, pathSize, pAppDatPath, appDatPathSize);//appDatPathSize
        delete[] pAppDatPath;
        pAppDatPath = nullptr;
        wcsncat_s(pConfigPath, pathSize, L"\\f2_ve.ini", 10);
    }*/

    //create config file if not found.
    if (pConfigPath && GetFileAttributes(pConfigPath) == INVALID_FILE_ATTRIBUTES)
        ConfigCreate();
}


//_______________________
void ConfigRefreshCache() {
    ConfigPathSetup();
    WritePrivateProfileString(nullptr, nullptr, nullptr, pConfigPath);
}


//___________________
void SFallPathSetup() {
    if (pSFallPath)return;

    pSFallPath = new wchar_t[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, pSFallPath);
    wcsncat_s(pSFallPath, MAX_PATH, L"\\ddraw.ini", 12);
}


//__________________________________________________________________________________________________________________________________________
DWORD ConfigReadString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize) {
    ConfigPathSetup();
    return GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, pConfigPath);
}


//_________________________________________________________________________________________________
BOOL ConfigWriteString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpString) {
    ConfigPathSetup();
    return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pConfigPath);
}


//__________________________________________________________________________________
UINT ConfigReadInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int nDefault) {
    ConfigPathSetup();
    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, pConfigPath);
}


//_________________________________________________________________________________
BOOL ConfigWriteInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int intVal) {
    ConfigPathSetup();
    wchar_t lpString[64];
    swprintf_s(lpString, 64, L"%d", intVal);
    return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pConfigPath);
}


//___________________________________________________________________________________________________
BOOL ConfigReadWinData(const wchar_t* lpAppName, const wchar_t* lpKeyName, WINDOWPLACEMENT* pWinData) {
    ConfigPathSetup();
    pWinData->length = sizeof(WINDOWPLACEMENT);
    return GetPrivateProfileStruct(lpAppName, lpKeyName, pWinData, sizeof(WINDOWPLACEMENT), pConfigPath);
}


//____________________________________________________________________________________________________
BOOL ConfigWriteWinData(const wchar_t* lpAppName, const wchar_t* lpKeyName, WINDOWPLACEMENT* pWinData) {
    ConfigPathSetup();
    return WritePrivateProfileStruct(lpAppName, lpKeyName, pWinData, sizeof(WINDOWPLACEMENT), pConfigPath);
}


//__________________________________________________________________________________________________________
BOOL ConfigReadStruct(const wchar_t* lpszSection, const wchar_t* lpszKey, LPVOID lpStruct, UINT uSizeStruct) {
    ConfigPathSetup();
    return GetPrivateProfileStruct(lpszSection, lpszKey, lpStruct, uSizeStruct, pConfigPath);
}


//___________________________________________________________________________________________________________
BOOL ConfigWriteStruct(const wchar_t* lpszSection, const wchar_t* lpszKey, LPVOID lpStruct, UINT uSizeStruct) {
    ConfigPathSetup();
    return WritePrivateProfileStruct(lpszSection, lpszKey, lpStruct, uSizeStruct, pConfigPath);
}


//_________________________________________________________________________________
UINT SfallReadInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int nDefault) {
    SFallPathSetup();
    return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, pSFallPath);
}


//________________________________________________________________________________
BOOL SfallWriteInt(const wchar_t* lpAppName, const wchar_t* lpKeyName, int intVal) {
    SFallPathSetup();
    wchar_t lpString[64];
    swprintf_s(lpString, 64, L"%d", intVal);
    return WritePrivateProfileString(lpAppName, lpKeyName, lpString, pSFallPath);
}


//_________________________________________________________________________________________________________________________________________
DWORD SfallReadString(const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize) {
    SFallPathSetup();
    return GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, pSFallPath);
}
