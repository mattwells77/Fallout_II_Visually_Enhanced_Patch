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


#include "mapper_tools.h"

#include "win_Mapper.h"
#include "win_ProtoBrowser.h"
#include "win_ToolBar.h"

#include "../game_map.h"

#include "../Fall_General.h"
#include "../Fall_GameMap.h"
#include "../Fall_Objects.h"
#include "../Fall_Scripts.h"
#include "../Fall_Graphics.h"
#include "../Fall_File.h"

#include "../Dx_Game.h"

// OBJStruct.combatFlags------
//renamed outline colour ids for mapper
#define FLG_MapperSelecting         FLG_combatUnk0x04
#define FLG_MapperSelected          FLG_NotVisByPC
#define FLG_MapperSelected_Focus    FLG_ItemUnderMouse
#define FLG_MapperObjUnderMouse     FLG_combatUnk0x04
#define FLG_MapperPasting_Good      FLG_PCTeamMem
#define FLG_MapperPasting_Bad       FLG_NonPCTeamMem


//mouse action vars
PERFORMING_ACTION performing_action = PERFORMING_ACTION::none;
LONG selectedActionLevel = -1;

//
BOOL obj_type_selectable[] = { 1, 1, 1, 1, 3, 1 };

//blocker object display vars
BOOL blocker_objects_visible = 0;
DWORD blocker_proID[]{ 0x02000043, 0x02000080, 0x0200008D, 0x02000158, 0x0300026D, 0x0300026E, 0x0500000C, 0x05000005, 0x02000031 };
DWORD blocker_frmID[]{ 0x02000016, 0x02000017, 0x02000018, 0x02000019, 0x03000016, 0x03000017, 0x0500000C, 0x05000019, 0x02000393 };

//new object vars
DWORD new_obj_proId = -1;
LONG new_obj_ori = 0;

//selecting objects vars
SEL_RC_TYPE selection_rect_type = SEL_RC_TYPE::isometric;
POINT mouse_select_area_start{ 0,0 };
POINT mouse_select_area_end{ 0,0 };
BOOL selectObject_isRemoving = FALSE;

//object lists
OBJNode* pObjs_Selecting = nullptr;
OBJNode* pObjs_Selected = nullptr;
OBJNode* pObjs_Copyed = nullptr;
OBJNode* pObjs_LevelCopyed = nullptr;
//tile lists
TILE_DATA_node* pTiles_Selecting = nullptr;
TILE_DATA_node* pTiles_Selected = nullptr;
TILE_DATA_node* pTiles_Copyed = nullptr;
TILE_DATA_node* pTiles_LevelCopyed = nullptr;

//editing edge vars
EDGE_SELECT selected_edge = EDGE_SELECT::none;
SEL_RC_TYPE selected_edge_type = SEL_RC_TYPE::none;
BOOL mapper_view_all_edge_lines = 0;
LONG mapper_selected_edge_rect_num = 0;
RECT* pRcEdgeTemp = nullptr;


LONG mapper_ambient_light_intensity = 0x010000;

//the object or tile currently under mouse
MAP_OBJECT map_obj_under_mouse(nullptr);
//the object or tile that currently has focus in the selected list
MAP_OBJECT map_obj_selected_focus(nullptr);

BOOL display_spacial_scripts = TRUE;


/*
#include <vector>
using namespace std;
class UNDO_REDO {
public:
    UNDO_REDO() {

    }
    ~UNDO_REDO() {

    }
protected:
    virtual BOOL Undo() {

    }
    virtual BOOL Redo() {

    }
private:
};

class UNDO_REDO_OBJ: public UNDO_REDO {
public:
    UNDO_REDO_OBJ(OBJStruct *p_inObj_undo, OBJStruct* p_inObj_redo){
        pObj_undo = nullptr;
        pObj_redo = nullptr;
        pObj_undo = new OBJStruct;
        //memcopy(pObj_undo, p_inObj_undo,0 );

    }
protected:
private:
    OBJStruct* pObj_undo;
    OBJStruct* pObj_redo;
};
std::vector<std::unique_ptr<UNDO_REDO>> UnDo;
vector<UNDO_REDO> UnDo;
vector<UNDO_REDO> ReDo;

BOOL Undo_Add_Objects(OBJStruct* pObj_undo, OBJStruct* pObj_redo) {
    if (pObj_undo && pObj_redo) {
        UnDo.push_back(std::unique_ptr<UNDO_REDO>(new UNDO_REDO_OBJ(pObj_undo, pObj_redo)));
        return TRUE;
    }
    return FALSE;
}
*/
BOOL MapChanges_Update() {

    ///yet to be implemented
    return TRUE;
}
BOOL MapChanges_Restore() {
    ///yet to be implemented
    return TRUE;
}


//____________________________________________________________
BOOL Obj_Set_Ori(OBJStruct* pObj, LONG ori, RECT* p_rc_return) {
    if (!pObj)
        return FALSE;
    if (ori < 0 || ori >= 6)
        return FALSE;
    pObj->ori = ori;
    if (p_rc_return) 
        GetObjRectDx(pObj, p_rc_return);
    
    return TRUE;
}


//________________________________________________________
void Map_Set_Start_Vars(LONG hexNum, LONG level, LONG ori) {
    MAP_HEADER* pMap = Get_Map_Header();
    if (!pMap)
        return;
    if (hexNum >= 0 && hexNum < *pNUM_HEXES)
        pMap->pcHexPos = hexNum;
    if (level >= 0 && level < 3)
        pMap->pcLevel = level;
    if (ori >= 0 && ori < 6)
        pMap->pcOri = ori;
}


//_______________________________________________________________________________
BOOL SpatialScript_Create(LONG scriptIndex, LONG hexNum, LONG level, LONG radius) {
    DWORD scriptID = 0;
    if (fall_Script_New(&scriptID, static_cast<LONG>(SCRIPT_TYPE::spatial)) == -1)
        return FALSE;

    SCRIPT_STRUCT* pScr = nullptr;
    if (fall_Script_Get(scriptID, &pScr) == -1)
        return FALSE;
    pScr->udata.sp.build_tile = ((level << 29) & 0xE0000000) | (hexNum);
    pScr->udata.sp.radius = radius;
    pScr->index = scriptIndex & 0x00FFFFFF;

    fall_Script_Find_Str_Run_Info(level, &pScr->unknown50, scriptID);

    //Fallout_Debug_Info("SpatialScript_Create created index:%d, hex:%d, level:%d, radius:%d", scriptIndex, hexNum, level, radius);

    if (display_spacial_scripts) {
        if (level >= 0 && level < 3 && hexNum >= 0 && hexNum < *pNUM_HEXES) {
            OBJStruct* pObj = nullptr;
            DWORD frmID = fall_GetFrmID(ART_INTRFACE, 3, 0, 0, 0);
            if (fall_Obj_Create(&pObj, frmID, -1) == 0) {
                RECT rc_obj{ 0,0,0,0 };
                pObj->flags |= FLG_NonEffect;
                fall_Obj_Toggle_Flat(pObj, nullptr);
                fall_Obj_Move(pObj, hexNum, level, &rc_obj);
                DrawMapChanges(&rc_obj, level, FLG_Obj | FLG_Floor);
                //Fallout_Debug_Info("SpatialScript_Create - marker object created");
            }
            else
                Fallout_Debug_Error("SpatialScript_Create - marker object could not be created");
        }
    }

    return TRUE;
}


//________________________________________________________
BOOL SpatialScripts_Remove_All_At(LONG hexNum, LONG level) {

    SCRIPT_STRUCT* pScr = fall_Script_Find_First_At(level);
    BOOL count = 0;
    while (pScr) {
        if ((pScr->udata.sp.build_tile & 0x03FFFFFF) == hexNum) {// shouldn't this be 0x01FFFFFFF ?
            fall_Script_Remove(pScr->id);
            count++;
            pScr = fall_Script_Find_First_At(level);
        }
        else
            pScr = fall_Script_Find_Next_At(level);
    }
    //Fallout_Debug_Info("SpatialScripts_Remove_All_At hex:%d, level:%d, count:%d", hexNum, level, count);

    SpatialScripts_Destroy_Marker_Objects(hexNum, level, FALSE);

    return count;
}


//___________________________________________________________________________
LONG SpatialScripts_List_At(DWORD** pp_ret_ScriptID, LONG hexNum, LONG level) {
    
    SCRIPT_STRUCT* pScr = fall_Script_Find_First_At(level);
    BOOL num_scripts = 0;
    while (pScr) {
        if ((pScr->udata.sp.build_tile & 0x03FFFFFF) == hexNum)
            num_scripts++;
        pScr = fall_Script_Find_Next_At(level);
    }
    //Fallout_Debug_Info("SpatialScripts_List_At hex:%d, level:%d, count:%d", hexNum, level, num_scripts);

    if(!pp_ret_ScriptID)
        return num_scripts;

    if (num_scripts) {
        DWORD* pScriptID = new DWORD[num_scripts];
        *pp_ret_ScriptID = pScriptID;
        BOOL count = 0;
        pScr = fall_Script_Find_First_At(level);
        while (pScr) {
            if ((pScr->udata.sp.build_tile & 0x03FFFFFF) == hexNum) {
                pScriptID[count] = pScr->id;
                count++;
            }
            pScr = fall_Script_Find_Next_At(level);
        }
    }
    return num_scripts;
}


//_________________________________________
BOOL SpatialScripts_Create_Marker_Objects() {
    //SpatialScripts_Destroy_Marker_Objects(-1, -1, FALSE);
    if (!display_spacial_scripts)
        return FALSE;
    LONG num_scripts = fall_Get_Number_Of_Active_Scripts(static_cast<LONG>(SCRIPT_TYPE::spatial));
    RECT rc_obj{ 0,0,0,0 };
    LONG num = 0;
    LONG scriptID_num = 0;
    LONG level = -1;
    LONG hexPos = -1;
    DWORD frmID = fall_GetFrmID(ART_INTRFACE, 3, 0, 0, 0);
    OBJStruct* pObj = nullptr;
    SCRIPT_STRUCT* pScr = nullptr;
    if (num_scripts > 0) {
        while (num < num_scripts) {
            if (fall_Script_Get(scriptID_num | 0x01000000, &pScr) != -1) {
                level = (pScr->udata.sp.build_tile & 0xE0000000) >> 0x1D;
                hexPos = pScr->udata.sp.build_tile & 0x03FFFFFF;
                if (level >= 0 && level < 3 && hexPos >= 0 && hexPos < *pNUM_HEXES) {
                    if (fall_Obj_Create(&pObj, frmID, -1) == 0) {
                        pObj->flags |= FLG_NonEffect;
                        fall_Obj_Toggle_Flat(pObj, nullptr);
                        fall_Obj_Move(pObj, hexPos, level, &rc_obj);
                        DrawMapChanges(&rc_obj, level, FLG_Obj | FLG_Floor);
                        //Fallout_Debug_Info("SpatialScripts_Create_Marker_Objects hex:%d, level:%d, objID:%d", hexPos, level, pObj->objID);
                    }
                }
                num++;
            }
            scriptID_num++;//search all possible scriptID's untill all active scripts are found.
        }
        //Fallout_Debug_Info("SpatialScripts_Create_Marker_Objects num objects created:%d num ID searched:%d", num, scriptID_num);
        return TRUE;
    }
    return FALSE;
}


//______________________________________________________________________________
BOOL SpatialScripts_Destroy_Marker_Objects(LONG hexNum, LONG level, BOOL single) {

    DWORD frmID = fall_GetFrmID(ART_INTRFACE, 3, 0, 0, 0);
    OBJStruct* pObj = GetNextEnabledObject(TRUE, level);
    RECT rc_obj{ 0,0,0,0 };
    BOOL count = 0;
    while (pObj != nullptr) {
        if ((hexNum == -1 || hexNum == pObj->hexNum) && frmID == pObj->frmID) {
            //Fallout_Debug_Info("SpatialScripts_Destroy_Marker_Objects hex:%d, level:%d, objID:%d", hexNum, level, pObj->objID);
            fall_Obj_Destroy(pObj, &rc_obj);
            DrawMapChanges(&rc_obj, level, FLG_Obj | FLG_Floor);
            count++;
            if (single)
                pObj = nullptr;
            else
                pObj = GetNextEnabledObject(TRUE, level);
        }
        else
            pObj = GetNextEnabledObject(FALSE, level);
    }

    //Fallout_Debug_Info("SpatialScripts_Destroy_Marker_Objects hex:%d, level:%d, count:%d", hexNum, level, count);
    return count;
}


//______________________
BOOL IsMapperScrolling() {
    if (!isRunAsMapper)
        return FALSE;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x80000000))
        return TRUE;
    return FALSE;
}


//_________________________________________________
BOOL Set_PerformingAction(PERFORMING_ACTION action) {
    if (performing_action != PERFORMING_ACTION::none)
        return FALSE;
    else
        performing_action = action;
    return TRUE;
}


//______________________________________
PERFORMING_ACTION Get_PerformingAction() {
    return performing_action;
}


//_________________________
void End_PerformingAction() {
    performing_action = PERFORMING_ACTION::none;
}


//_____________________________
OBJNode* Get_Selected_Objects() {
    return pObjs_Selected;
}

//__________________________________
TILE_DATA_node* Get_Selected_Tiles() {
    return pTiles_Selected;
}


//___________________________
OBJNode* Get_Copied_Objects() {
    return pObjs_Copyed;
}


//________________________________
TILE_DATA_node* Get_Copied_Tiles() {
    return pTiles_Copyed;
}


//_________________________________
OBJNode* Get_Copied_Level_Objects() {
    return pObjs_LevelCopyed;
}

//______________________________________
TILE_DATA_node* Get_Copied_Level_Tiles() {
    return pTiles_LevelCopyed;
}


//__________________________________________
void Set_SelectionRectType(SEL_RC_TYPE type) {
    selection_rect_type = type;
}


//_________________________________
SEL_RC_TYPE Get_SelectionRectType() {
    return selection_rect_type;
}


//_____________________________________________________
void Set_Mapper_Ambient_Light_Intensity(LONG intensity) {
    mapper_ambient_light_intensity = intensity;
}


//_______________________________________
LONG Get_Mapper_Ambient_Light_Intensity() {
    return mapper_ambient_light_intensity;
}


//____________________________________________________
BOOL Check_If_Blocker_Proto(DWORD proID, DWORD* frmID) {
    for (int i = 0; i < 9; i++) {
        if (frmID && proID == blocker_proID[i]) {
            *frmID = blocker_frmID[i];
            return TRUE;
        }
    }
    return FALSE;
}


//_____________________________________
void Set_BlockerObject(OBJStruct* pObj) {

    if (pObj == *ppObj_PC || pObj == *ppObj_Selected || pObj == *ppObj_Mouse)
        return;
    for (int i = 0; i < 9; i++) {
        if (pObj->proID == blocker_proID[i]) {
            if (AreBlockerObjectsVisible())
                pObj->frmID = blocker_frmID[i];
            else {
                PROTO* p_pro = nullptr;
                if (fall_GetPro(blocker_proID[i], &p_pro) == 0)
                    pObj->frmID = p_pro->frmID;
            }
            i = 9;
        }
    }
}


//______________________________________
void Show_BlockerObject(OBJStruct* pObj) {
    if (pObj == *ppObj_PC || pObj == *ppObj_Selected || pObj == *ppObj_Mouse)
        return;
    for (int i = 0; i < 9; i++) {
        if (pObj->proID == blocker_proID[i]) {
            pObj->frmID = blocker_frmID[i];
            i = 9;
        }
    }
}


//______________________________________
void Hide_BlockerObject(OBJStruct* pObj) {
    if (pObj == *ppObj_PC || pObj == *ppObj_Selected || pObj == *ppObj_Mouse)
        return;
    for (int i = 0; i < 9; i++) {
        if (pObj->proID == blocker_proID[i]) {
            PROTO* p_pro = nullptr;
            if (fall_GetPro(blocker_proID[i], &p_pro) == 0) {
                pObj->frmID = p_pro->frmID;
            }
            i = 9;
        }
    }
}


//_______________________
void Set_BlockerObjects() {
    OBJNode* p_node = nullptr;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        p_node = pMapObjNodeArray[hexPos];
        while (p_node) {
            Set_BlockerObject(p_node->pObj);
            if (p_node)
                p_node = p_node->next;
        }
    }
}


//________________________
void Show_BlockerObjects() {

    OBJNode* p_node = nullptr;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        p_node = pMapObjNodeArray[hexPos];
        while (p_node) {
            Show_BlockerObject(p_node->pObj);
            if (p_node)
                p_node = p_node->next;
        }
    }
}


//________________________
void Hide_BlockerObjects() {

    OBJNode* p_node = nullptr;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        p_node = pMapObjNodeArray[hexPos];
        while (p_node) {
            Hide_BlockerObject(p_node->pObj);
            if (p_node)
                p_node = p_node->next;
        }
    }
}


//__________________________
BOOL Toggle_BlockerObjects() {
    blocker_objects_visible = 1 - blocker_objects_visible;
    Set_BlockerObjects();
    return blocker_objects_visible;
}


//_____________________________
BOOL AreBlockerObjectsVisible() {
    return blocker_objects_visible;
}


//____________________________________________________________________
BOOL Get_Proto_Path(DWORD proID, char* p_ret_path, size_t path_length) {

    if (!p_ret_path || path_length == 0)
        return FALSE;
    if (proID == -1)
        return FALSE;

    LONG type = (proID >> 24) & 0xFF;

    char* p_type_name = GetArtTypeName(type);
    if (p_type_name == nullptr)
        return FALSE;

    sprintf_s(p_ret_path, path_length, "proto\\%s", p_type_name);

    return TRUE;
}


//_________________________________________________________________________
BOOL Get_Proto_File_Path(DWORD proID, char* p_ret_path, size_t path_length) {

    if (!p_ret_path || path_length == 0)
        return FALSE;
    if (proID == -1)
        return FALSE;

    LONG type = (proID >> 24) & 0xFF;
    LONG list_num = proID & 0x00FFFFFF;

    char* p_proto_path = new char[MAX_PATH] {0};

    BOOL retVal = Get_Proto_Path(proID, p_proto_path, MAX_PATH);

    if (retVal) {
        char* p_proto_list_path = new char[MAX_PATH] {0};
        sprintf_s(p_proto_list_path, MAX_PATH, "%s\\%s.lst", p_proto_path, GetArtTypeName(type));
        void* FileStream = fall_fopen(p_proto_list_path, "rt");
        if (FileStream) {
            char* p_file_name = new char[MAX_PATH] {0};
            LONG line_num = 0;
            if (line_num < list_num) {
                while (line_num < list_num) {
                    line_num++;
                    if (fall_fgets(p_file_name, MAX_PATH, FileStream) == nullptr) {
                        retVal = FALSE;
                        line_num = list_num + 1;
                    }
                }

                if (list_num == line_num) {
                    LONG char_num = 0;
                    while (p_file_name[char_num] != '\0' && char_num < MAX_PATH) {
                        if (p_file_name[char_num] == ' ' || p_file_name[char_num] == '\n')
                            p_file_name[char_num] = '\0';
                        else
                            char_num++;
                    }
                    sprintf_s(p_ret_path, path_length, "%s\\%s", p_proto_path, p_file_name);
                }
                else
                    retVal = FALSE;
            }
            fall_fclose(FileStream);
        }
        else
            retVal = FALSE;

        delete[] p_proto_list_path;
    }

    if (retVal == FALSE)
        p_ret_path[0] = '\0';

    delete[] p_proto_path;

    return retVal;
}


//_______________________
BOOL ToggleDrawAllEdges() {
    mapper_view_all_edge_lines = 1 - mapper_view_all_edge_lines;
    return mapper_view_all_edge_lines;
}


//_______________________
BOOL AreAllEdgesVisible() {
    return mapper_view_all_edge_lines;
}


//________________________
BOOL ToggleSelectedEdges() {

    GAME_AREA* currentEdge = GameAreas_Get(*pMAP_LEVEL);
    GAME_AREA* nextEdge = currentEdge->next;
    int num = 0;
    while (nextEdge) {
        if (num == mapper_selected_edge_rect_num) {
            mapper_selected_edge_rect_num++;
            return TRUE;
        }
        currentEdge = nextEdge;
        nextEdge = currentEdge->next;
        num++;
    }
    mapper_selected_edge_rect_num = 0;
    return FALSE;
}


//___________________________
LONG GetSelectedEdgeRectNum() {
    return mapper_selected_edge_rect_num;
}


//_____________________________
void ResetSelectedEdgeRectNum() {
    mapper_selected_edge_rect_num = 0;
}


//___________________________________
RECT* GetSelectedAreaRect(LONG level) {

    GAME_AREA* selectedEdge = GameAreas_Get(level);

    int num = 0;
    while (selectedEdge) {
        if (num == mapper_selected_edge_rect_num)
            return &selectedEdge->rect;
        selectedEdge = selectedEdge->next;
        num++;
    }

    return nullptr;
}


//____________________________________
GAME_AREA* GetSelectedArea(LONG level) {

    GAME_AREA* pArea = GameAreas_Get(level);

    int num = 0;
    while (pArea) {
        if (num == mapper_selected_edge_rect_num)
            return pArea;
        pArea = pArea->next;
        num++;
    }
    return nullptr;
}


//___________________________
BOOL MapArea_DeleteSelected() {
    if (mapper_selected_edge_rect_num <= 0) {
        mapper_selected_edge_rect_num = 0;
        return FALSE;
    }
    GAME_AREA* pArea = GameAreas_Get(*pMAP_LEVEL);
    
    GAME_AREA* pArea_prev = nullptr;
    int num = 0;
    while (pArea) {
        if (num == mapper_selected_edge_rect_num) {
            if (pArea_prev == nullptr) {//this shouldn't happen.
                mapper_selected_edge_rect_num = 0;
                return FALSE;
            }
            pArea_prev->next = pArea->next;
            if (pArea->next == nullptr)//sub selection number to that it points to the last valid area.
                mapper_selected_edge_rect_num--;
            pArea->next = nullptr;
            delete pArea;
            pArea = nullptr;
            return TRUE;
        }
        pArea_prev = pArea;
        pArea = pArea->next;
        num++;
    }
    return FALSE;
}


//___________________
BOOL MapArea_AddNew() {
    GAME_AREA* pArea = GameAreas_Get(*pMAP_LEVEL);
    int num = 0;
    while (pArea) {
        if (pArea->next == nullptr) {
            pArea->next = new GAME_AREA;
            pArea->next->tileLimits = { *pNUM_TILE_X,0,0,*pNUM_TILE_Y };
            pArea->next->rect = { -4800, 0, 3200, 3600 };

            if (Edges_GetActiveVersion() == 2) {
                pArea->next->rect.right -= 32;
                pArea->next->rect.bottom -= 24;
            }
            pArea->next->width = pArea->next->rect.right - pArea->next->rect.left;
            pArea->next->height = pArea->next->rect.bottom - pArea->next->rect.top;
            mapper_selected_edge_rect_num = num+1;
            return TRUE;
        }

        pArea = pArea->next;
        num++;
    }
    return FALSE;
}


//____________________________________________
EDGE_SELECT Get_SelectedEdge(SEL_RC_TYPE type) {

    if (type != selected_edge_type)
        return EDGE_SELECT::none;

    return selected_edge;
}


//____________________________________________________________________________
BOOL ToggleIsoEdge_NonFlat_Object_Visibility(EDGE_SELECT iso_edge, LONG level) {
    if (iso_edge == EDGE_SELECT::none)
        return FALSE;
    GAME_AREA* pArea = GameAreas_Get(level);
    if (!pArea)
        return FALSE;
    DWORD flag_on = 0x01000000 >> (static_cast<int>(iso_edge) * 8);

    if (pArea->tileLimitFlags & flag_on) {
        pArea->tileLimitFlags &= ~flag_on;
        return FALSE;
    }
    else {
        pArea->tileLimitFlags |= flag_on;
        return TRUE;
    }
}


//______________________________________________________________________
BOOL IsIsoEdge_NonFlat_Object_Visibile(EDGE_SELECT iso_edge, LONG level) {
    if (iso_edge == EDGE_SELECT::none)
        return FALSE;
    GAME_AREA* pArea = GameAreas_Get(level);
    if (!pArea)
        return FALSE;
    DWORD flag_on = 0x01000000 >> (static_cast<int>(iso_edge) * 8);
    if (pArea->tileLimitFlags & flag_on)
        return TRUE;

    return FALSE;
}


//_________________
BOOL ToggleRooves() {

    *pAreRoovesVisible = 1 - *pAreRoovesVisible;
    return *pAreRoovesVisible;
}

//_______________
void ShowRooves() {
    *pAreRoovesVisible = 1;
}


//_______________
void HideRooves() {
    *pAreRoovesVisible = 0;
}


//_____________________
BOOL AreRoovesVisible() {
    return *pAreRoovesVisible;
}


//________________
BOOL ToggleHexes() {
    *pAreHexesVisible = 1 - *pAreHexesVisible;
    return *pAreHexesVisible;
}


//______________
void ShowHexes() {
    *pAreHexesVisible = 1;
}


//______________
void HideHexes() {
    *pAreHexesVisible = 0;
}


//____________________
BOOL AreHexesVisible() {
    return *pAreHexesVisible;
}


//________________________________
void SetMapperMouse(BOOL isMapper) {
    *pIsMapperMouse = isMapper;
}

/*
//________________________________________________________
void DeleteMapObjs(OBJStruct** lpObj1, OBJStruct** lpObj2) {

    RECT rect;
    OBJStruct* pObj = nullptr;
    BOOL object_removed_from_selected_list = FALSE;
    if (lpObj2 && *lpObj2) {
        pObj = *lpObj2;
        if (pObj == map_obj_under_mouse.GetObj())
            ObjectUnderMouse_Clear();
        object_removed_from_selected_list |= SelectObject_Remove(pObj);

        fall_Obj_ClearAnimation(pObj);
        fall_Obj_Destroy(pObj, &rect);
        DrawMapChanges(&rect, pObj->level, FLG_Obj | FLG_Floor);
        *lpObj2 = nullptr;
    }
    if (lpObj1 && *lpObj1) {
        pObj = *lpObj1;
        if (pObj == map_obj_under_mouse.GetObj())
            ObjectUnderMouse_Clear();
        object_removed_from_selected_list |= SelectObject_Remove(pObj);

        fall_Obj_ClearAnimation(pObj);
        fall_Obj_Destroy(pObj, &rect);
        DrawMapChanges(&rect, pObj->level, FLG_Obj | FLG_Floor);
        *lpObj1 = nullptr;
    }

    if (object_removed_from_selected_list)
        Selected_List_Update_State();
    
}
*/

//___________________________________
void Delete_MapObj(OBJStruct** ppObj) {

    RECT rect{0,0,0,0};

    if (ppObj && *ppObj) {
        OBJStruct* pObj = *ppObj;
        fall_Obj_ClearAnimation(pObj);
        fall_Obj_Destroy(pObj, &rect);
        DrawMapChanges(&rect, pObj->level, FLG_Obj | FLG_Floor);
        *ppObj = nullptr;
    }
}


//_________________________________________
BOOL ObjectTypeSelectable_Toggle(LONG type) {

    if (type < 0 || type>5)
        return 0;

    if (type == ART_TILES) {
        obj_type_selectable[type]++;
        if (obj_type_selectable[type] > 3)
            obj_type_selectable[type] = 0;
    }
    else
        obj_type_selectable[type] = 1 - obj_type_selectable[type];

    return obj_type_selectable[type];
}


//____________________________________
BOOL ObjectTypeSelectable_ToggleAll() {
    BOOL is_any_toggle_set = 0;
    for (int i = 0; i < 6; i++) {
        if (obj_type_selectable[i] == 0)
            is_any_toggle_set = 1;
    }
    for (int type = 0; type < 6; type++) {
        obj_type_selectable[type] = is_any_toggle_set;
        if (type == ART_TILES) {
            obj_type_selectable[type] |= (is_any_toggle_set << 1);
        }
    }

    return is_any_toggle_set;
}


//_____________________________________
BOOL IsObjectTypeSelectable(LONG type) {

    if (type < 0 || type>5)
        return 0;

    return obj_type_selectable[type];
}


//_________________________________________________
BOOL IsObjectTypeSelectable(LONG type, BOOL isRoof) {

    if (type < 0 || type>5)
        return 0;

    if (type == ART_TILES && isRoof)
        return obj_type_selectable[type] & 2;

    return obj_type_selectable[type] & 1;
}


//______________________________________________________________________
BOOL ObjectTypeSelectable_Set(LONG type, BOOL isSelectable, BOOL isRoof) {
    
    if (type < 0 || type>5)
        return 0;
    
    BOOL selectable = 0;
    if (isSelectable)
        selectable = 1;

    if (type == ART_TILES) {
        if (isRoof)
            obj_type_selectable[type] = (obj_type_selectable[type] & 1) | selectable << 1;
        else
            obj_type_selectable[type] = (obj_type_selectable[type] & 2) | selectable;
    }
    else
        obj_type_selectable[type] = selectable;

    return obj_type_selectable[type];
}


//_______________________________________________________
OBJStruct* GetNextEnabledObject(BOOL isStart, LONG level) {
    static LONG CK_level = 0;
    static  LONG CK_hexNum = 0;
    static  OBJNode* CK_pObjNode = 0;

    OBJNode* pObjNode = nullptr;
    LONG artType = -1;

    if (isStart) {
        CK_hexNum = 0;
        CK_pObjNode = nullptr;
        CK_level = level;
        pObjNode = pMapObjNodeArray[CK_hexNum];
    }
    else
        pObjNode = CK_pObjNode->next;

    while (CK_hexNum < *pNUM_HEXES) {
        while (pObjNode != nullptr) {
            CK_pObjNode = pObjNode;
            if (CK_level == -1 || pObjNode->pObj->level == CK_level) {
                artType = (pObjNode->pObj->frmID & 0x0F000000) >> 24;
                if (IsArtTypeEnabled(artType)) {
                    CK_pObjNode = pObjNode;
                    return pObjNode->pObj;
                }
            }
            pObjNode = pObjNode->next;
        }
        CK_hexNum++;
        pObjNode = pMapObjNodeArray[CK_hexNum];
    }
    CK_pObjNode = pObjNode;
    return nullptr;
}


//______________________________________________
BOOL CreateNew_EmptyMapObject(OBJStruct** lpObj) {
    if (!lpObj)
        return FALSE;

    OBJStructDx* pObj = (OBJStructDx*)fall_Mem_Allocate(sizeof(OBJStructDx));
    *lpObj = (OBJStruct*)pObj;
    if (!pObj)
        return FALSE;

    memset(pObj, 0, sizeof(OBJStructDx));
    pObj->flags = -1;
    pObj->hexNum = -1;
    pObj->cID = -1;
    pObj->combatFlags = 0;
    pObj->proID = -1;
    pObj->scriptID = -1;
    pObj->pObj_owner = nullptr;
    pObj->scriptIndex = -1;
    pObj->frmObjDx = nullptr;
    return TRUE;
}


//____________________________________________________________
BOOL Copy_MapObject(OBJStruct** newObjOut, OBJStruct* fromObj) {

    if (!newObjOut)
        return FALSE;
    if(!fromObj)
        return FALSE;

    OBJStruct* pObj = nullptr;
    if (!CreateNew_EmptyMapObject(&pObj)) {
        Fallout_Debug_Error("CopyObject - Failed, CreateNew_EmptyMapObject.");
        return FALSE;
    }
    OBJNode* p_objNode_new = (OBJNode*)fall_Mem_Allocate(sizeof(OBJNode));
    p_objNode_new->pObj = pObj;
    p_objNode_new->next = nullptr;

    //memset(&pObj->pud, 0, sizeof(PUD));

    memcpy(pObj, fromObj, sizeof(OBJStructDx));
    //clear inv vars
    pObj->pud.general.inv_max = 0;
    pObj->pud.general.p_item = nullptr;
    pObj->pud.general.inv_size = 0;
    pObj->pObj_owner = nullptr;
    //set v.e. object var to null.
    ((OBJStructDx*)pObj)->frmObjDx = nullptr;

    fall_Obj_AddObjNodeToMapList(p_objNode_new);
    *newObjOut = pObj;

    pObj->objID = fall_Obj_GetNewObjID();

    if (pObj->scriptID != -1 && pObj->scriptIndex != -1) {
        pObj->scriptID = -1;
        //fall_Obj_New_ScriptID(pObj, &pObj->scriptID);
        fall_Obj_New_Script_Instance(pObj, pObj->scriptIndex >> 24, pObj->scriptIndex & 0x00FFFFFF);
        //Fallout_Debug_Info("Copy_MapObject - new script instance scriptID:%X", pObj->scriptID);
    }
    else
        pObj->scriptID = -1;


    if (pObj->ori < 0 || pObj->ori >= 6) {
        fall_Obj_Destroy(pObj, nullptr);
        *newObjOut = nullptr;
        Fallout_Debug_Error("CopyObject - Failed, objects orientation invalid.");
        return FALSE;
    }

    //AND BYTE PTR DS:[EAX+25],DF
    //exclude FLG_BeingUsed flag.
    pObj->flags &= ~FLG_BeingUsed;

    OBJStruct* pObj_item = nullptr;

    if (fromObj->pud.general.inv_size >= 0) {
        for (int i = 0; i < fromObj->pud.general.inv_size; i++) {
            if (Copy_MapObject(&pObj_item, fromObj->pud.general.p_item[i].p_obj)) {
                if (fall_Obj_Inventory_AddItems(pObj, pObj_item, fromObj->pud.general.p_item[i].num) != -1)
                    fall_Obj_Disconnect_From_Map(pObj_item, nullptr);
                else {
                    fall_Obj_Destroy(pObj_item, nullptr);
                    Fallout_Debug_Error("CopyObject - Failed to add copied item to inventory.");
                }
            }
            else
                Fallout_Debug_Error("CopyObject - Failed to copy Inventory item.");
        }
    }
    return TRUE;
}


//____________________________________________________________________________________________________________
BOOL CreateNew_MapObjectFromObject(LONG hexNum, LONG level, OBJStruct* pObjFrom, BOOL ignore_blocking_objects) {
    if (hexNum < 0 || hexNum >= *pNUM_HEXES) {
        Fallout_Debug_Error("CreateNew_MapObjectFromObject hexNum out of bounds:%d", hexNum);
        return FALSE;
    }
    if (!pObjFrom)
        return FALSE;
    DWORD proID = pObjFrom->proID;
    DWORD frmID = pObjFrom->frmID;

    OBJStruct* pObj = nullptr;
    PROTO* pProto = nullptr;

    ((OBJStruct*)&ppObj_Selected)->flags = ((OBJStruct*)&ppObj_Selected)->flags | FLG_Disabled;
    pObj = fall_Map_GetBlockingObjAtPos(nullptr, hexNum, level);
    ((OBJStruct*)&ppObj_Selected)->flags = ((OBJStruct*)&ppObj_Selected)->flags & (~FLG_Disabled);

    //don't add the new object if it is a blocking object and there is already a blocking object at the selected hex.
    //set ignore_blocking_objects to ignore this check.
    if (!ignore_blocking_objects) {
        if (pObj != nullptr && proID != -1 && (proID >> 24) != ART_TILES) {
            if (fall_GetPro(proID, &pProto) == 0) {
                if (!(pProto->flags & FLG_NoBlock)) {
                    Fallout_Debug_Error("CreateNew_MapObjectFromObject blocking object at hexNum:%d", hexNum);
                    return FALSE;
                }
            }
        }
    }

    //if object is money and there is already a money object at the selected hex, just update the quantity of the existing item.
    pObj = GetNextEnabledObject(TRUE, level);
    while (pObj != nullptr) {
        if (hexNum == pObj->hexNum) {
            if (frmID == pObj->frmID) {
                if (pObj != *ppObj_Selected && pObj != *ppObj_Mouse) {
                    if (pObj->proID == 41) {//"money" item
                        pObj->pud.general.pud.ammo.cur_ammo_quantity = pObj->pud.general.pud.ammo.cur_ammo_quantity++;
                    }
                    return TRUE;
                }
            }

        }
        pObj = GetNextEnabledObject(FALSE, level);
    }

    if (Copy_MapObject(&pObj, pObjFrom) == FALSE) {
        Fallout_Debug_Error("CreateNew_MapObjectFromObject - error copying object");
        return FALSE;
    }
    Set_BlockerObject(pObj);
    RECT rect;
    fall_Obj_Move(pObj, hexNum, level, &rect);

    //not sure how necessary this is but it is in the mapper code, can spatial scripts be attached to objects?
    //move obj's spatial script location
    DWORD scriptID = pObj->scriptID;
    if (scriptID != -1) {
        SCRIPT_STRUCT* pScript;
        if (fall_Script_Get(scriptID, &pScript) == 0) {
            if (scriptID >> 24 == static_cast<LONG>(SCRIPT_TYPE::spatial))
                pScript->udata.sp.build_tile = hexNum | ((level << 29) & 0xE0000000);
            pScript->pObj = pObj;
        }
        else {
            Fallout_Debug_Error("CreateNew_MapObjectFromObject - fall_Script_Get failure");
            pObj->scriptID = -1;
        }
    }

    DrawMapChanges(&rect, pObj->level, FLG_Obj | FLG_Hud_Outline);
    return TRUE;
}


//___________________________________________________________________________________________________
BOOL CreateNew_MapObject(LONG hexNum, LONG level, DWORD frmID, DWORD proID, OBJStruct** ppObj_return) {
    if (hexNum == -1)
        return FALSE;
    OBJStruct* pObj = nullptr;

    ((OBJStruct*)&ppObj_Selected)->flags = ((OBJStruct*)&ppObj_Selected)->flags | FLG_Disabled;
    pObj = fall_Map_GetBlockingObjAtPos(nullptr, hexNum, level);
    ((OBJStruct*)&ppObj_Selected)->flags = ((OBJStruct*)&ppObj_Selected)->flags & (~FLG_Disabled);

    if (pObj != nullptr && proID != -1 && (proID >> 24) != ART_TILES) {
        PROTO* pProto;
        fall_GetPro(proID, &pProto);
        if (!(pProto->flags & FLG_NoBlock))
            return FALSE;
    }
    pObj = GetNextEnabledObject(TRUE, level);
    while (pObj != nullptr) {
        if (hexNum == pObj->hexNum) {
            if (frmID == pObj->frmID) {
                if (pObj != *ppObj_Selected && pObj != *ppObj_Mouse) {
                    if (pObj->proID == 41) {//"money" item
                        pObj->pud.general.pud.ammo.cur_ammo_quantity = pObj->pud.general.pud.ammo.cur_ammo_quantity++;
                    }
                    return TRUE;
                }
            }
        }
        pObj = GetNextEnabledObject(FALSE, level);
    }

    fall_Obj_Create(&pObj, frmID, proID);
    *ppObj_return = pObj;
    Set_BlockerObject(pObj);
    RECT rect;
    fall_Obj_Move(pObj, hexNum, level, &rect);

    //not sure how necessary this is but it is in the mapper code, can spatial scripts be attached to objects?
    //move obj's spatial script location
    DWORD scriptID = pObj->scriptID;
    if (scriptID != -1) {
        SCRIPT_STRUCT* pScript;
        if (fall_Script_Get(scriptID, &pScript) == 0) {
            if (scriptID >> 24 == static_cast<LONG>(SCRIPT_TYPE::spatial))
                pScript->udata.sp.build_tile = hexNum | ((level << 29) & 0xE0000000);
            pScript->pObj = pObj;
        }
        else {
            Fallout_Debug_Error("CreateNew_MapObject - fall_Script_Get failure");
            pObj->scriptID = -1;
        }
    }

    DrawMapChanges(&rect, pObj->level, FLG_Obj | FLG_Hud_Outline);
    return TRUE;
}


//________________________________________________________________________________
BOOL CreateNew_MapTile(LONG xPos, LONG yPos, LONG level, BOOL isRoof, DWORD frmID) {

    LONG tileX = 0;
    LONG tileY = 0;
    LONG tileNum = -1;
    if (SqrToTile_GameOffset(xPos, yPos, &tileX, &tileY))
        tileNum = tileY * *pNUM_TILE_X + tileX;

    if (tileNum == -1)
        return FALSE;

    if (!GameMap_SetTileFrmID(tileNum, level, isRoof, frmID)) {
        Fallout_Debug_Error("GameMap_SetTileFrmID failed: frmID:%X", frmID);
        return FALSE;
    }
    GameArea_Update_Tile_Mapper(tileNum, level);
    RECT rcTile{ 0,0,0,0 };

    TileToSqr_GameOffsets(tileNum, &rcTile.left, &rcTile.top);
    if (isRoof)
        rcTile.top -= 96;
    
    FRMCached* pFrm = new FRMCached(frmID, 0);
    FRMframeDx* pFrame = pFrm->GetFrame(0, 0);
    if (pFrame) {
        rcTile.right = rcTile.left + pFrame->GetWidth();
        rcTile.bottom = rcTile.top + pFrame->GetHeight();
    }
    else {
        rcTile.right = rcTile.left + 80;
        rcTile.bottom = rcTile.top + 36;
    }
    delete pFrm;
    pFrm = nullptr;
    pFrame = nullptr;

    DWORD flags = FLG_Obj | FLG_Floor;
    if (isRoof)
        flags = FLG_Roof;

    DrawMapChanges(&rcTile, level, flags);
    return TRUE;
}


//_______________________________________________________________________________
BOOL CreatNew_MapTileFromTile(LONG tileNum, LONG level, TILE_DATA_node* tileNode) {
    if (!tileNode)
        return FALSE;
    if (tileNum < 0 || tileNum >= *pNUM_TILES)
        return FALSE;
    if (!GameMap_SetTileFrmID(tileNum, level, tileNode->isRoof, tileNode->frmID)) {
        Fallout_Debug_Error("GameMap_SetTileFrmID failed: frmID:%X", tileNode->frmID);
        return FALSE;
    }
    GameArea_Update_Tile_Mapper(tileNum, level);

    RECT rcTile{ 0,0,0,0 };

    TileToSqr_GameOffsets(tileNum, &rcTile.left, &rcTile.top);
    if (tileNode->isRoof)
        rcTile.top -= 96;

    FRMCached* pFrm = new FRMCached(tileNode->frmID, 0);
    FRMframeDx* pFrame = pFrm->GetFrame(0, 0);
    if (pFrame) {
        rcTile.right = rcTile.left + pFrame->GetWidth();
        rcTile.bottom = rcTile.top + pFrame->GetHeight();
    }
    else {
        rcTile.right = rcTile.left + 80;
        rcTile.bottom = rcTile.top + 36;
    }
    delete pFrm;
    pFrm = nullptr;
    pFrame = nullptr;

    DWORD flags = FLG_Obj | FLG_Floor;
    if (tileNode->isRoof)
        flags = FLG_Roof;

    DrawMapChanges(&rcTile, level, flags);
    return TRUE;
}


//________________________________________________________
void Delete_MapTile(LONG tileNum, BOOL isRoof, LONG level) {
    if (tileNum < 0 || tileNum >= *pNUM_TILES)
        return;

    DWORD frmID = (ART_TILES << 24) | 0x00000001;
    if (!GameMap_SetTileFrmID(tileNum, level, isRoof, frmID)) {
        Fallout_Debug_Error("GameMap_SetTileFrmID failed: frmID:%X", frmID);
        return;
    }
    GameArea_Update_Tile_Mapper(tileNum, level);

    GameMap_SetTile_Flags(tileNum, level, isRoof, 0);

    if (tileNum == map_obj_under_mouse.GetTileNumber())
        ObjectUnderMouse_Clear();

    RECT rcTile{ 0,0,0,0 };

    TileToSqr_GameOffsets(tileNum, &rcTile.left, &rcTile.top);
    if (isRoof)
        rcTile.top -= 96;

    FRMCached* pFrm = new FRMCached(frmID, 0);
    FRMframeDx* pFrame = pFrm->GetFrame(0, 0);
    if (pFrame) {
        rcTile.right = rcTile.left + pFrame->GetWidth();
        rcTile.bottom = rcTile.top + pFrame->GetHeight();
    }
    else {
        rcTile.right = rcTile.left + 80;
        rcTile.bottom = rcTile.top + 36;
    }
    delete pFrm;
    pFrm = nullptr;
    pFrame = nullptr;

    DWORD flags = FLG_Obj | FLG_Floor;
    if (isRoof)
        flags = FLG_Roof;

    DrawMapChanges(&rcTile, level, flags);
}


//_______________________________________
void CopiedObject_Delete(OBJStruct* pObj) {

    if (pObj) {
        if (pObj->pud.general.inv_size) {
            for (int i = 0; i < pObj->pud.general.inv_size; i++)
                CopiedObject_Delete(pObj->pud.general.p_item[i].p_obj);
            delete[] pObj->pud.general.p_item;
            pObj->pud.general.p_item = nullptr;
            pObj->pud.general.inv_size = 0;
        }
        delete (OBJStructDx*)pObj;
        pObj = nullptr;
    }
}


//_________________________
void CopiedObjects_Delete() {

    if (pObjs_Copyed) {
        OBJNode* objNode = nullptr;
        while (pObjs_Copyed) {
            if (pObjs_Copyed->pObj)
                CopiedObject_Delete(pObjs_Copyed->pObj);
            pObjs_Copyed->pObj = nullptr;
            objNode = pObjs_Copyed->next;
            pObjs_Copyed->next = nullptr;
            delete pObjs_Copyed;
            pObjs_Copyed = objNode;
        }
        objNode = nullptr;
    }
    pObjs_Copyed = nullptr;
}


//_______________________
void CopiedTiles_Delete() {

    if (pTiles_Copyed) {
        TILE_DATA_node* tileNode = nullptr;
        while (pTiles_Copyed) {
            tileNode = pTiles_Copyed->next;
            pTiles_Copyed->next = nullptr;
            delete pTiles_Copyed;
            pTiles_Copyed = tileNode;
        }
        tileNode = nullptr;
    }
    pTiles_Copyed = nullptr;
}


//__________________________________________________________
void CopiedObject_Create(OBJStruct** lpObj, OBJStruct* pObj) {

    if (pObj) {
        OBJStructDx* pObjC = new OBJStructDx;
        *lpObj = (OBJStruct*)pObjC;
        memcpy(pObjC, pObj, sizeof(OBJStructDx));
        pObjC->frmObjDx = nullptr;
        pObjC->combatFlags &= ~FLG_IsOutlined;

        //pObjC->scriptID = -1;

        pObjC->pud.general.inv_max = 0;
        pObjC->pud.general.inv_size = 0;
        pObjC->pud.general.p_item = nullptr;
        if (pObj->pud.general.inv_size) {
            pObjC->pud.general.inv_max = pObj->pud.general.inv_max;
            pObjC->pud.general.inv_size = pObj->pud.general.inv_size;
            pObjC->pud.general.p_item = new ITEMblock[pObjC->pud.general.inv_max];
            memset(pObjC->pud.general.p_item, 0, sizeof(ITEMblock) * pObjC->pud.general.inv_max);

            for (int i = 0; i < pObj->pud.general.inv_size; i++) {
                pObjC->pud.general.p_item[i].p_obj = nullptr;

                CopiedObject_Create(&pObjC->pud.general.p_item[i].p_obj, pObj->pud.general.p_item[i].p_obj);

                pObjC->pud.general.p_item[i].num = pObj->pud.general.p_item[i].num;
            }
        }
    }
}


//_______________________________
BOOL CopiedObjects_CopySelected() {

    CopiedObjects_Delete();

    if (!pObjs_Selected)
        return FALSE;

    pObjs_Copyed = new OBJNode;
    OBJNode* objNodeS = pObjs_Selected;
    OBJNode* objNodeC = pObjs_Copyed;
    CopiedObject_Create(&objNodeC->pObj, objNodeS->pObj);
    objNodeC->next = nullptr;
    while (objNodeS->next) {
        objNodeS = objNodeS->next;
        objNodeC->next = new OBJNode;
        objNodeC = objNodeC->next;
        objNodeC->pObj = nullptr;
        objNodeC->next = nullptr;
        if (objNodeS->pObj)
            CopiedObject_Create(&objNodeC->pObj, objNodeS->pObj);
    }

    return TRUE;
}


//____________________________________________
BOOL CopiedObjects_CopyObject(OBJStruct* pObj) {

    if (!pObj)
        return FALSE;

    CopiedObjects_Delete();
    if (pObjs_Copyed)
        return FALSE;

    pObjs_Copyed = new OBJNode;
    CopiedObject_Create(&pObjs_Copyed->pObj, pObj);
    pObjs_Copyed->next = nullptr;

    return TRUE;
}


//_____________________________
BOOL CopiedTiles_CopySelected() {

    CopiedTiles_Delete();

    if (!pTiles_Selected)
        return FALSE;

    TILE_DATA_node* tileNodeS = pTiles_Selected;
    TILE_DATA_node* tileNodeC = nullptr;

    while (tileNodeS) {
        if (!pTiles_Copyed) {
            pTiles_Copyed = new TILE_DATA_node;
            tileNodeC = pTiles_Copyed;
        }
        else {
            tileNodeC->next = new TILE_DATA_node;
            tileNodeC = tileNodeC->next;
        }
        tileNodeC->num = tileNodeS->num;
        tileNodeC->frmID = tileNodeS->frmID;
        tileNodeC->isRoof= tileNodeS->isRoof;
        tileNodeC->level = tileNodeS->level;
        tileNodeC->next = nullptr;

        tileNodeS = tileNodeS->next;
    }
    return TRUE;
}


//______________________________________________________________________________
TILE_DATA_node* Create_TileNode_From_Tile(LONG tileNum, BOOL isRoof, LONG level) {
    if (level < 0 || level>2)
        return nullptr;
    if (tileNum < 0 || tileNum >= *pNUM_TILES)
        return nullptr;

    TILE_DATA_node* p_tile = new TILE_DATA_node;

    p_tile->num = tileNum;
    p_tile->frmID = GameMap_GetTileFrmID(tileNum, level, isRoof, nullptr);
    p_tile->isRoof = isRoof;
    p_tile->level = level;
    p_tile->next = nullptr;
    return p_tile;
}


//______________________________________________________________
BOOL CopiedTiles_CopyTile(LONG tileNum, BOOL isRoof, LONG level) {

    CopiedTiles_Delete();
    if (pTiles_Copyed)
        return FALSE;

    pTiles_Copyed = Create_TileNode_From_Tile(tileNum, isRoof, level);// Create_TileNode_From_Tile(tileNum, floor_roof_mask, level);

    return TRUE;
}


//__________________________________________
BOOL Action_Initiate_CopiedObjects_Pasting() {
    SelectedAction_End(TRUE);
    if (Set_PerformingAction(PERFORMING_ACTION::pasting_objects)) 
        return TRUE;
    return FALSE;
}


//____________________________________________________________
BOOL Action_Update_CopiedObjects_Pasting(LONG xPos, LONG yPos) {

    if (!pObjs_Copyed && !pTiles_Copyed)
        return FALSE;

    DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);

    LONG hexX_Mouse = 0;
    LONG hexY_Mouse = 0;
    LONG hexNum_mouse = SqrToHexPos_GameOffset(xPos, yPos, &hexX_Mouse, &hexY_Mouse);

    LONG tileX_Mouse = hexX_Mouse / 2;
    LONG tileY_Mouse = hexY_Mouse / 2;

    //relative mouse offset in tile coordinates
    LONG tileX_rel = 0;
    LONG tileY_rel = 0;
    LONG tileNum_rel = 0;
    //relative mouse offset in hex coordinates
    LONG hexX_rel = 0;
    LONG hexY_rel = 0;
    LONG hexNum_rel = 0;

    //get the relative tile/hex position offset between the mouse and the first valid object in the copy list.
    if (pTiles_Copyed) { // as tiles are 2x2 hexes they are bound to even hex xy positions, all other objects need to move with them to maintain alignment.
        TileNumToTilePos(pTiles_Copyed->num, &tileX_rel, &tileY_rel);
        tileX_rel -= tileX_Mouse;
        tileY_rel -= tileY_Mouse;
        TilePosToTileNum(&tileNum_rel, tileX_rel, tileY_rel);
        hexX_rel = tileX_rel * 2;
        hexY_rel = tileY_rel * 2;
        HexPosToHexNum(&hexNum_rel, hexX_rel, hexY_rel);
    }
    else if (pObjs_Copyed) {
        OBJNode* objNodeC = pObjs_Copyed;
        while (objNodeC) {
            if (objNodeC->pObj) {
                HexNumToHexPos(objNodeC->pObj->hexNum, &hexX_rel, &hexY_rel);
                objNodeC = nullptr;
            }
            else
                objNodeC = objNodeC->next;
        }
        hexX_rel -= hexX_Mouse;
        hexY_rel -= hexY_Mouse;
        HexPosToHexNum(&hexNum_rel, hexX_rel, hexY_rel);
    }

    LONG tileY = 0;
    LONG tileX = 0;
    TILE_DATA_node* tileNodeC = pTiles_Copyed;
    //draw outline of copied tiles
    while (tileNodeC) {
        TileNumToTilePos(tileNodeC->num, &tileX, &tileY);
        tileX -= tileX_rel;
        tileY -= tileY_rel;
        if (tileX >= 0 && tileX < *pNUM_TILE_X && tileY >= 0 && tileY < *pNUM_TILE_Y) 
            GameArea_DrawTileOutline(GameAreas_GetCurrentArea(), tileNodeC->frmID, tileNodeC->num - tileNum_rel, selectedActionLevel, tileNodeC->isRoof, FLG_MapperPasting_Good);
        tileNodeC = tileNodeC->next;
    }

    LONG hexY = 0;
    LONG hexX = 0;
    LONG hexNum_bak = 0;
    DWORD combatFlags_bak = 0;
    OBJNode* objNodeC = pObjs_Copyed;
    //draw outline of copied objects
    while (objNodeC) {
        if (objNodeC->pObj) {
            hexNum_bak = objNodeC->pObj->hexNum;//store objects hex
            if ((hexX_rel & 1) && (objNodeC->pObj->hexNum & 1))//if the new relative hex position is odd and the objects original hex is odd, minus objects hex by one row "200 hexes" to maintain visual alignment.
                objNodeC->pObj->hexNum -= *pNUM_HEX_X;
            HexNumToHexPos(objNodeC->pObj->hexNum, &hexX, &hexY);
            hexX -= hexX_rel;
            hexY -= hexY_rel;
            if (hexX >= 0 && hexX < *pNUM_HEX_X && hexY >= 0 && hexY < *pNUM_HEX_Y) {

                combatFlags_bak = objNodeC->pObj->combatFlags;
                objNodeC->pObj->hexNum -= hexNum_rel;
                if (fall_Map_GetBlockingObjAtPos(nullptr, objNodeC->pObj->hexNum, selectedActionLevel) != nullptr)
                    objNodeC->pObj->combatFlags |= FLG_MapperPasting_Bad;
                else
                    objNodeC->pObj->combatFlags |= FLG_MapperPasting_Good;
                GameArea_DrawObjOutline(GameAreas_GetCurrentArea(), (OBJStruct*)objNodeC->pObj);
                objNodeC->pObj->combatFlags = combatFlags_bak;
            }
            objNodeC->pObj->hexNum = hexNum_bak;//restore objects hex
        }
        objNodeC = objNodeC->next;
    }

    return TRUE;
}


//_____________________________________________________________________________
BOOL Action_End_CopiedObjects_Pasting(LONG xPos, LONG yPos, BOOL cancel_action) {

    BOOL continue_action = FALSE;
    if (!pObjs_Copyed && !pTiles_Copyed)
        return continue_action;

    DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);

    if (cancel_action)
        return continue_action;

    LONG hexX_Mouse = 0;
    LONG hexY_Mouse = 0;
    LONG hexNum_mouse = SqrToHexPos_GameOffset(xPos, yPos, &hexX_Mouse, &hexY_Mouse);

    LONG tileX_Mouse = hexX_Mouse / 2;
    LONG tileY_Mouse = hexY_Mouse / 2;

    //relative mouse offset in tile coordinates
    LONG tileX_rel = 0;
    LONG tileY_rel = 0;
    LONG tileNum_rel = 0;
    //relative mouse offset in hex coordinates
    LONG hexX_rel = 0;
    LONG hexY_rel = 0;
    LONG hexNum_rel = 0;

    //get the relative tile/hex position offset between the mouse and the first valid object in the copy list.
    if (pTiles_Copyed) { // as tiles are 2x2 hexes they are bound to even hex xy positions, all other objects need to move with them to maintain alignment.
        TileNumToTilePos(pTiles_Copyed->num, &tileX_rel, &tileY_rel);
        tileX_rel -= tileX_Mouse;
        tileY_rel -= tileY_Mouse;
        TilePosToTileNum(&tileNum_rel, tileX_rel, tileY_rel);
        hexX_rel = tileX_rel * 2;
        hexY_rel = tileY_rel * 2;
        HexPosToHexNum(&hexNum_rel, hexX_rel, hexY_rel);
    }
    else if (pObjs_Copyed) {
        OBJNode* objNodeC = pObjs_Copyed;
        while (objNodeC) {
            if (objNodeC->pObj) {
                HexNumToHexPos(objNodeC->pObj->hexNum, &hexX_rel, &hexY_rel);
                objNodeC = nullptr;
            }
            else
                objNodeC = objNodeC->next;
        }
        hexX_rel -= hexX_Mouse;
        hexY_rel -= hexY_Mouse;
        HexPosToHexNum(&hexNum_rel, hexX_rel, hexY_rel);
    }

    LONG tileY = 0;
    LONG tileX = 0;
    TILE_DATA_node* tileNodeC = pTiles_Copyed;
    //paste copied tiles to map
    while (tileNodeC) {
        TileNumToTilePos(tileNodeC->num, &tileX, &tileY);
        tileX -= tileX_rel;
        tileY -= tileY_rel;
        if (tileX >= 0 && tileX < *pNUM_TILE_X && tileY >= 0 && tileY < *pNUM_TILE_Y)
            CreatNew_MapTileFromTile(tileNodeC->num - tileNum_rel, selectedActionLevel, tileNodeC);
        tileNodeC = tileNodeC->next;
    }

    LONG hexY = 0;
    LONG hexX = 0;
    LONG hexNum_bak = 0;
    OBJNode* objNodeC = pObjs_Copyed;
    OBJStruct* pObj_new = nullptr;
    //paste copied objects to map
    while (objNodeC) {
        if (objNodeC->pObj) {
            hexNum_bak = objNodeC->pObj->hexNum;//store objects hex
            if ((hexX_rel & 1) && (objNodeC->pObj->hexNum & 1))//if the new relative hex position is odd and the objects original hex is odd, minus objects hex by one row "200 hexes" to maintain visual alignment.
                objNodeC->pObj->hexNum -= *pNUM_HEX_X;
            HexNumToHexPos(objNodeC->pObj->hexNum, &hexX, &hexY);
            hexX -= hexX_rel;
            hexY -= hexY_rel;
            if (hexX >= 0 && hexX < *pNUM_HEX_X && hexY >= 0 && hexY < *pNUM_HEX_Y) {
                if (!CreateNew_MapObjectFromObject(objNodeC->pObj->hexNum - hexNum_rel, selectedActionLevel, objNodeC->pObj, TRUE)) {
                    Fallout_Debug_Error("Action_End_CopiedObjects_Pasting - error copying object");
                    return continue_action;
                }
            }
            objNodeC->pObj->hexNum = hexNum_bak;//restore objects hex
        }
        objNodeC = objNodeC->next;
    }


    if (GetAsyncKeyState(VK_CONTROL) & 0x80000000)
        continue_action = TRUE;

    return continue_action;
}


//____________________
void CopyLevelDelete() {

    if (pObjs_LevelCopyed) {
        OBJNode* objNode = nullptr;
        while (pObjs_LevelCopyed) {
            if (pObjs_LevelCopyed->pObj)
                CopiedObject_Delete(pObjs_LevelCopyed->pObj);
            pObjs_LevelCopyed->pObj = nullptr;
            objNode = pObjs_LevelCopyed->next;
            pObjs_LevelCopyed->next = nullptr;
            delete pObjs_LevelCopyed;
            pObjs_LevelCopyed = objNode;
        }
        objNode = nullptr;
    }
    pObjs_LevelCopyed = nullptr;


    if (pTiles_LevelCopyed) {
        TILE_DATA_node* tileNode = nullptr;
        while (pTiles_LevelCopyed) {
            tileNode = pTiles_LevelCopyed->next;
            pTiles_LevelCopyed->next = nullptr;
            delete pTiles_LevelCopyed;
            pTiles_LevelCopyed = tileNode;
        }
        tileNode = nullptr;
    }
    pTiles_LevelCopyed = nullptr;
}


//_____________________________________________________________________________________
BOOL ListObjectsInRect(POINT* p_start, POINT* p_end, LONG level, OBJNode** lpObjs_List) {
    if (!lpObjs_List)
        return FALSE;
    if (!p_start)
        return FALSE;
    if (!p_end)
        return FALSE;


    RECT rect{ 0,0,0,0 };

    if (Get_SelectionRectType() == SEL_RC_TYPE::isometric) {
        //convert square mouse start and end points to a rect struture in isometric space
        POINT mouse_iso_1{ 0,0 };
        SqrToHexPos_GameOffset(p_start->x, p_start->y, &mouse_iso_1.x, &mouse_iso_1.y);
        POINT mouse_iso_2{ 0,0 };
        SqrToHexPos_GameOffset(p_end->x, p_end->y, &mouse_iso_2.x, &mouse_iso_2.y);

        rect = { mouse_iso_1.x, mouse_iso_1.y, mouse_iso_2.x, mouse_iso_2.y };
        //arrange from lowest to highest along the x,y axes
        if (mouse_iso_1.x > mouse_iso_2.x) {
            rect.left = mouse_iso_2.x;
            rect.right = mouse_iso_1.x;
        }
        if (mouse_iso_1.y > mouse_iso_2.y) {
            rect.top = mouse_iso_2.y;
            rect.bottom = mouse_iso_1.y;
        }
    }
    else {
        rect = { p_start->x, p_start->y, p_end->x, p_end->y };
        //arrange from lowest to highest along the x,y axes
        if (p_end->x < p_start->x) {
            rect.left = p_end->x;
            rect.right = p_start->x;
        }
        if (p_end->y < p_start->y) {
            rect.top = p_end->y;
            rect.bottom = p_start->y;
        }
    }

    OBJNode* mapObj = nullptr;
    bool skipObj = false;

    LONG hexNum = 0;
    LONG xPos = 0;
    LONG yPos = 0;

    OBJNode* objNode = nullptr;

    for (hexNum = 0; hexNum < *pNUM_HEXES; hexNum++) {

        if (Get_SelectionRectType() == SEL_RC_TYPE::isometric)
            HexNumToHexPos(hexNum, &xPos, &yPos);
        else
            HexNumToSqr(hexNum, &xPos, &yPos);

        if (xPos > rect.left && xPos < rect.right && yPos > rect.top && yPos < rect.bottom) {
            mapObj = pMapObjNodeArray[hexNum];

            while (mapObj) {
                if (mapObj->pObj->level <= level) {
                    if (mapObj->pObj->level == level) {
                        if (mapObj->pObj != *ppObj_PC && mapObj->pObj != *ppObj_Egg && mapObj->pObj != *ppObj_Mouse && mapObj->pObj != *ppObj_Selected && IsObjectTypeSelectable((mapObj->pObj->proID >> 24))) {

                            if (!*lpObjs_List) {
                                *lpObjs_List = new OBJNode;
                                objNode = *lpObjs_List;
                                objNode->pObj = mapObj->pObj;
                                if (objNode->pObj) {
                                    if (selectObject_isRemoving && (objNode->pObj->combatFlags & FLG_MapperSelected))
                                        objNode->pObj->combatFlags |= FLG_MapperSelecting;
                                    else if (!selectObject_isRemoving && !(objNode->pObj->combatFlags & FLG_MapperSelected))
                                        objNode->pObj->combatFlags |= FLG_MapperSelecting;
                                }
                                objNode->next = nullptr;
                            }
                            else {
                                skipObj = false;
                                objNode = *lpObjs_List;
                                while (objNode->next) {
                                    if (mapObj->pObj == objNode->pObj)
                                        skipObj = true;
                                    objNode = objNode->next;
                                }
                                if (!skipObj) {
                                    objNode->next = new OBJNode;
                                    objNode = objNode->next;
                                    objNode->pObj = mapObj->pObj;
                                    if (objNode->pObj) {
                                        if (selectObject_isRemoving && (objNode->pObj->combatFlags & FLG_MapperSelected))
                                            objNode->pObj->combatFlags |= FLG_MapperSelecting;
                                        else if (!selectObject_isRemoving && !(objNode->pObj->combatFlags & FLG_MapperSelected))
                                            objNode->pObj->combatFlags |= FLG_MapperSelecting;
                                    }
                                    objNode->next = nullptr;
                                }
                            }
                        }
                    }
                }
                else
                    mapObj = nullptr;
                if (mapObj)
                    mapObj = mapObj->next;
            }
        }
    }

    return TRUE;
}


//________________________________________________________
BOOL ListObjectsOnLevel(LONG level, OBJNode** lpObjs_List) {
    if (!lpObjs_List)
        return FALSE;

    OBJNode* mapObj = nullptr;
    bool skipObj = false;

    LONG hexPos = 0;

    OBJNode* objNode = nullptr;

    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];

        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (mapObj->pObj != *ppObj_PC && mapObj->pObj != *ppObj_Egg && mapObj->pObj != *ppObj_Mouse && mapObj->pObj != *ppObj_Selected && !(mapObj->pObj->flags & FLG_Unk00000400) && mapObj->pObj->proID != -1){//}&& IsObjectTypeSelectable((mapObj->pObj->proID >> 24))) {

                        if (!*lpObjs_List) {
                            *lpObjs_List = new OBJNode;
                            objNode = *lpObjs_List;
                            objNode->pObj = mapObj->pObj;
                            objNode->next = nullptr;
                        }
                        else {
                            skipObj = false;
                            objNode = *lpObjs_List;
                            while (objNode->next) {
                                if (mapObj->pObj == objNode->pObj)
                                    skipObj = true;
                                objNode = objNode->next;
                            }
                            if (!skipObj) {
                                objNode->next = new OBJNode;
                                objNode = objNode->next;
                                objNode->pObj = mapObj->pObj;
                                objNode->next = nullptr;
                            }
                        }
                    }
                }
            }
            else
                mapObj = nullptr;
            if (mapObj)
                mapObj = mapObj->next;
        }
    }

    return TRUE;
}


//___________________________________________________________________________________________
BOOL ListTilesInRect(POINT* p_start, POINT* p_end, LONG level, TILE_DATA_node** lpTiles_List) {
    if (!lpTiles_List)
        return FALSE;
    if (!p_start)
        return FALSE;
    if (!p_end)
        return FALSE;
    if (!IsObjectTypeSelectable(ART_TILES))
        return FALSE;

    RECT rect{ 0,0,0,0 };

    if (Get_SelectionRectType() == SEL_RC_TYPE::isometric) {
        //convert square mouse start and end points to a rect struture in isometric space
        POINT mouse_iso_1{ 0,0 };
        SqrToTile_GameOffset(p_start->x, p_start->y, &mouse_iso_1.x, &mouse_iso_1.y);
        POINT mouse_iso_2{ 0,0 };
        SqrToTile_GameOffset(p_end->x, p_end->y, &mouse_iso_2.x, &mouse_iso_2.y);

        rect = { mouse_iso_1.x, mouse_iso_1.y, mouse_iso_2.x, mouse_iso_2.y };
        //arrange from lowest to highest along the x,y axes
        if (mouse_iso_1.x > mouse_iso_2.x) {
            rect.left = mouse_iso_2.x;
            rect.right = mouse_iso_1.x;
        }
        if (mouse_iso_1.y > mouse_iso_2.y) {
            rect.top = mouse_iso_2.y;
            rect.bottom = mouse_iso_1.y;
        }
    }
    else {
        rect = { p_start->x, p_start->y, p_end->x, p_end->y };
        //arrange from lowest to highest along the x,y axes
        if (p_end->x < p_start->x) {
            rect.left = p_end->x;
            rect.right = p_start->x;
        }
        if (p_end->y < p_start->y) {
            rect.top = p_end->y;
            rect.bottom = p_start->y;
        }
        rect.left += 64;
        rect.top += 12;
        rect.right -= 16;
        rect.bottom -= 24;
    }

    BOOL roof_tiles_selectable = IsObjectTypeSelectable(ART_TILES, TRUE);
    BOOL floor_tiles_selectable = IsObjectTypeSelectable(ART_TILES, FALSE);


    TILE_DATA_node* tileNode = nullptr;

    bool skipObj = false;
    LONG xPos = 0;
    LONG yPos = 0;

    DWORD frmID = 1;
    DWORD combatFlags = 0;

    for (LONG tileNum = 0; tileNum < *pNUM_TILES; tileNum++) {

        if (Get_SelectionRectType() == SEL_RC_TYPE::isometric)
            TileNumToTilePos(tileNum, &xPos, &yPos);
        else
            TileToSqr(tileNum, &xPos, &yPos);

        if (xPos > rect.left && xPos < rect.right && yPos > rect.top && yPos < rect.bottom) {
            for (BOOL isRoof = FALSE; isRoof < TRUE + TRUE; isRoof += TRUE) {
                frmID = GameMap_GetTileFrmID(tileNum, level, isRoof, nullptr);
                if ((frmID & 0x00FFFFFF) != 1 && ((floor_tiles_selectable && !isRoof) || roof_tiles_selectable && isRoof)) {
                    if (!*lpTiles_List) {
                        *lpTiles_List = new TILE_DATA_node;
                        tileNode = *lpTiles_List;
                        tileNode->level = level;
                        tileNode->num = tileNum;
                        tileNode->isRoof = isRoof;
                        tileNode->frmID = frmID;

                        combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
                        if (selectObject_isRemoving && (combatFlags & FLG_MapperSelected))
                            combatFlags |= FLG_MapperSelecting;
                        else if (!selectObject_isRemoving && !(combatFlags & FLG_MapperSelected))
                            combatFlags |= FLG_MapperSelecting;
                        GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);

                        tileNode->next = nullptr;
                    }
                    else {
                        skipObj = false;
                        tileNode = *lpTiles_List;
                        while (tileNode->next) {
                            if (tileNum == tileNode->num && level == tileNode->level && tileNode->isRoof == isRoof)
                                skipObj = true;
                            tileNode = tileNode->next;
                        }
                        if (!skipObj) {
                            tileNode->next = new TILE_DATA_node;
                            tileNode = tileNode->next;
                            tileNode->level = level;
                            tileNode->num = tileNum;
                            tileNode->isRoof = isRoof;
                            tileNode->frmID = frmID;

                            combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
                            if (selectObject_isRemoving && (combatFlags & FLG_MapperSelected))
                                combatFlags |= FLG_MapperSelecting;
                            else if (!selectObject_isRemoving && !(combatFlags & FLG_MapperSelected))
                                combatFlags |= FLG_MapperSelecting;
                            GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);

                            tileNode->next = nullptr;
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}


//______________________________________________________________
BOOL ListTilesOnLevel(LONG level, TILE_DATA_node** lpTiles_List) {
    if (!lpTiles_List)
        return FALSE;

    if (!IsObjectTypeSelectable(ART_TILES))
        return FALSE;

    BOOL roof_tiles_selectable = IsObjectTypeSelectable(ART_TILES, TRUE);
    BOOL floor_tiles_selectable = IsObjectTypeSelectable(ART_TILES, FALSE);

    TILE_DATA_node* tileNode = nullptr;

    DWORD frmID = 1;
    bool skipObj = false;

    for (LONG tileNum = 0; tileNum < *pNUM_TILES; tileNum++) {
        for (BOOL isRoof = FALSE; isRoof < TRUE + TRUE; isRoof += TRUE) {
            frmID = GameMap_GetTileFrmID(tileNum, level, isRoof, nullptr);
            if ((frmID & 0x00FFFFFF) != 1 && ((floor_tiles_selectable && !isRoof) || roof_tiles_selectable && isRoof)) {
                if (!*lpTiles_List) {
                    *lpTiles_List = new TILE_DATA_node;
                    tileNode = *lpTiles_List;
                    tileNode->level = level;
                    tileNode->num = tileNum;
                    tileNode->isRoof = isRoof;
                    tileNode->frmID = frmID;
                    tileNode->next = nullptr;
                }
                else {
                    skipObj = false;
                    tileNode = *lpTiles_List;
                    while (tileNode->next) {
                        if (tileNum == tileNode->num && level == tileNode->level && tileNode->isRoof == isRoof)
                            skipObj = true;
                        tileNode = tileNode->next;
                    }
                    if (!skipObj) {
                        tileNode->next = new TILE_DATA_node;
                        tileNode = tileNode->next;
                        tileNode->level = level;
                        tileNode->num = tileNum;
                        tileNode->isRoof = isRoof;
                        tileNode->frmID = frmID;
                        tileNode->next = nullptr;
                    }
                }
            }
        }
    }
    return TRUE;
}


//_____________________________
BOOL CopyLevel_Copy(LONG level) {

    CopyLevelDelete();
    //LONG count = 1;
    OBJNode* pObjs_Level = nullptr;
    ListObjectsOnLevel(level, &pObjs_Level);
    if (pObjs_Level) {
        pObjs_LevelCopyed = new OBJNode;
        OBJNode* objNodeS = pObjs_Level;
        OBJNode* objNodeC = pObjs_LevelCopyed;
        CopiedObject_Create(&objNodeC->pObj, objNodeS->pObj);
        objNodeC->next = nullptr;
        while (objNodeS->next) {
            objNodeS = objNodeS->next;
            objNodeC->next = new OBJNode;
            objNodeC = objNodeC->next;
            objNodeC->pObj = nullptr;
            objNodeC->next = nullptr;
            if (objNodeS->pObj) {
                CopiedObject_Create(&objNodeC->pObj, objNodeS->pObj);
                //count++;
            }
        }
        //Fallout_Debug_Info("CopyLevel_Copy num obj:%d", count);
        OBJNode* objNode = nullptr;
        while (pObjs_Level) {
            pObjs_Level->pObj = nullptr;
            objNode = pObjs_Level->next;
            pObjs_Level->next = nullptr;
            delete pObjs_Level;
            pObjs_Level = objNode;
        }
        objNode = nullptr;
    }
    pObjs_Level = nullptr;

    ListTilesOnLevel(level, &pTiles_LevelCopyed);

    return TRUE;
}


//______________________________
BOOL CopyLevel_Paste(LONG level) {

    if (!pObjs_LevelCopyed && !pTiles_LevelCopyed)
        return FALSE;

    MapChanges_Update();

    //destroy existing objects on level
    OBJStruct* pObj = GetNextEnabledObject(TRUE, level);
    while (pObj != nullptr) {
        if (pObj != *ppObj_PC && pObj != *ppObj_Mouse && pObj != *ppObj_Selected && !(pObj->flags & FLG_Unk00000400)) {
            fall_Obj_Destroy(pObj, nullptr);
            pObj = GetNextEnabledObject(TRUE, level);
        }
        else
            pObj = GetNextEnabledObject(FALSE, level);
    }

    //paste copied tiles
    TILE_DATA_node* tileNodeC = pTiles_LevelCopyed;
    while (tileNodeC) {
        CreatNew_MapTileFromTile(tileNodeC->num, level, tileNodeC);
        tileNodeC = tileNodeC->next;
    }

    //paste copied objects
    OBJNode* objNodeC = pObjs_LevelCopyed;
    //LONG count = 0;
    while (objNodeC) {
        if (objNodeC->pObj) {
            OBJStruct* pObj = nullptr;
            if (!CreateNew_MapObjectFromObject(objNodeC->pObj->hexNum, level, objNodeC->pObj, TRUE)) {
                Fallout_Debug_Error("CopyLevel_Paste - error copying object");
                return FALSE;
            }
            //else
            //count++;
        }
        objNodeC = objNodeC->next;
    }
    //Fallout_Debug_Info("CopyLevel_Paste num obj:%d", count);
    return TRUE;
}


//_______________________________________________________________________________
BOOL SelectTile_Add(TILE_DATA_node* tileNodeNew, BOOL update_selected_list_state) {
    if (!tileNodeNew)
        return FALSE;

    TILE_DATA_node* tileNode = nullptr;

    DWORD combatFlags = 0;


        combatFlags = GameMap_GetTile_Flags(tileNodeNew->num, tileNodeNew->level, tileNodeNew->isRoof);
        combatFlags |= FLG_MapperSelected;
        GameMap_SetTile_Flags(tileNodeNew->num, tileNodeNew->level, tileNodeNew->isRoof, combatFlags);
    


    if (!pTiles_Selected) {
        pTiles_Selected = new TILE_DATA_node;
        tileNode = pTiles_Selected;
    }
    else {
        tileNode = pTiles_Selected;

        TILE_DATA_node* tileNodeLast = nullptr;
        while (tileNode != nullptr) {
            if (tileNodeNew->num == tileNode->num && tileNodeNew->level == tileNode->level && tileNodeNew->isRoof == tileNode->isRoof)
                return FALSE;
            tileNodeLast = tileNode;
            tileNode = tileNode->next;
        }
        tileNodeLast->next = new TILE_DATA_node;
        tileNode = tileNodeLast->next;
    }

    tileNode->level = tileNodeNew->level;
    tileNode->num = tileNodeNew->num;
    tileNode->isRoof = tileNodeNew->isRoof;
    tileNode->frmID = tileNodeNew->frmID;
    tileNode->next = nullptr;

    if (update_selected_list_state)
        Selected_List_Update_State();
    return TRUE;
}


//______________________________________________________________________________________________________
BOOL SelectTile_Add(LONG tileNum, BOOL isRoof, LONG level, DWORD frmID, BOOL update_selected_list_state) {

    TILE_DATA_node* tileNode = nullptr;
    DWORD combatFlags = 0;


    combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
    combatFlags |= FLG_MapperSelected;
    GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);



    if (!pTiles_Selected) {
        pTiles_Selected = new TILE_DATA_node;
        tileNode = pTiles_Selected;
    }
    else {
        tileNode = pTiles_Selected;

        TILE_DATA_node* tileNodeLast = nullptr;
        while (tileNode != nullptr) {
            if (tileNum == tileNode->num && level == tileNode->level && isRoof == tileNode->isRoof)
                return FALSE;
            tileNodeLast = tileNode;
            tileNode = tileNode->next;
        }
        tileNodeLast->next = new TILE_DATA_node;
        tileNode = tileNodeLast->next;
    }

    tileNode->level = level;
    tileNode->num = tileNum;
    tileNode->isRoof = isRoof;
    tileNode->frmID = frmID;
    tileNode->next = nullptr;

    if (update_selected_list_state)
        Selected_List_Update_State();
    return TRUE;
}


//__________________________________________________________________________________
BOOL SelectTile_Remove(TILE_DATA_node* tileNodeNew, BOOL update_selected_list_state) {
    if (!tileNodeNew)
        return FALSE;

    DWORD combatFlags = GameMap_GetTile_Flags(tileNodeNew->num, tileNodeNew->level, tileNodeNew->isRoof);
    combatFlags &= ~FLG_MapperSelected;
    GameMap_SetTile_Flags(tileNodeNew->num, tileNodeNew->level, tileNodeNew->isRoof, combatFlags);

    if (tileNodeNew->num == map_obj_selected_focus.GetTileNumber() && tileNodeNew->level == map_obj_selected_focus.GetLevel() && map_obj_selected_focus.IsRoof() == FALSE)
        Selected_List_ClearFocus();

    TILE_DATA_node* tileNode = nullptr;
    TILE_DATA_node* tileNodePrev = nullptr;

    tileNode = pTiles_Selected;

    while (tileNode != nullptr) {
        if (tileNodeNew->num == tileNode->num && tileNodeNew->level == tileNode->level && tileNodeNew->isRoof == tileNode->isRoof) {
            if (tileNode == pTiles_Selected)
                pTiles_Selected = tileNode->next;
            if (tileNodePrev)
                tileNodePrev->next = tileNode->next;
            tileNode->next = nullptr;
            delete tileNode;
            tileNode = nullptr;
            if (update_selected_list_state)
                Selected_List_Update_State();
            return TRUE;
        }
        else {
            tileNodePrev = tileNode;
            tileNode = tileNode->next;
        }
    }
    return FALSE;
}


//____________________________________________________________________________________________
BOOL SelectTile_Remove(LONG tileNum, BOOL isRoof, LONG level, BOOL update_selected_list_state) {


    DWORD combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
    combatFlags &= ~FLG_MapperSelected;
    GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);

    if (tileNum == map_obj_selected_focus.GetTileNumber() && level == map_obj_selected_focus.GetLevel() && map_obj_selected_focus.IsRoof() == FALSE)
        Selected_List_ClearFocus();

    TILE_DATA_node* tileNode = nullptr;
    TILE_DATA_node* tileNodePrev = nullptr;

    tileNode = pTiles_Selected;

    while (tileNode != nullptr) {
        if (tileNum == tileNode->num && level == tileNode->level && isRoof == tileNode->isRoof) {
            if (tileNode == pTiles_Selected)
                pTiles_Selected = tileNode->next;
            if (tileNodePrev)
                tileNodePrev->next = tileNode->next;
            tileNode->next = nullptr;
            delete tileNode;
            tileNode = nullptr;
            if (update_selected_list_state)
                Selected_List_Update_State();
            return TRUE;
        }
        else {
            tileNodePrev = tileNode;
            tileNode = tileNode->next;
        }
    }
    return FALSE;
}


//_____________________________
void Selected_List_ClearFocus() {

    RECT rect{ 0,0,0,0 };
    bool draw_rect = false;

    OBJStruct* pObj = map_obj_selected_focus.GetObj();
    LONG tileNum = map_obj_selected_focus.GetTileNumber();

    if (pObj) {
        //remove the outline colour from object
        pObj->combatFlags &= ~FLG_MapperSelected_Focus;

        //draw object if it is on the current level.
        if (pObj->level == *pMAP_LEVEL) {
            if (GetObjRectDx(pObj, &rect))
                draw_rect = true;
        }
    }
    else if (tileNum != -1) {
        LONG level = map_obj_selected_focus.GetLevel();
        BOOL isRoof = map_obj_selected_focus.IsRoof();
        DWORD combatFlags = 0;
        combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
        //remove the outline colour from tile
        combatFlags &= ~FLG_MapperSelected_Focus;
        GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);
        
        //draw tile if it is on the current level.
        if (level == *pMAP_LEVEL) {
            TileToSqr_GameOffsets(tileNum, &rect.left, &rect.top);
            if (isRoof)
                rect.top -= 96;
            FRMframeDx* pFrame = GameMap_GetTile_Frame(tileNum, level, isRoof);

            if (pFrame) {
                rect.right = rect.left + pFrame->GetWidth();
                rect.bottom = rect.top + pFrame->GetHeight();
            }
            else {
                rect.right = rect.left + 80;
                rect.bottom = rect.top + 36;
            }
            pFrame = nullptr;
            draw_rect = true;
        }
    }

    if (draw_rect)
        DrawMapChanges(&rect, *pMAP_LEVEL, FLG_Hud_Outline);

    //clear focus map object vars.
    map_obj_selected_focus.Clear();
}


//__________________________________________________
BOOL Selected_List_SetFocus(MAP_OBJECT* p_mapObject) {
    //clear previous focus in selected list
    Selected_List_ClearFocus();

    if (!p_mapObject)
        return FALSE;

    RECT rect{ 0,0,0,0 };
    bool draw_rect = false;

    OBJStruct* pObj = p_mapObject->GetObj();
    LONG tileNum = p_mapObject->GetTileNumber();

    if (pObj) {
        //object must be part of the selected list to get focus.
        if (!(pObj->combatFlags & FLG_MapperSelected))
            return FALSE;
        map_obj_selected_focus.Set(pObj);
        //set the outline colour of object
        pObj->combatFlags |= FLG_MapperSelected_Focus;

        //draw object if it is on the current level.
        if (pObj->level == *pMAP_LEVEL) {
            if (GetObjRectDx(pObj, &rect))
                draw_rect = true;
        }

    }
    else if (tileNum != -1) {
        LONG level = p_mapObject->GetLevel();
        BOOL isRoof = p_mapObject->IsRoof();
        DWORD combatFlags = 0;
        combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
        
        //tile must be part of the selected list to get focus.
        if (!(combatFlags & FLG_MapperSelected))
            return FALSE;
        map_obj_selected_focus.Set(tileNum, isRoof, level);
        //set the outline colour of tile
        combatFlags |= FLG_MapperSelected_Focus;
        GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);

        //draw tile if it is on the current level.
        if (level == *pMAP_LEVEL) {
            TileToSqr_GameOffsets(tileNum, &rect.left, &rect.top);
            if (isRoof)
                rect.top -= 96;
            FRMframeDx* pFrame = GameMap_GetTile_Frame(tileNum, level, isRoof);

            if (pFrame) {
                rect.right = rect.left + pFrame->GetWidth();
                rect.bottom = rect.top + pFrame->GetHeight();
            }
            else {
                rect.right = rect.left + 80;
                rect.bottom = rect.top + 36;
            }
            pFrame = nullptr;
            draw_rect = true;
        }
    }
    else//invalid map object
        return FALSE;
    
    if(draw_rect)
        DrawMapChanges(&rect, *pMAP_LEVEL, FLG_Hud_Outline);
    return TRUE;
}


//_____________________________________________________________________
BOOL SelectObject_Add(OBJStruct* pObj, BOOL update_selected_list_state) {
    if (!pObj)
        return FALSE;

    //set the outline colour from object
    pObj->combatFlags |= FLG_MapperSelected;

    OBJNode* objNode = nullptr;

    if (!pObjs_Selected) {
        pObjs_Selected = new OBJNode;
        objNode = pObjs_Selected;
    }
    else {
        objNode = pObjs_Selected;
        while (objNode->next != nullptr) {
            if (pObj == objNode->pObj)
                return FALSE;
            objNode = objNode->next;
        }
        if (pObj == objNode->pObj)
            return FALSE;
        objNode->next = new OBJNode;
        objNode = objNode->next;
    }

    objNode->pObj = pObj;
    objNode->next = nullptr;

    if (update_selected_list_state)
        Selected_List_Update_State();
    return TRUE;
}


//________________________________________________________________________
BOOL SelectObject_Remove(OBJStruct* pObj, BOOL update_selected_list_state) {
    if (!pObj)
        return FALSE;

    //remove the outline colour from object
    pObj->combatFlags &= ~FLG_MapperSelected;
    
    //clear the focus object if is this object.
    if (pObj == map_obj_selected_focus.GetObj())
        Selected_List_ClearFocus();

    OBJNode* objNode = nullptr;

    if (pObjs_Selected) {
        objNode = pObjs_Selected;
        if (pObj == objNode->pObj) {
            pObjs_Selected = objNode->next;
            objNode->pObj = nullptr;
            objNode->next = nullptr;
            delete objNode;
            if (update_selected_list_state)
                Selected_List_Update_State();
            return TRUE;
        }

        OBJNode* objNodePrev = pObjs_Selected;
        objNode = pObjs_Selected->next;
        while (objNode != nullptr) {
            if (pObj == objNode->pObj) {
                objNodePrev->next = objNode->next;
                objNode->pObj = nullptr;
                objNode->next = nullptr;
                delete objNode;
                if (update_selected_list_state)
                    Selected_List_Update_State();
                return TRUE;
            }
            else {
                objNodePrev = objNode;
                objNode = objNode->next;
            }
        }
    }
    return FALSE;
}


//_______________________________
void Selected_List_Update_State() {
    BOOL isEnabled = FALSE;
    if (pObjs_Selected || pTiles_Selected)
        isEnabled = TRUE;
    Tool_SetState_ObjectsSelected(*phWinMain, isEnabled, TRUE);
    ObjectList_Refresh(pObjs_Selected, pTiles_Selected);
}


//_____________________________________________
void Selected_List_Destroy(BOOL destroy_object) {

    Selected_List_ClearFocus();
    if (pObjs_Selected) {
        OBJNode* objNode = nullptr;
        while (pObjs_Selected) {
            if (pObjs_Selected->pObj) {
                if (destroy_object) {
                    Delete_MapObj(&pObjs_Selected->pObj);
                }
                else
                    pObjs_Selected->pObj->combatFlags &= ~FLG_MapperSelected;
            }
            pObjs_Selected->pObj = nullptr;
            objNode = pObjs_Selected;
            pObjs_Selected = objNode->next;
            objNode->next = nullptr;
            delete objNode;
        }
    }
    if (pTiles_Selected) {
        DWORD combatFlags = 0;
        TILE_DATA_node* tileNode = nullptr;
        while (pTiles_Selected) {
            if (destroy_object)
                Delete_MapTile(pTiles_Selected->num, pTiles_Selected->isRoof, pTiles_Selected->level);
            else {
                combatFlags = GameMap_GetTile_Flags(pTiles_Selected->num, pTiles_Selected->level, pTiles_Selected->isRoof);
                combatFlags &= ~FLG_MapperSelected;
                GameMap_SetTile_Flags(pTiles_Selected->num, pTiles_Selected->level, pTiles_Selected->isRoof, combatFlags);
            }
            tileNode = pTiles_Selected;
            pTiles_Selected = tileNode->next;
            tileNode->next = nullptr;
            delete tileNode;
        }
    }

    Selected_List_Update_State();
}


//__________________________________________________________________________________________________________________________________________________
BOOL ListObjects_On_Map(LONG level, OBJNode** lpObjs_List, OBJ_SEARCH search_for, DWORD test_data, DWORD outline_flags, BOOL ignore_type_selectable) {
    //search whole map if level == -1
    if (!lpObjs_List)
        return FALSE;

    OBJNode* mapObj = nullptr;
    OBJNode* objNode = nullptr;

    bool skipObj = false;
    bool proccess_object = true;

    for (LONG hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];

        while (mapObj) {
            if (level == -1 || mapObj->pObj->level <= level) {
                if (level == -1 || mapObj->pObj->level == level) {
                    if (mapObj->pObj != *ppObj_PC && mapObj->pObj != *ppObj_Egg && mapObj->pObj != *ppObj_Mouse && mapObj->pObj != *ppObj_Selected && (ignore_type_selectable || IsObjectTypeSelectable((mapObj->pObj->proID >> 24)))) {
                        proccess_object = false;
                        switch (search_for) {
                        case OBJ_SEARCH::matching_proID:
                            //test_data contains proID
                            if (mapObj->pObj->proID == test_data)
                                proccess_object = true;
                            break;
                        case OBJ_SEARCH::matching_scriptIndex:
                            //test_data contains script lst index
                            if (mapObj->pObj->scriptIndex != -1 && mapObj->pObj->scriptIndex == test_data) {
                                proccess_object = true;
                            }
                            break;
                        case OBJ_SEARCH::has_scriptIndex:
                            if (mapObj->pObj->scriptIndex != -1)
                                proccess_object = true;
                            break;
                        case OBJ_SEARCH::has_scriptID:
                            //test_data contains script type
                            if (mapObj->pObj->scriptID != -1) {
                                if (test_data == -1 || mapObj->pObj->scriptID >> 24 == test_data)
                                    proccess_object = true;
                            }
                            break;
                        case OBJ_SEARCH::has_inventory:
                            if (mapObj->pObj->pud.general.inv_size > 0)
                                proccess_object = true;
                            break;
                        case OBJ_SEARCH::emits_light:
                            if (mapObj->pObj->light_intensity > 0 && mapObj->pObj->light_radius > 0)
                                proccess_object = true;
                            break;
                        case OBJ_SEARCH::matching_frmID:
                            //test_data contains frmID
                            if (mapObj->pObj->frmID == test_data)
                                proccess_object = true;
                            break;
                        default:
                            break;
                        }

                        if (proccess_object) {
                            //Fallout_Debug_Info("ListObjects_On_Map: obj found!");
                            mapObj->pObj->combatFlags |= outline_flags;
                            if (!*lpObjs_List) {
                                *lpObjs_List = new OBJNode;
                                objNode = *lpObjs_List;
                                objNode->pObj = mapObj->pObj;
                                objNode->next = nullptr;
                            }
                            else {
                                skipObj = false;
                                objNode = *lpObjs_List;
                                while (objNode->next) {
                                    if (mapObj->pObj == objNode->pObj)
                                        skipObj = true;
                                    objNode = objNode->next;
                                }
                                if (!skipObj) {
                                    objNode->next = new OBJNode;
                                    objNode = objNode->next;
                                    objNode->pObj = mapObj->pObj;
                                    objNode->next = nullptr;
                                }
                            }
                        }
                    }
                }
            }
            else
                mapObj = nullptr;
            if (mapObj)
                mapObj = mapObj->next;
        }
    }

    return TRUE;
}


//___________________________________________________________________________
BOOL SelectObjects_On_Map(LONG level, OBJ_SEARCH search_for, DWORD test_data) {
    //search whole map if level == -1
    Selected_List_Destroy(FALSE);

    BOOL ret_val = ListObjects_On_Map(level, &pObjs_Selected, search_for, test_data, FLG_MapperSelected, FALSE);
    DrawMapChanges(nullptr, level, FLG_Hud_Outline);
    Selected_List_Update_State();
    return ret_val;
}


//___________________________________________________________________________________________________________
BOOL ListTiles_With_Matching_FRM(LONG level, TILE_DATA_node** lpTiles_List, DWORD frmID, DWORD outline_flags) {
    //search whole map if level == -1

    if (!lpTiles_List)
        return FALSE;
    if (frmID == -1)
        return FALSE;
    LONG type = (frmID & 0x0F000000) >> 0x18;
    if (type != ART_TILES)
        return FALSE;

    if (!IsObjectTypeSelectable(ART_TILES))
        return FALSE;

    BOOL roof_tiles_selectable = IsObjectTypeSelectable(ART_TILES, TRUE);
    BOOL floor_tiles_selectable = IsObjectTypeSelectable(ART_TILES, FALSE);

    DWORD combatFlags = 0;

    bool whole_map = false;
    if (level == -1)
        whole_map = true;

    TILE_DATA_node* tileNode = nullptr;
    DWORD frmID_OnMap = 0;
    bool skipObj = false;
    for (LONG lev = 0; lev < 3; lev++) {

        if (whole_map == false)
            lev = level;

        for (LONG tileNum = 0; tileNum < *pNUM_TILES; tileNum++) {
            for (BOOL isRoof = FALSE; isRoof < TRUE + TRUE; isRoof += TRUE) {
                frmID_OnMap = GameMap_GetTileFrmID(tileNum, lev, isRoof, nullptr);

                if (frmID_OnMap == frmID) {
                    if (!*lpTiles_List) {
                        *lpTiles_List = new TILE_DATA_node;
                        tileNode = *lpTiles_List;
                        tileNode->level = lev;
                        tileNode->num = tileNum;
                        tileNode->isRoof = isRoof;
                        tileNode->frmID = frmID_OnMap;
                        tileNode->next = nullptr;
                    }
                    else {
                        skipObj = false;
                        tileNode = *lpTiles_List;
                        while (tileNode->next) {
                            if (tileNum == tileNode->num && lev == tileNode->level && isRoof == tileNode->isRoof)
                                skipObj = true;
                            tileNode = tileNode->next;
                        }
                        if (!skipObj) {
                            tileNode->next = new TILE_DATA_node;
                            tileNode = tileNode->next;
                            tileNode->level = lev;
                            tileNode->num = tileNum;
                            tileNode->isRoof = isRoof;
                            tileNode->frmID = frmID_OnMap;
                            tileNode->next = nullptr;
                        }
                    }
                    combatFlags = GameMap_GetTile_Flags(tileNode->num, tileNode->level, isRoof);
                    combatFlags |= outline_flags;
                    GameMap_SetTile_Flags(tileNode->num, tileNode->level, isRoof, combatFlags);
                }
            }
        }
        if (whole_map == false)
            lev = 3;
    }

    return TRUE;
}


//_________________________________________________________
BOOL SelectTiles_With_Matching_FRM(LONG level, DWORD frmID) {
    //search whole map if level == -1

    Selected_List_Destroy(FALSE);
    BOOL ret_val = ListTiles_With_Matching_FRM(level, &pTiles_Selected, frmID, FLG_MapperSelected);
    
    DrawMapChanges(nullptr, level, FLG_Hud_Outline);
    Selected_List_Update_State();
    return ret_val;
}


//_______________________________________________________
void Selecting_List_Destroy(BOOL dont_proccess_selecting) {

    if (pObjs_Selecting) {
        OBJNode* objNode = nullptr;
        while (pObjs_Selecting) {
            if (pObjs_Selecting->pObj)
                pObjs_Selecting->pObj->combatFlags &= ~FLG_MapperSelecting;
            if (!dont_proccess_selecting) {
                if (selectObject_isRemoving)
                    SelectObject_Remove(pObjs_Selecting->pObj, FALSE);
                else
                    SelectObject_Add(pObjs_Selecting->pObj, FALSE);
            }

            pObjs_Selecting->pObj = nullptr;
            objNode = pObjs_Selecting;
            pObjs_Selecting = objNode->next;
            objNode->next = nullptr;
            delete objNode;
        }
    }
    if (pTiles_Selecting) {
        DWORD combatFlags = 0;
        TILE_DATA_node* tileNode = nullptr;
        while (pTiles_Selecting) {
            combatFlags = GameMap_GetTile_Flags(pTiles_Selecting->num, pTiles_Selecting->level, pTiles_Selecting->isRoof);
            combatFlags &= ~FLG_MapperSelecting;
            GameMap_SetTile_Flags(pTiles_Selecting->num, pTiles_Selecting->level, pTiles_Selecting->isRoof, combatFlags);

            if (!dont_proccess_selecting) {
                if (selectObject_isRemoving)
                    SelectTile_Remove(pTiles_Selecting, FALSE);
                else
                    SelectTile_Add(pTiles_Selecting, FALSE);
            }

            tileNode = pTiles_Selecting;
            pTiles_Selecting = tileNode->next;
            tileNode->next = nullptr;
            delete tileNode;
        }
    }
    if (!dont_proccess_selecting)
        Selected_List_Update_State();
}


//list objects with unmasked pixels under mouse cursor. type(0-5) or type(-1)list all types. *lpObjInfoArray must be destroyed after use with fall_Mem_Deallocate function.
//________________________________________________________________________________________________
LONG GetObjectsAtPos_Mapper(LONG xPos, LONG yPos, LONG level, LONG type, OBJInfo** lpObjInfoArray) {

    if (type != -1 && !IsObjectTypeSelectable(type)) {
        *lpObjInfoArray = nullptr;
        return 0;
    }
    OBJNode* mapObj = nullptr;
    OBJInfo* pObjInfoArray = nullptr;
    *lpObjInfoArray = pObjInfoArray;

    DWORD objInfoArraySize = 0;

    bool exitLoop = false;
    LONG objType = 0;
    DWORD flags = 0;
    LONG numObjects = 0;
    for (LONG hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {

        mapObj = pMapObjNodeArray[hexPos];
        objInfoArraySize = sizeof(OBJInfo) * (numObjects + 1);

        exitLoop = false;
        while (mapObj && !exitLoop) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    objType = mapObj->pObj->frmID & 0x0F000000;
                    objType = objType >> 0x18;

                    if ((type == -1 || objType == type) && IsObjectTypeSelectable(objType) && mapObj->pObj != *ppObj_Egg) {
                        flags = fall_Obj_CheckFrmAtPos(mapObj->pObj, xPos, yPos);
                        if (flags) {
                            pObjInfoArray = (OBJInfo*)fall_Mem_Reallocate((BYTE*)pObjInfoArray, objInfoArraySize);
                            if (pObjInfoArray) {
                                pObjInfoArray[numObjects].obj = mapObj->pObj;
                                pObjInfoArray[numObjects].flags = flags;
                                numObjects++;
                                objInfoArraySize += sizeof(OBJInfo);
                            }
                        }
                    }
                }
                mapObj = mapObj->next;
            }
            else
                exitLoop = true;
        }
    }
    *lpObjInfoArray = pObjInfoArray;

    return numObjects;
}


//________________________________________________________________________________________________________________________
BOOL GetObjectOrTileAtPos(LONG xPos, LONG yPos, LONG level, OBJStruct** pp_ret_obj, LONG* p_ret_tile_num, BOOL* p_is_roof) {

    if (!pp_ret_obj || !p_ret_tile_num || !p_is_roof)
        return FALSE;

    *pp_ret_obj = nullptr;
    *p_ret_tile_num = -1;
    *p_is_roof = FALSE;
    OBJStruct* pObj = nullptr;

    DWORD frmID = 0;
    LONG tileNum = -1;

    if (IsObjectTypeSelectable(ART_TILES, TRUE) && AreRoovesVisible()) {//roof
        LONG tileX = 0;
        LONG tileY = 0;

        if (SqrToTile_Roof_GameOffset(xPos, yPos, &tileX, &tileY))
            tileNum = tileY * *pNUM_TILE_X + tileX;
        frmID = GameMap_GetTileFrmID(tileNum, level, TRUE, nullptr);
        if ((frmID & 0x00FFFFFF) != 1) {
            *p_is_roof = TRUE;
            *p_ret_tile_num = tileNum;
            return TRUE;
        }
    }

    OBJInfo* pObjInfoArray = nullptr;
    LONG numObjs = GetObjectsAtPos_Mapper(xPos, yPos, level, -1, &pObjInfoArray);

    for (LONG num = numObjs - 1; num >= 0; num--) {
        pObj = pObjInfoArray[num].obj;
        if ((pObjInfoArray[num].flags & 0x1)) {//position hits non transparent obj frm pixel.
            if (!(pObjInfoArray[num].flags & 0x4)) {
                DWORD objType = (pObj->frmID & 0x0F000000) >> 24;
                if (objType == ART_CRITTERS) {
                    if (!(pObj->pud.critter.combat_data.results & 0x81))
                        num = -1;
                }
                else
                    num = -1;
            }
        }
    }

    if (pObjInfoArray) {
        fall_Mem_Deallocate(pObjInfoArray);
        pObjInfoArray = nullptr;
    }

    if (pObj) {
        *pp_ret_obj = pObj;
        return TRUE;
    }

    if (IsObjectTypeSelectable(ART_TILES, FALSE)) {//floor
        LONG tileX = 0;
        LONG tileY = 0;
        if (SqrToTile_GameOffset(xPos, yPos, &tileX, &tileY))
            tileNum = tileY * *pNUM_TILE_X + tileX;
        frmID = GameMap_GetTileFrmID(tileNum, level, FALSE, nullptr);
        if ((frmID & 0x00FFFFFF) != 1) {
            *p_ret_tile_num = tileNum;
            return TRUE;
        }
    }

    return FALSE;
}


//________________________________
MAP_OBJECT* ObjectUnderMouse_Get() {

    if (map_obj_under_mouse.IsValid())
        return &map_obj_under_mouse;
    return nullptr;;
}


//_____________________________________________________________________________
BOOL MapObject_Copy(OBJStruct* pObj, LONG tileNum, BOOL isRoof, LONG tileLevel) {

    if (pObj) {
        CopiedObjects_CopyObject((OBJStruct*)pObj);
        CopiedTiles_CopyTile(-1, 0, tileLevel);
        return TRUE;
    }
    else if (tileNum != -1) {
        CopiedTiles_CopyTile(tileNum, isRoof, tileLevel);
        CopiedObjects_CopyObject(nullptr);
        return TRUE;
    }

    return FALSE;
}


//_______________________________________________________________________________
BOOL MapObject_Delete(OBJStruct* pObj, LONG tileNum, BOOL isRoof, LONG tileLevel) {

    BOOL object_removed = FALSE;

    if (pObj) {
        SelectObject_Remove(pObj, TRUE);
        Delete_MapObj(&pObj);
        object_removed = TRUE;
    }
    else if (tileNum != -1) {
        SelectTile_Remove(tileNum, isRoof, tileLevel, TRUE);
        Delete_MapTile(tileNum, isRoof, tileLevel);
        object_removed = TRUE;
    }

    return object_removed;
}


//__________________________
BOOL ObjectUnderMouse_Copy() {
    
    if(!map_obj_under_mouse.IsValid())
        return FALSE;

    if (map_obj_under_mouse.GetLevel() != *pMAP_LEVEL)
        return FALSE;

    OBJStruct* p_obj_under_mouse = map_obj_under_mouse.GetObj();
    LONG tilenum_under_mouse = map_obj_under_mouse.GetTileNumber();

    if (p_obj_under_mouse) {
        CopiedObjects_CopyObject(p_obj_under_mouse);
        CopiedTiles_CopyTile(-1, 0, *pMAP_LEVEL);
        return TRUE;
    }
    else if (tilenum_under_mouse != -1) {
        CopiedTiles_CopyTile(tilenum_under_mouse, map_obj_under_mouse.IsRoof(), *pMAP_LEVEL);
        CopiedObjects_CopyObject(nullptr);
        return TRUE;
    }

    return FALSE;
}


//____________________________
BOOL ObjectUnderMouse_Delete() {

    if (!map_obj_under_mouse.IsValid())
        return FALSE;

    if (map_obj_under_mouse.GetLevel() != *pMAP_LEVEL)
        return FALSE;

    OBJStruct* p_obj_under_mouse = map_obj_under_mouse.GetObj();
    LONG tilenum_under_mouse = map_obj_under_mouse.GetTileNumber();

    BOOL object_removed = FALSE;

    if (p_obj_under_mouse) {
        SelectObject_Remove(p_obj_under_mouse, TRUE);
        Delete_MapObj(&p_obj_under_mouse);
        object_removed = TRUE;
    }
    else if (tilenum_under_mouse != -1) {
        SelectTile_Remove(tilenum_under_mouse, map_obj_under_mouse.IsRoof(), *pMAP_LEVEL, TRUE);
        Delete_MapTile(tilenum_under_mouse, map_obj_under_mouse.IsRoof(), *pMAP_LEVEL);
        object_removed = TRUE;
    }

    ObjectUnderMouse_Clear();

    return object_removed;
}


//___________________________
BOOL ObjectUnderMouse_Clear() {

    if (!map_obj_under_mouse.IsValid())
        return FALSE;

    OBJStruct* p_obj_under_mouse = map_obj_under_mouse.GetObj();
    LONG tilenum_under_mouse = map_obj_under_mouse.GetTileNumber();

    BOOL draw_flag = FALSE;
    if (p_obj_under_mouse) {
        p_obj_under_mouse->combatFlags &= ~FLG_MapperObjUnderMouse;
        draw_flag = TRUE;
    }
    if (tilenum_under_mouse != -1) {
        LONG level = map_obj_under_mouse.GetLevel();
        BOOL isRoof = map_obj_under_mouse.IsRoof();
        DWORD combatFlags = GameMap_GetTile_Flags(tilenum_under_mouse, level, isRoof);
        combatFlags &= ~FLG_MapperObjUnderMouse;
        GameMap_SetTile_Flags(tilenum_under_mouse, level, isRoof, combatFlags);
        draw_flag = TRUE;
    }
    map_obj_under_mouse.Clear();
    return draw_flag;
}


//_______________________________________________________
void Action_Update_ObjectUnderMouse(LONG xPos, LONG yPos) {

    LONG level = *pMAP_LEVEL;

    static LONG last_xPos = -1;
    static LONG last_yPos = -1;

    if (xPos == last_xPos && yPos == last_yPos)
        return;

    OBJStruct* pObj = nullptr;
    LONG tileNum = -1;
    BOOL isRoof = FALSE;

    BOOL draw_flag = FALSE;

    if (Get_PerformingAction() != PERFORMING_ACTION::none || GetObjectOrTileAtPos(xPos, yPos, level, &pObj, &tileNum, &isRoof) == FALSE) {
        draw_flag = ObjectUnderMouse_Clear();
    }
    else {
        ObjectUnderMouse_Clear();

        selectedActionLevel = level;

        // set new object or tile flags
        if (pObj) {
            map_obj_under_mouse.Set(pObj);
            pObj->combatFlags |= FLG_MapperObjUnderMouse;
        }
        else if (tileNum != -1) {
            map_obj_under_mouse.Set(tileNum, isRoof, level);
            DWORD combatFlags = GameMap_GetTile_Flags(tileNum, level, isRoof);
            combatFlags |= FLG_MapperObjUnderMouse;
            GameMap_SetTile_Flags(tileNum, level, isRoof, combatFlags);
        }
        draw_flag = TRUE;
    }

    if(draw_flag)
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Hud_Outline);
}


//______________________________________________________
BOOL SelectObjectAtPos(LONG xPos, LONG yPos, LONG level) {

    if (!(GetAsyncKeyState(VK_CONTROL) & 0x80000000)) {
        Selected_List_Destroy(FALSE);
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Hud_Outline);
    }

    OBJStruct* pObj = nullptr;
    DWORD frmID = 0;
    LONG tileNum = -1;

    BOOL isRoofTile = FALSE;
    if (IsObjectTypeSelectable(ART_TILES, TRUE) && AreRoovesVisible()) {//roof
        LONG tileX = 0;
        LONG tileY = 0;

        if (SqrToTile_Roof_GameOffset(xPos, yPos, &tileX, &tileY))
            tileNum = tileY * *pNUM_TILE_X + tileX;
        frmID = GameMap_GetTileFrmID(tileNum, level, TRUE, nullptr);
        if ((frmID & 0x00FFFFFF) != 1) 
            isRoofTile = TRUE;
        
    }
    if (!isRoofTile) {
        OBJInfo* pObjInfoArray = nullptr;;
        LONG numObjs = GetObjectsAtPos_Mapper(xPos, yPos, level, -1, &pObjInfoArray);

        for (LONG num = numObjs - 1; num >= 0; num--) {
            pObj = pObjInfoArray[num].obj;
            if ((pObjInfoArray[num].flags & 0x1)) {//position hits non transparent obj frm pixel.
                if (!(pObjInfoArray[num].flags & 0x4)) {
                    DWORD objType = (pObj->frmID & 0x0F000000) >> 24;
                    if (objType == ART_CRITTERS) {
                        if (!(pObj->pud.critter.combat_data.results & 0x81))
                            num = -1;
                    }
                    else
                        num = -1;
                }
            }
        }

        if (pObjInfoArray) {
            fall_Mem_Deallocate(pObjInfoArray);
            pObjInfoArray = nullptr;
        }

        if (pObj) {
            //Fallout_Debug_Info("select type %d", type);
            if (selectObject_isRemoving)
                return SelectObject_Remove(pObj, TRUE);
            return SelectObject_Add(pObj, TRUE);
        }

        if (IsObjectTypeSelectable(ART_TILES, FALSE)) {//floor
            LONG tileX = 0;
            LONG tileY = 0;
            if (SqrToTile_GameOffset(xPos, yPos, &tileX, &tileY))
                tileNum = tileY * *pNUM_TILE_X + tileX;
            frmID = GameMap_GetTileFrmID(tileNum, level, FALSE, nullptr);
        }
        if (tileNum == -1 || (frmID & 0x00FFFFFF) == 1)
            return FALSE;
    }

    if (selectObject_isRemoving)
        return SelectTile_Remove(tileNum, isRoofTile, level, TRUE);
    return SelectTile_Add(tileNum, isRoofTile, level, frmID, TRUE);
}


//______________________________________________________
BOOL Action_Initiate_SelectObjects(LONG xPos, LONG yPos) {
    
    //cancel any running action. 
    SelectedAction_End(TRUE);
    
    //if the control key isn't down, the selection data is cleared. 
    if (!(GetAsyncKeyState(VK_CONTROL) & 0x80000000)) {
        Selected_List_Destroy(FALSE);
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Hud_Outline);
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
        selectObject_isRemoving = TRUE;
    else
        selectObject_isRemoving = FALSE;

    mouse_select_area_start.x = xPos;
    mouse_select_area_start.y = yPos;

    mouse_select_area_end.x = mouse_select_area_start.x + 1;
    mouse_select_area_end.y = mouse_select_area_start.y + 1;

    return Set_PerformingAction(PERFORMING_ACTION::selecting_objects);
}


//_____________________________________________________________________
BOOL Action_Update_SelectObjects(LONG xPos, LONG yPos, BOOL update_now) {

    mouse_select_area_end.x = xPos;
    mouse_select_area_end.y = yPos;

    //reduce update frequency for better responsiveness when changing rect with mouse. 
    static ULONGLONG  last_time = 0;
    if (!update_now) {
        ULONGLONG time = GetTickCount64();
        if (time >= last_time + 60) {
            update_now = TRUE;
            last_time = time;
        }
    }

    if (update_now) {
        //As the selecting rect has changed - discard and unmark any objects currently in the select-ING list.
        Selecting_List_Destroy(TRUE);
        //Add objects in the selecting rect to the select-ING list.
        ListObjectsInRect(&mouse_select_area_start, &mouse_select_area_end, selectedActionLevel, &pObjs_Selecting);
        ListTilesInRect(&mouse_select_area_start, &mouse_select_area_end, selectedActionLevel, &pTiles_Selecting);

        SelectingRectVB_Set(&mouse_select_area_start, &mouse_select_area_end);

        DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);
    }

    return TRUE;
}


//_____________________________________________________________________
void Action_End_SelectObjects(LONG xPos, LONG yPos, BOOL cancel_action) {

    Action_Update_SelectObjects(xPos, yPos, TRUE);


    if (!cancel_action && abs(mouse_select_area_end.x - mouse_select_area_start.x) < 16 && abs(mouse_select_area_end.y - mouse_select_area_start.y) < 12)
        SelectObjectAtPos(xPos, yPos, selectedActionLevel);
    else
        Selecting_List_Destroy(cancel_action);

    SelectingRectVB_Destroy();

    DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);

    //Selected_List_Update_State();
    return;
}


//___________________________
void SelectedObjects_Delete() {

    if (!pObjs_Selected && !pTiles_Selected)
        return;
    MapChanges_Update();

    Selected_List_Destroy(TRUE);
    DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Obj | FLG_Hud_Outline);
}


//_________________________
void PlaceNewObject_Clear() {

    new_obj_proId = -1;
    new_obj_ori = 0;
    if (*ppObj_Selected)
        (*ppObj_Selected)->ori = new_obj_ori;
    fall_Mouse_SetFrm(fall_GetFrmID(ART_INTRFACE, 0, 0, 0, 0));
}


//______________________________________
void PlaceNewObject_Update_Ori(LONG ori) {

    if (new_obj_proId == -1)
        return;
    PROTO* pPro = nullptr;
    if (fall_GetPro(new_obj_proId, (PROTO**)&pPro) == -1) {
        return;
    }
    if (!*ppObj_Selected)
        return;

    FRMCached* pfrm = new FRMCached(pPro->frmID);
    FRMframeDx* pFrame = pfrm->GetFrame(ori, 0);
    if (pFrame) {
        new_obj_ori = ori;
        RECT rc_obj1{ 0,0,0,0 };
        fall_Obj_GetRect(*ppObj_Selected, &rc_obj1);
        (*ppObj_Selected)->ori = ori;
        RECT rc_obj2{ 0,0,0,0 };
        fall_Obj_GetRect(*ppObj_Selected, &rc_obj2);
        UnionRect(&rc_obj1, &rc_obj1, &rc_obj2);
        DrawMapChanges(&rc_obj1, (*ppObj_Selected)->level, FLG_Floor | FLG_Obj);
    }
    if (pfrm)
        delete pfrm;
    pfrm = nullptr;
}


//____________________________________
void PlaceNewObject_Rotate_Clockwise() {

    LONG ori = new_obj_ori;
    if (ori < 5)
        ori++;
    else
        ori = 0;
    PlaceNewObject_Update_Ori(ori);
}


//________________________________________
void PlaceNewObject_Rotate_AntiClockwise() {

    LONG ori = new_obj_ori;
    if (ori > 0)
        ori--;
    else
        ori = 5;
    PlaceNewObject_Update_Ori(ori);
}


//______________________________________________________________________
BOOL Action_End_PlaceNewObject(LONG xPos, LONG yPos, BOOL cancel_action) {

    BOOL continue_action = FALSE;
    if (cancel_action || new_obj_proId == -1)
        PlaceNewObject_Clear();
    else {
        PROTO* proto;
        if (fall_GetPro(new_obj_proId, &proto) == 0) {
            MapChanges_Update();
            INT32 objType = proto->frmID & 0x0F000000;
            objType = objType >> 0x18;

            if (objType == ART_TILES) {
                BOOL isRoof = FALSE;
                if (AreRoovesVisible())
                    isRoof = TRUE;
                CreateNew_MapTile(xPos, yPos, selectedActionLevel, isRoof, proto->frmID);
            }
            else {
                LONG hexNum = SqrToHexNum_GameOffset(xPos, yPos);
                OBJStruct* pObj = nullptr;
                CreateNew_MapObject(hexNum, selectedActionLevel, proto->frmID, new_obj_proId, &pObj);

                RECT rc_obj1{ 0,0,0,0 };
                fall_Obj_GetRect(pObj, &rc_obj1);
                pObj->ori = new_obj_ori;
                RECT rc_obj2{ 0,0,0,0 };
                fall_Obj_GetRect(pObj, &rc_obj2);
                UnionRect(&rc_obj1, &rc_obj1, &rc_obj2);
                DrawMapChanges(&rc_obj1, pObj->level, FLG_Floor | FLG_Obj);

            }
        }

        if (GetAsyncKeyState(VK_CONTROL) & 0x80000000)
            continue_action = TRUE;
        else
            PlaceNewObject_Clear();
    }

    return continue_action;
}


//______________________________________________
BOOL Action_Initiate_PlaceNewObject(DWORD proID) {

    SelectedAction_End(TRUE);
    PROTO* pPro = nullptr;
    if (fall_GetPro(proID, (PROTO**)&pPro) == -1) {
        return FALSE;
    }
   
    LONG type = proID >> 24;
    DWORD frmID = pPro->frmID;

    DWORD tile1_frmID = fall_GetFrmID(ART_TILES, 1, 0, 0, 0);

    if (frmID == tile1_frmID) {

        frmID = fall_GetFrmID(type, 1, 0, 0, 0);

        if (!fall_CheckFrmFileExists(frmID))
            frmID = fall_GetFrmID(ART_TILES, 1, 0, 0, 0);

    }

    if (fall_CheckFrmFileExists(frmID) && frmID != -1) {
        if (!Set_PerformingAction(PERFORMING_ACTION::create_new_object))
            return FALSE;
        new_obj_proId = proID;
        fall_Mouse_SetFrm(frmID);
        return TRUE;
    }
    
    return FALSE;
}


//____________________________________________________
BOOL Action_Update_AdjustMapEdge(LONG xPos, LONG yPos) {
    if (!pRcEdgeTemp)
        return FALSE;
    
    if (selected_edge_type == SEL_RC_TYPE::square) {
        GAME_AREA* pArea_Mapper = GameAreas_GetCurrentArea();
        if (!pArea_Mapper)
            return FALSE;
        GAME_AREA* pArea = GetSelectedArea(selectedActionLevel);
        if (!pArea)
            return FALSE;
        
        if (SqrToHexPos_GameOffset(xPos, yPos, &xPos, &yPos) == -1 && Edges_GetActiveVersion() == 2) {
            if (xPos < 0)
                xPos = 0;
            else if (xPos >= *pNUM_HEX_X)
                xPos = *pNUM_HEX_X - 1;
            if (yPos < 0)
                yPos = 0;
            else if (yPos >= *pNUM_HEX_Y)
                yPos = *pNUM_HEX_Y - 1;
        }
           
        LONG x = 0;
        LONG y = 0;
        LONG temp_val = 0;
        switch (selected_edge) {
        case EDGE_SELECT::top:
            Hex2Sqr_Scroll(xPos, yPos, &x, &temp_val);
            if (temp_val > pArea_Mapper->rect.top && temp_val < pArea->rect.bottom)
                pArea->rect.top = temp_val;
            break;
        case EDGE_SELECT::bottom:
            Hex2Sqr_Scroll(xPos, yPos, &x, &temp_val);
            if (Edges_GetActiveVersion() > 2)
                temp_val += 24;
            if (temp_val <= pArea_Mapper->rect.bottom && temp_val > pArea->rect.top)
                pArea->rect.bottom = temp_val;
            break;
        case EDGE_SELECT::right:
            Hex2Sqr_Scroll(xPos, yPos, &temp_val, &y);
            if (Edges_GetActiveVersion() > 2)
                temp_val += 32;
            if (temp_val <= pArea_Mapper->rect.right && temp_val > pArea->rect.left)
                pArea->rect.right = temp_val;
            break;
        case EDGE_SELECT::left:
            Hex2Sqr_Scroll(xPos, yPos, &temp_val, &y);
            if (temp_val > pArea_Mapper->rect.left && temp_val < pArea->rect.right)
                pArea->rect.left = temp_val;
            break;
        default:
            break;
        }
        pArea->width = pArea->rect.right - pArea->rect.left;
        pArea->height = pArea->rect.bottom - pArea->rect.top;
    }
    else if (selected_edge_type == SEL_RC_TYPE::isometric) {
        GAME_AREA* pArea = GameAreas_Get(selectedActionLevel);
        if (!pArea)
            return FALSE;

        LONG x = 0;
        LONG y = 0;
        LONG temp_val = 0;
        switch (selected_edge) {
        case EDGE_SELECT::top:
            if (SqrToTile(xPos, yPos, &x, &temp_val)) {
                if (temp_val < pArea->tileLimits.bottom) {
                    if (temp_val != pArea->tileLimits.top) {
                        pArea->tileLimits.top = temp_val;
                        TileLimitsRectVB_Set();
                    }
                }
            }
            break;
        case EDGE_SELECT::bottom:
            SqrToTile(xPos, yPos, &x, &temp_val);//cannot use return value here as will be false if greater than 99, max is 100.
                if (temp_val > pArea->tileLimits.top) {
                    if (temp_val <= *pNUM_TILE_Y && temp_val != pArea->tileLimits.bottom) {
                        pArea->tileLimits.bottom = temp_val;
                        TileLimitsRectVB_Set();
                    }
                }
            break;
        case EDGE_SELECT::right:
            if (SqrToTile(xPos, yPos, &temp_val, &y)) {
                if (temp_val < pArea->tileLimits.left) {
                    if (temp_val != pArea->tileLimits.right) {
                        pArea->tileLimits.right = temp_val;
                        TileLimitsRectVB_Set();
                    }
                }
            }
            break;
        case EDGE_SELECT::left:
            SqrToTile(xPos, yPos, &temp_val, &y);//cannot use return value here as will be false if greater than 99, max is 100.
                if (temp_val > pArea->tileLimits.right) {
                    if (temp_val <= *pNUM_TILE_X && temp_val != pArea->tileLimits.left) {
                        pArea->tileLimits.left = temp_val;
                        TileLimitsRectVB_Set();
                    }
                }           
            break;
        default:
            break;
        }
    }

    DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);
    return TRUE;
}

//_____________________________________________________
BOOL Action_Perform_AdjustMapEdge(LONG xPos, LONG yPos) {

    if (pRcEdgeTemp)
        delete pRcEdgeTemp;
    pRcEdgeTemp = nullptr;

    if (selected_edge_type == SEL_RC_TYPE::square) {
        GAME_AREA* pArea = GetSelectedArea(selectedActionLevel);
        if (pArea) {
            pRcEdgeTemp = new RECT{ 0,0,0,0 };
            CopyRect(pRcEdgeTemp, &pArea->rect);
        }
    }
    else if (selected_edge_type == SEL_RC_TYPE::isometric) {
        GAME_AREA* pArea = GameAreas_Get(selectedActionLevel);
        if (pArea) {
            pRcEdgeTemp = new RECT{ 0,0,0,0 };
            CopyRect(pRcEdgeTemp, &pArea->tileLimits);
        }
    }
    return Action_Update_AdjustMapEdge(xPos, yPos);
}


//_______________________________________________
void Action_End_AdjustMapEdge(BOOL cancel_action) {

    selected_edge = EDGE_SELECT::none;

    Tool_SetState_Edges_SelectEdge(*phWinMain, FALSE);
    
    if (selected_edge_type == SEL_RC_TYPE::square) {
        GAME_AREA* pArea = GetSelectedArea(selectedActionLevel);
        if (pArea) {
            if (cancel_action) {
                if (pRcEdgeTemp)
                    CopyRect(&pArea->rect, pRcEdgeTemp);
            }
            pArea->width = pArea->rect.right - pArea->rect.left;
            pArea->height = pArea->rect.bottom - pArea->rect.top;
            DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);
        }
    }
    else if (selected_edge_type == SEL_RC_TYPE::isometric) {
        GAME_AREA* pArea = GameAreas_Get(selectedActionLevel);
        if (pArea) {
            if (cancel_action) {
                if (pRcEdgeTemp) {
                    CopyRect(&pArea->tileLimits, pRcEdgeTemp);
                    TileLimitsRectVB_Set();
                }
                
            }
            DrawMapChanges(nullptr, selectedActionLevel, FLG_Hud_Outline);
        }
    }

    selected_edge_type = SEL_RC_TYPE::none;

    if (pRcEdgeTemp)
        delete pRcEdgeTemp;
    pRcEdgeTemp = nullptr;
}


//_________________________________________________________________________
BOOL Action_Initiate_AdjustMapEdge(SEL_RC_TYPE edge_type, EDGE_SELECT edge) {
    
    SelectedAction_End(TRUE);

    selected_edge_type = edge_type;
    selected_edge = edge;

    if (selected_edge != EDGE_SELECT::none && Set_PerformingAction(PERFORMING_ACTION::area_edge_adjustment))
        return TRUE;

    selected_edge_type = SEL_RC_TYPE::none;
    selected_edge = EDGE_SELECT::none;
    return FALSE;
}


//___________________________
void SelectedAction_Preform() {

    fall_Update_Mouse_State();
    LONG xPos = 0;
    LONG yPos = 0;
    if (!GetMousePosOnGameMap(&xPos, &yPos))
        return;

    selectedActionLevel = *pMAP_LEVEL;

    BOOL action_start_success = TRUE;

    switch (Get_PerformingAction()) {
    case PERFORMING_ACTION::create_new_object:
        break;
    case PERFORMING_ACTION::area_edge_adjustment:
        action_start_success = Action_Perform_AdjustMapEdge(xPos, yPos);
        break;
    //case PERFORMING_ACTION::selecting_objects:
    //    break;
    case PERFORMING_ACTION::pasting_objects:
        break;
    default:
        action_start_success = Action_Initiate_SelectObjects(xPos, yPos);
        break;
    }

    if(!action_start_success)
        End_PerformingAction();
}


//__________________________
void SelectedAction_Update() {
    LONG xPos = 0;
    LONG yPos = 0;
    if (!GetMousePosOnGameMap(&xPos, &yPos))
        return;

    wchar_t msg[12];
    LONG m_hex_x = 0;
    LONG m_hex_y = 0;
    SqrToHexPos_GameOffset(xPos, yPos, &m_hex_x, &m_hex_y);
    LONG m_hexnum = -1;
    if (!HexPosToHexNum(&m_hexnum, m_hex_x, m_hex_y))
        m_hexnum = -1;

    //update the mouse position on the status bar.
    swprintf_s(msg, L"x:%d", m_hex_x);
    StatusBar_SetText(1, msg);
    swprintf_s(msg, L"y:%d", m_hex_y);
    StatusBar_SetText(2, msg);
    swprintf_s(msg, L"hex:%d", m_hexnum);
    StatusBar_SetText(3, msg);

    Action_Update_ObjectUnderMouse(xPos, yPos);

    switch (Get_PerformingAction()) {
    case PERFORMING_ACTION::create_new_object:
        break;
    case PERFORMING_ACTION::area_edge_adjustment:
        Action_Update_AdjustMapEdge(xPos, yPos);
        break;
    case PERFORMING_ACTION::selecting_objects:
        Action_Update_SelectObjects(xPos, yPos, FALSE);
        break;
    case PERFORMING_ACTION::pasting_objects:
        Action_Update_CopiedObjects_Pasting(xPos, yPos);
        break;
    default:
        break;
    }

}


//_________________________________________
void SelectedAction_End(BOOL cancel_action) {
    fall_Update_Mouse_State();
    LONG xPos = 0;
    LONG yPos = 0;
    if (!GetMousePosOnGameMap(&xPos, &yPos))
        return;

    BOOL continue_action = FALSE;

    switch (Get_PerformingAction()) {
    case PERFORMING_ACTION::create_new_object:
        continue_action = Action_End_PlaceNewObject(xPos, yPos, cancel_action);
        break;
    case PERFORMING_ACTION::area_edge_adjustment:
        Action_End_AdjustMapEdge(cancel_action);
        break;
    case PERFORMING_ACTION::selecting_objects:
        Action_End_SelectObjects(xPos, yPos, cancel_action);
        break;
    case PERFORMING_ACTION::pasting_objects:
        continue_action = Action_End_CopiedObjects_Pasting(xPos, yPos, cancel_action);
        break;
    default:
        break;
    }
    
    if(!continue_action)
        End_PerformingAction();
}
