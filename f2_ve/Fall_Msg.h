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

#include "pch.h"

//for holding a message
struct MSGNode {
       DWORD ref;
       DWORD a;
       char *msg1;//unused
       char *msg2;
       //MSGNode() {
       //   ref=0;
       //   a=0;
       //   msg1=nullptr;
       //   msg2=nullptr;
       //}
};


//for holding msg array
struct MSGList {
       long numMsgs;
       void *MsgNodes;

       //MSGList() {
       //   MsgNodes=nullptr;
       //   numMsgs=0;
       //}
};

extern MSGList* pMsgList_Proto_Type_0;
extern MSGList* pMsgList_Proto_Main;

LONG fall_MsgList_Destroy(MSGList *MsgList);
LONG fall_MsgList_Save(MSGList *MsgList, const char *MsgFilePath);
BOOL fall_LoadMsgList(MSGList *MsgList, const char *MsgFilePath);

MSGNode *GetMsgNode(MSGList *MsgList, DWORD msgRef);
char* GetMsg(MSGList *MsgList, DWORD msgRef, int msgNum);

void Fallout_Functions_Setup_Msg();






