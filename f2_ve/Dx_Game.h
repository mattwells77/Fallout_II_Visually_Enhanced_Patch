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

#define FLG_Floor       1
#define FLG_Obj         2
#define FLG_Roof        4
#define FLG_Hud         8
#define FLG_Hud_Outline 16

#define RT_Floor        0
#define RT_Lights_Flat  1
#define RT_Object_Flat  5
#define RT_Object       2
#define RT_Roof         3
#define RT_Hud          4

//main pal colour offsets
#define PAL_TRANS_WALL      0x21
#define PAL_TRANS_GLASS     0x23
#define PAL_TRANS_STEAM     0xDC
#define PAL_TRANS_ENERGY    0x3A
#define PAL_TRANS_RED       0x85

#define PAL_OUTLINE_NOT_TEAM            0xF3
#define PAL_OUTLINE_MOUSE               0x85
#define PAL_OUTLINE_UNKNOWN04           0x09
#define PAL_OUTLINE_TEAM                0xE5
#define PAL_OUTLINE_ITEM_UNDER_MOUSE    0x3A
#define PAL_OUTLINE_NOT_VISIBLE         0x3D
#define PAL_OUTLINE_OTHER               0x31


#include "Dx_General.h"
#include "Dx_Graphics.h"
#include "Dx_RenderTarget.h"
#include "Dx_Windows.h"
#include "Fall_Objects.h"


extern RECT rcGame_GUI;//the dimensions of the game window on the gui - different from the visible game area that is affected by game portal scaling.
extern RECT rcGame_PORTAL;//the dimensions of the visible area on the map, left to right (-4800 to 3200) & top to bottom (0 to 3600);

extern LONG game_PortalWidth;
extern LONG game_PortalHeight;




struct OBJlight {
    OBJStruct* pObj;
    DirectX::XMFLOAT4 lightDx;// {xPos, yPos, hexDist*32.0f, light_intensity/65536.0f};
    DirectX::XMFLOAT4 colour;
    RenderTarget* pRT_ShadowMask;
    RECT rect;
    OBJlight* next;
};

struct FloatingTextObjDx {
    DWORD flags;    //0x00 //flags  //0x02 = centre on scrn, no associated obj
    OBJStruct* pObj; //0x04 //associated map obj
    LONG time;      //0x08 //creation time
    LONG numLines;  //0x0C //num lines of text
    LONG xOffset;   //0x10
    LONG yOffset;   //0x14
    LONG hexPos;    //0x18
    LONG scrnX;     //0x1C
    LONG scrnY;     //0x20
    LONG imgWidth;  //0x24
    LONG imgHeight; //0x28
    BYTE* pImgBuff; //0x2C
    FRMframeDx* frameDX;
};

class FRMobj {
public:
    float opaqueness;
    FRMobj(OBJStruct* pObj, bool in_isPositionUpdatedOften) {
        pD3DDevContext = GetD3dDeviceContext();
        frmID = pObj->frmID;
        frmNode = nullptr;
        lpLightTex = nullptr;
        ori = pObj->ori;
        frameNum = pObj->frameNum;
        relight = true;
        pParentObj = pObj;

        LONG artType = ((0x0F000000 & frmID) >> 24);
        //if ((FOG_OF_WAR > 2 || artType == ART_CRITTERS || artType == ART_ITEMS) && !(pObj->flags & FLG_MarkedByPC))
        if ((artType == ART_CRITTERS || artType == ART_ITEMS) && !(pObj->flags & FLG_MarkedByPC))
        //if (!(pObj->flags & FLG_MarkedByPC))
            opaqueness = 0.0f;
        else
            opaqueness = 1.0f;

        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        frmNode = frmDxCache->Get(frmID);
        DrawObjLights();
    }
    ~FRMobj() {
        if (frmID && frmDxCache)
            frmDxCache->Forget(frmID);
        frmNode = nullptr;
        frmID = 0;
        ReleaseLightTexture();
        pParentObj = nullptr;
        pD3DDevContext = nullptr;
    }
    FRMframeDx* GetFrame() {
        if (!pParentObj) {
            //imonitorInsertText("frame has no parent obj");
            return nullptr;
        }
        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        if (frmID != pParentObj->frmID) {
            frmNode = frmDxCache->Get(pParentObj->frmID);
            frmDxCache->Forget(frmID);
            frmID = pParentObj->frmID;
            ori = pParentObj->ori;
            frameNum = pParentObj->frameNum;
            relight = true;
        }
        if (frmNode) {
            if (ori != pParentObj->ori || frameNum != pParentObj->frameNum || relight == true) {
                ori = pParentObj->ori;
                frameNum = pParentObj->frameNum;
                DrawObjLights();
            }
            return frmNode->GetFrame(pParentObj->ori, (WORD)pParentObj->frameNum);
        }
        frmID = 0;
        return nullptr;
    };
    void DrawObjLights();
    void Relight() {
        relight = true;
    };
    bool isUniformlyLit() {
        if (frmNode) {
            FRMdx* frm = frmNode->GetFrm();
            if (frm)
                return frm->isUniformlyLit();
        }
        return false;
    };
    ID3D11ShaderResourceView* GetLightResourceView() {
        if (!lpLightTex)
            return nullptr;
        return lpLightTex->GetShaderResourceView();
    };
    void ReleaseLightTexture() {
        if (lpLightTex)
            delete lpLightTex;
        lpLightTex = nullptr;
        relight = true;
    };
protected:
private:
    ID3D11DeviceContext* pD3DDevContext;
    DWORD frmID;
    FRMDXnode* frmNode;
    RenderTarget* lpLightTex;
    int ori;
    int frameNum;
    bool relight;
    OBJStruct* pParentObj;
};


//Modified fallout map object structure, additional pointer to FRMobj class.
struct OBJStructDx {
    DWORD objID;//0x00 //pc = PF00
    LONG hexNum;//0x04
    LONG xShift;//0x08
    LONG yShift;//0x0C
    LONG viewScrnX;//0x10
    LONG viewScrnY;//0x14
    DWORD frameNum;//0x18
    LONG ori;//0x1C
    DWORD frmID;//0x20
    DWORD flags;//0x24 //critter 24240060 //inv 0x000000FF = 1=item1, 2 = item2 4 = armor
    LONG level;//0x28
    PUD pud;
    DWORD proID;//0x64  01
    DWORD cID;//0x68  05
    DWORD light_dist;//0x6C 04 //Light strength of this object?   lightRadius
    DWORD light_intensity;//0x70 0100 //Something to do with radiation?  lightIntensity
    DWORD combatFlags;//0x74 set to =0   //only valid in combat //read and written but set to 0 on load.
    DWORD scriptID1;//0x78   50460004  34000004 related to load time  /map scrip ID ?
    DWORD unknown7C;//0x7C set to =0  //not read but written but set to 0 on load.
    DWORD scriptID2;//0x80  //objScriptID?
    FRMobj* frmObjDx;
};


struct OBJFadeNode {
    OBJStructDx* pObj;
    ULONGLONG fadeStartTime;
    bool fadeIn;
    OBJFadeNode* next;
};


class FRMtile {
    DWORD frmID;
    FRMDXnode* frmNode;
public:
    DWORD combatFlags;
    FRMtile(DWORD inFrmID, DWORD flags) {
        combatFlags = 0;
        frmID = inFrmID;
        frmNode = nullptr;
        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        frmNode = frmDxCache->Get(frmID);
    }
    ~FRMtile() {
        if (frmID && frmDxCache)
            frmDxCache->Forget(frmID);
        frmNode = nullptr;
        frmID = 0;
    }
    FRMframeDx* GetFrame(DWORD inFrmID, DWORD ori, WORD frameNum) {
        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        if (frmID != inFrmID) {
            frmNode = frmDxCache->Get(inFrmID);
            frmDxCache->Forget(frmID);
            frmID = inFrmID;
        }
        FRMframeDx* pFrame = frmNode->GetFrame(ori, frameNum);
        if (!pFrame)
            frmID = 0;
        return pFrame;
    }
};


struct TILEnode {
    LONG tileNum;
    LONG x;
    LONG y;
    DWORD frmID;
    FRMtile* pFrm;
    TILEnode* next;
};


class TILE_AREA_Node {
public:
    TILEnode* tiles;
    RECT rect;
    int width;
    int height;
    RenderTarget2* renderTarget;
    ///RenderTarget *renderTarget_lights;
    ULONGLONG fadeStartTime;
    bool fadeIn;
    TILE_AREA_Node* next;

    void deleteTiles() {
        if (tiles) {
            TILEnode* pNextTile = nullptr;
            TILEnode* pThisTile = nullptr;
            pThisTile = tiles;
            while (pThisTile) {
                pNextTile = pThisTile->next;
                if (pThisTile->pFrm)
                    delete pThisTile->pFrm;
                pThisTile->pFrm = nullptr;
                delete pThisTile;
                pThisTile = pNextTile;
            }
        }
        tiles = nullptr;
    }
    void deleteRenderTargets() {
        if (renderTarget)
            delete renderTarget;
        renderTarget = nullptr;
    }
    TILE_AREA_Node() {
        tiles = nullptr;
        rect = { -1,-1,-1,-1 };
        width = 0;
        height = 0;
        renderTarget = nullptr;
        fadeStartTime = 0;
        fadeIn = true;
        next = nullptr;
    }
    ~TILE_AREA_Node() {
        deleteTiles();
        deleteRenderTargets();
        if (next)
            delete next;
        next = nullptr;
    }
};


struct checkTileNODE {
    int x;
    int y;
    int level;
    int areaNum;
    checkTileNODE* next;
};


struct COMPASS {
    LONG east;
    LONG north;
    LONG west;
    LONG south;
};


class GAME_AREA {
public:
    TILE_AREA_Node* rooves;
    TILEnode* tiles;
    COMPASS tileLimits;//floor tile coordinates boundary for isometric angular draw clipping; vals = (0-99)left-right (0-99)top-bottom;
    DWORD tileLimitFlags;
    RECT rect; //pixel coordinates on map left to right (-4800 to 3200) & top to bottom (0 to 3600);
    LONG width;
    LONG height;

    RenderTarget2* pFloor_RT;
    RenderTarget2* pLight_RT;
    RenderTarget2* pFog_RT;
    RenderTarget2* pObjects_RT;
    RenderTarget2* pHud_RT;
    RenderTarget2* pHud_RT_Outline;
    int* pMarkedRoofTile;

    GAME_AREA* next;
    GAME_AREA() {
        tiles = nullptr;
        tileLimits = { 0, 0, 0, 0 };
        tileLimitFlags = 0;
        rect = { 0, 0, 0, 0 };
        width = 0;
        height = 0;
        scaleX = 1.0f;
        scaleY = 1.0f;

        pFloor_RT = nullptr;
        pLight_RT = nullptr;
        pFog_RT = nullptr;
        pObjects_RT = nullptr;
        pHud_RT = nullptr;
        pHud_RT_Outline = nullptr;

        rooves = nullptr;
        pMarkedRoofTile = nullptr;
        next = nullptr;
    }
    ~GAME_AREA() {
        deleteTiles();
        deleteAllRoofData();
        deleteRenderTargets();
        if (next)
            delete next;
        next = nullptr;
    }
    void deleteTiles() {
        if (tiles) {
            TILEnode* pNextTile = nullptr;
            TILEnode* pThisTile = nullptr;
            pThisTile = tiles;
            while (pThisTile) {
                pNextTile = pThisTile->next;
                if (pThisTile->pFrm)
                    delete pThisTile->pFrm;
                pThisTile->pFrm = nullptr;
                delete pThisTile;
                pThisTile = pNextTile;
            }
        }
        tiles = nullptr;
    }
    void deleteRenderTargets() {
        if (pFloor_RT)
            delete pFloor_RT;
        pFloor_RT = nullptr;
        if (pLight_RT)
            delete pLight_RT;
        pLight_RT = nullptr;
        if (pFog_RT)
            delete pFog_RT;
        pFog_RT = nullptr;
        if (pObjects_RT)
            delete pObjects_RT;
        pObjects_RT = nullptr;
        if (pHud_RT)
            delete pHud_RT;
        pHud_RT = nullptr;
        if (pHud_RT_Outline)
            delete pHud_RT_Outline;
        pHud_RT_Outline = nullptr;
    }
    void SetPosition(float xPos, float yPos) {

        if (pFloor_RT)
            pFloor_RT->SetPosition(rect.left + xPos, rect.top + yPos);
        if (pObjects_RT)
            pObjects_RT->SetPosition(rect.left + xPos, rect.top + yPos);
        if (pHud_RT)
            pHud_RT->SetPosition(rect.left + xPos, rect.top + yPos);
        if (pFog_RT)
            pFog_RT->SetPosition(rect.left + xPos, rect.top + yPos);
        pHud_RT_Outline->ClearRenderTarget(nullptr);
    };
    void SetScale(float in_scaleX, float in_scaleY) {
        scaleX = in_scaleX;
        scaleY = in_scaleY;
        if (pFloor_RT)
            pFloor_RT->SetScale(scaleX, scaleY);
        if (pObjects_RT)
            pObjects_RT->SetScale(scaleX, scaleY);
        if (pHud_RT)
            pHud_RT->SetScale(scaleX, scaleY);
        if (pFog_RT)
            pFog_RT->SetScale(scaleX, scaleY);
        if (pHud_RT_Outline) {
            if (pHud_RT_Outline->GetWidth() != game_PortalWidth || pHud_RT_Outline->GetHeight() != game_PortalHeight) {
                delete pHud_RT_Outline;
                pHud_RT_Outline = new RenderTarget2(0, 0, game_PortalWidth, game_PortalHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
                pHud_RT_Outline->ScalePosition(true);
                pHud_RT_Outline->ClearRenderTarget(nullptr);
            }
            pHud_RT_Outline->SetScale(scaleX, scaleY);
        }
    };
    void deleteAllRoofData() {
        if (rooves)
            delete rooves;
        rooves = nullptr;
        if (pMarkedRoofTile)
            delete[] pMarkedRoofTile;
        pMarkedRoofTile = nullptr;
    }
    void deleteRoofRenderTargets() {
        if (!rooves)
            return;
        TILE_AREA_Node* pRoof = rooves;
        while (pRoof) {
            pRoof->deleteRenderTargets();
            pRoof = pRoof->next;
        }
    }
    bool CreateRendertargets();

protected:
private:
    float scaleX;
    float scaleY;

};


void DrawMapChanges(RECT* pRect, LONG level, DWORD flags);

void MapLight_Reset();
void MapLight_Release_RenderTargets();

void SetAmbientLight_ShaderEffect();
void DrawMouseObjDx(float inScaleX, float inScaleY, LONG layer);


bool GameAreas_Save(const char* MapName);
void GameAreas_Load_Default();
bool GameAreas_Load(const char* MapName);
void GameAreas_Load_Tiles();
void GameAreas_Destroy();
void GameAreas_Display();
void GameAreas_SetScale();
bool GameAreas_Set_Current_Area_Hex(LONG hexNum);
GAME_AREA* GameAreas_GetCurrentArea();
void GameAreas_ShiftWallRoofOpaqueness(int direction);
void GameAreas_FadingObjects_Add(OBJStruct* pObj, bool fadeIn);
void GameAreas_FadingObjects_Draw();

bool GameAreas_DrawToWindow(Window_DX* pWin, LONG hexNum, LONG portalWidth, LONG portalHeight);

LONG SetViewPosition_Hex(LONG hexNum, DWORD flags);
LONG MapScroller(LONG xMove, LONG yMove);

void CheckIfPcUnderRoof();
void PC_ScanLineOfSight();

void DrawPalAniObjs(LONG level, DWORD flags);

void CheckLitObjectRect(RECT* rect, LONG level);

void DrawLightShadows(OBJlight* light, LONG level);
