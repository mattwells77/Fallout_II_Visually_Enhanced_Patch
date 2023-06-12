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
#include "Fall_Objects.h"
#include "memwrite.h"


struct PROlist {
   LONG size;
   LONG unknown1;
   LONG unknown2;
   LONG unknown3;
};

OBJStruct** ppObj_PC = nullptr;
OBJStruct** ppObj_ActiveCritter = nullptr;
OBJStruct** ppObj_DialogFocus = nullptr;
OBJStruct** ppObj_Egg = nullptr;
OBJStruct** ppObj_Selected = nullptr;
OBJStruct** ppObj_Mouse = nullptr;


PC_MAP_STATE* p_last_pc_map_state = nullptr;

PROlist* p_proto_list;

LONG* p_num_ai_packets = nullptr;

void* pfall_get_pro = nullptr;

void* pfall_get_obj_under_mouse = nullptr;
void* pfall_obj_clear_animation = nullptr;

void* pfall_obj_create = nullptr;
void* pfall_obj_move = nullptr;
void* pfall_obj_destroy = nullptr;

void* pfall_obj_set_frmid = nullptr;

void* pfall_obj_add_objnode_to_maplist = nullptr;

void* pfall_obj_get_new_objid = nullptr;

void* pfall_obj_get_obj_name = nullptr;


void* pfall_obj_critter_get_ai_packet_name = nullptr;

void* pfall_obj_check_frm_at_pos = nullptr;
void* pfall_obj_place_onto_map = nullptr;
void* pfall_obj_get_rect = nullptr;

void* pfall_obj_get_held_item_mp_cost = nullptr;
void* pfall_obj_get_held_item_mp_cost_weapon_check = nullptr;
void* pfall_obj_get_held_item_animation = nullptr;

void* pfall_ai_get_disposition = nullptr;
void* pfall_ai_get_burst_value = nullptr;
void* pfall_ai_get_run_away_value = nullptr;
void* pfall_ai_get_weapon_pref_value = nullptr;
void* pfall_ai_get_distance_value = nullptr;
void* pfall_ai_get_attack_who_value = nullptr;
void* pfall_ai_get_chem_use_value = nullptr;

void* pfall_ai_set_burst_value = nullptr;
void* pfall_ai_set_run_away_value = nullptr;
void* pfall_ai_set_weapon_pref_value = nullptr;
void* pfall_ai_set_distance_value = nullptr;
void* pfall_ai_set_attack_who_value = nullptr;
void* pfall_ai_set_chem_use_value = nullptr;

void* pfall_obj_get_inv_item_held_slot_1 = nullptr;
void* pfall_obj_get_inv_item_held_slot_2 = nullptr;
void* pfall_obj_get_inv_item_wearing = nullptr;

void* pfall_obj_get_pro_name = nullptr;

void* pfall_obj_get_name = nullptr;

void* pfall_obj_get_skill_points = nullptr;

void* pfall_skill_get_name = nullptr;

void* pfall_obj_inv_get_total_weight = nullptr;

void* pfall_obj_inv_get_total_cost = nullptr;

void* pfall_obj_get_stat = nullptr;

void* pfall_obj_attempt_weapon_reload = nullptr;

DWORD* p_combat_state_flags = nullptr;

void* pfall_obj_get_total_caps = nullptr;

void* pfall_obj_inv_add_items = nullptr;

void* pfall_obj_inv_remove_items = nullptr;

void* pfall_obj_item_get_max_ammo = nullptr;

void* pfall_obj_item_get_current_ammo = nullptr;

void* pfall_obj_item_get_type = nullptr;

void* pfall_obj_get_name_from_script = nullptr;

void* pfall_obj_critter_get_weapon_get_range = nullptr;

void* pfall_obj_item_weapon_get_damage = nullptr;

void* pfall_obj_item_weapon_get_sub_type = nullptr;

void* pfall_obj_item_weapon_ammo_pid = nullptr;

void* pfall_get_pro_name = nullptr;

void* pfall_obj_examine = nullptr;

void* pfall_obj_item_get_weight = nullptr;

void* pfall_obj_get_script_id = nullptr;

void* pfall_obj_new_script_instance = nullptr;

void* pfall_obj_new_scriptID = nullptr;

void* pfall_obj_copy = nullptr;

void* pfall_obj_clear_inv = nullptr;

void* pfall_obj_disconnect_from_map = nullptr;
void* pfall_obj_connect_to_map = nullptr;


void* pfall_obj_critter_adjust_ac = nullptr;

void* pfall_obj_item_can_weapon_unload = nullptr;
void* pfall_obj_item_weapon_unload = nullptr;


void* pfall_obj_item_can_weapon_load = nullptr;
void* pfall_obj_item_weapon_load = nullptr;


void* pfall_obj_destroy_obj = nullptr;

void* pfall_obj_item_set_current_ammo = nullptr;


void* pfall_obj_item_weapon_anim_code = nullptr;

void* pfall_obj_toggle_flat = nullptr;

//________________________________________________________
LONG fall_Obj_Toggle_Flat(OBJStruct* pObj, RECT* p_rc_ret) {
    LONG retVal = -1;
    __asm {
        mov edx, p_rc_ret
        mov eax, pObj
        call pfall_obj_toggle_flat
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________
DWORD fall_Obj_Weapon_AnimationCode(OBJStruct* pObj_weapon) {
    DWORD retVal = 0;
    __asm {
        mov eax, pObj_weapon
        call pfall_obj_item_weapon_anim_code
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________
LONG fall_Obj_Item_SetCurrentAmmo(OBJStruct* pObj, LONG quantity) {
    LONG retVal = 0;
    __asm {
        mov edx, quantity
        mov eax, pObj
        call pfall_obj_item_set_current_ammo
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
LONG fall_Obj_Destroy_InvObj(OBJStruct* pObj) {
    LONG retVal = -1;
    __asm {
        mov eax, pObj
        call pfall_obj_destroy_obj
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________
BOOL fall_Obj_Weapon_Can_Load(OBJStruct* pObj_weapon, OBJStruct* pObj_ammo) {
    BOOL retVal = FALSE;
    __asm {
        mov edx, pObj_ammo
        mov eax, pObj_weapon
        call pfall_obj_item_can_weapon_load
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________
LONG fall_Obj_Weapon_Load(OBJStruct* pObj_weapon, OBJStruct* pObj_ammo) {
    LONG retVal = 0;
    __asm {
        mov edx, pObj_ammo
        mov eax, pObj_weapon
        call pfall_obj_item_weapon_load
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
BOOL fall_Obj_Weapon_Can_Unload(OBJStruct* pObj_weapon) {
    BOOL retVal = FALSE;
    __asm {
        mov eax, pObj_weapon
        call pfall_obj_item_can_weapon_unload
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________
OBJStruct* fall_Obj_Weapon_Unload(OBJStruct* pObj_weapon) {
    OBJStruct* retVal = nullptr;
    __asm {
        mov eax, pObj_weapon
        call pfall_obj_item_weapon_unload
        mov retVal, eax
    }
    return retVal;
}

//____________________________________________________________________________________________________________
LONG fall_Obj_Critter_Adjust_AC(OBJStruct* pObj_critter, OBJStruct* pObj_armor_old, OBJStruct* pObj_armor_new) {
    LONG retVal = -1;
    __asm {
        mov ebx, pObj_armor_new
        mov edx, pObj_armor_old
        mov eax, pObj_critter
        call pfall_obj_critter_adjust_ac
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________________
LONG fall_Obj_Disconnect_From_Map(OBJStruct* pObj, RECT* p_rc_ret) {
    LONG retVal = -1;
    __asm {
        mov edx, p_rc_ret
        mov eax, pObj
        call pfall_obj_disconnect_from_map
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________________________
LONG fall_Obj_Connect_To_Map(OBJStruct* pObj, LONG hex_num, LONG level, RECT* p_rc_ret) {
    LONG retVal = -1;
    __asm {
        mov ecx, p_rc_ret
        mov ebx, level
        mov edx, hex_num
        mov eax, pObj
        call pfall_obj_connect_to_map
        mov retVal, eax
    }
    return retVal;
}

//__________________________________________
LONG fall_Obj_Clear_Inventory(PUD* pObj_Pud) {
    LONG retVal = -1;
    __asm {
        mov eax, pObj_Pud
        call pfall_obj_clear_inv
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________________
LONG fall_Obj_Copy(OBJStruct** pObj_out, OBJStruct* pObj_to_copy) {
    LONG retVal = -1;
    __asm {

        mov edx, pObj_to_copy
        mov eax, pObj_out
        call pfall_obj_copy
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________
LONG fall_Obj_New_ScriptID(OBJStruct* pObj, DWORD* pret_scriptID) {

    LONG retVal = -1;
    __asm {

        mov edx, pret_scriptID
        mov eax, pObj
        call pfall_obj_new_scriptID
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________________________________________________
LONG fall_Obj_New_Script_Instance(OBJStruct* pObj, LONG script_type, LONG script_ref_number) {

    LONG retVal = -1;
    __asm {
        mov ebx, script_ref_number
        mov edx, script_type
        mov eax, pObj
        call pfall_obj_new_script_instance
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________
LONG fall_Obj_Item_GetWeight(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_item_get_weight
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________________________________
LONG fall_Obj_Examine(OBJStruct* pObj_holder, OBJStruct* pObj_Item, void (*p_func)(void)) {//pfunc has one argument, char* passed on EAX register.
    LONG retVal = 0;
    __asm {
        mov ebx, p_func
        mov edx, pObj_Item
        mov eax, pObj_holder
        call pfall_obj_examine
        mov retVal, eax
    }
    return retVal;
}


//________________________________________
char* fall_Get_Prototype_Name(DWORD proID) {
    char* retVal = 0;
    __asm {
        mov eax, proID
        call pfall_get_pro_name
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
LONG fall_Obj_Item_Weapon_GetAmmoProID(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_item_weapon_ammo_pid
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________
LONG fall_Obj_Item_Weapon_GetSubType(OBJStruct* pObj, LONG attackType) {
    LONG retVal = 0;
    __asm {
        mov edx, attackType
        mov eax, pObj
        call pfall_obj_item_weapon_get_sub_type
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________________________
LONG fall_Obj_Item_Weapon_GetDamage(OBJStruct* pObj, LONG* p_ret_min, LONG* p_ret_max) {
    LONG retVal = 0;
    __asm {
        mov ebx, p_ret_max
        mov edx, p_ret_min
        mov eax, pObj
        call pfall_obj_item_weapon_get_damage
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________________________________
LONG fall_Obj_Critter_GetWeaponRange(OBJStruct* pObj, LONG attackType) {
    LONG retVal = 0;
    __asm {
        mov edx, attackType
        mov eax, pObj
        call pfall_obj_critter_get_weapon_get_range
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
LONG fall_Obj_Item_GetType(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_item_get_type
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________
LONG fall_Obj_Item_GetMaxAmmo(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_item_get_max_ammo
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________
LONG fall_Obj_Item_GetCurrentAmmo(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_item_get_current_ammo
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________________________________________________________
LONG fall_Obj_Inventory_AddItems(OBJStruct* pObj_Owner, OBJStruct* pObj_Item, LONG num_items) {
    LONG retVal = 0;
    __asm {
        mov ebx, num_items
        mov edx, pObj_Item
        mov eax, pObj_Owner
        call pfall_obj_inv_add_items
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________________________________________________
LONG fall_Obj_Inventory_RemoveItems(OBJStruct* pObj_Owner, OBJStruct* pObj_Item, LONG num_items) {
    LONG retVal = 0;
    __asm {
        mov ebx, num_items
        mov edx, pObj_Item
        mov eax, pObj_Owner
        call pfall_obj_inv_remove_items
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
LONG fall_Obj_GetTotalCaps(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_total_caps
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________________
LONG fall_Obj_AttemptWeaponReload(OBJStruct* pObj, LONG flag_displayText) {
    LONG retVal = 0;
    __asm {
        mov edx, flag_displayText
        mov eax, pObj
        call pfall_obj_attempt_weapon_reload
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________
LONG Obj_GetSkillWithHighestPoints(OBJStruct* pObj) {
    if (!pObj)
        return 0;
    LONG type = pObj->proID >> 24;
    if (type != 1)
        return 0;
    LONG points = 0;
    LONG max_points = 0;
    LONG max_skill = 0;
    for (int i = 0; i < 12; i++) {
        points = fall_Obj_GetSkillPoints(pObj, i);
        if (points > max_points) {
            max_points = points;
            max_skill = i;
        }
    }
    return max_skill;
}


//__________________________________________________________
LONG fall_Obj_GetSkillPoints(OBJStruct* pObj, LONG skillNum) {
    LONG retVal = 0;
    __asm {
        mov edx, skillNum
        mov eax, pObj
        call pfall_obj_get_skill_points
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
char* fall_obj_GetSkillName(LONG skill_num) {
    char* retVal = 0;
    __asm {

        mov eax, skill_num
        call pfall_skill_get_name
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
LONG fall_Obj_Inventory_GetTotalWeight(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_inv_get_total_weight
        mov retVal, eax
    }
    return retVal;
}

//___________________________________________________
LONG fall_Obj_Inventory_GetTotalCost(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_inv_get_total_cost
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________
LONG fall_Obj_GetStat(OBJStruct* pObj, LONG statNum) {
    LONG retVal = 0;
    __asm {
        mov edx, statNum
        mov eax, pObj
        call pfall_obj_get_stat
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________
char* fall_obj_GetProtoName(OBJStruct* pObj) {
    char* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_pro_name
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________
char* fall_obj_GetName(OBJStruct* pObj) {
    char* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_name
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________
char* fall_obj_GetNameFromScript(OBJStruct* pObj) {
    char* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_name_from_script
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________________
OBJStruct* fall_obj_getInvItem_HeldInSlot_1(OBJStruct* pObj) {
    OBJStruct* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_inv_item_held_slot_1
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________
OBJStruct* fall_obj_getInvItem_HeldInSlot_2(OBJStruct* pObj) {
    OBJStruct* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_inv_item_held_slot_2
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________
OBJStruct* fall_obj_getInvItem_Wearing(OBJStruct* pObj) {
    OBJStruct* retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_get_inv_item_wearing
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________
LONG fall_AI_GetDisposition(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_disposition
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________
LONG fall_AI_GetBurstValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_burst_value
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________
LONG fall_AI_GetRunAwayValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_run_away_value
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________
LONG fall_AI_GetWeaponPrefValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_weapon_pref_value
        mov retVal, eax
    }
    return retVal;
}


//____________________________________________
LONG fall_AI_GetDistanceValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_distance_value
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________
LONG fall_AI_GetAttackWhoValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_attack_who_value
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________
LONG fall_AI_GetChemUseValue(OBJStruct* pObj) {
    LONG retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_ai_get_chem_use_value;
        mov retVal, eax
    }
    return retVal;
}


//___________________________________________________
LONG fall_AI_SetBurstValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_burst_value
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
LONG fall_AI_SetRunAwayValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_run_away_value
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________
LONG fall_AI_SetWeaponPrefValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_weapon_pref_value
        mov retVal, eax
    }
    return retVal;
}


//______________________________________________________
LONG fall_AI_SetDistanceValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_distance_value
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________
LONG fall_AI_SetAttackWhoValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_attack_who_value
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________
LONG fall_AI_SetChemUseValue(OBJStruct* pObj, LONG val) {
    LONG retVal = 0;
    __asm {
        mov edx, val
        mov eax, pObj
        call pfall_ai_set_chem_use_value;
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________________________
LONG fall_Obj_GetHeldItemAnimation(OBJStruct* pObj, LONG attackType) {
    LONG retVal = 0;
    __asm {
        mov edx, attackType
        mov eax, pObj
        call pfall_obj_get_held_item_animation
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________________________________________________________________
LONG fall_Obj_GetHeldItemMovementPointCost_WeaponCheck(OBJStruct* pObj, LONG attackType, LONG isCalledShot) {
    LONG retVal = 0;
    __asm {
        mov ebx, isCalledShot
        mov edx, attackType
        mov eax, pObj
        call pfall_obj_get_held_item_mp_cost_weapon_check
        mov retVal, eax
    }
    return retVal;
}


//_____________________________________________________________________________________________
LONG fall_Obj_GetHeldItemMovementPointCost(OBJStruct* pObj, LONG attackType, LONG isCalledShot) {
    LONG retVal = 0;
    __asm {
        mov ebx, isCalledShot
        mov edx, attackType
        mov eax, pObj
        call pfall_obj_get_held_item_mp_cost
        mov retVal, eax
    }
    return retVal;
}



//_________________________________________________________________
DWORD fall_Obj_CheckFrmAtPos(OBJStruct* pObj, LONG xPos, LONG yPos) {
    DWORD flags = 0;
    __asm {
        mov ebx, yPos
        mov edx, xPos
        mov eax, pObj
        call pfall_obj_check_frm_at_pos
        mov flags, eax
    }
    return flags;
}


//__________________________________________________________________________________
LONG fall_Obj_PlaceOntoMap(OBJStruct* pObj, LONG hexPos, LONG level, RECT* rcRetObj) {
    LONG retVal = 0;
    __asm {
        mov ecx, rcRetObj
        mov ebx, level
        mov edx, hexPos
        mov eax, pObj
        call pfall_obj_place_onto_map
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________
void fall_Obj_GetRect(OBJStruct* pObj, RECT* rcObj) {
    __asm {
        mov edx, rcObj
        mov eax, pObj
        call pfall_obj_get_rect
    }
}


//_____________________________
LONG fall_Obj_GetNumAIPackets() {
    return *p_num_ai_packets;
}


//______________________________________________________
char* fall_Obj_Critter_GetAIPacketName(LONG aiPacketNum) {
    char* aiPacketName = nullptr;
    __asm {
        mov eax, aiPacketNum
        call pfall_obj_critter_get_ai_packet_name
        mov aiPacketName, eax
    }
    return aiPacketName;
}


//_______________________________________
char* fall_Obj_GetObjName(OBJStruct* pObj) {
   char* objName=nullptr;
   __asm {
      mov eax, pObj
      call pfall_obj_get_obj_name
      mov objName, eax
   }
   return objName;
}


//_________________________
LONG fall_Obj_GetNewObjID() {
    LONG newID = 0;
    __asm {
        call pfall_obj_get_new_objid
        mov newID, eax
    }
    return newID;
}


//_________________________________________________
void fall_Obj_AddObjNodeToMapList(OBJNode* objNode) {
    __asm {
        mov eax, objNode
        call pfall_obj_add_objnode_to_maplist
    }
}


//_______________________________________________________________
LONG fall_Obj_SetFrmId(OBJStruct* pObj, DWORD frmID, RECT* rcOut) {
    int retVal = 0;
    __asm {
        mov ebx, rcOut
        mov edx, frmID
        mov eax, pObj
        call pfall_obj_set_frmid
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________________________
OBJStruct* fall_GetObjUnderMouse(int objType, DWORD flag, int level) {
    OBJStruct* p_ret_obj = nullptr;
    __asm {
        mov ebx, level
        mov edx, flag
        mov eax, objType
        call pfall_get_obj_under_mouse
        mov p_ret_obj, eax
    }
    return p_ret_obj;
}


//_______________________________
LONG GetProListSize(LONG proType) {
    return p_proto_list[proType].size;
}


//__________________________________________
LONG fall_GetPro(DWORD proID, PROTO** proto) {
    LONG retVal = 0;
    __asm {
        mov edx, proto
        mov eax, proID
        call pfall_get_pro
        mov retVal, eax
    }
    return retVal;
}


//________________________________________
DWORD GetProID(LONG proType, LONG listNum) {
    listNum++;
    //if(listNum <= fall_GetProListSize(proType))
    if (listNum >= GetProListSize(proType))
        return -1;
    proType = proType << 24;
    return proType | listNum;
}





//__________________________________________
int fall_Obj_ClearAnimation(OBJStruct* pObj) {

    int retVal = 0;
    __asm {
        mov eax, pObj
        call pfall_obj_clear_animation
        mov retVal, eax
    }
    return retVal;
}


//__________________________________________________________
LONG fall_Obj_GetScriptID(OBJStruct* pObj, DWORD* pScriptID) {
    LONG retVal = -1;
    __asm {
        mov edx, pScriptID
        mov eax, pObj
        call pfall_obj_get_script_id
        mov retVal, eax
    }
    return retVal;
}


//________________________________________________________________________
LONG fall_Obj_Move(OBJStruct* pObj, DWORD hexPos, LONG level, RECT* pRect) {
    LONG retVal = 0;
    __asm {
        mov ecx, pRect
        mov ebx, level
        mov edx, hexPos
        mov eax, pObj
        call pfall_obj_move
        mov retVal, eax
    }
    return retVal;
}


//_________________________________________________
LONG fall_Obj_Destroy(OBJStruct* pObj, RECT* pRect) {
    LONG retVal = 0;
    __asm {
        mov edx, pRect
        mov eax, pObj
        call pfall_obj_destroy
        mov retVal, eax
    }
    return retVal;
}


//_______________________________________________________________
LONG fall_Obj_Create(OBJStruct** lpObj, DWORD frmID, DWORD proID) {
    LONG retVal = -1;
    __asm {
        mov ebx, proID
        mov edx, frmID
        mov eax, lpObj
        call pfall_obj_create
        mov retVal, eax
    }
    return retVal;
}



//____________________________________
void Fallout_Functions_Setup_Objects() {


    if (fallout_exe_region == EXE_Region::Chinese) {

        ppObj_PC = (OBJStruct**)0x671638;
        ppObj_ActiveCritter = (OBJStruct**)0x57D2B0;
        ppObj_DialogFocus = (OBJStruct**)0x528638;
        ppObj_Egg = (OBJStruct**)0x67162C;
        ppObj_Selected = (OBJStruct**)0x5A71EC;
        ppObj_Mouse = (OBJStruct**)0x5A71F0;

        pfall_obj_get_script_id = (void*)0x4996A0;

        pfall_get_pro = (void*)0x4A0E08;

        pfall_get_obj_under_mouse = (void*)0x44C614;

        p_proto_list = (PROlist*)0x52C08C;
        
        p_combat_state_flags = (DWORD*)0x520734;
        
        //To-Do p_num_ai_packets = (LONG*)0x;
       
        //To-Do p_last_pc_map_state = (PC_MAP_STATE*)0x;

        pfall_obj_clear_animation = (void*)0x413C4C;
        pfall_obj_create = (void*)0x488E84;
        pfall_obj_move = (void*)0x489968;
        pfall_obj_destroy = (void*)0x48A4FC;
        pfall_obj_set_frmid = (void*)0x489E3C;
        pfall_obj_place_onto_map = (void*)0x48CF28;
        pfall_obj_get_rect = (void*)0x48AA6C;
        pfall_obj_check_frm_at_pos = (void*)0x48B740;

/* To-Do
        pfall_obj_add_objnode_to_maplist = (void*);

        pfall_obj_get_new_objid = (void*);

        pfall_obj_get_obj_name = (void*);

        pfall_obj_critter_get_ai_packet_name = (void*);


        pfall_obj_get_held_item_mp_cost = (void*);
        pfall_obj_get_held_item_mp_cost_weapon_check = (void*);
        pfall_obj_get_held_item_animation = (void*);

        pfall_ai_get_disposition = (void*);

        pfall_ai_get_burst_value = (void*);
        pfall_ai_get_run_away_value = (void*);
        pfall_ai_get_weapon_pref_value = (void*);
        pfall_ai_get_distance_value = (void*);
        pfall_ai_get_attack_who_value = (void*);
        pfall_ai_get_chem_use_value = (void*);

        pfall_ai_set_burst_value = (void*);
        pfall_ai_set_run_away_value = (void*);
        pfall_ai_set_weapon_pref_value = (void*);
        pfall_ai_set_distance_value = (void*);
        pfall_ai_set_attack_who_value = (void*);
        pfall_ai_set_chem_use_value = (void*);

        pfall_obj_get_inv_item_held_slot_2 = (void*);
        pfall_obj_get_inv_item_held_slot_1 = (void*);
        pfall_obj_get_inv_item_wearing = (void*);

        pfall_obj_get_pro_name = (void*);


        pfall_obj_get_name = (void*);

        pfall_obj_get_skill_points = (void*);

        pfall_skill_get_name = (void*);

        pfall_obj_inv_get_total_weight = (void*);
        pfall_obj_inv_get_total_cost = (void*);

        pfall_obj_get_stat = (void*);

        pfall_obj_attempt_weapon_reload = (void*);

        pfall_obj_get_total_caps = (void*);

        pfall_obj_inv_remove_items = (void*);

        pfall_obj_item_get_max_ammo = (void*);
        pfall_obj_item_get_current_ammo = (void*);
        pfall_obj_item_get_type = (void*);

        pfall_obj_get_name_from_script = (void*);

        pfall_obj_critter_get_weapon_get_range = (void*);

        pfall_obj_item_weapon_get_damage = (void*);
        pfall_obj_item_weapon_get_sub_type = (void*);
        pfall_obj_item_weapon_ammo_pid = (void*);

        pfall_get_pro_name = (void*);

        pfall_obj_examine = (void*);

        pfall_obj_item_get_weight = (void*);
*/


    }
    else {

        ppObj_PC = (OBJStruct**)FixAddress(0x6610B8);
        ppObj_ActiveCritter = (OBJStruct**)FixAddress(0x56D2B0);
        ppObj_DialogFocus = (OBJStruct**)FixAddress(0x518848);
        ppObj_Egg = (OBJStruct**)FixAddress(0x6610AC);
        ppObj_Selected = (OBJStruct**)FixAddress(0x596C6C);
        ppObj_Mouse = (OBJStruct**)FixAddress(0x596C70);

        pfall_obj_get_script_id = (void*)FixAddress(0x49A9A0);

        pfall_get_pro = (void*)FixAddress(0x4A2108);

        pfall_get_obj_under_mouse = (void*)FixAddress(0x44CEC4);

        p_proto_list = (PROlist*)FixAddress(0x51C29C);
        
        p_combat_state_flags = (DWORD*)FixAddress(0x510944);
        
        p_num_ai_packets = (LONG*)FixAddress(0x518060);

        p_last_pc_map_state = (PC_MAP_STATE*)FixAddress(0x631D28);

        pfall_obj_clear_animation = (void*)FixAddress(0x413C4C);
        pfall_obj_create = (void*)FixAddress(0x489A84);
        pfall_obj_move = (void*)FixAddress(0x48A568);
        pfall_obj_destroy = (void*)FixAddress(0x48B0FC);
        pfall_obj_set_frmid = (void*)FixAddress(0x48AA3C);
        pfall_obj_place_onto_map = (void*)FixAddress(0x48DB28);
        pfall_obj_get_rect = (void*)FixAddress(0x48B66C);
        pfall_obj_check_frm_at_pos = (void*)FixAddress(0x48C340);

        pfall_obj_add_objnode_to_maplist = (void*)FixAddress(0x48D8E8);

        pfall_obj_get_new_objid = (void*)FixAddress(0x4A386C);

        pfall_obj_get_obj_name = (void*)FixAddress(0x48C8E4);

        pfall_obj_critter_get_ai_packet_name = (void*)FixAddress(0x428060);


        pfall_obj_get_held_item_mp_cost = (void*)FixAddress(0x478B24);
        pfall_obj_get_held_item_mp_cost_weapon_check = (void*)FixAddress(0x478040);
        pfall_obj_get_held_item_animation = (void*)FixAddress(0x4785DC);

        pfall_ai_get_disposition = (void*)FixAddress(0x428340);

        pfall_ai_get_burst_value = (void*)FixAddress(0x428184);
        pfall_ai_get_run_away_value = (void*)FixAddress(0x428190);
        pfall_ai_get_weapon_pref_value = (void*)FixAddress(0x4281FC);
        pfall_ai_get_distance_value = (void*)FixAddress(0x428208);
        pfall_ai_get_attack_who_value = (void*)FixAddress(0x428214);
        pfall_ai_get_chem_use_value = (void*)FixAddress(0x428220);

        pfall_ai_set_burst_value = (void*)FixAddress(0x42822C);
        pfall_ai_set_run_away_value = (void*)FixAddress(0x428248);
        pfall_ai_set_weapon_pref_value = (void*)FixAddress(0x4282D0);
        pfall_ai_set_distance_value = (void*)FixAddress(0x4282EC);
        pfall_ai_set_attack_who_value = (void*)FixAddress(0x428308);
        pfall_ai_set_chem_use_value = (void*)FixAddress(0x428324);

        pfall_obj_get_inv_item_held_slot_2 = (void*)FixAddress(0x471B70);
        pfall_obj_get_inv_item_held_slot_1 = (void*)FixAddress(0x471BBC);
        pfall_obj_get_inv_item_wearing = (void*)FixAddress(0x471C08);

        pfall_obj_get_pro_name = (void*)FixAddress(0x477AE4);


        pfall_obj_get_name = (void*)FixAddress(0x48C8E4);

        pfall_obj_get_skill_points = (void*)FixAddress(0x4AA558);

        pfall_skill_get_name = (void*)FixAddress(0x4AAB9C);

        pfall_obj_inv_get_total_weight = (void*)FixAddress(0x477E98);
        pfall_obj_inv_get_total_cost = (void*)FixAddress(0x477DAC);

        pfall_obj_get_stat = (void*)FixAddress(0x4AEF48);

        pfall_obj_attempt_weapon_reload = (void*)FixAddress(0x42AECC);

        pfall_obj_get_total_caps = (void*)FixAddress(0x47A6A8);

        
        pfall_obj_inv_add_items = (void*)FixAddress(0x4772B8);
        pfall_obj_inv_remove_items = (void*)FixAddress(0x477490);

        pfall_obj_item_get_max_ammo = (void*)FixAddress(0x478674);
        pfall_obj_item_get_current_ammo = (void*)FixAddress(0x4786A0);
        pfall_obj_item_get_type = (void*)FixAddress(0x477AFC);

        pfall_obj_get_name_from_script = (void*)FixAddress(0x42D0A8);

        pfall_obj_critter_get_weapon_get_range = (void*)FixAddress(0x478A1C);

        pfall_obj_item_weapon_get_damage = (void*)FixAddress(0x4783B8);
        pfall_obj_item_weapon_get_sub_type = (void*)FixAddress(0x478280);
        pfall_obj_item_weapon_ammo_pid = (void*)FixAddress(0x478DF8);

        pfall_get_pro_name = (void*)FixAddress(0x49EAFC);

        pfall_obj_examine = (void*)FixAddress(0x49AD88);

        pfall_obj_item_get_weight = (void*)FixAddress(0x477B88);

        pfall_obj_new_script_instance = (void*)FixAddress(0x49AAC0);

        pfall_obj_new_scriptID = (void*)FixAddress(0x49A9B4);

        pfall_obj_copy = (void*)FixAddress(0x489CCC);

        pfall_obj_clear_inv = (void*)FixAddress(0x48B1B0);

        pfall_obj_disconnect_from_map = (void*)FixAddress(0x489F34);
        pfall_obj_connect_to_map = (void*)FixAddress(0x489EC4);


        //00478874 / $  53            PUSH EBX; fallout2.item_w_can_reload_(EAX pObj_weapon, EDX pObj_ammo ? )(guessed void)
        //00478918 / $  53            PUSH EBX; fallout2.ITEM_W_RELOAD(EAX pOBJ_weapon, EDX pObj_ammo ? )(guessed void)
        pfall_obj_item_can_weapon_load = (void*)FixAddress(0x478874);
        pfall_obj_item_weapon_load = (void*)FixAddress(0x478918);


        pfall_obj_item_can_weapon_unload = (void*)FixAddress(0x478EF4);
        pfall_obj_item_weapon_unload = (void*)FixAddress(0x478F80);

        pfall_obj_item_weapon_anim_code = (void*)FixAddress(0x478DA8);
        //
        // 
        //0047650C / $  56            PUSH ESI; fallout2.DROP_AMMO_INTO_WEAPON(EAX pObj_weapon, EDX pObj_ammo ? , EBX, ECX, Arg1)(guessed Arg1)
 
        //004715F8 / $  51            PUSH ECX; fallout2.ADJUST_CRITTER_AC(EAX pObj_critter, EDX pObj_armor_old, EBX pObj_armor_new)(guessed void)
        pfall_obj_critter_adjust_ac = (void*)FixAddress(0x4715F8);

        //0049B9A0 / $ \53            PUSH EBX; fallout2.OBJ_DESTROY_(EAX pObj)(guessed void)
        pfall_obj_destroy_obj = (void*)FixAddress(0x49B9A0);


        //00478714 / $  53            PUSH EBX; fallout2.item_w_set_curr_ammo_(EAX pObj_weapon, EDX quantity ? )(guessed void)
        pfall_obj_item_set_current_ammo = (void*)FixAddress(0x478714);


        //0048AF2C / $  53            PUSH EBX; fallout2.obj_toggle_flat_(EAX* pObj, EDX* p_rect)(guessed void)
        pfall_obj_toggle_flat = (void*)FixAddress(0x48AF2C);

    }

}
