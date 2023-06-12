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

#include "../Fall_Objects.h"

struct TILE_DATA_node {
    LONG level;
    LONG num;
    BOOL isRoof;
    DWORD frmID;
    TILE_DATA_node* next;
};

//for holding a pointer to a map object or the tile location if the object is a floor or roof tile.
class MAP_OBJECT {
public:
    MAP_OBJECT() { Clear(); };
    MAP_OBJECT(OBJStruct* p_in_Obj) { Set(p_in_Obj); };
    MAP_OBJECT(LONG in_tile_num, BOOL in_is_roof, LONG in_level) { Set(in_tile_num, in_is_roof, in_level); };
    void Set(OBJStruct* p_in_Obj) {
        Clear();
        if (p_in_Obj) {
            pObj = p_in_Obj;
            level = pObj->level;
        }
    };
    void Set(LONG in_tile_num, BOOL in_is_roof, LONG in_level) {
        Clear();
        tileNum = in_tile_num;
        isRoof = in_is_roof;
        level = in_level;
    };
    void Clear() {
        pObj = nullptr;
        tileNum = -1;
        isRoof = FALSE;
        level = -1;
    }
    BOOL IsValid() {
        if (pObj == nullptr && tileNum == -1)
            return FALSE;
        return TRUE;
    };
    OBJStruct* GetObj() { return pObj; };
    LONG GetTileNumber() { return tileNum; };
    BOOL IsRoof() { return isRoof; };
    LONG GetLevel() { return level; };
protected:
private:
    OBJStruct* pObj;
    LONG tileNum;
    BOOL isRoof;
    LONG level;
};


enum class SEL_RC_TYPE : BOOL {
    none = -1,
    square = 0,
    isometric = 1,
};

enum class EDGE_SELECT : int {
    none = -1,
    left = 0,
    top = 1,
    right = 2,
    bottom = 3
};

enum class PERFORMING_ACTION : int {
    none = 0,
    create_new_object = 1,
    selecting_objects = 2,
    pasting_objects = 3,
    area_edge_adjustment = 4

};

enum class OBJ_SEARCH : BOOL {
    none = 0,
    matching_proID = 1,
    matching_scriptIndex = 2,
    has_scriptIndex = 3,
    has_scriptID = 4,
    has_inventory = 5,
    emits_light = 6,
    matching_frmID = 7
};

enum class MAP_OBJECT_TYPE : int {
    none = -1,
    item = 0,
    critter = 1,
    wall = 2,
    tile = 3,
    misc = 4

};


BOOL CopiedObjects_CopySelected();
BOOL CopiedTiles_CopySelected();

BOOL Action_Initiate_CopiedObjects_Pasting();
BOOL Action_Initiate_PlaceNewObject(DWORD proID);

BOOL CopyLevel_Copy(LONG level);
BOOL CopyLevel_Paste(LONG level);


void SelectedObjects_Delete();


BOOL ObjectTypeSelectable_Toggle(LONG type);
BOOL ObjectTypeSelectable_ToggleAll();
BOOL ObjectTypeSelectable_Set(LONG type, BOOL isSelectable, BOOL isRoof);
BOOL IsObjectTypeSelectable(LONG type);
BOOL IsObjectTypeSelectable(LONG type, BOOL isRoof);

void SelectedAction_Preform();
void SelectedAction_Update();
void SelectedAction_End(BOOL cancel_action);

BOOL ToggleSelectedEdges();
BOOL ToggleDrawAllEdges();
BOOL AreAllEdgesVisible();

LONG GetSelectedEdgeRectNum();
void ResetSelectedEdgeRectNum();

BOOL MapArea_DeleteSelected();
BOOL MapArea_AddNew();

BOOL ToggleRooves();
void ShowRooves();
void HideRooves();
BOOL AreRoovesVisible();

BOOL ToggleHexes();
void ShowHexes();
void HideHexes();
BOOL AreHexesVisible();

BOOL Toggle_BlockerObjects();
void Show_BlockerObjects();
void Hide_BlockerObjects();
BOOL AreBlockerObjectsVisible();
void Set_BlockerObject(OBJStruct* pObj);
void Set_BlockerObjects();
//Sets the frmID if proID matches one of the blocker prototypes, return true if so.
BOOL Check_If_Blocker_Proto(DWORD proID, DWORD* frmID);

void SetMapperMouse(BOOL isMapper);

BOOL Get_Proto_Path(DWORD proID, char* p_ret_path, size_t path_length);
BOOL Get_Proto_File_Path(DWORD proID, char* p_ret_path, size_t path_length);

void Set_Mapper_Ambient_Light_Intensity(LONG intensity);
LONG Get_Mapper_Ambient_Light_Intensity();

void Set_SelectionRectType(SEL_RC_TYPE type);
SEL_RC_TYPE Get_SelectionRectType();

OBJNode* Get_Selected_Objects();
TILE_DATA_node* Get_Selected_Tiles();

OBJNode* Get_Copied_Objects();
TILE_DATA_node* Get_Copied_Tiles();

OBJNode* Get_Copied_Level_Objects();
TILE_DATA_node* Get_Copied_Level_Tiles();


BOOL Action_Initiate_AdjustMapEdge(SEL_RC_TYPE edge_type, EDGE_SELECT edge);
EDGE_SELECT Get_SelectedEdge(SEL_RC_TYPE type);

//BOOL Set_PerformingAction(PERFORMING_ACTION action);
PERFORMING_ACTION Get_PerformingAction();

BOOL IsMapperScrolling();

BOOL ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT iso_edge, LONG level);
BOOL IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT iso_edge, LONG level);

MAP_OBJECT* ObjectUnderMouse_Get();
BOOL ObjectUnderMouse_Copy();
BOOL ObjectUnderMouse_Delete();
BOOL ObjectUnderMouse_Clear();

BOOL CopiedObjects_CopyObject(OBJStruct* pObj);
BOOL CopiedTiles_CopyTile(LONG tileNum, BOOL isRoof, LONG level);

void Delete_MapObj(OBJStruct** ppObj);
void Delete_MapTile(LONG tileNum, BOOL isRoof, LONG level);


BOOL SelectObject_Remove(OBJStruct* pObj, BOOL update_selected_list_state);
BOOL SelectObject_Add(OBJStruct* pObj, BOOL update_selected_list_state);

BOOL SelectTile_Remove(TILE_DATA_node* tileNodeNew, BOOL update_selected_list_state);

void Selected_List_Destroy(BOOL destroy_object);
void Selected_List_Update_State();

BOOL SelectTiles_With_Matching_FRM(LONG level, DWORD frmID);
BOOL SelectObjects_On_Map(LONG level, OBJ_SEARCH search_for, DWORD test_data);


BOOL Selected_List_SetFocus(MAP_OBJECT* p_mapObject);
void Selected_List_ClearFocus();

BOOL MapObject_Copy(OBJStruct* pObj, LONG tileNum, BOOL isRoof, LONG tileLevel);
BOOL MapObject_Delete(OBJStruct* pObj, LONG tileNum, BOOL isRoof, LONG tileLevel);

BOOL ListObjects_On_Map(LONG level, OBJNode** lpObjs_List, OBJ_SEARCH search_for, DWORD test_data, DWORD outline_flags, BOOL ignore_type_selectable);

//Gets the next enabled object from the current map object array. Run first with isStart true to set level and reset variables.
//Set level to -1 to scan all levels
OBJStruct* GetNextEnabledObject(BOOL isStart, LONG level);

//BOOL CopyObject(OBJStruct** newObjOut, OBJStruct* fromObj);
//BOOL DestroyCopiedObject(OBJStruct* pObj);
//void CopiedObject_Create(OBJStruct** lpObj, OBJStruct* pObj);
//void CopiedObject_Delete(OBJStruct* pObj);

BOOL SpatialScript_Create(LONG scriptIndex, LONG hexPos, LONG level, LONG radius);
//Set hexNum to -1 to scan all hexes, set level to -1 to scan all levels.
BOOL SpatialScripts_Remove_All_At(LONG hexNum, LONG level);

BOOL SpatialScripts_Create_Marker_Objects();
//Set hexNum to -1 to scan all hexes, set level to -1 to scan all levels.
BOOL SpatialScripts_Destroy_Marker_Objects(LONG hexNum, LONG level, BOOL single);
LONG SpatialScripts_List_At(DWORD** pp_ret_ScriptID, LONG hexNum, LONG level);

//set any var to -1(out of bounds), to ignore.
void Map_Set_Start_Vars(LONG hexNum, LONG level, LONG ori);

BOOL Copy_MapObject(OBJStruct** newObjOut, OBJStruct* fromObj);

void PlaceNewObject_Rotate_Clockwise();
void PlaceNewObject_Rotate_AntiClockwise();
