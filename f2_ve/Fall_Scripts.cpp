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
#include "Fall_Scripts.h"
#include "memwrite.h"



LONG(*fall_script_set_dude_script)() = nullptr;
LONG(*fall_script_clear_dude_script)() = nullptr;


LONG(*fall_scripts_initiate)() = nullptr;
LONG(*fall_scripts_reset)() = nullptr;
LONG(*fall_scripts_exit)() = nullptr;

LONG(*fall_scripts_game_initiate)() = nullptr;
LONG(*fall_scripts_game_reset)() = nullptr;
LONG(*fall_scripts_game_clear)() = nullptr;


void(*fall_scripts_free_msglist)() = nullptr;

void* pfall_script_get = nullptr;

void* pfall_script_get_path = nullptr;

LONG(*fall_script_proccessing_enable)() = nullptr;
void(*fall_script_proccessing_disable)() = nullptr;
void(*fall_script_remove_all)() = nullptr;

void* pfall_script_remove = nullptr;

void* pfall_set_map_script = nullptr;

void* pfall_get_num_active_scripts = nullptr;

void* pfall_script_find_str_run_info = nullptr;

void* pfall_script_new = nullptr;

void* pfall_script_find_first_at = nullptr;
void* pfall_script_find_next_at = nullptr;


//__________________________________________________
SCRIPT_STRUCT* fall_Script_Find_First_At(LONG level) {

    SCRIPT_STRUCT* retVal = nullptr;
    __asm {
        mov eax, level
        call pfall_script_find_first_at
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________
SCRIPT_STRUCT* fall_Script_Find_Next_At(LONG level) {

    SCRIPT_STRUCT* retVal = nullptr;
    __asm {
        call pfall_script_find_next_at
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________
LONG fall_Script_New(DWORD* p_scriptID, LONG script_type) {

    LONG retVal = -1;
    __asm {

        mov edx, script_type
        mov eax, p_scriptID
        call pfall_script_new
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________________
LONG fall_Script_Find_Str_Run_Info(LONG level, DWORD* p_scr_50, DWORD scriptID) {

    LONG retVal = -1;
    __asm {
        mov ebx, scriptID
        mov edx, p_scr_50
        mov eax, level
        call pfall_script_find_str_run_info
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
LONG fall_Get_Number_Of_Active_Scripts(LONG scriptType) {

    LONG retVal = 0;
    __asm {
        mov eax, scriptType
        call pfall_get_num_active_scripts
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
LONG fall_Set_Map_Script(LONG scriptIndex) {

    LONG retVal = -1;
    __asm {
        mov eax, scriptIndex
        call pfall_set_map_script
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________
LONG fall_Script_Remove(DWORD scriptID) {

    LONG retVal = -1;
    __asm {
        mov eax, scriptID
        call pfall_script_remove
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________
void fall_GetScriptPath(char* ret_path) {

    __asm {
        mov eax, ret_path
        call pfall_script_get_path
    }
    return;
}


//_________________________________________________________
LONG fall_Script_Get(DWORD scriptID, SCRIPT_STRUCT** lpScr) {

    LONG retVal = -1;
    __asm {
        mov edx, lpScr
        mov eax, scriptID
        call pfall_script_get
        mov retVal, eax
    }
    return retVal;
}


//____________________________________
void Fallout_Functions_Setup_Scripts() {

    if (fallout_exe_region == EXE_Region::Chinese) {

        pfall_script_get = (void*)0x4A4B34;

        fall_script_set_dude_script = (LONG(*)())0x4A3C90;
        fall_script_clear_dude_script = (LONG(*)())0x4A3D44;

        fall_scripts_initiate = (LONG(*)())0x4A3DA8;
        fall_scripts_reset = (LONG(*)())0x4A3E20;
        fall_scripts_exit = (LONG(*)())0x4A3F74;

        fall_scripts_game_initiate = (LONG(*)())0x4A3E38;
        fall_scripts_game_reset = (LONG(*)())0x4A3F40;
        fall_scripts_game_clear = (LONG(*)())0x4A405C;

        fall_scripts_free_msglist = (void (*)())0x4A3FF4;
        
        //To_Do pfall_script_get_path = (void*)0x;

        //To_Do fall_script_proccessing_enable = (LONG(*)())0x;
        //To_Do fall_script_proccessing_disable = (void (*)())0x;
        //To_Do fall_script_remove_all = (void (*)())0x);
    }
    else {

        pfall_script_get = (void*)FixAddress(0x4A5E34);

        fall_script_set_dude_script = (LONG(*)())FixAddress(0x4A4F90);
        fall_script_clear_dude_script = (LONG(*)())FixAddress(0x4A5044);

        fall_scripts_initiate = (LONG(*)())FixAddress(0x4A50A8);
        fall_scripts_reset = (LONG(*)())FixAddress(0x4A5120);
        fall_scripts_exit = (LONG(*)())FixAddress(0x4A5274);

        fall_scripts_game_initiate = (LONG(*)())FixAddress(0x4A5138);
        fall_scripts_game_reset = (LONG(*)())FixAddress(0x4A5240);
        fall_scripts_game_clear = (LONG(*)())FixAddress(0x4A535C);

        fall_scripts_free_msglist = (void (*)())FixAddress(0x4A52F4);

        pfall_script_get_path = (void*)FixAddress(0x4A47BC);

        fall_script_proccessing_enable = (LONG(*)())FixAddress(0x4A53A8);
        fall_script_proccessing_disable = (void (*)())FixAddress(0x4A53D0);
        fall_script_remove_all = (void (*)())FixAddress(0x4A63E0);

        pfall_script_remove = (void*)FixAddress(0x4A61D4);

        pfall_set_map_script = (void*)FixAddress(0x482360);

        pfall_get_num_active_scripts = (void*)FixAddress(0x4A672C);

        pfall_script_find_str_run_info = (void*)FixAddress(0x4A4F28);

        pfall_script_new = (void*)FixAddress(0x4A5F28);


        pfall_script_find_first_at = (void*)FixAddress(0x4A6524);
        pfall_script_find_next_at = (void*)FixAddress(0x4A6564);
    }

}
