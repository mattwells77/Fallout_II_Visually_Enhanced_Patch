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
#include "Fall_Msg.h"
#include "Fall_General.h"
#include "memwrite.h"


void* pfall_msglist_destroy = nullptr;
void* pfall_msglist_save = nullptr;
void* pfall_msglist_load = nullptr;

MSGList* pMsgList_Proto_Type_0 = nullptr;
MSGList* pMsgList_Proto_Main = nullptr;


//__________________________________________
LONG fall_MsgList_Destroy(MSGList* pMsgList) {
    int retVal;
    __asm {
        mov eax, pMsgList
        call pfall_msglist_destroy
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________________
LONG fall_MsgList_Save(MSGList* pMsgList, const char* MsgFilePath) {
    int retVal;
    __asm {
        mov edx, MsgFilePath
        mov eax, pMsgList
        call pfall_msglist_save
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________
BOOL fall_LoadMsgList(MSGList* pMsgList, const char* MsgFilePath) {
    int retVal;
    __asm {
        mov edx, MsgFilePath
        mov eax, pMsgList
        call pfall_msglist_load
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________
MSGNode* GetMsgNode(MSGList* MsgList, DWORD msgRef) {

    if (MsgList == nullptr)return nullptr;
    if (MsgList->numMsgs <= 0)return nullptr;

    MSGNode* pMsgNode = (MSGNode*)MsgList->MsgNodes;

    long last = MsgList->numMsgs - 1;
    long first = 0;
    long mid;

    //Use Binary Search to find msg
    while (first <= last) {
        mid = (first + last) / 2;
        if (msgRef > pMsgNode[mid].ref)
            first = mid + 1;
        else if (msgRef < pMsgNode[mid].ref)
            last = mid - 1;
        else
            return &pMsgNode[mid];
    }
    return nullptr;
}


//______________________________________________________
char* GetMsg(MSGList* MsgList, DWORD msgRef, int msgNum) {
    MSGNode* pMsgNode = GetMsgNode(MsgList, msgRef);
    if (pMsgNode) {
        if (msgNum == 2)
            return pMsgNode->msg2;
        else if (msgNum == 1)
            return pMsgNode->msg1;
    }

    fall_Debug_printf("/n **String not found @ getmsg() , MESSAGE.C** /n");
    return (char*)"Error";
}


//________________________________
void Fallout_Functions_Setup_Msg() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        pfall_msglist_destroy = (void*)0x483D88;
        pfall_msglist_save = (void*)0x483DF8;
        pfall_msglist_load = (void*)0x483EC8;

        pMsgList_Proto_Type_0 = (MSGList*)0x674D38;
        //To-Do pMsgList_Proto_Main = (MSGList*)0x;

    }
    else {
        pfall_msglist_destroy = (void*)FixAddress(0x484964);
        pfall_msglist_save = (void*)FixAddress(0x4849D4);
        pfall_msglist_load = (void*)FixAddress(0x484AA4);

        pMsgList_Proto_Type_0 = (MSGList*)FixAddress(0x6647AC);
        pMsgList_Proto_Main = (MSGList*)FixAddress(0x6647FC);
    }

}




