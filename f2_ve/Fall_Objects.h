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

struct DAMAGEstats {
	DWORD normal;//0x04
	DWORD laser;//0x08
	DWORD fire;//0x0C
	DWORD plasma;//0x10
	DWORD electrical;//0x14
	DWORD emp;//0x18
	DWORD explosive;//0x1C
};


struct PRIMEStats {
	DWORD Strength;//0x24        0
	DWORD Perception;//0x28      1
	DWORD Endurance;//0x2C       2
	DWORD Charisma;//0x30        3
	DWORD Intelligence;//0x34    4
	DWORD Agility;//0x38         5
	DWORD Luck;//0x3C            6
};


struct SECONDStats {
	DWORD HitPoints;//0x40               7
	DWORD ActionPoints;//0x44            8
	DWORD ArmorClass;//0x48              9
	DWORD Unused4C;//0x4C                A
	DWORD MeleeDamage;//0x50             B
	DWORD CarryWeight;//0x54             C
	DWORD Sequence;//0x58                D
	DWORD HealingRate;//0x5C             E
	DWORD CriticalChance;//0x60          F
	DWORD CriticalHitModifier;//0x64     10
	DAMAGEstats damageDT;//
	//DWORD DTnormal;//0x68 //DT section 11
	//DWORD DTlaser;//0x6C               12
	//DWORD DTfire;//0x70                13
	//DWORD DTplasma;//0x74              14
	//DWORD DTelectrical;//0x78          15
	//DWORD DTemp;//0x7C                 16
	//DWORD DTexplosive;//0x80           17
	DAMAGEstats damageDR;
	//DWORD DRnormal;//0x84 //DR section 18
	//DWORD DRlaser;//0x88               19
	//DWORD DRfire;//0x8C                1A
	//DWORD DRplasma;//0x90              1B
	//DWORD DRelectrical;//0x94          1C
	//DWORD DRemp;//0x98                 1D
	//DWORD DRexplosive;//0x9C           1E
	DWORD RadiationResistance;//0xA0     1F
	DWORD PoisonResistance;//0xA4        20
	DWORD Age;//0xA8                     21
	DWORD Gender;//0xAC                  22
};


struct SKILLS {
	DWORD smallGuns;//0x13C
	DWORD bigGuns;//0x140
	DWORD energyWeapons;//0x144
	DWORD unarmed;//0x148
	DWORD melee;//0x14C
	DWORD throwing;//0x150
	DWORD firstAid;//0x154
	DWORD doctor;//0x158
	DWORD sneak;//0x15C
	DWORD lockpick;//0x160
	DWORD steal;//0x164
	DWORD traps;//0x168
	DWORD science;//0x16C
	DWORD repair;//0x170
	DWORD speech;//0x174
	DWORD barter;//0x178
	DWORD gambling;//0x17C
	DWORD outdoorsman;//0x180
};


struct ITEMTYPEdata {
	//		                  //0 armor           1 Containers    2 Drugs
	DWORD data01;//0x24       AC                  MaxSize
	DWORD data02;//0x28       DR DAMAGEstats      OpenFlags
	DWORD data03;//0x2C       --                  //
	DWORD data04;//0x30       --                  //
	DWORD data05;//0x34       --                  //
	DWORD data06;//0x38       --                  //
	DWORD data07;//0x3C       DT DAMAGEstats      //
	DWORD data08;//0x40       --                  //
	DWORD data09;//0x44       --                  //
	DWORD data10;//0x48       --                  //
	DWORD data11;//0x4C       --                  //
	DWORD data12;//0x50       --                  //
	DWORD data13;//0x54       --                  //
	DWORD data14;//0x58       Perk                //
	DWORD data15;//0x5C       MaleFID             //
	DWORD data16;//0x60       FemaleFID           //
	DWORD data17;//0x64       //
	DWORD data18;//0x68       //
};


struct PROTOitem {
	//DWORD proID;//0x00
	//DWORD txtID;//0x04
	//DWORD frmID;//0x08
	//DWORD light_radius;//0x0C
	//DWORD light_intensity;//0x10
	//DWORD flags;//0x14
	DWORD actionFlags;//0x18
	DWORD scriptID;//0x1C
	DWORD itemType;//0x20
	ITEMTYPEdata itemTypeData;//0x24
	DWORD materialID;//0x6C
	DWORD size;//0x70
	DWORD weight;//0x74
	DWORD cost;//0x78
	DWORD invFrmID;//0x7C
	BYTE soundID;//80
};


struct PROTOcritter {
	//DWORD proID;//0x00
	//DWORD txtID;//0x04
	//DWORD frmID;//0x08
	//DWORD light_radius;//0x0C
	//DWORD light_intensity;//0x10
	//DWORD flags;//0x14
	DWORD actionFlags;//0x18
	DWORD scriptID;//0x1C
	DWORD crittFlags;//0x20
	PRIMEStats primeStats;//0x24
	SECONDStats secondStats;//0x40
	PRIMEStats primeStatsBonus;//0xB0
	SECONDStats secondStatsBonus;//0xCC
	SKILLS skills;//0x13C
	DWORD bodyType;//0x184
	DWORD expVal;//0x188
	DWORD killType;//0x18C
	DWORD damageType;//0x190
	DWORD frmIDHead; //0x194
	DWORD aiPacket; //0x198
	DWORD teamNum; //0x19C
};


struct SCENERYTYPEdata {
	DWORD data01;//0x24  //generic == 0xCCCCCCCC, doors == 0x0, ladders & stairs == 0xFFFFFFFF
	DWORD data02;//0x28
};


struct PROTOscenery {
	//DWORD proID;//0x00
	//DWORD txtID;//0x04
	//DWORD frmID;//0x08
	//DWORD light_radius;//0x0C
	//DWORD light_intensity;//0x10
	//DWORD flags;//0x14
	DWORD actionFlags;//0x18
	DWORD scriptID;//0x1C
	DWORD sceneryType;//0x20
	SCENERYTYPEdata sceneryTypeData;//0x24
	DWORD materialID;//0x2C
	DWORD unknown; //0x30
	BYTE soundID;//0x34
};


struct PROTOwall {
	//DWORD proID;//0x00
	//DWORD txtID;//0x04
	//DWORD frmID;//0x08
	//DWORD light_radius;//0x0C
	//DWORD light_intensity;//0x10
	//DWORD flags;//0x14
	DWORD actionFlags;//0x18
	DWORD scriptID;//0x1C
	DWORD materialID;//0x20
};


struct PROTOtile { // same as misc
	//DWORD proID;//0x00
	//DWORD txtID;//0x04
	//DWORD frmID;//0x08
	//DWORD light_radius;//0x0C
	//DWORD light_intensity;//0x10
	//DWORD flags;//0x14
	DWORD materialID;//0x18  //misc = unknown
};

/*
union PROTO {
   PROTOitem item;
   PROTOcritter critter;
   PROTOscenery scenery;
   PROTOwall wall;
   PROTOtile tile;
   PROTOtile misc;
};
*/

struct PROTO {
	DWORD proID;//0x00
	DWORD txtID;//0x04
	DWORD frmID;//0x08
	DWORD light_radius;//0x0C
	DWORD light_intensity;//0x10
	DWORD flags;//0x14
	union {
		PROTOitem item;
		PROTOcritter critter;
		PROTOscenery scenery;
		PROTOwall wall;
		PROTOtile tile;
		PROTOtile misc;
	};

};


struct OBJStruct;

struct ITEMblock {
      OBJStruct *p_obj;
	  LONG num;
};


struct COMBAT_DATA {
   OBJStruct *who_hit_me;       //0x03C
   LONG currentAP;             //0x040
   LONG results;               //0x044
   LONG damage_last_turn;      //0x048
   LONG aiPacket;              //0x04C
   LONG teamNum;               //0x050
   DWORD flags;                //0x054
};


struct PUD_CRITTER {
   LONG inv_size;              //0x00 //0x02C
   LONG inv_max;               //0x04 //0x030
   ITEMblock* p_item;              //0x08 //0x034
   LONG reaction_to_pc;        //0x0C //0x038
   COMBAT_DATA combat_data;     //0x10 //0x03C
   LONG current_hp;            //0x2C //0x058
   LONG current_rad;           //0x30 //0x05C
   LONG current_poison;        //0x34 //0x060
};


struct PUD_WEAPON {
   LONG cur_ammo_quantity;
   LONG cur_ammo_type_pid;
};


struct PUD_AMMO {
   LONG cur_ammo_quantity;
   LONG none;
};


struct PUD_MISC_ITEM {
   LONG curr_charges;
   LONG none;
};


struct PUD_KEY_ITEM {
   LONG cur_key_code;
   LONG none;
};


struct PUD_PORTAL {
   LONG cur_open_flags;//flag 0x00000001 = is opened
   LONG none;
};


struct PUD_ELEVATOR {
   LONG elev_type;
   LONG elev_level;
};


struct PUD_STAIRS {
   LONG destMap;
   LONG destBuiltTile;
};


union PUD_EXT {
   PUD_WEAPON weapon;
   PUD_AMMO ammo;
   PUD_MISC_ITEM misc_item;
   PUD_KEY_ITEM key_item;
   PUD_PORTAL portal;
   PUD_ELEVATOR elevator;
   PUD_STAIRS stairs;
};


struct PUD_GENERAL {
   LONG inv_size;              //0x00 //0x02C
   LONG inv_max;               //0x04 //0x030
   ITEMblock* p_item;              //0x08 //0x034
   LONG updated_flags;         //0x0C //0x038
   PUD_EXT pud;                    //0x10 //0x03C
};


union PUD {
   PUD_CRITTER critter;
   PUD_GENERAL general;
};


struct OBJStruct {
	  DWORD objID;//0x00 //pc = PF00
	  LONG hexNum;//0x04
	  LONG xShift;//0x08
	  LONG yShift;//0x0C
	  LONG viewScrnX;//0x10
	  LONG viewScrnY;//0x14
	  DWORD frameNum;//0x18
	  LONG ori;//0x1C
	  DWORD frmID;//0x20
	  DWORD flags;//0x24
	  LONG level;//0x28
	  PUD pud;
	  DWORD proID;//0x64
	  DWORD cID;//0x68
	  DWORD light_radius;//0x6C //radius in hexes
	  LONG light_intensity;//0x70
	  DWORD combatFlags;//0x74
	  DWORD scriptID;//0x78
	  OBJStruct* pObj_owner;//0x7C //not read but written but set to 0 on load.
	  DWORD scriptIndex;//0x80
};


struct OBJNode {
      OBJStruct* pObj;
      OBJNode* next;
};


struct PC_MAP_STATE {
	DWORD mapID;
	LONG level;
	LONG hexNum;
	LONG ori;
};


///OBJStruct.flags-------
///ProtoStruct.flags-------
#define FLG_Disabled		0x00000001 //???
#define FLG_NonEffect		0x00000004 //Does not react with other objects, not saved to file. Used by mouse hex and spatial script markers in the mapper etc.
#define FLG_Flat			0x00000008
#define FLG_NoBlock			0x00000010
#define FLG_Emits_light		0x00000020

#define FLG_Unk00000400		0x00000400
#define FLG_MultiHex		0x00000800
#define FLG_NoHighlight		0x00001000
#define FLG_BeingUsed		0x00002000 //queued item
#define FLG_TransRed		0x00004000
#define FLG_TransNone		0x00008000
#define FLG_TransWall		0x00010000
#define FLG_TransGlass		0x00020000
#define FLG_TransSteam		0x00040000
#define FLG_TransEnergy		0x00080000
//items
#define FLG_IsHeldSlot1		0x01000000
#define FLG_IsHeldSlot2		0x02000000
#define FLG_IsWornArmor		0x04000000
#define FLG_Unk08000000		0x08000000 // related to shootThru flag

#define FLG_WallTransEnd	0x10000000
#define FLG_LightThru		0x20000000
#define FLG_MarkedByPC		0x40000000
#define FLG_ShootThru		0x80000000


#define FLG_TransAny       0x000FC000 // for checking if any trans flags set

///OBJStruct.flags-------



///OBJStruct.combatFlags-------
#define FLG_NonPCTeamMem        0x00000001 //red animated//enemy combatant?
#define FLG_MouseHex            0x00000002 //red
#define FLG_combatUnk0x04       0x00000004 //white
#define FLG_PCTeamMem           0x00000008 //green animated
#define FLG_ItemUnderMouse      0x00000010 //brighter yellow
#define FLG_NotVisByPC          0x00000020 //yellow
#define FLG_IsOutlined          0x00FFFFFF //check if any above flags set
#define FLG_IsPC                0x00000000
#define FLG_IsTransparent       0x40000000 //?
#define FLG_NonInteractive      0x80000000  //not interactive ?



///OBJStruct.PUD_CRITTER.COMBAT_DATA.flags-------
#define FLG_BarterYes       0x00000002// - Barter (can trade with)
#define FLG_StealNo         0x00000020// - Steal (cannot steal from)
#define FLG_DropItemsNo     0x00000040// - Drop (doesn't drop items)
#define FLG_LoseLimbsNo     0x00000080// - Limbs (can not lose limbs)
#define FLG_CorpseStays     0x00000100// - Ages (dead body does not disappear)
#define FLG_HealsNo         0x00000200// - Heal (damage is not cured with time)
#define FLG_Invulnerable    0x00000400// - Invulnerable (cannot be hurt)
#define FLG_CorpseNo        0x00000800// - Flatten (leaves no dead body)
#define FLG_SpecialDeath    0x00001000// - Special (there is a special type of death)
#define FLG_RangedMelee     0x00002000// - Range (melee attack is possible at a distance)
#define FLG_KnockedDownNo   0x00004000// - Knock (cannot be knocked down)



///PROTOall.actionFlags-----------------

/*
Extended Flags

Item Flags:

    0x08000000 - Hidden Item (invisible on map)

Action Flags:

    0x00001000 - Use On Smth (object can be used on something)
    0x00008000 - PickUp (object can be picked up)
    0x00000800 - Use (object can be used)

Weapon Flag:

    0x00000100 - Big Gun (weapon is Big Guns)
    0x00000200 - 2Hnd (Two-handed weapon)

Primary Attack Type:

    0x00000000 - stand
    0x00000001 - throw punch
    0x00000002 - kick leg
    0x00000003 - swing anim
    0x00000004 - thrust anim
    0x00000005 - throw anim
    0x00000006 - fire single
    0x00000007 - fire burst
    0x00000008 - fire continuous

Secondary Attack Type:

    0x00000000 - stand
    0x00000010 - throw punch
    0x00000020 - kick leg
    0x00000030 - swing anim
    0x00000040 - thrust anim
    0x00000050 - throw anim
    0x00000060 - fire single
    0x00000070 - fire burst
    0x00000080 - fire continuous
*/
//Weapon Flag:

#define FLG_Weapon_BigGun		0x00000100// - Big Gun(weapon is Big Guns)
#define FLG_Weapon_TwoHanded	0x00000200// - 2Hnd(Two - handed weapon)

#define FLG_UseOn           0x00001000// - Use On Smth (object can be used on something)
#define FLG_LookAt          0x00002000// - Look (The critter can be inspected)
#define FLG_TalkTo          0x00004000// - Talk (You can speak to the critter)
#define FLG_PickUp          0x00008000// - PickUp (object can be picked up)
#define FLG_Use             0x00000800// - Use (object can be used)
///wall lights
#define FLG_NorthSouth      0x00000000// - North / South
#define FLG_EastWest        0x08000000// - East / West
#define FLG_NorthCorner     0x10000000// - North Corner
#define FLG_SouthCorner     0x20000000// - South Corner
#define FLG_EastCorner      0x40000000// - East Corner
#define FLG_WestCorner      0x80000000// - West Corner


///PROTOscenery.sceneryType--------
#define FLG_Portal           0x00000000
#define FLG_Stairs           0x00000001
#define FLG_Elevators        0x00000002
#define FLG_LadderBottom     0x00000003
#define FLG_LadderTop        0x00000004
#define FLG_Generic          0x00000005


enum ITEM_TYPE : LONG {
	armor		= 0,
	container	= 1,
	drug		= 2,
	weapon		= 3,
	ammo		= 4,
	misc		= 5,
	key			= 6
};


enum ATTACK_TYPE : LONG {
	left_primary = 0,
	left_secondary = 1,
	right_primary = 2,
	right_secondary = 3,
	punch = 4,
	kick = 5,
	reload_left = 6,
	reload_right = 7,
	strong_punch = 8,
	hammer_punch = 9,
	hay_maker = 10,
	jab = 11,
	palm_strike = 12,
	piercing_strike = 13,
	strong_kick = 14,
	snap_kick = 15,
	power_kick = 16,
	hip_kick = 17,
	hook_kick = 18,
	piercing_kick = 19
};


enum ATTACK_SUB_TYPE : LONG {
	none = 0,
	unarmed = 1,
	melee = 2,
	throwing = 3,
	guns = 4
};


enum STAT : LONG {
	strenth = 0,
	perception = 1,
	endurance = 2,
	charisma = 3,
	intelligence = 4,
	agility = 5,
	luck = 6,
	max_hit_points = 7,
	max_move_points = 8,
	ac = 9,
	unused = 10,
	melee_dmg = 11,
	carry_amt = 12,
	sequence = 13,
	heal_rate = 14,
	crit_chance = 15,
	better_crit = 16,
	dmg_thresh = 17,
	dmg_thresh_laser = 18,
	dmg_thresh_fire = 19,
	dmg_thresh_plasma = 20,
	dmg_thresh_electrical = 21,
	dmg_thresh_emp = 22,
	dmg_thresh_explosion = 23,
	dmg_resist = 24,
	dmg_resist_laser = 25,
	dmg_resist_fire = 26,
	dmg_resist_plasma = 27,
	dmg_resist_electrical = 28,
	dmg_resist_emp = 29,
	dmg_resist_explosion = 30,
	rad_resist = 31,
	poison_resist = 32,
	age = 33,
	gender = 34,
	current_hp = 35,
	current_poison = 36,
	current_rad = 37,
	real_max_stat = 38
};


extern PC_MAP_STATE* p_last_pc_map_state;

extern DWORD* p_combat_state_flags;

extern OBJStruct **ppObj_PC;
extern OBJStruct **ppObj_ActiveCritter;
extern OBJStruct** ppObj_DialogFocus;
extern OBJStruct **ppObj_Selected;
extern OBJStruct **ppObj_Mouse;
extern OBJStruct **ppObj_Egg;


void Fallout_Functions_Setup_Objects();


LONG GetProListSize(LONG proType);

DWORD GetProID(LONG proType, LONG listNum);

LONG fall_GetPro(DWORD proID, PROTO **proto);

OBJStruct* fall_GetObjUnderMouse(int type, DWORD flag, int level);

LONG fall_Obj_Create(OBJStruct **lpObj, DWORD frmID, DWORD proID);
LONG fall_Obj_Move(OBJStruct* pObj, DWORD hexPos, LONG level, RECT *pRect);
LONG fall_Obj_Destroy(OBJStruct* pObj, RECT *pRect);

int fall_Obj_ClearAnimation(OBJStruct* pObj);

LONG fall_Obj_SetFrmId(OBJStruct* pObj, DWORD frmID, RECT *rcOut);

void fall_Obj_AddObjNodeToMapList(OBJNode *objNode);
LONG fall_Obj_GetNewObjID();
char* fall_Obj_GetObjName(OBJStruct* pObj);
char* fall_Obj_Critter_GetAIPacketName(LONG aiPacketNum);
LONG fall_Obj_GetNumAIPackets();

LONG fall_Obj_PlaceOntoMap(OBJStruct* pObj, LONG hexPos, LONG level, RECT *rcRetObj);
void fall_Obj_GetRect(OBJStruct* pObj, RECT *rcObj);
DWORD fall_Obj_CheckFrmAtPos( OBJStruct* pObj, LONG xPos, LONG yPos);

LONG fall_Obj_GetHeldItemMovementPointCost(OBJStruct *pObj, LONG attackType, LONG isCalledShot);
LONG fall_Obj_GetHeldItemMovementPointCost_WeaponCheck(OBJStruct *pObj, LONG attackType, LONG isCalledShot);
LONG fall_Obj_GetHeldItemAnimation(OBJStruct *pObj, LONG attackType);


LONG fall_AI_GetDisposition(OBJStruct* pObj);

LONG fall_AI_GetBurstValue(OBJStruct* pObj);
LONG fall_AI_GetRunAwayValue(OBJStruct* pObj);
LONG fall_AI_GetWeaponPrefValue(OBJStruct* pObj);
LONG fall_AI_GetDistanceValue(OBJStruct* pObj);
LONG fall_AI_GetAttackWhoValue(OBJStruct* pObj);
LONG fall_AI_GetChemUseValue(OBJStruct* pObj);

LONG fall_AI_SetBurstValue(OBJStruct* pObj, LONG val);
LONG fall_AI_SetRunAwayValue(OBJStruct* pObj, LONG val);
LONG fall_AI_SetWeaponPrefValue(OBJStruct* pObj, LONG val);
LONG fall_AI_SetDistanceValue(OBJStruct* pObj, LONG val);
LONG fall_AI_SetAttackWhoValue(OBJStruct* pObj, LONG val);
LONG fall_AI_SetChemUseValue(OBJStruct* pObj, LONG val);



OBJStruct* fall_obj_getInvItem_HeldInSlot_1(OBJStruct* pObj);
OBJStruct* fall_obj_getInvItem_HeldInSlot_2(OBJStruct* pObj);
OBJStruct* fall_obj_getInvItem_Wearing(OBJStruct* pObj);

char* fall_obj_GetProtoName(OBJStruct* pObj);
char* fall_obj_GetName(OBJStruct* pObj);

LONG fall_Obj_GetSkillPoints(OBJStruct* pObj, LONG skillNum);
char* fall_obj_GetSkillName(LONG skill_num);
LONG fall_Obj_GetStat(OBJStruct* pObj, LONG statNum);
LONG Obj_GetSkillWithHighestPoints(OBJStruct* pObj);

LONG fall_Obj_AttemptWeaponReload(OBJStruct* pObj, LONG flag_displayText);

LONG fall_Obj_GetTotalCaps(OBJStruct* pObj);

LONG fall_Obj_Inventory_GetTotalWeight(OBJStruct* pObj);
LONG fall_Obj_Inventory_GetTotalCost(OBJStruct* pObj);

LONG fall_Obj_Inventory_AddItems(OBJStruct* pObj_Owner, OBJStruct* pObj_Item, LONG num_items);
LONG fall_Obj_Inventory_RemoveItems(OBJStruct* pObj_Owner, OBJStruct* pObj_Item, LONG num_items);

LONG fall_Obj_Item_GetType(OBJStruct* pObj);
LONG fall_Obj_Item_GetMaxAmmo(OBJStruct* pObj);
LONG fall_Obj_Item_GetCurrentAmmo(OBJStruct* pObj);

char* fall_obj_GetNameFromScript(OBJStruct* pObj);

LONG fall_Obj_Critter_GetWeaponRange(OBJStruct* pObj, LONG attackType);
LONG fall_Obj_Item_Weapon_GetDamage(OBJStruct* pObj, LONG* p_ret_min, LONG* p_ret_max);
LONG fall_Obj_Item_Weapon_GetSubType(OBJStruct* pObj, LONG attackType);
LONG fall_Obj_Item_Weapon_GetAmmoProID(OBJStruct* pObj);
char* fall_Get_Prototype_Name(DWORD proID);
LONG fall_Obj_Examine(OBJStruct* pObj_holder, OBJStruct* pObj_Item, void (*p_func)(void));//pfunc has one argument, char* passed on EAX register.
LONG fall_Obj_Item_GetWeight(OBJStruct* pObj);

LONG fall_Obj_GetScriptID(OBJStruct* pObj, DWORD* pScriptID);

LONG fall_Obj_New_Script_Instance(OBJStruct* pObj, LONG script_type, LONG script_ref_number);
LONG fall_Obj_New_ScriptID(OBJStruct* pObj, DWORD* pret_scriptID);
LONG fall_Obj_Copy(OBJStruct** pObj_out, OBJStruct* pObj_to_copy);
LONG fall_Obj_Clear_Inventory(PUD* pObj_Pud);
LONG fall_Obj_Disconnect_From_Map(OBJStruct* pObj, RECT* p_rc_ret);
LONG fall_Obj_Connect_To_Map(OBJStruct* pObj, LONG hex_num, LONG level, RECT* p_rc_ret);

LONG fall_Obj_Critter_Adjust_AC(OBJStruct* pObj_critter, OBJStruct* pObj_armor_old, OBJStruct* pObj_armor_new);

BOOL fall_Obj_Weapon_Can_Load(OBJStruct* pObj_weapon, OBJStruct* pObj_ammo);
//returns remaining ammo
LONG fall_Obj_Weapon_Load(OBJStruct* pObj_weapon, OBJStruct* pObj_ammo);

OBJStruct* fall_Obj_Weapon_Unload(OBJStruct* pObj_weapon);
BOOL fall_Obj_Weapon_Can_Unload(OBJStruct* pObj_weapon);

LONG fall_Obj_Destroy_InvObj(OBJStruct* pObj);

LONG fall_Obj_Item_SetCurrentAmmo(OBJStruct* pObj, LONG quantity);

DWORD fall_Obj_Weapon_AnimationCode(OBJStruct* pObj_weapon);

LONG fall_Obj_Toggle_Flat(OBJStruct* pObj, RECT* p_rc_ret);