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
#include "hiResPatcher.h"

const wchar_t* exeName = L"fallout2.exe";


EXEdata* exeData = nullptr;

EXEdata exeData_US = {
   {0xD4880},
   {0xEE5C0},
   {0x00019D4B},
   {0x004FE1C0},
   {0x006C020C},
   {0xC7,0x05,0x88,0x27,0x6B,0x00,0x00,0xE7,0x4D,0x00},
   {1},//{"US 1.02d"},
   {false}
};

EXEdata exeData_FG = {
   {0xD4B50},
   {0xEE890},
   {0x00019D4B},
   {0x004FE490},
   {0x006C020C},
   {0xC7,0x05,0x88,0x27,0x6B,0x00,0xD0,0xE9,0x4D,0x00},
   {2},//{"French/German 1.02d"},
   {false}
};

EXEdata exeData_UK = {
   {0xD4A30},
   {0xEE770},
   {0x00019D4B},
   {0x004FE370},
   {0x006C020C},
   {0xC7,0x05,0x88,0x27,0x6B,0x00,0xB0,0xE8,0x4D,0x00},
   {3},//{"UK 1.02e"},
   {false}
};

EXEdata exeData_CH = {
   {0xDF304},
   {0xFB452},
   {0x0001C159},
   {0x0050B052},
   {0x006D0264},
   {0xC7,0x05,0xD8,0x2C,0x6C,0x00,0xAA,0x6E,0x4E,0x00},
   {4},//{"Chinese"},
   {false}
};

EXEdata exeData_RU = {
   {0xD4750},
   {0xEE490},
   {0x00019D4B},
   {0x004FE090},
   {0x006C020C},
   {0xC7,0x05,0x88,0x27,0x6B,0x00,0xD0,0xE5,0x4D,0x00},
   {5},//{"LEV CORP"},
   {false}
};

BYTE exeHookString[10] = {0xE8,0x4B,0x9D,0x01,0x00,0x90,0x90,0x90,0x90,0x90};

BYTE dllLoadString[39] = {'f','2','_','r','e','s','.','d','l','l','\0','\0','\0','\0','\0','\0', //16 dll Name
                         0x68,0xC0,0xE1,0x4F,0x00,//5 push dll Name offset
                         0x2E,0xFF,0x15,0x0C,0x02,0x6C,0x00, //7 call LoadLibrary
                         0xC7,0x05,0x88,0x27,0x6B,0x00,0x00,0xE7,0x4D,0x00,//10 insert original code
                         0xC3};//return from dll load


//_____________________
bool SetDllLoadString() {
    if (!exeData)
        return false;
    DWORD* ptr = nullptr;
    ptr = (DWORD*)(dllLoadString + 17);
    *ptr = exeData->memOff_dllName;
    ptr = (DWORD*)(dllLoadString + 24);
    *ptr = exeData->memOff_LoadLibrary;
    memcpy(dllLoadString + 28, exeData->original, 10);

    ptr = (DWORD*)(exeHookString + 1);
    *ptr = exeData->memOff_dllLoad;

    return true;
}


//_________________________________________________
EXEdata* CheckExeData(FILE* exeFile, EXEdata* data) {
    int exeFlag = 0;
    BYTE buff[10];
    int error = fseek(exeFile, data->fileOff_hook, SEEK_SET);
    if (!error)
        fread(buff, 1, 10, exeFile); //copy code at origin to buffer 10 bytes
    if (!error) {
        if (memcmp(buff, data->original, 10) == 0)
            exeFlag = 1, data->isPatched = false;
        else if (buff[0] == 0xE8 && *(DWORD*)(buff + 1) == data->memOff_dllLoad)
            exeFlag = -1, data->isPatched = true;
    }
    if (exeFlag)
        return data;
    return nullptr;
}


//________________________
LONG HiRes_SetPatchData() {
    FILE* exeFile = nullptr;

    if (_wfopen_s(&exeFile, exeName, L"rb") || exeFile == nullptr)// open file for binary read write
        return 1;

    exeData = CheckExeData(exeFile, &exeData_US);
    if (!exeData)
        exeData = CheckExeData(exeFile, &exeData_FG);
    if (!exeData)
        exeData = CheckExeData(exeFile, &exeData_UK);
    if (!exeData)
        exeData = CheckExeData(exeFile, &exeData_CH);
    if (!exeData)
        exeData = CheckExeData(exeFile, &exeData_RU);

    fclose(exeFile);

    if (exeData) {
        SetDllLoadString();
        return 0;
    }
    return 2;
}


//___________________
bool HiRes_PatchExe() {
    if (!exeData)
        return false;
    FILE* exeFile = nullptr;

    if (_wfopen_s(&exeFile, exeName, L"rb+") || exeFile == nullptr) {// open file for binary read write
        wchar_t* msg = new wchar_t[128];
        swprintf_s(msg, 128, L"Could not open %s. Make sure %s is not set read-only.", exeName, exeName);
        MessageBox(0, msg, exeName, MB_ICONINFORMATION);
        delete[] msg;
        return false;
    }

    int error = fseek(exeFile, exeData->fileOff_hook, SEEK_SET);
    if (!error) {
        if (!exeData->isPatched)
            fwrite(exeHookString, 1, 10, exeFile);//install call to new code
        else
            fwrite(exeData->original, 1, 10, exeFile);//restore old code
    }
    error = fseek(exeFile, exeData->fileOff_dllLoad, SEEK_SET);
    if (!error) {
        if (!exeData->isPatched)
            fwrite(dllLoadString, 1, 39, exeFile);//install load library function
        else {
            for (int i = 0; i < 39; i++)  //zero out load library function
                fputc(0, exeFile);
        }
    }
    fclose(exeFile);

    if (!exeData->isPatched)
        exeData->isPatched = true;
    else
        exeData->isPatched = false;

    return true;
}


//_______________________
void HiRes_PatchInstall() {

    wchar_t exeName[MAX_PATH];
    GetModuleFileName(nullptr, exeName, MAX_PATH);

    SHELLEXECUTEINFO shExInfo = { 0 };
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = L"runas";                 // Operation to perform
    shExInfo.lpFile = exeName;                  // Application to start
    shExInfo.lpParameters = L"/patch_install";  // Additional parameters
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = 0;

    if (ShellExecuteEx(&shExInfo) && shExInfo.hProcess != nullptr) {
        WaitForSingleObject(shExInfo.hProcess, INFINITE);
        CloseHandle(shExInfo.hProcess);
    }
}