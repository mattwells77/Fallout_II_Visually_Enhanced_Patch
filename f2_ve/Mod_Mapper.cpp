
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

#include "win_fall.h"
#include "configTools.h"

#include "game_map.h"
#include "memwrite.h"
#include "Fall_GameMap.h"
#include "Fall_Objects.h"
#include "Fall_General.h"
#include "Fall_File.h"
#include "Fall_Scripts.h"


#include "Dx_Game.h"

#include "mapper\win_Mapper.h"
#include "mapper\mapper_tools.h"
#include "mapper\win_ProtoBrowser.h"
#include "mapper\win_ToolBar.h"

const char exeTitle[] = "VE Mapper";



//________________________________________________
LONG MapperMain(LONG pathLength, const char* path) {

    if (fall_Fallout_Initiate(exeTitle, TRUE, 4, 0, pathLength, path) == -1)
        return 0;
    isMapperInitiated = true;

    fall_map_init();


    fall_scripts_game_clear();
    fall_script_clear_dude_script();

    fall_EnableGameEvents();


    *pDRAW_VIEW_FLAG = 1;


    fall_Mouse_SetImage(0);
    fall_Mouse_Show();

    SetMapperMouse(TRUE);

    ProtoList_Refresh();

    fall_Palette_Set(pLOADED_PAL);

    Set_Mapper_Ambient_Light_Intensity(ConfigReadInt(L"MAPPER", L"AMBIENT_LIGHT_INTENSITY", 0x010000));
 
    fall_Map_SetAmbientLightIntensity(Get_Mapper_Ambient_Light_Intensity(), 1);

    fall_map_reset();

    GameAreas_Load_Default();
 ;
    GameAreas_Load_Mapper();

    MapLight_Release_RenderTargets();

    GameAreas_SetScale();

    StatusBar_Print_Map_Vars();
    StatusBar_Print_Map_Script_Name();

    LONG button = -1;
    while (!isMapperExiting) {
        button = fall_Get_Input();

       // if(button == 0x1B)
      //      isMapperExiting = true;
    }
    Fallout_Debug_Info("MapperMain Exiting");
    Mapper_On_Exit();
    Fallout_On_Exit();

    fall_map_exit();
    fall_Background_Sound_Stop();
    fall_game_exit();
    Fallout_Debug_Info("MapperMain Exit");
    return 1;
}


//__________________________________
void __declspec(naked) mapper_main() {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call MapperMain
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//________________________________________________
void __declspec(naked) check_mapper_messages(void) {

    __asm {
        call CheckMessages
        add esp, 0x20
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }

}


//_______________________________________
void Set_Map_Level_Tool_State(LONG level) {
    Tool_SetState_Level(*phWinMain, level);
}
//________________________________________
void __declspec(naked) set_map_level(void) {

    __asm {
        mov ebx, pMAP_LEVEL
        mov dword ptr ds : [ebx] , ecx
        pushad
        push ecx
        call Set_Map_Level_Tool_State
        add esp, 0x4
        popad
        ret
    }
}


//________________________________________________________________________________
LONG Copy_Inv_Obj_Fix(OBJStruct* pObj_Owner, OBJStruct* pObj_Item, LONG num_items) {
    LONG retVal = fall_Obj_Inventory_AddItems(pObj_Owner, pObj_Item, num_items);
    if (retVal != -1)
        fall_Obj_Disconnect_From_Map(pObj_Item, nullptr);

    return retVal;
}

//___________________________________________
void __declspec(naked) copy_inv_obj_fix(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call Copy_Inv_Obj_Fix
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}

//_________________________
void Modifications_Mapper() {

    if (fallout_exe_region != EXE_Region::USA)
        return;

    FuncWrite32(0x4DE7CC, 0xFFFA21CC, (DWORD)&mapper_main);


    ///on load-map
    ///prevent saving previous map
    MemWrite8(0x482B96, 0xE8, 0x90);
    MemWrite32(0x482B97, 0x10FD, 0x90909090);

    ///prevent wind sound effect from playing
    //MemWrite8(0x482BA0, 0xE8, 0x90);
    //MemWrite32(0x482BA1, 0xFFFCDAD7, 0x90909090);

    ///prevent iface init
    MemWrite8(0x483073, 0xE8, 0x90);
    MemWrite32(0x483074, 0xFFFDB998, 0x90909090);


    //If CPU_USAGE_FIX is enabled then this is already set in "winfall.cpp". This needs to be enabled for mapper keys to function, so set it here only if CPU_USAGE_FIX is disabled.
    if (ConfigReadInt(L"OTHER_SETTINGS", L"CPU_USAGE_FIX", 0) == 0) {
        MemWrite8(0x4C9DA9, 0x53, 0xE9);
        FuncWrite32(0x4C9DAA, 0x8D535353, (DWORD)&check_mapper_messages);
    }

    MemWrite16(0x4821AC, 0x0D89, 0xE890);
    FuncWrite32(0x4821AE, 0x519578, (DWORD)&set_map_level);


    //
    //copy object function memory leak fix - copied items need to be disconnected from the map after being successfully added to copied objects inventory.
    FuncReplace32(0x489E52, 0xFFFED462, (DWORD)&copy_inv_obj_fix);
}
