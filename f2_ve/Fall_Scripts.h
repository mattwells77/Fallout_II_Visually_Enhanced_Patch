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

#include "Fall_Objects.h"


enum class SCRIPT_TYPE : LONG {
    none = -1,
    system = 0,
    spatial = 1,
    time = 2,
    item = 3,
    critter = 4,
};


struct UDATA_SP {
    LONG build_tile; //= hexpos | ((level << 29) & 0xE0000000);
    LONG radius;
};
struct UDATA_TM {
    LONG time;
    LONG none;
};
union UDATA {
    UDATA_SP sp;
    UDATA_TM tm;
};

struct SCRIPT_STRUCT {
    DWORD id;                   //0x00
    SCRIPT_STRUCT* next;        //0x04
    UDATA udata;                //0x08
    LONG flags;                 //0x10
    LONG index;                 //0x14
    DWORD unknown18;            //0x18
    DWORD objID;                //0x1C
    DWORD local_var_offset;     //0x20
    DWORD num_local_vars;       //0x24

    DWORD unknown28;            //0x28
    DWORD unknown2C;            //0x2C
    DWORD unknown30;            //0x30
    OBJStruct* pObj;            //0x34
    OBJStruct* pObj_unknown38;  //0x38 //*obj interact with?
    OBJStruct* pObj_unknown3C;  //0x3C //*obj interact with? pickup
    DWORD unknown40;            //0x40
    DWORD unknown44;            //0x44//isScenery?
    DWORD unknown48;            //0x48
    DWORD unknown4C;            //0x4C
    DWORD unknown50;            //0x50
};


void Fallout_Functions_Setup_Scripts();

LONG fall_Script_Get(DWORD scriptID, SCRIPT_STRUCT **lpScr);

void fall_GetScriptPath(char *ret_path);

LONG fall_Script_Remove(DWORD scriptID);

LONG fall_Set_Map_Script(LONG scriptIndex);

LONG fall_Get_Number_Of_Active_Scripts(LONG scriptType);

//p_scr_50 usually points to SCRIPT_STRUCT.unknown50 but is ignored in this function.
LONG fall_Script_Find_Str_Run_Info(LONG level, DWORD* p_scr_50, DWORD scriptID);

LONG fall_Script_New(DWORD* p_scriptID, LONG script_type);

SCRIPT_STRUCT* fall_Script_Find_First_At(LONG level);
SCRIPT_STRUCT* fall_Script_Find_Next_At(LONG level);

extern LONG(*fall_script_set_dude_script)();
extern LONG(*fall_script_clear_dude_script)();

extern LONG(*fall_scripts_initiate)();
extern LONG(*fall_scripts_reset)();
extern LONG(*fall_scripts_exit)();

extern LONG(*fall_scripts_game_initiate)();
extern LONG(*fall_scripts_game_reset)();
extern LONG(*fall_scripts_game_clear)();

extern void(*fall_scripts_free_msglist)();

extern LONG(*fall_script_proccessing_enable)();
extern void(*fall_script_proccessing_disable)();
extern void(*fall_script_remove_all)();