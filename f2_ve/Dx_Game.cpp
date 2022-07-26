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

#include "Dx_Game.h"
#include "Dx_General.h"
#include "Dx_Windows.h"
#include "Dx_RenderTarget.h"
#include "Dx_Graphics.h"
#include "Dx_Mouse.h"

#include "win_fall.h"
#include "modifications.h"
#include "memwrite.h"
#include "game_map.h"
#include "configTools.h"

#include "Fall_General.h"
#include "Fall_Objects.h"
#include "Fall_Graphics.h"
#include "Fall_File.h"
#include "Fall_Text.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

MAP_GLOBAL_BUFF_DATA mapData;
OBJECT_BUFF_DATA objData;
PC_POS_BUFF_DATA PCPosData;
GEN_SURFACE_BUFF_DATA genSurfaceData;

PORTAL_DIMENSIONS_DATA portalDimensionsData;

RECT rcGame_PORTAL = { 0,0,0,0 };

LONG EDGE_OFF_X = 0;
LONG EDGE_OFF_Y = 0;
LONG EDGE_X_DIFF = 0;
LONG EDGE_Y_DIFF = 0;

float opaqueness_WallRoof=1.0f;

bool SFALL_MoreTiles = false;

bool useOriginalLighting_Floor = false;
bool useOriginalLighting_Objects = false;

bool isLoadingMapData=false;

//pointer to function that loads map data
//void *pfall_load_map_data = nullptr;

GAME_AREA* pGameArea[3] = { nullptr,nullptr,nullptr };
GAME_AREA* pGameArea_Current = nullptr;

OBJlight* objLight[3] = { nullptr, nullptr, nullptr };

OBJFadeNode* frmObjFadeList = nullptr;

//An array for holding the combined light colour values for each hex on the map, for the original style hex map lighting.
XMFLOAT4* pHexLight_ColourMap[3] = { nullptr, nullptr, nullptr };

//The current active light colour for adding or subtracting from the hex light colour map", for the original style hex map lighting.
XMFLOAT4 hexLight_ActiveColour = { 1.0f, 1.0f, 1.0f, 1.0f };

XMFLOAT4 lightColour_PC = { 1.0f, 1.0f, 1.0f, 1.0f };

// For adjusting the light draw rect for new object lighting.
#define LIT_OBJ_HEIGHT_ADJUSTMENT 60 

bool gameArea_ReDraw = false;
LONG gameArea_RoofRef = 0;
float gameArea_scaleX = 1.0f;
float gameArea_scaleY = 1.0f;

int mapLight_SkipReset = 0;
bool mapLightChange = false;
bool mapLight_recreateRenderTardets = false;

float ambientLightIntensity_old = 0;

FRMdx* mouseInfoframeDx = nullptr;
FRMdx* mouseInfoframeDx_Hit = nullptr;


//___________________________________________________
void GameAreas_ShiftWallRoofOpaqueness(int direction) {
    if (direction < 0) {
        if (opaqueness_WallRoof > 0.1f)
            opaqueness_WallRoof -= 0.1f;
        else
            opaqueness_WallRoof = 0.1f;
    }
    else {
        if (opaqueness_WallRoof < 1.0f)
            opaqueness_WallRoof += 0.1f;
        else
            opaqueness_WallRoof = 1.0f;
    }
}


//___________________________________________
bool DoRectsOverlap(RECT* rect1, RECT* rect2) {
    if (rect1->left > rect2->right)
        return false;
    if (rect1->right < rect2->left)
        return false;
    if (rect1->top > rect2->bottom)
        return false;
    if (rect1->bottom < rect2->top)
        return false;
    return true;
}



//_________________________________________
void BeforeLoadMapData(const char* mapName) {
    if (ConfigReadInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_FLOOR", 0))
        useOriginalLighting_Floor = true;
    if (ConfigReadInt(L"MAPS", L"USE_ORIGINAL_LIGHTING_OBJECTS", 0))
        useOriginalLighting_Objects = true;

    if (!GameAreas_Load(mapName))
        GameAreas_Load_Default();
    MapLight_Release_RenderTargets();//fixes first light not drawing shadows - render targets are recreated in DrawMapChanges
}


//_____________________
void AfterLoadMapData() {
    GameAreas_Load_Tiles();
}


/*latest sfall overrides this - loading_map_data_first & loading_map_data_last used instead.
//____________________________________________________
void __declspec(naked) loading_map_data(void) {
//00482AEC  |.  E8 83000000   CALL 00482B74                                                                               ; [fallout2.00482B74, int load_map_data(EAX *FileStream)

   __asm {
       mov isLoadingMapData, 1
       pushad
       push ebx
       call BeforeLoadMapData
       pop ebx
       popad

      call pfall_load_map_data
     
      pushad
      //push ebx
      call AfterLoadMapData
      //pop ebx
      popad
      mov isLoadingMapData, 0
      ret
   }
}
*/

//_________________________________________________
void __declspec(naked) loading_map_data_first(void) {

    __asm {
        mov isLoadingMapData, 1
        pushad
        push ebx //ebx should still hold the map name from before call to pfall_load_map_data
        call BeforeLoadMapData
        add esp, 0x4
        popad

        mov eax, 1
        ret
    }
}


//________________________________________________
void __declspec(naked) loading_map_data_last(void) {

    __asm {
        pushad
        call AfterLoadMapData
        popad

        mov isLoadingMapData, 0

        mov eax, -1
        ret
    }
}


//___________________________________________
void MapLight(OBJStruct* pObj, LONG subLight) {
    if (!pObj)
        return;

    float lightIntensity = pObj->light_intensity / 65536.0f;
    XMFLOAT4 lightColour = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (pObj == *ppObj_PC)
        lightColour = lightColour_PC;
    else if ((pObj->light_dist & 0xFFFFFF00) != 0) {
        lightColour.x = (float)((pObj->light_dist & 0xFF000000) >> 24) / 256.0f;
        lightColour.y = (float)((pObj->light_dist & 0x00FF0000) >> 16) / 256.0f;
        lightColour.z = (float)((pObj->light_dist & 0x0000FF00) >> 8) / 256.0f;
    }

    //We don't want the light colour to affect the intensity to much, so here we make sure at least one colour component is at the max of 1.0f with other components adjusted to maintain ratios.
    float max_component_intensity = lightColour.x;
    if (lightColour.y > max_component_intensity)
        max_component_intensity = lightColour.y;
    if (lightColour.z > max_component_intensity)
        max_component_intensity = lightColour.z;
    if (max_component_intensity == 0)
        lightColour = { 1.0f, 1.0f, 1.0f, 1.0f };
    else {
        lightColour.x /= max_component_intensity;
        lightColour.y /= max_component_intensity;
        lightColour.z /= max_component_intensity;
    }

    hexLight_ActiveColour = lightColour;
    lightColour = { lightColour.x * lightIntensity, lightColour.y * lightIntensity, lightColour.z * lightIntensity, 1.0f };

    if (mapLight_SkipReset != 0)
        return;

    bool addLight = true;
    if (subLight)
        return;

    if (pObj->light_intensity <= 0)
        return;
    if (pObj->flags & FLG_Disabled)
        return;
    if (!(pObj->flags & FLG_Emits_light))
        return;
    if (pObj->hexNum < 0 || pObj->hexNum >= *pNUM_HEXES)
        return;

    if (useOriginalLighting_Floor && useOriginalLighting_Objects)
        return;

    LONG ambientLightIntensity = GetAmbientLightIntensity();

    LONG xPos = 0;
    LONG yPos = 0;
    HexNumToSqr(pObj->hexNum, &xPos, &yPos);

    xPos -= 1;
    yPos -= 1;
    xPos += pObj->xShift;
    yPos += pObj->yShift;

    int lightHexDist = pObj->light_dist & 0x000000FF;
    int lightDistx = lightHexDist * 32;
    int lightDisty = lightHexDist * 12 + 24;

    RECT rect{ xPos - lightDistx, yPos - lightDisty, xPos + lightDistx, yPos + lightDisty };
    if (!objLight[pObj->level]) {
        if (!addLight)
            return;
        objLight[pObj->level] = new OBJlight;
        objLight[pObj->level]->pObj = pObj;
        objLight[pObj->level]->lightDx = { (float)xPos, (float)yPos, (float)lightHexDist * 32.0f, (float)pObj->light_intensity / 65536.0f };
        CopyRect(&objLight[pObj->level]->rect, &rect);
        objLight[pObj->level]->pRT_ShadowMask = nullptr;
        objLight[pObj->level]->colour = lightColour;
        objLight[pObj->level]->next = nullptr;

        DrawLightShadows(objLight[pObj->level], pObj->level);
        CheckLitObjectRect(&rect, pObj->level);

        if (pObj->level == *pMAP_LEVEL && pObj->light_intensity > ambientLightIntensity)
            mapLightChange = true;
        return;
    }

    OBJlight* pNode = objLight[pObj->level];
    OBJlight* pNodeNext = nullptr;
    OBJlight* pNodePrev = nullptr;
    while (pNode) {
        pNodeNext = pNode->next;
        if (pObj == pNode->pObj) {
            if (addLight) {
                pNode->lightDx = { (float)xPos, (float)yPos, (float)lightHexDist * 32.0f, (float)pObj->light_intensity / 65536.0f };
                CopyRect(&pNode->rect, &rect);
                DrawLightShadows(pNode, pObj->level);

            }
            else {
                if (pNodePrev)
                    pNodePrev->next = pNode->next;
                else
                    objLight[pObj->level] = nullptr;

                pNode->pObj = nullptr;
                if (pNode->pRT_ShadowMask)
                    delete pNode->pRT_ShadowMask;
                pNode->pRT_ShadowMask = nullptr;

                pNode->next = nullptr;
                delete pNode;
                pNode = nullptr;
            }
            CheckLitObjectRect(&rect, pObj->level);

            if (pObj->level == *pMAP_LEVEL && pObj->light_intensity > ambientLightIntensity)
                mapLightChange = true;
            return;
        }
        else if (pNodeNext == nullptr) {
            if (addLight) {
                pNode->next = new OBJlight;
                pNode->next->pObj = pObj;
                pNode->next->lightDx = { (float)xPos, (float)yPos, (float)lightHexDist * 32.0f, (float)pObj->light_intensity / 65536.0f };
                CopyRect(&pNode->next->rect, &rect);
                pNode->next->pRT_ShadowMask = nullptr;
                pNode->next->colour = lightColour;
                pNode->next->next = nullptr;

                DrawLightShadows(pNode->next, pObj->level);
                CheckLitObjectRect(&rect, pObj->level);

                if (pObj->level == *pMAP_LEVEL && pObj->light_intensity > ambientLightIntensity)
                    mapLightChange = true;
                return;
            }
        }
        pNodePrev = pNode;
        pNode = pNodeNext;
    }
}


//____________________________________
void __declspec(naked) map_light(void) {

    __asm {
        pushad

        push edx
        push eax
        call MapLight
        add esp, 0x8

        popad

        cmp esi, 0x9C40
        ret
    }
}


//___________________
void MapLight_Reset() {
    for (int level = 0; level < 3; level++) {
        if (pHexLight_ColourMap[level])
            delete[] pHexLight_ColourMap[level];
        pHexLight_ColourMap[level] = nullptr;
    }

    if (mapLight_SkipReset != 0)
        return;

    OBJlight* pNode = nullptr;
    OBJlight* pNodeNext = nullptr;
    for (int level = 0; level < 3; level++) {
        pNode = objLight[level];
        while (pNode) {
            pNodeNext = pNode->next;
            pNode->next = nullptr;
            pNode->pObj = nullptr;
            if (pNode->pRT_ShadowMask)
                delete pNode->pRT_ShadowMask;
            pNode->pRT_ShadowMask = nullptr;

            delete pNode;
            pNode = pNodeNext;
        }
        objLight[level] = nullptr;
    }
}


//___________________________________
void MapLight_Release_RenderTargets() {

    OBJlight* pNode = nullptr;
    for (int level = 0; level < 3; level++) {
        pNode = objLight[level];
        while (pNode) {
            if (pNode->pRT_ShadowMask)
                delete pNode->pRT_ShadowMask;
            pNode->pRT_ShadowMask = nullptr;
            pNode = pNode->next;;
        }
    }
    mapLight_recreateRenderTardets = true;
}


//____________________________________
void MapLight_ReCreate_RenderTargets() {
    if (useOriginalLighting_Floor)
        return;
    if (!mapLight_recreateRenderTardets)
        return;
    else
        mapLight_recreateRenderTardets = false;

    OBJlight* pNode = nullptr;
    for (int level = 0; level < 3; level++) {
        pNode = objLight[level];
        while (pNode) {
            DrawLightShadows(pNode, level);
            pNode = pNode->next;;
        }
    }
}

//this is also called when exiting fallout, so "objLight" mem is deallocated safely.
//__________________________________________
void __declspec(naked) map_light_reset(void) {

    __asm {
        pushad
        call MapLight_Reset
        popad
        mov ecx, 0x27100
        ret
    }
}


//____________________________________________________
bool CheckWallPosition_1(LONG hexNum_1, LONG hexNum_2) {
    LONG xp1, xp2, yp1, yp2;
    HexNumToSqr(hexNum_1, &xp1, &yp1);
    HexNumToSqr(hexNum_2, &xp2, &yp2);

    float xDiff = (float)xp1 - (float)xp2;
    float yDiff = (float)yp2 - (float)yp1;

    yDiff *= 4.0f; //100/25 drop off to left
    if (yDiff > xDiff)
        return false;
    return true;
}


//____________________________________________________
bool CheckWallPosition_2(LONG hexNum_1, LONG hexNum_2) {
    LONG xp1, xp2, yp1, yp2;
    HexNumToSqr(hexNum_1, &xp1, &yp1);
    HexNumToSqr(hexNum_2, &xp2, &yp2);

    float xDiff = (float)xp1 - (float)xp2;
    float yDiff = (float)yp2 - (float)yp1;

    yDiff *= -1.333333333333333f;//1.333333333333333f; //100/75 drop off to right
    if (yDiff > xDiff)
        return false;
    return true;
}


//__________________________________________________________
void DrawMouseObjDx(float scaleX, float scaleY, LONG layer) {
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    OBJStructDx* pObjDx = ((OBJStructDx*)*ppObj_Mouse);
    if (!pObjDx)
        return;
    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;

    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum != -1) {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;

    }
    if (!pObjDx->frmObjDx)
        pObjDx->frmObjDx = new FRMobj(*ppObj_Mouse, true);

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (pFrame == nullptr)
        return;

    xPos -= rcGame_PORTAL.left;
    yPos -= rcGame_PORTAL.top;
    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    MATRIX_DATA posData;
    XMMATRIX xmManipulation;
    XMMATRIX xmScaling;

    xmManipulation = DirectX::XMMatrixTranslation((float)xPos * scaleX, (float)yPos * scaleY, 0.0f);
    xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
    posData.World = XMMatrixMultiply(xmScaling, xmManipulation);
    XMMATRIX* pOrtho2D_SCRN = GetScreenProjectionMatrix_XM();
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D_SCRN);

    pPS_BuffersFallout->UpdatePositionBuff(&posData);

    XMFLOAT2 pal_colour = { 0.0f, 0.0f };
    if (pObjDx->combatFlags & FLG_NonPCTeamMem)
        pal_colour = { (float)PAL_OUTLINE_NOT_TEAM / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_MouseHex)
        pal_colour = { (float)PAL_OUTLINE_MOUSE / 256.0f, 0.3f };
    else if (pObjDx->combatFlags & FLG_combatUnk0x04)
        pal_colour = { (float)PAL_OUTLINE_UNKNOWN04 / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_PCTeamMem)
        pal_colour = { (float)PAL_OUTLINE_TEAM / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_ItemUnderMouse)
        pal_colour = { (float)PAL_OUTLINE_ITEM_UNDER_MOUSE / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_NotVisByPC)
        pal_colour = { (float)PAL_OUTLINE_NOT_VISIBLE / 256.0f, 1.0f };
    else
        pal_colour = { (float)PAL_OUTLINE_OTHER / 256.0f, 1.0f };

    //objData.PixelData.x = pFrame->pixelSize.x;
    //objData.PixelData.y = pFrame->pixelSize.y;
    pFrame->GetTexturePixelSize(&objData.PixelData.x, &objData.PixelData.y);
    objData.PalEffects.x = pal_colour.x;
    objData.PalEffects.y = pal_colour.y;

    pPS_BuffersFallout->UpdateObjBuff(&objData);

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

    pPS_BuffersFallout->SetPositionRender();

    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
    UINT frame_pixelWidth = pFrame->GetPixelWidth();
    if (layer == 0) {
        //set pixel shader stuff
        if (frame_pixelWidth == 1)
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        else
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

        //set texture stuff
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
        pD3DDevContext->DrawIndexed(4, 0, 0);
    }
    else if (layer == 1) {
        //----draw outlines-----------------------------------------
        if (pObjDx->combatFlags & FLG_IsOutlined) {
            //set pixel shader stuff
            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge8, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge32, nullptr, 0);

            pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
            pD3DDevContext->DrawIndexed(4, 0, 0);
        }
        //----draw-mouse-hex-info---num-action-points-from-pc-in-combat--etc----
        if (ppObj_Selected != nullptr && ((OBJStruct*)*ppObj_Selected)->frmID == 0x06000001 && mouseInfoframeDx != nullptr) {

            //set pixel shader stuff
            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

            pFrame = mouseInfoframeDx->GetFrame(0, 0);
            pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
            pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
            pD3DDevContext->DrawIndexed(4, 0, 0);
        }
    }
    pFrame = nullptr;
    pObjDx = nullptr;
}


//__________________________
void FRMobj::DrawObjLights() {

    if (useOriginalLighting_Objects)
        return;
    if (pD3DDevContext == nullptr)
        return;
    if (!pParentObj)
        return;
    if (pParentObj == *ppObj_Mouse)
        return;
    if ((pParentObj->flags & FLG_Flat))
        return;

    DWORD fID = pParentObj->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;

    XMFLOAT4 objPos = { 0,0,0,0 };

    LONG xPos = 0;
    LONG yPos = 0;
    if (pParentObj->hexNum != -1) {
        HexNumToSqr(pParentObj->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        objPos.x = (float)xPos;
        objPos.y = (float)yPos;
        xPos += pParentObj->xShift;
        yPos += pParentObj->yShift;
    }
    else {
        xPos = pParentObj->viewScrnX;
        yPos = pParentObj->viewScrnY;

        objPos.x = (float)xPos;
        objPos.y = (float)yPos;
    }
    DWORD flagsDx = pParentObj->flags;

    if (!frmNode) {
        imonitorInsertText("no frm node - draw obj lights");
        return;
    }
    FRMframeDx* pFrame = frmNode->GetFrame(pParentObj->ori, (WORD)pParentObj->frameNum);
    if (pFrame == nullptr)
        return;

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    objPos.x -= xPos;
    objPos.y -= yPos;

    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
    UINT frame_pixelWidth = pFrame->GetPixelWidth();
    DWORD frame_width = pFrame->GetWidth();
    DWORD frame_height = pFrame->GetHeight();
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);// , nullptr);

    MATRIX_DATA posData;
    XMMATRIX  Ortho2D = XMMatrixOrthographicOffCenterLH(0.0f, (float)(frame_width), (float)(frame_height), 0.0f, -0.5f, 0.5f);
    posData.World = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, Ortho2D);
    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();

    bool isEW_Wall = false, isNS_Wall = false;
    LONG xPosWall_iso = 0, yPosWall_iso = 0;

    if ((artType == ART_WALLS || artType == ART_SCENERY) && !(((OBJStruct*)*ppObj_PC)->flags & FLG_Disabled) && !(pParentObj->flags & FLG_TransAny)) {
        HexNumToHexPos(pParentObj->hexNum, &xPosWall_iso, &yPosWall_iso);
        PROTO* pProto = 0;
        fall_GetPro(pParentObj->proID, &pProto);
        DWORD actionFlags = pProto->wall.actionFlags;
        if (actionFlags & FLG_EastWest || actionFlags & FLG_EastCorner)
            isEW_Wall = true;
        else if (actionFlags & FLG_NorthCorner)
            isNS_Wall = true, isEW_Wall = true;
        else if (actionFlags & FLG_SouthCorner)
            isNS_Wall = true, isEW_Wall = true;
        else
            isNS_Wall = true;
    }

    if (artType == ART_WALLS) {
        //if(opaqueness < opaqueness_WallRoof)
        //    objData.PixelData.w = opaqueness;
        //else
            objData.PixelData.w = opaqueness_WallRoof;
    }
    else
        objData.PixelData.w = opaqueness;

    //objData.PixelData.x = pFrame->pixelSize.x;
    //objData.PixelData.y = pFrame->pixelSize.y;
    pFrame->GetTexturePixelSize(&objData.PixelData.x, &objData.PixelData.y);

    //set vertex buff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

    if (lpLightTex != nullptr) {
        delete lpLightTex;
        lpLightTex = nullptr;
    }
    lpLightTex = new RenderTarget(0, 0, frame_width, frame_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);

    if (!lpLightTex || !lpLightTex->GetShaderResourceView()) {
        Fallout_Debug_Error("CreateTexture  lpLightTex Fail");
        return;
    }

    float colour[4] = { 0,0,0,0 };
    lpLightTex->ClearRenderTarget(GetDepthStencilView(), colour);
    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    lpLightTex->SetRenderTargetAndViewPort(nullptr);

    pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

    LONG xLight_iso = 0, yLight_iso = 0;
    LONG xLightSqu = 0, yLightSqu = 0;

    pD3DDevContext->OMSetBlendState(pBlendState_Two, nullptr, -1);
    bool is_a1 = false;
    OBJlight* objLightNode = nullptr;
    XMFLOAT4 lightVec{ 0,0,0,0 };

    float tan75 = 3.7320508075688772935274463415059f;//tan(1.308);
    //float atan15 = 0.25624050515348744825170971977317f;//atan(0.262);
    //float tan52 = 1.279941632193078780311029847572;//tan(0.908);
    //float tan38 = 0.78128562650671739706294997196227f;//tan(0.663);
    float tan35 = 0.70020753820970977945852271944483f;
    float tan15 = 0.26794919243112270647255365849413f;//atan(0.262);

    objLightNode = objLight[pParentObj->level];
    while (objLightNode) {
        lightVec = objLightNode->lightDx;
        if (isEW_Wall || isNS_Wall) {
            HexNumToHexPos(objLightNode->pObj->hexNum, &xLight_iso, &yLight_iso);//get iso coord of light
            if (isNS_Wall && xLight_iso < xPosWall_iso) {
                objLightNode = objLightNode->next;
                continue;
            }
            else if (isEW_Wall && yLight_iso < yPosWall_iso) {
                objLightNode = objLightNode->next;
                continue;
            }
            if ((xLight_iso & 1))
                is_a1 = true;
            else is_a1 = false;

            if (isEW_Wall) {
                yLight_iso = yPosWall_iso;
                objPos.w = 0.8f;//make shape of light of wall thiner
            }
            else {
                xLight_iso = xPosWall_iso;
                objPos.w = 0.4f;// 0.5f; //make shape of light of wall thiner
            }
            HexToSqr(xLight_iso, yLight_iso, &xLightSqu, &yLightSqu);
            if (isNS_Wall) {
                if (is_a1)
                    xLightSqu -= 12;
            }
            else {
                if (is_a1) {
                    yLightSqu += 12;
                    xLightSqu += 8;
                }
                else {
                    xLightSqu -= 8;
                }
            }

            xLightSqu -= 1;
            yLightSqu -= 1;

            XMFLOAT2 distWall = { fabs(xLightSqu - lightVec.x), fabs(yLightSqu - lightVec.y) };

            lightVec.x = (float)xLightSqu - xPos;
            lightVec.y = (float)yLightSqu - yPos - 48;
            objPos.z = sqrt(fabs(distWall.x * distWall.x * tan35) + fabs(distWall.y * distWall.y * tan75));//how bright to object will be at its distance from the light source.

            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_DrawWallLight8, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_DrawWallLight32, nullptr, 0);
        }
        else {
            if (IsInLineOfSightBlocked(objLightNode->pObj->hexNum, pParentObj->hexNum, pParentObj->level, FLG_LightThru | FLG_TransNone, false)) {
                objLightNode = objLightNode->next;
                continue;
            }

            lightVec.x -= xPos;
            lightVec.y -= yPos;

            XMFLOAT2 distObj = { fabs(objPos.x - lightVec.x), fabs(objPos.y - lightVec.y) };

            // dont light objects between light and camera(south of light)
            if (objPos.y > lightVec.y) {

                if (objPos.x > lightVec.x) {//if obj east of light
                    if (objPos.y - (distObj.x * tan35) > lightVec.y - 12) {// -12)// dont light below angle of north/south wall   //sub 12 looks better
                        objLightNode = objLightNode->next;
                        continue;
                    }
                }
                else { //if obj west of light
                    if (objPos.y - (distObj.x * tan15) > lightVec.y) {// - 12)// dont light below angle of east/west wall
                        objLightNode = objLightNode->next;
                        continue;
                    }
                }
            }
            objPos.z = sqrt(fabs(distObj.x * distObj.x * tan35) + fabs(distObj.y * distObj.y * tan75));//how bright to object will be at its distance from the light source.
            lightVec.y -= 48;


            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_DrawObjLight8, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_DrawObjLight32, nullptr, 0);

        }

        objPos.z = 1.0f - objPos.z * objPos.z / (lightVec.z * lightVec.z);//set max brightnes for obj from distance from floor light
        if (objPos.z > 1.0f)
            objPos.z = 1.0f;
        else if (objPos.z < 0.0f)
            objPos.z = 0.0f;

        objData.objPos = objPos;
        objData.lightDetails = lightVec;
        objData.lightColour = objLightNode->colour;

        pPS_BuffersFallout->UpdateObjBuff(&objData);
        pD3DDevContext->DrawIndexed(4, 0, 0);

        objLightNode = objLightNode->next;
    }
    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);

    relight = false;
}


//__________________________________________
void ReleaseLitObjTextureDx(OBJStruct* pObj) {

    if (!pObj)
        return;
    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    if (pObjDx->frmObjDx)
        pObjDx->frmObjDx->ReleaseLightTexture();
}


//__________________________________________________________________
void DrawObjShadow(OBJStruct* pObj, RECT* pRect, XMMATRIX* pOrtho2D) {

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    if (pObj == *ppObj_Mouse)
        return;
    OBJStructDx* pObjDx = (OBJStructDx*)pObj;

    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;
    bool reduceFauxShadows = true;
    if (artType == ART_WALLS)
        reduceFauxShadows = false;

    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum != -1) {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }
    else {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }
    bool positionUpdatesOften = false;
    if (artType == ART_CRITTERS)
        positionUpdatesOften = true;
    if (!pObjDx->frmObjDx) {
        pObjDx->frmObjDx = new FRMobj(pObj, positionUpdatesOften);
    }

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();

    if (pFrame == nullptr)
        return;

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    DWORD frame_width = pFrame->GetWidth();
    DWORD frame_height = pFrame->GetHeight();
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);

    int xOffset = 0;
    if (xPos + (int)frame_width < pRect->left || xPos > pRect->right || yPos + (int)frame_height < pRect->top || yPos > pRect->bottom)
        return;

    MATRIX_DATA posData;

    posData.World = XMMatrixTranslation((float)xPos - pRect->left, (float)yPos - pRect->top, (float)0);
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D);
    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PS_Shadow1_DrawBase, nullptr, 0);

    //set texture stuff  -draw-
    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetBaseTex(reduceFauxShadows);
    if (pframe_Tex_shaderResourceView) {
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
        pD3DDevContext->DrawIndexed(4, 0, 0);
    }

    pFrame = nullptr;
    pObjDx = nullptr;
}


//________________________________________________
void DrawLightShadows(OBJlight* light, LONG level) {
    if (useOriginalLighting_Floor)
        return;

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;

    UINT width = (UINT)(light->rect.right - light->rect.left);
    UINT height = (UINT)(light->rect.bottom - light->rect.top);
    if (width < 1 || height < 1)
        return;
    int lightDistx = width / 2;
    int lightDisty = height / 2;

    RECT rect = { (int)light->lightDx.x - lightDistx, (int)light->lightDx.y - lightDisty, (int)light->lightDx.x + lightDistx, (int)light->lightDx.y + lightDisty };

    if (light->pRT_ShadowMask)
        delete light->pRT_ShadowMask;
    light->pRT_ShadowMask = new RenderTarget(0, 0, width, height, DXGI_FORMAT_R8_UNORM, 0x00000000);

    RenderTarget* pRT_ShadowMask2 = new RenderTarget(0, 0, width, height, DXGI_FORMAT_R8_UNORM, 0x00000000);

    float colour[4] = { 0,0,0,0 };
    light->pRT_ShadowMask->ClearRenderTarget(nullptr, colour);
    pRT_ShadowMask2->ClearRenderTarget(nullptr, colour);

    if (pAreObjectsInitiated == nullptr || *pAreObjectsInitiated == 0) {
        delete pRT_ShadowMask2;
        return;
    }

    XMMATRIX Ortho2D_XM;
    light->pRT_ShadowMask->GetOrthoMatrix(&Ortho2D_XM);

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    light->pRT_ShadowMask->SetRenderTarget(nullptr);
    Set_ViewPort(width, height);

    OBJNode* mapObj = nullptr;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];
        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (!(mapObj->pObj->flags & FLG_Flat) && !(mapObj->pObj->flags & FLG_Disabled) && !(mapObj->pObj->flags & (FLG_LightThru)))//|FLG_ShootThru|FLG_TransNone)))
                        DrawObjShadow(mapObj->pObj, &rect, &Ortho2D_XM);
                }
            }
            else
                mapObj = nullptr;
            if (mapObj)
                mapObj = mapObj->next;
        }
    }

    objData.PixelData.x = 1.0f / (float)width;
    objData.PixelData.y = 1.0f / (float)height;
    objData.lightColour = light->colour;
    objData.lightDetails = light->lightDx;

    pPS_BuffersFallout->UpdateObjBuff(&objData);

    ID3D11Buffer* pd3dVB;
    light->pRT_ShadowMask->GetVertexBuffer(&pd3dVB);

    MATRIX_DATA posData;
    posData.World = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, Ortho2D_XM);
    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    ID3D11ShaderResourceView* pTex_shaderResourceView = nullptr;

    //build shadows
    for (int i = 0; i < 4; i++) {
        //set pixel shader stuff
        pD3DDevContext->PSSetShader(pd3d_PS_Shadow2_Build, nullptr, 0);

        pRT_ShadowMask2->SetRenderTarget(nullptr);
        pTex_shaderResourceView = light->pRT_ShadowMask->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

        pD3DDevContext->DrawIndexed(4, 0, 0);

        light->pRT_ShadowMask->SetRenderTarget(nullptr);
        pTex_shaderResourceView = pRT_ShadowMask2->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

        pD3DDevContext->DrawIndexed(4, 0, 0);
    }

    pD3DDevContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Linear);
    genSurfaceData.genData4_1 = { (float)width , (float)height , 0.0f, 1.0f };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    for (int i = 0; i < 3; i++) {
        pD3DDevContext->PSSetShader(pd3d_PS_GaussianBlurU, nullptr, 0);
        pRT_ShadowMask2->SetRenderTarget(nullptr);
        pTex_shaderResourceView = light->pRT_ShadowMask->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
        pD3DDevContext->DrawIndexed(4, 0, 0);

        pD3DDevContext->PSSetShader(pd3d_PS_GaussianBlurV, nullptr, 0);
        light->pRT_ShadowMask->SetRenderTarget(nullptr);
        pTex_shaderResourceView = pRT_ShadowMask2->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
        pD3DDevContext->DrawIndexed(4, 0, 0);

    }
    pD3DDevContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Point);

    delete pRT_ShadowMask2;
    pRT_ShadowMask2 = nullptr;
}


//________________________________________
//prevent game from recreating all dx lights and drawing the whole screen when opening and closing doors.
//regular hex light map updates as normal
void RedrawAffectedLights(OBJStruct* pObj) {

    if (useOriginalLighting_Floor && useOriginalLighting_Objects)
        return;
    if (pObj == *ppObj_Mouse)
        return;
    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;

    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum != -1) {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }
    else {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }

    if (!pObjDx->frmObjDx)
        return;

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (pFrame == nullptr)
        return;
    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    OBJlight* objLightNode = nullptr;
    objLightNode = objLight[pObjDx->level];
    RECT* pDrawRect = nullptr;

    while (objLightNode) {
        if (xPos + (int)pFrame->GetWidth() < objLightNode->rect.left || xPos > objLightNode->rect.right || yPos + (int)pFrame->GetHeight() < objLightNode->rect.top || yPos > objLightNode->rect.bottom) {
            objLightNode = objLightNode->next;
        }
        else if (IsInLineOfSightBlocked(objLightNode->pObj->hexNum, pObj->hexNum, pObjDx->level, FLG_LightThru | FLG_TransNone, false)) {
            objLightNode = objLightNode->next;
        }
        else {
            if (!pDrawRect) {
                pDrawRect = new RECT;
                CopyRect(pDrawRect, &objLightNode->rect);
            }
            else
                UnionRect(pDrawRect, &objLightNode->rect, pDrawRect);
            objLightNode = objLightNode->next;
        }

    }
    if (pDrawRect && pObjDx->level == *pMAP_LEVEL) {
        DrawMapChanges(pDrawRect, *pMAP_LEVEL, FLG_Floor | FLG_Obj);
        delete pDrawRect;
        pDrawRect = nullptr;
    }
}


//____________________________________________
void __declspec(naked) redraw_door_light(void) {

    __asm {
        pushad
        mov mapLight_SkipReset, 1
        call fall_Map_RefreshLights
        mov mapLight_SkipReset, 0
        push esi
        call RedrawAffectedLights
        add esp, 0x4
        popad
        ret
    }
}


//______________________________________________
bool CheckLitObjDx(OBJStruct* pObj, RECT* pRect) {

    if (pObj == *ppObj_Mouse)
        return false;
    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return false;

    LONG xPos = 0;
    LONG yPos = 0;

    if (pObjDx->hexNum == -1) {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }
    else {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }

    if (!pObjDx->frmObjDx)
        return false;

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();

    if (pFrame == nullptr)
        return false;

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    if (xPos + (int)pFrame->GetWidth() < pRect->left || xPos > pRect->right || yPos + (int)pFrame->GetHeight() < pRect->top - LIT_OBJ_HEIGHT_ADJUSTMENT || yPos > pRect->bottom)
        return false;

    pObjDx->frmObjDx->Relight();

    return true;
}


//_____________________________________________
void CheckLitObjectRect(RECT* rect, LONG level) {
    if (pAreObjectsInitiated == nullptr || *pAreObjectsInitiated == 0)
        return;

    OBJNode* mapObj = nullptr;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];
        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (!(mapObj->pObj->flags & FLG_Disabled)) {
                        CheckLitObjDx(mapObj->pObj, rect);
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


//________________________________________________________________
bool GetObjRectDxPalAni(OBJStruct* pObj, RECT* pRect, DWORD flags) {
    *pRect = { 0,0,0,0 };
    if (!pObj)
        return false;
    DWORD fID = pObj->frmID;
    if (!IsArtTypeEnabled((0x0F000000 & fID) >> 24))
        return false;

    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    if (!pObjDx->frmObjDx)
        return false;
    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (!pFrame)
        return false;
    if (!pFrame->IsAnimated() && !(pObjDx->combatFlags & FLG_NonPCTeamMem) && !(pObjDx->combatFlags & FLG_PCTeamMem))
        return false;

    if (!pFrame->IsAnimated()) {//combat flags set to FLG_NonPCTeamMem|FLG_PCTeamMem - cycling colours
        if (!pFrame->GetRect(pRect))
            return false;
    }
    else {
        if (!(flags & pFrame->Animation_ZoneFlags()))
            return false;
        if (!pFrame->GetAniRect(pRect))
            return false;
    }
    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum == -1) {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }
    else {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }
    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();


    pRect->left += xPos;
    pRect->top += yPos;
    pRect->right += xPos;
    pRect->bottom += yPos;

    return true;
}


//_____________________________________________
bool GetObjRectDx(OBJStruct* pObj, RECT* pRect) {
    *pRect = { 0,0,0,0 };
    if (!pObj)
        return false;
    DWORD fID = pObj->frmID;
    if (!IsArtTypeEnabled((0x0F000000 & fID) >> 24))
        return false;

    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    if (!pObjDx->frmObjDx)
        return false;

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (!pFrame)
        return false;
    if (!pFrame->GetRect(pRect))
        return false;

    LONG xPos = 0;
    LONG yPos = 0;

    if (pObjDx->hexNum == -1) {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }
    else {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);

        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    pRect->left += xPos;
    pRect->top += yPos;
    pRect->right += xPos;
    pRect->bottom += yPos;
    return true;
}


//_____________________________________________
void __declspec(naked) get_object_rect_dx(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call GetObjRectDx
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }

}


//____________________________________________________________
void GameAreas_FadingObjects_Add(OBJStruct* pObj, bool fadeIn) {
    ULONGLONG time = GetTickCount64();
    if (!frmObjFadeList) {
        frmObjFadeList = new OBJFadeNode;
        frmObjFadeList->pObj = (OBJStructDx*)pObj;
        frmObjFadeList->next = nullptr;
        frmObjFadeList->fadeStartTime = time;
        frmObjFadeList->fadeIn = fadeIn;
        return;
    }

    OBJFadeNode* node = frmObjFadeList;
    while (node) {
        if (node->pObj == (OBJStructDx*)pObj) {
            node->fadeStartTime = time;
            node->fadeIn = fadeIn;
            node = nullptr;
            return;
        }
        if (node->next == nullptr) {
            node->next = new OBJFadeNode;
            node = node->next;
            node->pObj = (OBJStructDx*)pObj;
            node->next = nullptr;
            node->fadeStartTime = time;
            node->fadeIn = fadeIn;
            node = nullptr;
            return;
        }
        node = node->next;
    }
}


//_______________________________________________
void GameAreas_FadingObjects_Sub(OBJStruct* pObj) {
    if (!frmObjFadeList)
        return;
    OBJFadeNode* prvNode = nullptr;
    OBJFadeNode* node = frmObjFadeList;
    OBJFadeNode* nextNode = nullptr;
    while (node) {
        if (node->pObj == (OBJStructDx*)pObj) {
            nextNode = node->next;
            node->pObj = nullptr;
            node->next = nullptr;
            delete node;
            node = prvNode;
            if (prvNode == nullptr)
                frmObjFadeList = nextNode;
            else
                node->next = nextNode;
            prvNode = nullptr;
            nextNode = nullptr;
            node = nullptr;
            return;
        }
        prvNode = node;
        node = node->next;
    }
}


//___________________________________
void GameAreas_FadingObjects_delete() {
    if (!frmObjFadeList)
        return;
    OBJFadeNode* node = frmObjFadeList;
    OBJFadeNode* nextNode = nullptr;
    while (node) {
        nextNode = node->next;
        node->pObj = nullptr;
        node->next = nullptr;
        delete node;
        node = nextNode;
    }
    frmObjFadeList = nullptr;
    node = nullptr;
    nextNode = nullptr;
}


//_________________________________
void GameAreas_FadingObjects_Draw() {

    OBJFadeNode* prvNode = nullptr;
    OBJFadeNode* node = frmObjFadeList;
    OBJFadeNode* nextNode = nullptr;
    ULONGLONG time = GetTickCount64();
    ULONGLONG timeDiff = 0;
    RECT rect;
    LONG type;
    while (node) {
        timeDiff = time - node->fadeStartTime;
        type = (node->pObj->frmID & 0x0F000000) >> 24;
        //if (timeDiff < 500){// && (FOG_OF_WAR > 2 || type == ART_CRITTERS || type == ART_ITEMS)) {
        if (timeDiff < 500 && (type == ART_CRITTERS || type == ART_ITEMS)) {
            if (GetObjRectDx((OBJStruct*)node->pObj, &rect)) {
                node->pObj->frmObjDx->opaqueness = (float)timeDiff / 500.0f;
                if (!node->fadeIn)
                    node->pObj->frmObjDx->opaqueness = 1.0f - node->pObj->frmObjDx->opaqueness;
                DrawMapChanges(&rect, *pMAP_LEVEL, FLG_Obj);
            }
            prvNode = node;
            node = node->next;
        }
        else {
            if (GetObjRectDx((OBJStruct*)node->pObj, &rect)) {
                if (!node->fadeIn)
                    node->pObj->frmObjDx->opaqueness = 0.0f;
                else
                    node->pObj->frmObjDx->opaqueness = 1.0f;
                DrawMapChanges(&rect, *pMAP_LEVEL, FLG_Obj);

                nextNode = node->next;
                node->pObj = nullptr;
                node->next = nullptr;
                delete node;
                node = prvNode;
                if (!prvNode) {
                    node = nextNode;
                    frmObjFadeList = node;
                    if (!node)
                        return;
                }
                else {
                    node->next = nextNode;
                    prvNode = node;
                    node = node->next;
                }
            }
            else {
                prvNode = node;
                node = node->next;
            }
        }
    }
}


//___________________________________
bool GAME_AREA::CreateRendertargets() {
    bool redraw = false;

    if (!pLight_RT)
        pLight_RT = new RenderTarget2((float)rect.left - rcGame_PORTAL.left, (float)rect.top - rcGame_PORTAL.top, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);

    if (!pFloor_RT) {///recreate render targets after screen reset
        pFloor_RT = new RenderTarget2((float)rect.left - rcGame_PORTAL.left, (float)rect.top - rcGame_PORTAL.top, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
        pFloor_RT->ScalePosition(true);
        pFloor_RT->SetScale(gameArea_scaleX, gameArea_scaleY);
        redraw = true;
    }
    if (!pObjects_RT) {
        pObjects_RT = new RenderTarget2((float)rect.left - rcGame_PORTAL.left, (float)rect.top - rcGame_PORTAL.top, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
        pObjects_RT->ScalePosition(true);
        pObjects_RT->SetScale(gameArea_scaleX, gameArea_scaleY);
        redraw = true;
    }
    if (!pHud_RT_Outline) {
        pHud_RT_Outline = new RenderTarget2(0, 0, game_PortalWidth, game_PortalHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
        pHud_RT_Outline->ScalePosition(true);
        pHud_RT_Outline->SetScale(gameArea_scaleX, gameArea_scaleY);
        redraw = true;
    }
    if (!pHud_RT) {
        pHud_RT = new RenderTarget2((float)rect.left - rcGame_PORTAL.left, (float)rect.top - rcGame_PORTAL.top, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
        pHud_RT->ScalePosition(true);
        pHud_RT->SetScale(gameArea_scaleX, gameArea_scaleY);
        redraw = true;
    }
    if (FOG_OF_WAR && fogLight && !pFog_RT) {
        pFog_RT = new RenderTarget2((float)rect.left - rcGame_PORTAL.left, (float)rect.top - rcGame_PORTAL.top, width, height, DXGI_FORMAT_R8_UNORM, 0x00000000);
        pFog_RT->ScalePosition(true);
        pFog_RT->SetScale(gameArea_scaleX, gameArea_scaleY);
        redraw = true;
    }
    return redraw;
}


//___________________________________
GAME_AREA* GameAreas_GetCurrentArea() {
    return pGameArea_Current;
}


//______________________________
void PS_UpdatePortalDimensions() {
    portalDimensionsData.portal_dim.x = (float)rcGame_PORTAL.left;
    portalDimensionsData.portal_dim.y = (float)rcGame_PORTAL.top;
    pPS_BuffersFallout->UpdatePortalDimensions(&portalDimensionsData);
}


//_______________________
void GameAreas_SetScale() {
    if (pWinRef_GameArea && *pWinRef_GameArea != -1) {
        WinStruct* pWin = fall_Win_Get(*pWinRef_GameArea);
        float scaleX = (float)pWin->width / (float)(game_PortalWidth);
        float scaleY = (float)pWin->height / (float)(game_PortalHeight);

        gameArea_scaleX = scaleX;
        gameArea_scaleY = scaleY;
        if (pGameArea_Current) {
            pGameArea_Current->SetScale(gameArea_scaleX, gameArea_scaleY);
            SetViewPosition_Hex(*pVIEW_HEXPOS, 0);
        }
    }
}

//To-Do Update floating text and mouse text code.
//_________________________________________________________
void FloatingText_CreateFrameDx(FloatingTextObjDx* txtObj) {
    txtObj->frameDX = new FRMframeDx(nullptr, txtObj->imgWidth, txtObj->imgHeight, txtObj->pImgBuff, nullptr, false, false);
}


//_____________________________________________________
void __declspec(naked) floating_text_create_frame(void) {

    __asm {
        pushad

        push esi
        call FloatingText_CreateFrameDx
        add esp, 0x4

        popad

        push eax
        call fall_SetFont
        add esp, 0x4
        ret
    }
}


//__________________________________________________________
void FloatingText_DestroyFrameDx (FloatingTextObjDx *txtObj) {
   if(txtObj->frameDX)
      delete txtObj->frameDX;
   txtObj->frameDX=nullptr;
}


//_____________________________________________________
void __declspec(naked) floating_text_destroy_frame(void) {

    __asm {
        pushad

        push eax
        call FloatingText_DestroyFrameDx
        add esp, 0x4

        popad

        pushad

        mov eax, dword ptr ds : [eax + 0x2C]
        push eax
        call fall_Mem_Deallocate
        add esp, 0x4

        popad
        ret
    }
}


//____________________
void DestroyHitMouse() {

    if (mouseInfoframeDx_Hit)
        delete mouseInfoframeDx_Hit;
    mouseInfoframeDx_Hit = nullptr;
}


//____________________________________________
void __declspec(naked) destroy_hit_mouse(void) {

    __asm {
        pushad
        push eax
        call fall_Frm_Unload
        add esp, 0x4
        call DestroyHitMouse
        popad
        ret
    }
}


//______________________________________________________________________________________________________
void ReDrawHitMouse(BYTE* txtBackBuff, LONG subWidth, LONG subHeight, LONG buffWidth, DWORD Colour) {
    //Draw out line around text to buffer
    fall_DrawTextOutline(txtBackBuff, subWidth, subHeight, buffWidth, Colour);

    if (!mouseInfoframeDx_Hit)
        mouseInfoframeDx_Hit = new FRMdx(0x0600011C);


    //txtBackBuff has been offset by the cursor width for drawing text,
    //must remove subtract it to allow whole buffer to be drawn to frameDx.
    DWORD buff_Offset = buffWidth - subWidth;

    FRMframeDx* frameDx = mouseInfoframeDx_Hit->GetFrame(0, 0);
    if (frameDx)
        frameDx->DrawToFrame(txtBackBuff - buff_Offset, nullptr, false, buffWidth, subHeight, 0, 0, buffWidth, subHeight, 0, 0);
    frameDx = nullptr;
}


//___________________________________________
void __declspec(naked) redraw_hit_mouse(void) {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ds : [esp + 0x10]
        push ecx
        push ebx
        push edx
        push eax
        call ReDrawHitMouse
        add esp, 0x14

        pop ebp
        pop edi
        pop esi
        ret 4
    }
}


//____________________
void DestroyHexMouse() {

    if (mouseInfoframeDx)
        delete mouseInfoframeDx;
    mouseInfoframeDx = nullptr;
}


//____________________________________________
void __declspec(naked) destroy_hex_mouse(void) {

    __asm {
        pushad
        push eax
        call fall_Frm_Unload
        add esp, 0x4
        call DestroyHexMouse
        popad
        ret
    }
}


//_________________________________________________________________________________________________
void ReDrawHexMouse(BYTE* txtBackBuff, LONG subWidth, LONG subHeight, LONG buffWidth, DWORD Colour) {
    //Draw out line around text to buffer
    fall_DrawTextOutline(txtBackBuff, subWidth, subHeight, buffWidth, Colour);

    if (!mouseInfoframeDx)
        mouseInfoframeDx = new FRMdx(0x06000001);

    FRMframeDx* frameDx = mouseInfoframeDx->GetFrame(0, 0);
    if (frameDx)
        frameDx->DrawToFrame(txtBackBuff, nullptr, false, subWidth, subHeight, 0, 0, buffWidth, subHeight, 1, 1);
    frameDx = nullptr;
}


//___________________________________________
void __declspec(naked) redraw_hex_mouse(void) {

    __asm {
        push esi
        push edi
        push ebp

        push dword ptr ds : [esp + 0x10]
        push ecx
        push ebx
        push edx
        push eax
        call ReDrawHexMouse
        add esp, 0x14

        pop ebp
        pop edi
        pop esi
        ret 4
    }
}


int tilecount = 0;//debug counter
int tilenodes_deleted = 0;//debug counter

//______________________
void GameAreas_Destroy() {
    for (int level = 0; level < 3; level++) {
        if (pGameArea[level])
            delete pGameArea[level];
        pGameArea[level] = nullptr;
    }
    pGameArea_Current = nullptr;

    GameAreas_FadingObjects_delete();
}


//__________________________________
void DrawRoof(TILE_AREA_Node* pRoof) {

    if (!pRoof)
        return;
    if (!pRoof->tiles)
        return;
    if (!pRoof->renderTarget)
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    FRMframeDx* pFrame = nullptr;
    LONG xPos = 0, yPos = 0;

    TILEnode* pTile = pRoof->tiles;
    while (pTile) {
        if (!pTile->pFrm) {
            //imonitorInsertText("no frm for roof tile");
            pTile = pTile->next;
            continue;
        }
        pFrame = pTile->pFrm->GetFrame(pTile->frmID, 0, 0);
        if (pFrame == nullptr) {
            // imonitorInsertText("no frame for roof tile");
            pTile = pTile->next;
            continue;
        }
        xPos = pTile->x - pRoof->rect.left;
        yPos = pTile->y - pRoof->rect.top;

        ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
        ID3D11Buffer* frame_pd3dVB = nullptr;
        pFrame->GetVertexBuffer(&frame_pd3dVB);// , nullptr);

        //set vertex stuff
        UINT stride = sizeof(VERTEX_BASE);
        UINT offset = 0;
        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
        //set texture stuff
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

        //set pixel shader stuff
        if (pFrame->GetPixelWidth() == 1) {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        }
        else {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);
        }

        pRoof->renderTarget->DrawIndexed(4, 0, 0, nullptr, (float)xPos, (float)yPos, pFrame->GetWidth(), pFrame->GetHeight(), nullptr);

        pTile = pTile->next;
    }

    SetAmbientLight_ShaderEffect();
}


//________________________________________________________________________________________________________________________
bool GameArea_CheckRoofTile(int tileX, int tileY, int level, int areaNum, GAME_AREA* pGameArea, TILE_AREA_Node* pAreaNode) {
    if (!pAreaNode)
        return false;
    if (level < 0 || level >= 3)
        return false;
    if (tileX < 0 || tileX >= *pNUM_TILE_X)
        return false;
    if (tileY < 0 || tileY >= *pNUM_TILE_Y)
        return false;
    if (tileX < pGameArea->tileLimits.west || tileX >= pGameArea->tileLimits.east)
        return false;
    if (tileY < pGameArea->tileLimits.north || tileY >= pGameArea->tileLimits.south)
        return false;

    LONG tilePosNum = tileX + (tileY * *pNUM_TILE_X);


    int* pMarkedTile = pGameArea->pMarkedRoofTile;

    if (pMarkedTile[tilePosNum])
        return false;

    DWORD** levelOffset = *pMapTileLevelOffset;
    LONG tileLstNum = levelOffset[level][tilePosNum];
    LONG tileFlag = 0;

    tileFlag = (tileLstNum & 0xF0000000) >> 28;
    tileLstNum = (tileLstNum & 0x0FFF0000) >> 16;

    if (tileLstNum == 1)
        return false;

    tilecount++;//debug counter
    pMarkedTile[tilePosNum] = areaNum;

    TILEnode* thisTile = nullptr;
    FRMframeDx* pFrame = nullptr;

    if (!pAreaNode->tiles) {
        pAreaNode->tiles = new TILEnode;
        thisTile = pAreaNode->tiles;
    }
    else {
        thisTile = pAreaNode->tiles;
        while (thisTile->next != nullptr)
            thisTile = thisTile->next;
        thisTile->next = new TILEnode;
        thisTile = thisTile->next;
    }
    thisTile->tileNum = tilePosNum;
    thisTile->frmID = 0x04000000 | tileLstNum;
    thisTile->pFrm = new FRMtile(thisTile->frmID, 0);
    pFrame = thisTile->pFrm->GetFrame(thisTile->frmID, 0, 0);
    thisTile->next = nullptr;
    LONG xPos = 0, yPos = 0;
    TileToSqr(tilePosNum, &xPos, &yPos);

    yPos += 24;
    xPos += -25;

    yPos -= 96;

    if (pFrame) {
        xPos += pFrame->GetOffset_OriCentre_X();
        yPos += pFrame->GetOffset_OriCentre_Y();
    }
    thisTile->x = xPos;
    thisTile->y = yPos;

    if (xPos < pGameArea->rect.left)
        xPos = pGameArea->rect.left;
    if (xPos + 80 > pGameArea->rect.right)
        xPos = pGameArea->rect.right - 80;

    if (yPos < pGameArea->rect.top)
        yPos = pGameArea->rect.top;
    if (yPos + 36 > pGameArea->rect.bottom)
        yPos = pGameArea->rect.bottom - 36;


    if (pAreaNode->rect.left == -1) {
        pAreaNode->rect.left = xPos;
        pAreaNode->rect.right = xPos + 80;
        pAreaNode->rect.top = yPos;
        pAreaNode->rect.bottom = yPos + 36;
    }
    else {
        if (pAreaNode->rect.left > xPos)
            pAreaNode->rect.left = xPos;
        else if (pAreaNode->rect.right < xPos + 80)
            pAreaNode->rect.right = xPos + 80;
        if (pAreaNode->rect.top > yPos)
            pAreaNode->rect.top = yPos;
        else if (pAreaNode->rect.bottom < yPos + 36)
            pAreaNode->rect.bottom = yPos + 36;
    }
    pAreaNode->width = pAreaNode->rect.right - pAreaNode->rect.left + 1;
    pAreaNode->height = pAreaNode->rect.bottom - pAreaNode->rect.top + 1;
    thisTile = nullptr;

    return true;
}


//____________________________________________________________________________________________________________________________________________________________
bool GameArea_CheckAdjoiningRoofTiles(int tileX, int tileY, int level, int areaNum, GAME_AREA* pGameArea, checkTileNODE** pch_tile, TILE_AREA_Node* pAreaNode) {

    checkTileNODE* ch_tile = nullptr;
    checkTileNODE* selected = nullptr;

    if (GameArea_CheckRoofTile(tileX - 1, tileY, level, areaNum, pGameArea, pAreaNode)) {
        if (!ch_tile) {
            ch_tile = new checkTileNODE;
            selected = ch_tile;
        }
        selected->areaNum = areaNum;
        selected->level = level;
        selected->x = tileX - 1;
        selected->y = tileY;
        selected->next = nullptr;
    }
    if (GameArea_CheckRoofTile(tileX + 1, tileY, level, areaNum, pGameArea, pAreaNode)) {
        if (!ch_tile) {
            ch_tile = new checkTileNODE;
            selected = ch_tile;
        }
        else {
            selected->next = new checkTileNODE;
            selected = selected->next;
        }

        selected->areaNum = areaNum;
        selected->level = level;
        selected->x = tileX + 1;
        selected->y = tileY;
        selected->next = nullptr;
    }
    if (GameArea_CheckRoofTile(tileX, tileY - 1, level, areaNum, pGameArea, pAreaNode)) {
        if (!ch_tile) {
            ch_tile = new checkTileNODE;
            selected = ch_tile;
        }
        else {
            selected->next = new checkTileNODE;
            selected = selected->next;
        }

        selected->areaNum = areaNum;
        selected->level = level;
        selected->x = tileX;
        selected->y = tileY - 1;
        selected->next = nullptr;
    }
    if (GameArea_CheckRoofTile(tileX, tileY + 1, level, areaNum, pGameArea, pAreaNode)) {
        if (!ch_tile) {
            ch_tile = new checkTileNODE;
            selected = ch_tile;
        }
        else {
            selected->next = new checkTileNODE;
            selected = selected->next;
        }

        selected->areaNum = areaNum;
        selected->level = level;
        selected->x = tileX;
        selected->y = tileY + 1;
        selected->next = nullptr;
    }

    *pch_tile = ch_tile;
    return true;
}


//________________________________________________________________________________________________________________________
bool GameArea_FindRoofTiles(int tileX, int tileY, int level, int roofNum, GAME_AREA* pGameArea, TILE_AREA_Node* pRoofNode) {

    if (!pGameArea)
        return false;
    if (!pRoofNode)
        return false;
    if (!GameArea_CheckRoofTile(tileX, tileY, level, roofNum, pGameArea, pRoofNode))
        return false;

    checkTileNODE* more_tiles = nullptr;
    GameArea_CheckAdjoiningRoofTiles(tileX, tileY, level, roofNum, pGameArea, &more_tiles, pRoofNode);
    if (!more_tiles)
        return true;//area must be only one tile - success


    checkTileNODE* selected = more_tiles;
    checkTileNODE* deleting = nullptr;
    checkTileNODE* holding = nullptr;

    while (selected) {
        more_tiles = nullptr;
        GameArea_CheckAdjoiningRoofTiles(selected->x, selected->y, selected->level, selected->areaNum, pGameArea, &more_tiles, pRoofNode);
        if (more_tiles) {
            holding = selected->next; //hold current next selection
            selected->next = more_tiles; //move more_tiles to  next selection
            while (more_tiles->next) //find last more_tiles and
                more_tiles = more_tiles->next;
            more_tiles->next = holding;//add the old next selected to the end
        }
        deleting = selected; //current selected processed - mark for deletion
        selected = selected->next;  //move selected to next tile
        delete deleting; // delete old selected
        deleting = nullptr;
        tilenodes_deleted++; //debug counter
    }
    return true;
}


//___________________________________________________________
void GameArea_CreateRoofList(GAME_AREA* pGameArea, int level) {
    if (!pGameArea)
        return;

    DWORD** levelOffset = *pMapTileLevelOffset;
    LONG tileLstNum = 0;
    LONG tileFlag = 0;
    int tileY = 0, tileX = 0;
    int roofNum = 0;

    TILE_AREA_Node* pThisRoof = nullptr;

    if (pGameArea->rooves)
        pGameArea->deleteAllRoofData();

    if (pGameArea->pMarkedRoofTile)
        delete[] pGameArea->pMarkedRoofTile;
    pGameArea->pMarkedRoofTile = new int[*pNUM_TILES];
    memset(pGameArea->pMarkedRoofTile, 0, *pNUM_TILES * sizeof(int));

    tilenodes_deleted = 0;
    tilecount = 0;
    roofNum = 0;

    int yTileLine = pGameArea->tileLimits.north * *pNUM_TILE_X;
    for (int yTile = pGameArea->tileLimits.north; yTile < pGameArea->tileLimits.south; yTile++) {
        for (int xTile = pGameArea->tileLimits.west; xTile < pGameArea->tileLimits.east; xTile++) {

            tileLstNum = levelOffset[level][yTileLine + xTile];

            if (SFALL_MoreTiles) {
                tileFlag = (tileLstNum & 0xC0000000) >> 30;
                tileLstNum = tileLstNum & 0x3FFF0000 >> 16;
            }
            else {
                tileFlag = (tileLstNum & 0xF0000000) >> 28;
                tileLstNum = (tileLstNum & 0x0FFF0000) >> 16;
            }


            if (tileLstNum != 1 && pGameArea->pMarkedRoofTile[yTileLine + xTile] == 0) {
                roofNum++;
                if (pGameArea->rooves == nullptr) {
                    pGameArea->rooves = new TILE_AREA_Node;
                    pThisRoof = pGameArea->rooves;

                }
                else {
                    pThisRoof->next = new TILE_AREA_Node;
                    pThisRoof = pThisRoof->next;
                }
                GameArea_FindRoofTiles(xTile, yTile, level, roofNum, pGameArea, pThisRoof);
            }
        }
        yTileLine += *pNUM_TILE_X;
    }

    //char msg[128];
    //sprintf_s(msg, 128, "num roof tiles = %d, deleted = %d", tilecount, tilenodes_deleted);
    //imonitorInsertText(msg);
    //sprintf_s(msg, 128, "level %d num rooves = %d", level, roofNum);
    //imonitorInsertText(msg);
}


//___________________________________________________________________________
void GameArea_DisplayRooves(GAME_AREA* pGameArea, float scaleX, float scaleY) {
    if (!pGameArea)
        return;
    TILE_AREA_Node* pThisRoof = pGameArea->rooves;//pRoofList[*pMAP_LEVEL];
    ULONGLONG timeDiff = 0;
    ULONGLONG time = GetTickCount64();

    while (pThisRoof) {
        if (!pThisRoof->renderTarget) {///recreate render targets after screen reset
            float colour[4] = { 0,0,0,0 };
            if (pThisRoof->width > 0 && pThisRoof->height > 0) {
                pThisRoof->renderTarget = new RenderTarget2((float)pThisRoof->rect.left, (float)pThisRoof->rect.top, pThisRoof->width, pThisRoof->height, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
                pThisRoof->renderTarget->ClearRenderTarget(nullptr, colour);
                pThisRoof->renderTarget->ScalePosition(true);
                DrawRoof(pThisRoof);
            }
        }
        if (pThisRoof->renderTarget) {
            timeDiff = time - pThisRoof->fadeStartTime;
            if (timeDiff < 500) {
                float opaqueness = (float)timeDiff / 500.0f;
                if (opaqueness > opaqueness_WallRoof)
                    opaqueness = opaqueness_WallRoof;

                if (!pThisRoof->fadeIn)
                    pThisRoof->renderTarget->SetOpaqueness(opaqueness_WallRoof - opaqueness);
                else
                    pThisRoof->renderTarget->SetOpaqueness(opaqueness);
            }
            else {
                if (pThisRoof->fadeIn)
                    pThisRoof->renderTarget->SetOpaqueness(opaqueness_WallRoof);
                else
                    pThisRoof->renderTarget->SetOpaqueness(0.0f);
            }

            objData.PixelData.x = scaleX;
            objData.PixelData.y = scaleY;
            objData.PixelData.w = pThisRoof->renderTarget->GetOpaqueness();
            pPS_BuffersFallout->UpdateObjBuff(&objData);

            pThisRoof->renderTarget->SetScale(scaleX, scaleY);
            pThisRoof->renderTarget->SetPosition((float)(pThisRoof->rect.left - rcGame_PORTAL.left), (float)(pThisRoof->rect.top - rcGame_PORTAL.top));
            pThisRoof->renderTarget->Display(pd3d_PS_RenderRoof32, nullptr);
        }
        pThisRoof = pThisRoof->next;
    }
}


//__________________________________________________________________________________
void GameArea_DrawFloorRect(GAME_AREA* pArea, RECT* pRect, LONG level, DWORD flags) {

    if (!pArea)
        return;
    if (!pArea->tiles)
        return;
    if (!pArea->pFloor_RT)
        return;
    if (!pRect)
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    if (pRect->right < 0 || pRect->left >(LONG)pArea->width || pRect->bottom < 0 || pRect->top >(LONG)pArea->height) {
        return;
    }

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    FRMframeDx* pFrame = nullptr;

    TILEnode* pTile = pArea->tiles;
    while (pTile) {
        if (!pTile->pFrm) {
            imonitorInsertText("no frm for floor tile");
            pTile = pTile->next;
            continue;
        }
        pFrame = pTile->pFrm->GetFrame(pTile->frmID, 0, 0);
        if (pFrame == nullptr) {
            pTile = pTile->next;
            continue;
        }

        DWORD frame_width = pFrame->GetWidth();
        DWORD frame_height = pFrame->GetHeight();

        if (pTile->x > pRect->right || pTile->x + (int)frame_width < pRect->left || pTile->y > pRect->bottom || pTile->y + (int)frame_height < pRect->top) {
            pTile = pTile->next;
            continue;
        }
        ID3D11Buffer* frame_pd3dVB = nullptr;
        pFrame->GetVertexBuffer(&frame_pd3dVB);
        //set vertex stuff
        UINT stride = sizeof(VERTEX_BASE);
        UINT offset = 0;
        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
        //set texture stuff
        ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

        //set pixel shader stuff
        if (pFrame->GetPixelWidth() == 1) {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        }
        else {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);
        }

        pArea->pFloor_RT->DrawIndexed(4, 0, 0, nullptr, (float)pTile->x, (float)pTile->y, frame_width, frame_height, pRect);
        pTile = pTile->next;
    }

    SetAmbientLight_ShaderEffect();
}



//__________________________________________________________________
bool GameArea_DrawFogRect(GAME_AREA* pArea, RECT* pRect, LONG level) {
    if (!FOG_OF_WAR || !fogLight)
        return false;
    if (!pArea)
        return false;
    if (!pArea->pFog_RT)
        return false;
    if (!pRect)
        return false;
    ID3D11Device* pD3DDev = GetD3dDevice();
    if (pD3DDev == nullptr)
        return false;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return false;

    //if rect is outside of visible area consider it processed and return true.
    if (pRect->left >= pArea->width || pRect->right < 0 || pRect->top >= pArea->height || pRect->bottom < 0)
        return true;

    RECT rect;
    //To-Do rects arn't quite right, also blend state trouble ?
    //if (pRect)
        CopyRect(&rect, pRect);
    //else
    //    rect = { 0, 0, pArea->width, pArea->height };


    //Don't draw hexes ouside of map area.
    if (rect.left < 0)
        rect.left = 0;
    if (rect.right > pArea->width)
        rect.right = pArea->width;
    if (rect.top < 0)
        rect.top = 0;
    if (rect.bottom > pArea->height)
        rect.bottom = pArea->height;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

   // rect.left -= 128;
   // rect.right += 128;
   // rect.top -= 128;
   // rect.bottom += 128;

    XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };
    pArea->pFog_RT->ClearRect(clearColour, &rect);


    ID3D11Buffer* pd3dVB;
    CreateQuadVB(pD3DDev, FOG_QUAD_WIDTH, FOG_QUAD_HEIGHT, &pd3dVB);
    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PS_DrawHexFog, nullptr, 0);

    LONG xSqr = 0;
    LONG ySqr = 0;

    pD3DDevContext->OMSetBlendState(pBlendState_Four, nullptr, -1);

    LONG isoX = 0;
    LONG isoY = 0;

    LONG startX = 0;
    SqrToHex(rect.right + pArea->rect.left + FOG_QUAD_WIDTH, rect.top + pArea->rect.top, &startX, &isoY);
    LONG endX = 0;
    SqrToHex(rect.left + pArea->rect.left - FOG_QUAD_WIDTH, rect.bottom + pArea->rect.top, &endX, &isoY);

    LONG startY = 0;
    SqrToHex(rect.left + pArea->rect.left, rect.top + pArea->rect.top - FOG_QUAD_HEIGHT, &isoX, &startY);
    LONG endY = 0;
    SqrToHex(rect.right + pArea->rect.left, rect.bottom + pArea->rect.top + FOG_QUAD_HEIGHT, &isoX, &endY);

    startX -= 1;
    endX += 1;
    startY -= 1;
    endY += 1;
    GEN_SURFACE_BUFF_DATA genSurfaceData;
    genSurfaceData.genData4_1 = {0,0,0,(float)fogLight / 65536.0f };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    LONG hexNum = 0;
    for (isoY = startY; isoY < endY; isoY++) {
        for (isoX = startX; isoX < endX; isoX++) {
            if (isoX >= 0 && isoX < *pNUM_HEX_X && isoY >= 0 && isoY < *pNUM_HEX_Y) {
                hexNum = isoY * *pNUM_HEX_X + isoX;
                if (Is_Hex_Fogged(level, hexNum)) {
                    HexToSqr(isoX, isoY, &xSqr, &ySqr);
                    xSqr -= FOG_QUAD_WIDTH / 2;
                    ySqr -= FOG_QUAD_HEIGHT / 2;
                    xSqr -= 1;
                    ySqr -= 1;
                    xSqr -= pArea->rect.left;
                    ySqr -= pArea->rect.top;
                    if (xSqr + FOG_QUAD_WIDTH < rect.left || xSqr > rect.right || ySqr + FOG_QUAD_HEIGHT < rect.top || ySqr > rect.bottom)
                        continue;

                    pArea->pFog_RT->DrawIndexed(4, 0, 0, nullptr, (float)xSqr, (float)ySqr, FOG_QUAD_WIDTH, FOG_QUAD_HEIGHT, &rect);
                }
            }
        }
    }
    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);

    if (pd3dVB)
        pd3dVB->Release();
    pd3dVB = nullptr;
    return true;
}


//_______________________________________________________________________
void GameArea_DrawHexLightRect(GAME_AREA* pArea, RECT* pRect, LONG level) {
    if (!useOriginalLighting_Floor)
        return;
    if (!pArea)
        return;
    if (!pArea->pLight_RT)
        return;
    if (!pRect)
        return;
    ID3D11Device* pD3DDev = GetD3dDevice();
    if (pD3DDev == nullptr)
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;

    //if rect is outside of visible area consider it processed and return true.
    if (pRect->left >= pArea->width || pRect->right < 0 || pRect->top >= pArea->height || pRect->bottom < 0)
        return;

    RECT rect;
    //if (pRect)
    CopyRect(&rect, pRect);
    //else
    //    rect = { 0, 0, pArea->width, pArea->height };
    //Don't draw hexes ouside of map area.
    if (rect.left < 0)
        rect.left = 0;
    if (rect.right > pArea->width)
        rect.right = pArea->width;
    if (rect.top < 0)
        rect.top = 0;
    if (rect.bottom > pArea->height)
        rect.bottom = pArea->height;


    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };
    pArea->pLight_RT->ClearRect(clearColour, &rect);

    int quad_size_x = 96;
    int quad_size_y = 36;
    ID3D11Buffer* pd3dVB;
    CreateQuadVB(pD3DDev, quad_size_x, quad_size_y, &pd3dVB);
    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PS_DrawHexLight_OriginalLighting, nullptr, 0);

    LONG xSqr = 0;
    LONG ySqr = 0;

    pD3DDevContext->OMSetBlendState(pBlendState_Two, nullptr, -1);

    LONG isoX = 0;
    LONG isoY = 0;

    LONG startX = 0;
    SqrToHex(rect.right + pArea->rect.left + quad_size_x, rect.top + pArea->rect.top, &startX, &isoY);
    LONG endX = 0;
    SqrToHex(rect.left + pArea->rect.left - quad_size_x, rect.bottom + pArea->rect.top, &endX, &isoY);

    LONG startY = 0;
    SqrToHex(rect.left + pArea->rect.left, rect.top + pArea->rect.top - quad_size_y, &isoX, &startY);
    LONG endY = 0;
    SqrToHex(rect.right + pArea->rect.left, rect.bottom + pArea->rect.top + quad_size_y, &isoX, &endY);

    startX -= 1;
    endX += 1;
    startY -= 1;
    endY += 1;

    LONG hexNum = 0;
    for (isoY = startY; isoY < endY; isoY++) {
        for (isoX = startX; isoX < endX; isoX++) {
            if (isoX >= 0 && isoX < *pNUM_HEX_X && isoY >= 0 && isoY < *pNUM_HEX_Y) {
                hexNum = isoY * *pNUM_HEX_X + isoX;
                if (pHexLight_ColourMap[level]) {
                    //divided the accumulated light affecting the hex by the number of lights affecting hex.
                    objData.lightColour.x = pHexLight_ColourMap[level][hexNum].x / pHexLight_ColourMap[level][hexNum].w;
                    objData.lightColour.y = pHexLight_ColourMap[level][hexNum].y / pHexLight_ColourMap[level][hexNum].w;
                    objData.lightColour.z = pHexLight_ColourMap[level][hexNum].z / pHexLight_ColourMap[level][hexNum].w;
                }
                else {
                    objData.lightColour.x = 1.0f;
                    objData.lightColour.y = 1.0f;
                    objData.lightColour.z = 1.0f;
                }
                objData.lightColour.w = (float)GetHexLight(level, hexNum, 0) / 65536.0f;

                pPS_BuffersFallout->UpdateObjBuff(&objData);
                HexToSqr(isoX, isoY, &xSqr, &ySqr);
                xSqr -= quad_size_x / 2;
                ySqr -= quad_size_y / 2;
                xSqr -= 1;
                ySqr -= 1;
                xSqr -= pArea->rect.left;
                ySqr -= pArea->rect.top;
                if (xSqr + quad_size_x < rect.left || xSqr > rect.right || ySqr + quad_size_y < rect.top || ySqr > rect.bottom)
                    continue;
                pArea->pLight_RT->DrawIndexed(4, 0, 0, nullptr, (float)xSqr, (float)ySqr, quad_size_x, quad_size_y, &rect);
            }
        }
    }

    SetAmbientLight_ShaderEffect();
    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);

    if (pd3dVB)
        pd3dVB->Release();
    pd3dVB = nullptr;
}


//_______________________________________________________________________
void GameArea_AddHexLightColour(LONG level, LONG hexNum, LONG lightLevel) {
    if (hexNum >= *pNUM_HEXES)
        return;
    if (!pHexLight_ColourMap[level]) {
        pHexLight_ColourMap[level] = new XMFLOAT4[*pNUM_HEXES];
        memset(pHexLight_ColourMap[level], 0, *pNUM_HEXES * sizeof(XMFLOAT4));
    }

    //add the colour of the current light to the accumulated light affecting the hex.
    pHexLight_ColourMap[level][hexNum].x += hexLight_ActiveColour.x;
    pHexLight_ColourMap[level][hexNum].y += hexLight_ActiveColour.y;
    pHexLight_ColourMap[level][hexNum].z += hexLight_ActiveColour.z;
    pHexLight_ColourMap[level][hexNum].w++;//add to the number of lights affecting hex.
}


//________________________________________
void __declspec(naked) add_hex_light(void) {
    // in function - ADD_TO_LIGHT_MAP(EAX level, EDX hexPos, EBX light) light 0-65536

    __asm {
        pushad
        push ecx
        push edx
        push eax
        call GameArea_AddHexLightColour
        add esp, 0xC
        popad

        mov eax, pNUM_HEXES
        cmp edx, dword ptr ds : [eax]
        ret
    }
}


//_______________________________________________________________________
void GameArea_SubHexLightColour(LONG level, LONG hexNum, LONG lightLevel) {
    if (hexNum >= *pNUM_HEXES)
        return;
    if (!pHexLight_ColourMap[level]) {
        pHexLight_ColourMap[level] = new XMFLOAT4[*pNUM_HEXES];
        memset(pHexLight_ColourMap[level], 0, *pNUM_HEXES * sizeof(XMFLOAT4));
    }

    //subtract the colour of the current light from the accumulated light affecting the hex.
    pHexLight_ColourMap[level][hexNum].x -= hexLight_ActiveColour.x;
    pHexLight_ColourMap[level][hexNum].y -= hexLight_ActiveColour.y;
    pHexLight_ColourMap[level][hexNum].z -= hexLight_ActiveColour.z;
    pHexLight_ColourMap[level][hexNum].w--;//subtract from the number of lights affecting hex.
}


//________________________________________
void __declspec(naked) sub_hex_light(void) {
    //in function - SUBTRACT_FROM_LIGHT_MAP(EAX level, EDX hexPos, EBX light) light 0-65536
    __asm {
        pushad
        push esi
        push edx
        push eax
        call GameArea_SubHexLightColour
        add esp, 0xC
        popad

        mov eax, pNUM_HEXES
        cmp edx, dword ptr ds : [eax]
        ret
    }
}


//_____________________________________________________________________
void GameArea_DrawLightRect(GAME_AREA* pArea, RECT* pRect, LONG level) {
    if (useOriginalLighting_Floor)
        return;
    if (!pArea)
        return;
    if (!pArea->pLight_RT)
        return;
    if (!pRect)
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };

    pArea->pLight_RT->ClearRect(clearColour, pRect);

    UINT width = 0;
    UINT height = 0;
    int lightDistx = 0;
    int lightDisty = 0;
    RECT rect = { 0,0,0,0 };
    XMFLOAT4 lightVec{ 0,0,0,0 };
    XMFLOAT2 pixelSize;

    pD3DDevContext->OMSetBlendState(pBlendState_Two, nullptr, -1);

    OBJlight* objLightNode = nullptr;
    objLightNode = objLight[level];
    while (objLightNode) {
        width = (UINT)(objLightNode->rect.right - objLightNode->rect.left);
        height = (UINT)(objLightNode->rect.bottom - objLightNode->rect.top);
        lightDistx = width / 2;
        lightDisty = height / 2;
        rect = { (int)objLightNode->lightDx.x - lightDistx - pArea->rect.left, (int)objLightNode->lightDx.y - lightDisty - pArea->rect.top, (int)objLightNode->lightDx.x + lightDistx - pArea->rect.left, (int)objLightNode->lightDx.y + lightDisty - pArea->rect.top };

        if (!objLightNode->pRT_ShadowMask || rect.right < pRect->left || rect.left > pRect->right || rect.bottom < pRect->top || rect.top > pRect->bottom)
            objLightNode = objLightNode->next;
        else {
            pixelSize.x = 1.0f / (float)width;
            pixelSize.y = 1.0f / (float)height;
            lightVec = objLightNode->lightDx;

            objData.PixelData.x = 1.0f / (float)width;
            objData.PixelData.y = 1.0f / (float)height;
            objData.lightColour = objLightNode->colour;
            objData.lightDetails = objLightNode->lightDx;

            pPS_BuffersFallout->UpdateObjBuff(&objData);

            //set vertex stuff
            ID3D11Buffer* pd3dVB;
            objLightNode->pRT_ShadowMask->GetVertexBuffer(&pd3dVB);
            UINT stride = sizeof(VERTEX_BASE);
            UINT offset = 0;
            pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
            //set texture stuff
            ID3D11ShaderResourceView* pTex_shaderResourceView = objLightNode->pRT_ShadowMask->GetShaderResourceView();
            pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
            //set pixel shader stuff
            pD3DDevContext->PSSetShader(pd3d_PS_Shadow5_Combine, nullptr, 0);

            pArea->pLight_RT->DrawIndexed(4, 0, 0, nullptr, (float)rect.left, (float)rect.top, rect.right - rect.left, rect.bottom - rect.top, pRect);

            objLightNode = objLightNode->next;
        }
    }

    SetAmbientLight_ShaderEffect();
    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
}


//___________________________________________________________________
RECT* DrawLightChanges(RECT* lprcDst, RECT* lprcSrc1, RECT* lprcSrc2) {
    //This function is called when adding a pre-defined light radius rectangle. these rectangles are defined at offset 0x51964C of the exe, an array of 8 RECT structures for light radiuses of 1 to 9 hexes.
    UnionRect(lprcDst, lprcSrc1, lprcSrc2);

    if (!pGameArea_Current)
        return lprcDst;
    if (!lprcDst)
        return lprcDst;

    RECT rect = { lprcDst->left - 1 - pGameArea_Current->rect.left,lprcDst->top - 1 - pGameArea_Current->rect.top,lprcDst->right + 1 - pGameArea_Current->rect.left,lprcDst->bottom + 1 - pGameArea_Current->rect.top };
    
    if (!(rect.right < 0 || rect.left > pGameArea_Current->width || rect.bottom < 0 || rect.top > pGameArea_Current->height)) {

            if (useOriginalLighting_Floor)
                GameArea_DrawHexLightRect(pGameArea_Current, &rect, *pMAP_LEVEL);
            else 
                GameArea_DrawLightRect(pGameArea_Current, &rect, *pMAP_LEVEL);
    }
    //adjust the light draw rect for new object lighting as they are taller than the game expects.
    if(!useOriginalLighting_Objects)
        lprcDst->top -= LIT_OBJ_HEIGHT_ADJUSTMENT;
    return lprcDst;
}


//_____________________________________________
void __declspec(naked) draw_light_changes(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call DrawLightChanges
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//_____________________________________________________________________________________
void GameArea_DrawObj(GAME_AREA* pArea, OBJStruct* pObj, RECT* pRect, DWORD drawFlags) {

    if (!pArea)
        return;
    if (!pObj)
        return;
    if (!pRect)
        return;
    if (pObj == *ppObj_Mouse)
        return;

    RenderTarget2* pTarget_RT = nullptr;
    if ((pObj->flags & FLG_Flat)) {
        if(drawFlags & FLG_Floor)
            pTarget_RT = pArea->pFloor_RT;
    }
    else {
        if (drawFlags & FLG_Obj)
            pTarget_RT = pArea->pObjects_RT;
    }
    if (!pTarget_RT)
        return;

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    OBJStructDx* pObjDx = (OBJStructDx*)pObj;

    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;

    XMFLOAT2 objPos = { 0,0 };

    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum != -1) {
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);
        xPos -= 1;
        yPos -= 1;
        xPos -= pArea->rect.left;
        yPos -= pArea->rect.top;
        objPos.x = (float)xPos;
        objPos.y = (float)yPos;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;

    }
    else {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
        xPos -= pArea->rect.left;
        yPos -= pArea->rect.top;
        objPos.x = (float)xPos;
        objPos.y = (float)yPos;
    }
    DWORD flagsDx = pObjDx->flags;
    if (pObj == *ppObj_PC)
        flagsDx = flagsDx | FLG_MarkedByPC;

    bool positionUpdatesOften = false;
    if (artType == ART_CRITTERS)
        positionUpdatesOften = true;
    if (!pObjDx->frmObjDx) {
        pObjDx->frmObjDx = new FRMobj(pObj, positionUpdatesOften);
    }

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (pFrame == nullptr)
        return;

    DWORD frame_width = pFrame->GetWidth();
    DWORD frame_height = pFrame->GetHeight();

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    int xOffset = 0;
    if (xPos + (int)frame_width < pRect->left || xPos > pRect->right || yPos + (int)frame_height < pRect->top || yPos > pRect->bottom)
        return;

    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
    UINT frame_pixelWidth = pFrame->GetPixelWidth();
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);

    ID3D11ShaderResourceView* lightSRV = pObjDx->frmObjDx->GetLightResourceView();
    pD3DDevContext->PSSetShaderResources(3, 1, &lightSRV);

    bool isUniformlyLit = pObjDx->frmObjDx->isUniformlyLit();

    objData.objPos.x = objPos.x;
    objData.objPos.y = objPos.y;

    if ((artType == ART_WALLS || artType == ART_SCENERY) && !(((OBJStruct*)*ppObj_PC)->flags & FLG_Disabled) && !(pObjDx->flags & FLG_TransAny)) {
        OBJStruct* pObj_PC = *ppObj_PC;
        PROTO* pProto = 0;
        fall_GetPro(pObjDx->proID, &pProto);
        DWORD actionFlags = pProto->wall.actionFlags;
        objData.flags.x = 0;
        objData.flags.y = 0;

        if (actionFlags & FLG_EastWest || actionFlags & FLG_WestCorner) {

            if (CheckWallPosition_1(pObjDx->hexNum, pObj_PC->hexNum) == true) {
                objData.flags.z = 1.0f;//Enable Egg
                if (CheckWallPosition_2(pObjDx->hexNum, pObj_PC->hexNum) == true && pObjDx->flags & FLG_WallTransEnd) {
                    objData.flags.z = 0.0f;//Disable Egg
                }
            }
            else
                objData.flags.z = 0.0f;//Disable Egg
        }
        else if (actionFlags & FLG_NorthCorner) {
            if (CheckWallPosition_1(pObjDx->hexNum, pObj_PC->hexNum) == false && CheckWallPosition_2(pObj_PC->hexNum, pObjDx->hexNum) == false)
                objData.flags.z = 0.0f;//Disable Egg
            else
                objData.flags.z = 1.0f;//Enable Egg
        }
        else if (actionFlags & FLG_SouthCorner) {
            if (CheckWallPosition_1(pObjDx->hexNum, pObj_PC->hexNum) == false || CheckWallPosition_2(pObj_PC->hexNum, pObjDx->hexNum) == false)
                objData.flags.z = 0.0f;//Disable Egg
            else
                objData.flags.z = 1.0f;//Enable Egg
        }
        else {
            if (CheckWallPosition_2(pObj_PC->hexNum, pObjDx->hexNum) == true) {
                objData.flags.z = 1.0f;//Enable Egg
                if (CheckWallPosition_1(pObj_PC->hexNum, pObjDx->hexNum) == true && pObjDx->flags & FLG_WallTransEnd) {
                    objData.flags.z = 0.0f;//Disable Egg
                }
            }
            else
                objData.flags.z = 0.0f;//Disable Egg
        }
    }
    else
        objData.flags.z = 0.0f;//Disable Egg

    if (artType == ART_WALLS) {
        //if (pObjDx->frmObjDx->opaqueness < opaqueness_WallRoof)
        //    objData.PixelData.w = pObjDx->frmObjDx->opaqueness;
        //else
            objData.PixelData.w = opaqueness_WallRoof;
    }
    else
        objData.PixelData.w = pObjDx->frmObjDx->opaqueness;//opaqueness

    pFrame->GetTexturePixelSize(&objData.PixelData.x, &objData.PixelData.y);

    if (pObjDx->flags & (FLG_TransRed | FLG_TransWall | FLG_TransGlass | FLG_TransSteam | FLG_TransEnergy)) {
        objData.flags.z = 0.0f;//Disable Egg
        objData.flags.w = 1.0f;//Is Transparent
        XMFLOAT2 pal_colour = { 0.0f, 0.0f };
        if (pObjDx->flags & FLG_TransRed)
            pal_colour = { (float)PAL_TRANS_RED / 256.0f, 0.6f };
        else if (pObjDx->flags & FLG_TransWall)
            pal_colour = { (float)PAL_TRANS_WALL / 256.0f, 0.6f };
        else if (pObjDx->flags & FLG_TransGlass)
            pal_colour = { (float)PAL_TRANS_GLASS / 256.0f, 0.6f };
        else if (pObjDx->flags & FLG_TransSteam)
            pal_colour = { (float)PAL_TRANS_STEAM / 256.0f, 0.6f };
        else if (pObjDx->flags & FLG_TransEnergy)
            pal_colour = { (float)PAL_TRANS_ENERGY / 256.0f, 0.6f };

        objData.PalEffects.z = pal_colour.x;
        objData.PalEffects.w = pal_colour.y;
    }
    else
        objData.flags.w = 0; //Is Not Transparent

    if (pObjDx->combatFlags & FLG_IsOutlined) {
        XMFLOAT2 pal_colour = { 0.0f, 0.0f };
        if (pObjDx->combatFlags & FLG_NonPCTeamMem)
            pal_colour = { (float)PAL_OUTLINE_NOT_TEAM / 256.0f, 1.0f };
        else if (pObjDx->combatFlags & FLG_MouseHex)
            pal_colour = { (float)PAL_OUTLINE_MOUSE / 256.0f, 0.6f };
        else if (pObjDx->combatFlags & FLG_combatUnk0x04)
            pal_colour = { (float)PAL_OUTLINE_UNKNOWN04 / 256.0f, 1.0f };
        else if (pObjDx->combatFlags & FLG_PCTeamMem)
            pal_colour = { (float)PAL_OUTLINE_TEAM / 256.0f, 1.0f };
        else if (pObjDx->combatFlags & FLG_ItemUnderMouse)
            pal_colour = { (float)PAL_OUTLINE_ITEM_UNDER_MOUSE / 256.0f, 1.0f };
        else if (pObjDx->combatFlags & FLG_NotVisByPC)
            pal_colour = { (float)PAL_OUTLINE_NOT_VISIBLE / 256.0f, 1.0f };
        else
            pal_colour = { (float)PAL_OUTLINE_OTHER / 256.0f, 1.0f };

        objData.PalEffects.x = pal_colour.x;
        objData.PalEffects.y = pal_colour.y;
    }
    if (useOriginalLighting_Objects || isUniformlyLit) {
        if (pHexLight_ColourMap[pObjDx->level]) {
            objData.lightColour.x = pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].x / pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].w;
            objData.lightColour.y = pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].y / pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].w;
            objData.lightColour.z = pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].z / pHexLight_ColourMap[pObjDx->level][pObjDx->hexNum].w;
        }
        else {
            objData.lightColour.x = 1.0f;
            objData.lightColour.y = 1.0f;
            objData.lightColour.z = 1.0f;
        }
        objData.lightColour.w = (float)GetHexLight(pObjDx->level, pObjDx->hexNum, GetAmbientLightIntensity()) / 65536.0f;

    }
    pPS_BuffersFallout->UpdateObjBuff(&objData);

    //set vertex
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

    //set texture
    pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

    //set pixel shader
    if ((pObjDx->flags & FLG_Flat)) {
        if (frame_pixelWidth == 1)
            pD3DDevContext->PSSetShader(pd3d_PS_ObjFlat8, nullptr, 0);
        else
            pD3DDevContext->PSSetShader(pd3d_PS_ObjFlat32, nullptr, 0);
    }
    else {
        if (useOriginalLighting_Objects || isUniformlyLit) {
            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_ObjUpright8_OriginalLighting, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_ObjUpright32_OriginalLighting, nullptr, 0);
        }
        else {
            if (frame_pixelWidth == 1)
                pD3DDevContext->PSSetShader(pd3d_PS_ObjUpright8, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_ObjUpright32, nullptr, 0);
        }
    }
    pTarget_RT->DrawIndexed(4, 0, 0, nullptr, (float)xPos, (float)yPos, frame_width, frame_height, pRect);

    //----draw outlines-----------------------------------------
    if (pArea->pHud_RT_Outline && (pObjDx->combatFlags & FLG_IsOutlined) && !(pObjDx->combatFlags & FLG_NonInteractive)) {
        //set pixel shader
        if (frame_pixelWidth == 1)
            pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge8, nullptr, 0);
        else
            pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge32, nullptr, 0);

        RECT rcHud = { pRect->left - rcGame_PORTAL.left + pArea->rect.left, pRect->top - rcGame_PORTAL.top + pArea->rect.top, pRect->right - rcGame_PORTAL.left + pArea->rect.left,  pRect->bottom - rcGame_PORTAL.top + pArea->rect.top };
        pArea->pHud_RT_Outline->DrawIndexed(4, 0, 0, nullptr, (float)xPos - rcGame_PORTAL.left + pArea->rect.left, (float)yPos - rcGame_PORTAL.top + pArea->rect.top, frame_width, frame_height, &rcHud);

    }

    pFrame = nullptr;
    pObjDx = nullptr;
    pTarget_RT = nullptr;
}


//_____________________________________________________________
void GameArea_DrawObjOutline(GAME_AREA* pArea, OBJStruct* pObj) {

    if (!(pObj->combatFlags & FLG_IsOutlined) || (pObj->combatFlags & FLG_NonInteractive))
        return;

    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    if (!pArea)
        return;
    if (!pArea->pHud_RT_Outline)
        return;
    if (!pObj)
        return;
    if (pObj == *ppObj_Mouse)
        return;

    OBJStructDx* pObjDx = (OBJStructDx*)pObj;
    DWORD fID = pObjDx->frmID;

    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return;

    LONG xPos = 0;
    LONG yPos = 0;
    if (pObjDx->hexNum != -1) {
        if (HexNumToHexPos(pObjDx->hexNum, &xPos, &yPos)) {
            xPos = 0;
            yPos = 0;
        }
        HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);
        xPos -= 1;
        yPos -= 1;
        xPos += pObjDx->xShift;
        yPos += pObjDx->yShift;
    }
    else {
        xPos = pObjDx->viewScrnX;
        yPos = pObjDx->viewScrnY;
    }
    xPos -= rcGame_PORTAL.left;
    yPos -= rcGame_PORTAL.top;

    DWORD flagsDx = pObjDx->flags;
    if (pObj == *ppObj_PC)
        flagsDx = flagsDx | FLG_MarkedByPC;

    bool positionUpdatesOften = false;
    if (artType == ART_CRITTERS)
        positionUpdatesOften = true;
    if (!pObjDx->frmObjDx) {
        pObjDx->frmObjDx = new FRMobj(pObj, positionUpdatesOften);
    }

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();

    if (pFrame == nullptr)
        return;

    xPos += pFrame->GetOffset_OriCentre_X();
    yPos += pFrame->GetOffset_OriCentre_Y();

    int xOffset = 0;

    DWORD frame_width = pFrame->GetWidth();
    DWORD frame_height = pFrame->GetHeight();

    if (xPos + (int)frame_width < 0 || xPos > game_PortalWidth || yPos + (int)frame_height < 0 || yPos > game_PortalHeight)
        return;

    XMMATRIX Ortho2D;
    pArea->pHud_RT_Outline->GetOrthoMatrix(&Ortho2D);

    MATRIX_DATA posData;
    posData.World = XMMatrixTranslation((float)xPos, (float)yPos, (float)0.0f);
    XMMATRIX* pOrtho2D_SCRN = GetScreenProjectionMatrix_XM();
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, Ortho2D);

    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();

    //objData.PixelData.x = pFrame->pixelSize.x;
    //objData.PixelData.y = pFrame->pixelSize.y;
    pFrame->GetTexturePixelSize(&objData.PixelData.x, &objData.PixelData.y);

    XMFLOAT2 pal_colour = { 0.0f, 0.0f };
    if (pObjDx->combatFlags & FLG_NonPCTeamMem)
        pal_colour = { (float)PAL_OUTLINE_NOT_TEAM / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_MouseHex)
        pal_colour = { (float)PAL_OUTLINE_MOUSE / 256.0f, 0.6f };
    else if (pObjDx->combatFlags & FLG_combatUnk0x04)
        pal_colour = { (float)PAL_OUTLINE_UNKNOWN04 / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_PCTeamMem)
        pal_colour = { (float)PAL_OUTLINE_TEAM / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_ItemUnderMouse)
        pal_colour = { (float)PAL_OUTLINE_ITEM_UNDER_MOUSE / 256.0f, 1.0f };
    else if (pObjDx->combatFlags & FLG_NotVisByPC)
        pal_colour = { (float)PAL_OUTLINE_NOT_VISIBLE / 256.0f, 1.0f };
    else
        pal_colour = { (float)PAL_OUTLINE_OTHER / 256.0f, 1.0f };

    objData.PalEffects.x = pal_colour.x;
    objData.PalEffects.y = pal_colour.y;

    pPS_BuffersFallout->UpdateObjBuff(&objData);

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

    //set texture stuff
    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
    pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

    //set pixel shader stuff
    UINT frame_pixelWidth = pFrame->GetPixelWidth();
    if (frame_pixelWidth == 1)
        pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge8, nullptr, 0);
    else
        pD3DDevContext->PSSetShader(pd3d_PS_Outline_OuterEdge32, nullptr, 0);

    pArea->pHud_RT_Outline->DrawIndexed(4, 0, 0, nullptr, (float)xPos, (float)yPos, frame_width, frame_height, nullptr);

    pFrame = nullptr;
    pObjDx = nullptr;
}


//__________________________________________________________________________________
void GameArea_DrawObjectRect(GAME_AREA* pArea, RECT* pRect, LONG level, DWORD flags) {

    if (pAreObjectsInitiated == nullptr || *pAreObjectsInitiated == 0)
        return;
    if (!pArea)
        return;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    if (pArea->pObjects_RT) {
        XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };
        pArea->pObjects_RT->ClearRect(clearColour, pRect);
    }

    OBJNode* mapObj = nullptr;
    int isVisPC = 0;
    LONG hexPos = 0;
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];
        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (!(mapObj->pObj->flags & FLG_Disabled)) {
                        IsVisibleByPCDx(mapObj->pObj);
                        GameArea_DrawObj(pArea, mapObj->pObj, pRect, flags);
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


//______________________________________________________________________
void GameArea_DrawPalAniTiles(GAME_AREA* pArea, LONG level, DWORD flags) {

    if (!pArea)
        return;
    if (!pArea->tiles)
        return;
    if (!pArea->pFloor_RT)
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    RECT rect = { 0, 0, 0, 0 };

    FRMframeDx* pFrame = nullptr;

    TILEnode* pTile = pArea->tiles;
    while (pTile) {
        if (!pTile->pFrm) {
            ///imonitorInsertText("no frm for roof tile");
            pTile = pTile->next;
            continue;
        }
        pFrame = pTile->pFrm->GetFrame(pTile->frmID, 0, 0);
        if (pFrame == nullptr || !pFrame->IsAnimated() || !(pFrame->Animation_ZoneFlags() & flags)) {
            pTile = pTile->next;
            continue;
        }
        ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
        ID3D11Buffer* frame_pd3dVB = nullptr;
        pFrame->GetVertexBuffer(&frame_pd3dVB);

        //set vertex stuff
        UINT stride = sizeof(VERTEX_BASE);
        UINT offset = 0;
        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
        //set texture stuff
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

        //set pixel shader stuff
        if (pFrame->GetPixelWidth() == 1) {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        }
        else {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);
        }

        pArea->pFloor_RT->DrawIndexed(4, 0, 0, nullptr, (float)pTile->x, (float)pTile->y, pFrame->GetWidth(), pFrame->GetHeight(), nullptr);

        pFrame->GetAniRect(&rect);
        LONG x_offset = pFrame->GetOffset_OriCentre_X();
        LONG y_offset = pFrame->GetOffset_OriCentre_Y();
        rect.left += x_offset;
        rect.top += y_offset;
        rect.right += x_offset;
        rect.bottom += y_offset;

        if (DoRectsOverlap(&rect, &rcGame_PORTAL))
            GameArea_DrawObjectRect(pArea, &rect, level, FLG_Floor);
        pTile = pTile->next;
    }

    SetAmbientLight_ShaderEffect();
}


//_________________________________________________________________________________________
void GameArea_ClearHUD_Outline_Rect(GAME_AREA* pArea, RECT* pRect, LONG level, DWORD flags) {
    if (!pArea)
        return;
    if (!pArea->pHud_RT_Outline)
        return;
    XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };
    RECT rcHud = { pRect->left - rcGame_PORTAL.left + pArea->rect.left, pRect->top - rcGame_PORTAL.top + pArea->rect.top, pRect->right - rcGame_PORTAL.left + pArea->rect.left,  pRect->bottom - rcGame_PORTAL.top + pArea->rect.top };
    pArea->pHud_RT_Outline->ClearRect(clearColour, &rcHud);
}


//________________________________________________________________________________
void GameArea_ClearHUDRect(GAME_AREA* pArea, RECT* pRect, LONG level, DWORD flags) {
    if (!pArea)
        return;
    if (!pArea->pHud_RT)
        return;

    XMFLOAT4 clearColour = { 0.0f,0.0f,0.0f,0.0f };
    pArea->pHud_RT->ClearRect(clearColour, pRect);
}


//_____________________________________________________________________________________
void GameArea_Draw_FloatingText(GAME_AREA* pArea, RECT* pRect, LONG level, DWORD flags) {

    if (*pFloatingText_IsInitiated == 0)
        return;
    if (*pFloatingText_NumTotal <= 0)
        return;
    if (!pArea)
        return;
    if ((!pArea->pHud_RT))
        return;
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    if (pD3DDevContext == nullptr)
        return;
    FloatingTextObjDx** lpTxtObj = (FloatingTextObjDx**)lpFloatingText_Obj;
    FRMframeDx* pFrame = nullptr;
    LONG xPos = 0;
    LONG yPos = 0;
    XMFLOAT2 objPos;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    pD3DDevContext->RSSetScissorRects(1, pRect);


    for (int txtNum = 0; txtNum < *pFloatingText_NumTotal; txtNum++) {
        pFrame = lpTxtObj[txtNum]->frameDX;
        if (pFrame == nullptr)
            continue;
        if (lpTxtObj[txtNum]->hexPos == -1)
            continue;
        HexNumToSqr(lpTxtObj[txtNum]->hexPos, &xPos, &yPos);

        xPos += lpTxtObj[txtNum]->xOffset;
        yPos += lpTxtObj[txtNum]->yOffset;

        xPos -= 16;
        yPos -= 8;

        lpTxtObj[txtNum]->scrnX = xPos;
        lpTxtObj[txtNum]->scrnY = yPos;

        xPos -= 1;
        yPos -= 1;
        xPos -= pArea->rect.left;
        yPos -= pArea->rect.top;

        objPos.x = (float)xPos;
        objPos.y = (float)yPos;

        int xOffset = 0;

        DWORD frame_width = pFrame->GetWidth();
        DWORD frame_height = pFrame->GetHeight();

        if (xPos + (int)frame_width<pRect->left || xPos>pRect->right || yPos + (int)frame_height<pRect->top || yPos>pRect->bottom)
            continue;

        ID3D11Buffer* frame_pd3dVB = nullptr;
        pFrame->GetVertexBuffer(&frame_pd3dVB);
        UINT stride = sizeof(VERTEX_BASE);
        UINT offset = 0;
        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
        //set texture stuff
        ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

        //set pixel shader stuff
        if (pFrame->GetPixelWidth() == 1)
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        else
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);


        pArea->pHud_RT->DrawIndexed(4, 0, 0, nullptr, (float)xPos, (float)yPos, frame_width, frame_height, pRect);
    }
}


//______________________________________________
bool GameAreas_Set_Current_Area_Hex(LONG hexNum) {
    LONG sqrX = 0;
    LONG sqrY = 0;
    HexNumToSqr(hexNum, &sqrX, &sqrY);

    GAME_AREA* pThisArea = pGameArea[*pMAP_LEVEL];

    while (pThisArea) {
        if (sqrX > pThisArea->rect.left && sqrX < pThisArea->rect.right && sqrY > pThisArea->rect.top && sqrY < pThisArea->rect.bottom) {
            if (pGameArea_Current == pThisArea) {
                //imonitorInsertText("GameAreas_Set_Current_Area_Hex - area already set");
            }
            else {
                if (pGameArea_Current != nullptr) {
                    pGameArea_Current->deleteRenderTargets();
                    pGameArea_Current->deleteRoofRenderTargets();
                    //imonitorInsertText("GameAreas_Set_Current_Area_Hex - delete previous area RT's");

                }
                pGameArea_Current = pThisArea;
                pGameArea_Current->CreateRendertargets();
                gameArea_RoofRef = 0;
                gameArea_ReDraw = true;
                //imonitorInsertText("GameAreas_Set_Current_Area_Hex - set new area");
                CheckIfPcUnderRoof();
                //GameArea_DrawFogRect(pGameArea_Current, nullptr, *pMAP_LEVEL);
                //PC_ScanLineOfSight();
            }
            return true;
        }
        else {
            pThisArea = pThisArea->next;
            //areaNum++;
        }
    }
    //imonitorInsertText("GameAreas_Set_Current_Area_Hex - area not found");
    pGameArea_Current = nullptr;
    return false;
}


//______________________
void GameAreas_Display() {
    if (!pGameArea_Current)
        return;
    if (*pDRAW_VIEW_FLAG == 0 || isLoadingMapData)
        return;
    float scaleX = gameArea_scaleX;
    float scaleY = gameArea_scaleY;

    pGameArea_Current->pFloor_RT->Display(pd3d_PS_RenderFloorLight32, pGameArea_Current->pLight_RT);
    GEN_SURFACE_BUFF_DATA genSurfaceData;
    genSurfaceData.genData4_1 = { 0,0,0,0 };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    if(pGameArea_Current->pFog_RT)
        pGameArea_Current->pFog_RT->Display(pd3d_PS_Colour_32_Alpha, nullptr);

    DrawMouseObjDx(scaleX, scaleY, 0);

    pGameArea_Current->pObjects_RT->Display(nullptr, nullptr);


    GameArea_DisplayRooves(pGameArea_Current, scaleX, scaleY);
    /*float x_fog = 0;
    float y_fog = 0;
    pGameArea_Current->pFog_RT->GetPosition(&x_fog, &y_fog);
    pGameArea_Current->pFog_RT->SetPosition(x_fog, y_fog - 96.0f);
    pGameArea_Current->pFog_RT->Display(pd3d_PS_DrawTextAlpha, nullptr);
    pGameArea_Current->pFog_RT->SetPosition(x_fog, y_fog);*/

    pGameArea_Current->pHud_RT->Display(nullptr, nullptr);

    DrawMouseObjDx(scaleX, scaleY, 1);

    if (pGameArea_Current->pHud_RT_Outline)
        pGameArea_Current->pHud_RT_Outline->Display(pd3d_PS_Basic_Tex_32, nullptr);

    return;
}


//____________________________________________________________________________________________
bool GameAreas_DrawToWindow(Window_DX* pWin, LONG hexNum, LONG portalWidth, LONG portalHeight) {
    if (!pWin)
        return false;
    if (isMapperSizing)
        return false;
    if (*pWinRef_GameArea == -1)
        return false;
    if (!pGameArea_Current)
        return false;
    if (*pDRAW_VIEW_FLAG == 0 || isLoadingMapData)
        return false;

    game_PortalWidth = (LONG)(portalWidth);
    game_PortalHeight = (LONG)(portalHeight);

    EDGE_X_DIFF = (game_PortalWidth / 2) & 0x1F;
    EDGE_Y_DIFF = (game_PortalHeight / 2) % 24;

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    pWin->SetRenderTarget(nullptr);
    SetScreenProjectionMatrix_XM(portalWidth, portalHeight);

    gameArea_scaleX = 1.0f;
    gameArea_scaleY = 1.0f;
    pGameArea_Current->SetScale(gameArea_scaleX, gameArea_scaleY);

    LONG viewCentrePos = *pVIEW_HEXPOS;
    SetViewPosition_Hex(hexNum, 0x0);

    pGameArea_Current->pFloor_RT->Display(pd3d_PS_RenderFloorLight32, pGameArea_Current->pLight_RT);
    pGameArea_Current->pObjects_RT->Display(nullptr, nullptr);
    GameArea_DisplayRooves(pGameArea_Current, gameArea_scaleX, gameArea_scaleY);

    SetScreenProjectionMatrix_XM(SCR_WIDTH, SCR_HEIGHT);

    ResizeGameWin();
    SetViewPosition_Hex(viewCentrePos, 0x0);
    return true;
}


//__________________________________________________________________________________
bool CheckTileAreaRect(int tilePosNum, int level, GAME_AREA* pAreaNode, bool isRoof) {
    if (!pAreaNode)
        return false;
    if (level < 0 || level >= 3)
        return false;
    if (tilePosNum < 0 || tilePosNum >= *pNUM_TILES)
        return false;

    DWORD** levelOffset = *pMapTileLevelOffset;
    LONG tileLstNum = levelOffset[level][tilePosNum];
    LONG tileFlag = 0;


    //sfall MoreTiles
    if (SFALL_MoreTiles) {
        tileFlag = (tileLstNum & 0x0000C000) >> 14;
        tileLstNum = tileLstNum & 0x00003FFF;
    }
    else {
        tileFlag = (tileLstNum & 0x0000F000) >> 12;
        tileLstNum = tileLstNum & 0x00000FFF;
    }

    if (tileLstNum == 1)
        return false;

    TILEnode* thisTile = nullptr;
    DWORD frmID = 0x04000000 | tileLstNum;

    FRMtile* pFrm = new FRMtile(frmID, 0);
    FRMframeDx* pFrame = pFrm->GetFrame(frmID, 0, 0);
    LONG xPos = 0, yPos = 0;
    TileToSqr(tilePosNum, &xPos, &yPos);

    yPos += 24;
    xPos += -25;

    if (pFrame) {
        xPos += pFrame->GetOffset_OriCentre_X();
        yPos += pFrame->GetOffset_OriCentre_Y();
    }
    int areaNum = 0;
    while (pAreaNode) {
        //check if tile is in area rect and add to tile list
        if (xPos + 80 >= pAreaNode->rect.left && xPos <= pAreaNode->rect.right && yPos + 36 >= pAreaNode->rect.top && yPos <= pAreaNode->rect.bottom) {
            if (!pAreaNode->tiles) {
                pAreaNode->tiles = new TILEnode;
                thisTile = pAreaNode->tiles;
            }
            else {
                thisTile = pAreaNode->tiles;
                while (thisTile->next != nullptr)
                    thisTile = thisTile->next;
                thisTile->next = new TILEnode;
                thisTile = thisTile->next;
            }
            thisTile->x = xPos - pAreaNode->rect.left;
            thisTile->y = yPos - pAreaNode->rect.top;
            thisTile->tileNum = tilePosNum;
            thisTile->frmID = frmID;
            thisTile->pFrm = pFrm;
            thisTile->next = nullptr;
            return true;
        }
        //try next area
        pAreaNode = pAreaNode->next;
        areaNum++;
    }
    //tile not in any areas
    thisTile = nullptr;
    pFrame = nullptr;
    delete pFrm;

    return false;
}


//_________________________________________________________
bool SetEdgesFromHexNumbers(RECT* phexRect, RECT* pSquRect) {
    if (!phexRect)
        return false;
    if (!pSquRect)
        return false;
    LONG x = 0;
    LONG y = 0;
    HexNumToSqr_Scroll(phexRect->left, &pSquRect->left, &y);
    HexNumToSqr_Scroll(phexRect->right, &pSquRect->right, &y);
    HexNumToSqr_Scroll(phexRect->top, &x, &pSquRect->top);
    HexNumToSqr_Scroll(phexRect->bottom, &x, &pSquRect->bottom);
    return true;
}


//______________________________________
bool GameAreas_Save(const char* MapName) {


    char mapPath[32];
    sprintf_s(mapPath, 32, "maps\\%s", MapName);
    memcpy(strchr(mapPath, '.'), ".edg\0", 5);

    void* FileStream = fall_fopen(mapPath, "wb");
    if (FileStream == nullptr)
        return false;

    fall_fwrite32_BE(FileStream, 0x45444745);
    fall_fwrite32_BE(FileStream, 0x03);//version3 - includes angled edges and use rectangular coordinates for map edges instead of tile numbers used in versions 1 and 2;

    GAME_AREA* pThisArea = nullptr;
    for (int level = 0; level < 3; level++) {
        pThisArea = pGameArea[level];
        fall_fwrite32_BE(FileStream, level);

        fall_fwrite32_Array_BE(FileStream, (DWORD*)&pThisArea->tileLimits.east, 4);
        fall_fwrite32_BE(FileStream, pThisArea->tileLimitFlags);

        fall_fwrite32_Array_BE(FileStream, (DWORD*)&pThisArea->rect.left, 4);
        while (pThisArea->next) {//load extra areas
            fall_fwrite32_BE(FileStream, level);
            pThisArea = pThisArea->next;
            fall_fwrite32_Array_BE(FileStream, (DWORD*)&pThisArea->rect.left, 4);
        }
    }
    fall_fclose(FileStream);

    return true;
}


//______________________________________
bool GameAreas_Load(const char* MapName) {

    char mapPath[32];
    sprintf_s(mapPath, 32, "maps\\%s", MapName);
    memcpy(strchr(mapPath, '.'), ".edg\0", 5);

    void* FileStream = fall_fopen(mapPath, "rb");
    if (FileStream == nullptr)
        return false;
    DWORD isEdge = 0;
    fall_fread32_BE(FileStream, &isEdge);
    if (isEdge != 0x45444745) {//check if edge file "EDGE"
        fall_fclose(FileStream);
        return false;
    }
    DWORD version = 0;
    fall_fread32_BE(FileStream, &version);
    if (version != 0x01 && version != 0x02 && version != 0x03) {//check file version 1
        fall_fclose(FileStream);
        return false;
    }
    bool hasTileLimits = false;
    if (version == 0x02 || version == 0x03)//get angled edges for level
        hasTileLimits = true;

    LONG currentLev = 0;

    fall_fread32_BE(FileStream, (DWORD*)&currentLev);
    if (currentLev) {//this should be level zero
        fall_fclose(FileStream);
        return false;
    }

    DWORD** levelOffset = *pMapTileLevelOffset;
    LONG tileLstNum = 0;
    LONG tileFlag = 0;
    int tileY = 0, tileX = 0;
    int floorNum = 0;
    GAME_AREA* pThisArea = nullptr;
    pGameArea_Current = nullptr;
    DWORD tileLimitFlags = 0;
    COMPASS tileLimits = { 0,0,0,0 };

    for (int level = 0; level < 3; level++) {
        if (pGameArea[level]) {
            delete pGameArea[level];
            pGameArea[level] = nullptr;
        }
        pGameArea[level] = new GAME_AREA;
        pThisArea = pGameArea[level];
        floorNum = 0;

        if (hasTileLimits) {//get angled edges for level
            if (fall_fread32_Array_BE(FileStream, (DWORD*)&tileLimits.east, 4) == -1) {
                fall_fclose(FileStream);
                return false;
            }
            if (fall_fread32_BE(FileStream, &tileLimitFlags) == -1) {
                fall_fclose(FileStream);
                return false;
            }
            tileLimits.east += 1;
            tileLimits.south += 1;
        }
        else {
            tileLimits = { 99, 0, 0, 99 };
            tileLimitFlags = 0;
        }
        if (tileLimits.west < 0)
            tileLimits.west = 0;
        if (tileLimits.east > *pNUM_TILE_X)
            tileLimits.east = *pNUM_TILE_X;
        if (tileLimits.north < 0)
            tileLimits.north = 0;
        if (tileLimits.south > *pNUM_TILE_Y)
            tileLimits.south = *pNUM_TILE_Y;


        RECT edgeRect = { 0,0,0,0 };
        while (currentLev == level) {
            if (fall_fread32_Array_BE(FileStream, (DWORD*)&edgeRect.left, 4) == -1) {
                fall_fclose(FileStream);
                return false;
            }

            if (version == 0x3)
                CopyRect(&pThisArea->rect, &edgeRect);
            else
                SetEdgesFromHexNumbers(&edgeRect, &pThisArea->rect);

            pThisArea->width = pThisArea->rect.right - pThisArea->rect.left;
            pThisArea->height = pThisArea->rect.bottom - pThisArea->rect.top;
            pThisArea->tileLimits = { tileLimits.east, tileLimits.north, tileLimits.west, tileLimits.south };
            pThisArea->tileLimitFlags = tileLimitFlags;
            floorNum++;

            if (fall_fread32_BE(FileStream, (DWORD*)&currentLev) == -1) {
                if (level == 2)//exit loop if eof and level 3 done
                    currentLev = -1;
                else {//read has failed
                    fall_fclose(FileStream);
                    return false;
                }
            }

            if (currentLev == level) {//add new area if same level as last
                pThisArea->next = new GAME_AREA;
                pThisArea = pThisArea->next;
            }
        }
    }
    fall_fclose(FileStream);

    GameAreas_Set_Current_Area_Hex(*pVIEW_HEXPOS);
    return true;
}


//___________________________
void GameAreas_Load_Default() {

    DWORD** levelOffset = *pMapTileLevelOffset;
    LONG tileLstNum = 0;
    LONG tileFlag = 0;
    int tileY = 0, tileX = 0;
    int floorNum = 0;

    GAME_AREA* pThisArea = nullptr;
    pGameArea_Current = nullptr;
    //pGameArea
    for (int level = 0; level < 3; level++) {
        if (pGameArea[level]) {
            delete pGameArea[level];
            pGameArea[level] = nullptr;
        }
        floorNum = 0;
        pGameArea[level] = new GAME_AREA;
        pThisArea = pGameArea[level];


        pThisArea->tileLimits = { 99,0,0,99 };
        pThisArea->rect = { -4800, 0, 3200, 3600 };
        pThisArea->width = 8000;
        pThisArea->height = 3600;

        floorNum++;
    }

    GameAreas_Set_Current_Area_Hex(*pVIEW_HEXPOS);
}


//_________________________
void GameAreas_Load_Tiles() {

    GAME_AREA* pThisArea = nullptr;
    for (int level = 0; level < 3; level++) {
        pThisArea = pGameArea[level];
        while (pThisArea) {
            GameArea_CreateRoofList(pThisArea, level);

            tilecount = 0;
            int yTileLine = pGameArea[level]->tileLimits.north * *pNUM_TILE_X;
            for (int yTile = pGameArea[level]->tileLimits.north; yTile < pGameArea[level]->tileLimits.south; yTile++) {
                for (int xTile = pGameArea[level]->tileLimits.west; xTile < pGameArea[level]->tileLimits.east; xTile++) {
                    if (CheckTileAreaRect(yTileLine + xTile, level, pGameArea[level], false))
                        tilecount++;//debug counter
                }
                yTileLine += *pNUM_TILE_X;
            }
            pThisArea = pThisArea->next;
        }
    }
}


//_______________________
void CheckIfPcUnderRoof() {
    if (!*ppObj_PC)
        return;
    OBJStructDx* pObjDx = (OBJStructDx*)((OBJStruct*)*ppObj_PC);

    LONG tileNum = HexNumToTileNum(pObjDx->hexNum);

    if (!pGameArea_Current)
        return;
    if (!pGameArea_Current->rooves)
        return;
    if (!pGameArea_Current->pMarkedRoofTile)
        return;

    if (gameArea_RoofRef != pGameArea_Current->pMarkedRoofTile[tileNum]) {

        TILE_AREA_Node* pThisRoof = pGameArea_Current->rooves;
        int count = 1;
        if (gameArea_RoofRef != 0) {
            while (pThisRoof && count < gameArea_RoofRef) {
                pThisRoof = pThisRoof->next;
                count++;
            }
            if (pThisRoof) {
                pThisRoof->fadeStartTime = GetTickCount64();
                pThisRoof->fadeIn = true;
            }
        }

        gameArea_RoofRef = pGameArea_Current->pMarkedRoofTile[tileNum];
        pThisRoof = pGameArea_Current->rooves;
        count = 1;
        while (pThisRoof && count < gameArea_RoofRef) {
            pThisRoof = pThisRoof->next;
            count++;
        }

        if (pThisRoof) {
            if (gameArea_RoofRef) {
                pThisRoof->fadeStartTime = GetTickCount64();
                pThisRoof->fadeIn = false;
            }
        }
    }
}


//________________________
bool SetPcPositionVector() {
    if (!*ppObj_PC)
        return false;
    OBJStructDx* pObjDx = (OBJStructDx*)((OBJStruct*)*ppObj_PC);

    DWORD fID = pObjDx->frmID;
    LONG artType = ((0x0F000000 & fID) >> 24);
    if (!IsArtTypeEnabled(artType))
        return false;
    if (pObjDx->hexNum == -1)
        return false;
    if (pObjDx->flags & FLG_Disabled)
        return false;
    if (!pObjDx->frmObjDx)
        return false;

    FRMframeDx* pFrame = pObjDx->frmObjDx->GetFrame();
    if (!pFrame)
        return false;
    XMFLOAT2 objPos;
    LONG xPos = 0, yPos = 0;
    HexNumToSqr(pObjDx->hexNum, &xPos, &yPos);
    objPos.x = (float)xPos;
    objPos.y = (float)yPos;
    objPos.x -= 1;
    objPos.y -= 1;
    objPos.x += pObjDx->xShift;
    objPos.y += pObjDx->yShift;
    objPos.y -= (pFrame->GetHeight() / 2 + 1);


    //position - for drawing egg on rooves.
    PCPosData.PcEgg_Pos.z = objPos.x;
    PCPosData.PcEgg_Pos.w = objPos.y;

    //minus position of game area inside of map area
    if (pGameArea_Current) {
        objPos.x -= pGameArea_Current->rect.left;
        objPos.y -= pGameArea_Current->rect.top;
    }
    //position on game area - for drawing egg on walls
    PCPosData.PcEgg_Pos.x = objPos.x;
    PCPosData.PcEgg_Pos.y = objPos.y;

    pPS_BuffersFallout->UpdatePCObjBuff(&PCPosData);

 

    return true;
}


//_______________________
void PC_ScanLineOfSight() {
    
    if (!FOG_OF_WAR)
        return;
    if (!*ppObj_PC)
        return;

    OBJStructDx* pObjDx = (OBJStructDx*)((OBJStruct*)*ppObj_PC);
    if (pObjDx->hexNum == -1)
        return;
    if (pObjDx->level != *pMAP_LEVEL)
        return;
    if (pObjDx->flags & FLG_Disabled)
        return;

    Scan_Line_Of_Sight(pObjDx->hexNum, pObjDx->level);
    if (p_rc_Fog && pGameArea_Current) {
        OffsetRect(p_rc_Fog, -pGameArea_Current->rect.left, -pGameArea_Current->rect.top);
        if (GameArea_DrawFogRect(pGameArea_Current, p_rc_Fog, pObjDx->level)) {
            delete p_rc_Fog;
            p_rc_Fog = nullptr;
        }
    }
    return;
}

//Things to update on pc movement
//________________________________________________
void __declspec(naked) check_pc_obj_position(void) {

    __asm {
        mov esi, ppObj_PC
        cmp eax, dword ptr ds : [esi]
        jne exitFunc

        pushad
        call SetPcPositionVector
        call CheckIfPcUnderRoof
        call PC_ScanLineOfSight
        popad

        exitFunc :
        mov esi, ppObj_PC
        cmp eax, dword ptr ds : [esi]
        ret
    }
}


//rect is relative to absolute map rect 0,0,8000,3600
//_______________________________________________________
void DrawMapChanges(RECT* pRect, LONG level, DWORD flags) {

    if (level != *pMAP_LEVEL)
        return;
    if (*pDRAW_VIEW_FLAG == 0)
        return;
    if (isLoadingMapData)
        return;

    if (!pGameArea_Current) {
        GameAreas_Set_Current_Area_Hex(*pVIEW_HEXPOS);
        if (!pGameArea_Current)
            return;
    }

    MapLight_ReCreate_RenderTargets();
    RECT rect;

    if (gameArea_ReDraw == true || pRect == nullptr)//Draw the whole Game Area, if flag set or pRect is null.
        rect = { 0, 0, (LONG)pGameArea_Current->width, (LONG)pGameArea_Current->height };
    else
        rect = { pRect->left - pGameArea_Current->rect.left - 1, pRect->top - pGameArea_Current->rect.top - 1, pRect->right - pGameArea_Current->rect.left + 1, pRect->bottom - pGameArea_Current->rect.top + 1 };

    if (gameArea_ReDraw == true) {//redraw everything after reloading the game win render targets
        gameArea_ReDraw = false;
        flags = FLG_Floor | FLG_Obj | FLG_Roof | FLG_Hud;
        //redraw lights
        if (useOriginalLighting_Floor)
            GameArea_DrawHexLightRect(pGameArea_Current, &rect, level);
        else
            GameArea_DrawLightRect(pGameArea_Current, &rect, level);
        //redraw fog
        GameArea_DrawFogRect(pGameArea_Current, &rect, level);
        if (p_rc_Fog)
            delete p_rc_Fog;
        p_rc_Fog = nullptr;
    }


    if (mapLightChange) {
        if ((flags & FLG_Roof) && !(flags & 0xFFFFFFFB))//if only roof flag set dont redraw floor
            mapLightChange = true;
        else if (!(flags & FLG_Floor)) {
            flags |= FLG_Floor;
        }
        mapLightChange = false;
    }

    if (flags & FLG_Floor) {
        GameArea_DrawFloorRect(pGameArea_Current, &rect, level, flags);
    }
    if (flags & FLG_Obj || flags & FLG_Hud_Outline) {
        GameArea_ClearHUD_Outline_Rect(pGameArea_Current, &rect, level, flags);
        GameArea_DrawObjectRect(pGameArea_Current, &rect, level, flags);
    }
    if (flags & FLG_Roof) {
    }
    if (flags & FLG_Hud) {
        GameArea_ClearHUDRect(pGameArea_Current, &rect, level, flags);
        GameArea_Draw_FloatingText(pGameArea_Current, &rect, level, flags);

    }
}


//_______________________________________
void __declspec(naked) map_draw_obj(void) {

    __asm {
        pushad

        push FLG_Obj
        push edx
        push eax
        call DrawMapChanges
        add esp, 0xC

        popad
        ret
    }
}


//________________________________________
void __declspec(naked) map_draw_roof(void) {

    __asm {
        pushad

        push FLG_Roof
        mov eax, pMAP_LEVEL
        mov eax, dword ptr ds : [eax]
        push eax
        push edx
        call DrawMapChanges
        add esp, 0xC

        popad
        ret
    }
}


//_______________________________________
void __declspec(naked) map_draw_hud(void) {

    __asm {
        pushad

        push FLG_Hud
        push edx
        push eax
        call DrawMapChanges
        add esp, 0xC

        popad
        ret
    }
}


//___________________________________________
void __declspec(naked) map_draw_hud_obj(void) {

    __asm {
        pushad

        xor ebx, ebx
        mov ebx, FLG_Hud_Outline
        or ebx, FLG_Obj
        push ebx
        push edx
        push eax
        call DrawMapChanges
        add esp, 0xC

        popad
        ret
    }
}


//_____________________________________________
void __declspec(naked) map_draw_floor_obj(void) {

   __asm {
      pushad

      xor ebx, ebx
      mov ebx,FLG_Floor
      or ebx, FLG_Obj
      push ebx
      push edx
      push eax
      call DrawMapChanges
      add esp, 0xC

      popad
      ret
   }
}


//_____________________________________________
void DrawMapChangesAll(RECT* pRect, LONG level) {
    DrawMapChanges(nullptr, level, FLG_Floor | FLG_Obj | FLG_Hud | FLG_Hud_Outline);
}


//_______________________________________
void __declspec(naked) map_draw_all(void) {

    __asm {
        pushad

        push edx
        push eax
        call DrawMapChangesAll
        add esp, 0x8

        popad
        ret
    }
}


//________________________________________________
void ClearOutlineHud(OBJStruct* pObj, RECT* pRect) {

    *pRect = { 0,0,0,0 };
    if (pObj->level != *pMAP_LEVEL)
        return;
    RECT rcHud;
    GetObjRectDx(pObj, &rcHud);

    XMFLOAT4 clearColour = { 1.0f,0.0f,0.0f,1.0f };

    rcHud.left -= rcGame_PORTAL.left;
    rcHud.left -= 1;
    rcHud.top -= rcGame_PORTAL.top;
    rcHud.top -= 1;
    rcHud.right -= rcGame_PORTAL.left;
    rcHud.right += 1;
    rcHud.bottom -= rcGame_PORTAL.top;
    rcHud.bottom += 1;
    if (pGameArea_Current)
        pGameArea_Current->pHud_RT_Outline->ClearRect(clearColour, &rcHud);
}


//To-Do dont remember if this is a valid To-Do - clear hud rect when this is called
//0048C2FB  |.  E8 6CF3FFFF   CALL void GetObjectRect(ObjStruct *pObj, RECT *pReturnRect)      ; [fallout2.void GetObjectRect(ObjStruct *pObj, RECT *pReturnRect), get_obj_rect(EAX objStruct, EDX *rect)
//____________________________________________
void __declspec(naked) clear_outline_hud(void) {

    __asm {
        push edx
        push eax
        call ClearOutlineHud
        add esp, 0x8
        ret
    }
}


//______________________________
void DrawObjOutlines(LONG level) {
    if (pAreObjectsInitiated == nullptr || *pAreObjectsInitiated == 0)
        return;

    OBJNode* mapObj = nullptr;
    LONG hexPos = 0;
    //draw flat objects first
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];
        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (!(mapObj->pObj->flags & FLG_Disabled)) {
                        GameArea_DrawObjOutline(pGameArea_Current, mapObj->pObj);
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


//__________________________________________
void DrawPalAniObjs(LONG level, DWORD flags) {

    if (pAreObjectsInitiated == nullptr || *pAreObjectsInitiated == 0)
        return;
    if (pIsPaletteCyclingEnabled == nullptr || *pIsPaletteCyclingEnabled == 0)
        return;

    /*if (!pGame_Hud_2) {
        pGame_Hud_2 = new RenderTarget(0, 0, game_PortalWidth, game_PortalHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
            //display03->SetScale(gameArea_scaleX, gameArea_scaleY);
    }
    else
        pGame_Hud_2->ClearRenderTarget(nullptr);*/
        /*if (!display04) {
            display04 = new RenderTarget(0, 0, game_PortalWidth, game_PortalHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000);
             //display04->SetScale(gameArea_scaleX, gameArea_scaleY);
        }
        else
            display04->ClearRenderTarget(nullptr);*/

    OBJNode* mapObj = nullptr;
    RECT rect;
    LONG hexPos = 0;
    //draw flat objects first
    for (hexPos = 0; hexPos < *pNUM_HEXES; hexPos++) {
        mapObj = pMapObjNodeArray[hexPos];
        while (mapObj) {
            if (mapObj->pObj->level <= level) {
                if (mapObj->pObj->level == level) {
                    if (!(mapObj->pObj->flags & FLG_Disabled)) {
                        if (GetObjRectDxPalAni(mapObj->pObj, &rect, flags)) {
                            if (DoRectsOverlap(&rect, &rcGame_PORTAL)) {//only draw if inside viewable area
                                if (mapObj->pObj->flags & FLG_Flat)
                                    DrawMapChanges(&rect, level, FLG_Obj | FLG_Floor);
                                else
                                    DrawMapChanges(&rect, level, FLG_Obj);
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

    if (pGameArea_Current)
        GameArea_DrawPalAniTiles(pGameArea_Current, level, flags);
    /*
    if (pGame_Hud_2) {
        ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
        if (pD3DDevContext == nullptr)
            return;
        pD3DDevContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Linear);
        SetScreenProjectionMatrix_XM(pGame_Hud_2->GetWidth(), pGame_Hud_2->GetHeight());
        //display03->Display(pd3d_PS_Basic_Tex_32);
        pGame_Hud_2->SetScale(1.0f, 1.0f);
        //display04->SetScale(1.0f, 1.0f);
        pGame_Hud_2->SetPosition(0, 0);
        //display04->SetPosition(0, 0);

        //display04->ClearRenderTarget(nullptr);
        //genSurfaceData.genData4_1 = { static_cast<float>(display03->GetWidth()*2.5) , static_cast<float>(display03->GetHeight()*2.5) , 0.0f, 1.0f };
        genSurfaceData.genData4_1 = { (float)pGame_Hud_2->GetWidth() * 2.5f, (float)pGame_Hud_2->GetHeight() * 2.5f, 0.0f, 1.0f };
        pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
        for (int i = 0; i < 1; i++) {
            display04->SetRenderTargetAndViewPort(g_d3dDepthStencilView);
            display03->Display(pd3d_PS_GaussianBlurU);
            display03->SetRenderTargetAndViewPort(g_d3dDepthStencilView);
            display04->Display(pd3d_PS_GaussianBlurV);
        }

        SetScreenProjectionMatrix_XM(SCR_WIDTH, SCR_HEIGHT);

        pD3DDevContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Point);
    }
    */
}


//_____________________________________
BYTE* AllocateMemObjDx(DWORD sizeBytes) {
    BYTE* mem = (BYTE*)fall_Mem_Allocate(sizeBytes);
    if (mem)
        memset(mem, 0, sizeBytes);
    return mem;
}


//___________________________________________
void __declspec(naked) allocate_mem_obj(void) {
    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push eax
        call AllocateMemObjDx
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//_________________________________
void DeleteObjDx(OBJStructDx *pObj) {

    if(!pObj)
       return;

    GameAreas_FadingObjects_Sub((OBJStruct*)pObj);

    if(pObj->frmObjDx)
       delete pObj->frmObjDx;
    pObj->frmObjDx = nullptr;

}


//____________________________________________
void __declspec(naked) delete_obj_dx_ptr(void) {

    __asm {
        pushad

        push eax
        call DeleteObjDx
        add esp, 0x4

        popad

        pushad

        push eax
        call fall_Mem_Deallocate
        add esp, 0x4

        popad
        ret
    }
}


//_________________________________
void SetAmbientLight_ShaderEffect() {

    float intensity = (float)GetAmbientLightIntensity() / 65536.0f;
    mapData.AmbientLight = { 1.0f * intensity, 1.0f * intensity, 1.0f * intensity, 1.0f };
    pPS_BuffersFallout->UpdateMapBuff(&mapData);

    if (intensity != ambientLightIntensity_old) {
        ambientLightIntensity_old = intensity;
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Obj | FLG_Roof);
    }

}


//____________________________________________
void __declspec(naked) set_ambient_light(void) {

    __asm {
        pushad

        call SetAmbientLight_ShaderEffect

        popad
        ret
    }
}


//________________________________________________
LONG SetViewPosition_Hex(LONG hexNum, DWORD flags) {

    if (hexNum < 0 || hexNum > *pNUM_HEXES)
        return -1;
    LONG sqrX = 0;
    LONG sqrY = 0;
    HexNumToSqr_Scroll(hexNum, &sqrX, &sqrY);

    GAME_AREA* pCurrent_temp = pGameArea_Current;

    GameAreas_Set_Current_Area_Hex(hexNum);
    if (!pGameArea_Current) { //set the view hex even if no area is set, as this can be set before edges are loaded - position is then set in GameAreas_Load(const char* MapName).
        SetViewHexNum(sqrX, sqrY);
        return 0;
    }
    if (pGameArea_Current != nullptr && pCurrent_temp != pGameArea_Current) {
        SetAmbientLight_ShaderEffect();
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof | FLG_Hud);
    }

    LONG newPosX = sqrX - (game_PortalWidth >> 1) - EDGE_OFF_X;
    LONG newPosY = sqrY - (game_PortalHeight >> 1) - EDGE_OFF_Y;

    if (newPosX == rcGame_PORTAL.left && newPosY == rcGame_PORTAL.top)
        return 0;

    if (*pSCROLL_BLOCK_FLAG) {
        if (pGameArea_Current->width > game_PortalWidth) {
            if (newPosX <= pGameArea_Current->rect.left) {
                newPosX = pGameArea_Current->rect.left;
                EDGE_OFF_X = -EDGE_X_DIFF;
            }
            else if (newPosX >= pGameArea_Current->rect.right - game_PortalWidth) {
                newPosX = pGameArea_Current->rect.right - game_PortalWidth;
                EDGE_OFF_X = EDGE_X_DIFF;
            }
        }
        else {
            LONG edgeWidth = ((game_PortalWidth - pGameArea_Current->width) >> 1);
            newPosX = pGameArea_Current->rect.left - (edgeWidth);
        }

        if (pGameArea_Current->height > game_PortalHeight) {
            if (newPosY <= pGameArea_Current->rect.top) {
                newPosY = pGameArea_Current->rect.top;
                EDGE_OFF_Y = -EDGE_Y_DIFF;
            }
            else if (newPosY >= pGameArea_Current->rect.bottom - game_PortalHeight) {
                newPosY = pGameArea_Current->rect.bottom - game_PortalHeight;
                EDGE_OFF_Y = EDGE_Y_DIFF;
            }
        }
        else {
            LONG edgeHeight = (game_PortalHeight - pGameArea_Current->height) >> 1;
            newPosY = pGameArea_Current->rect.top - (edgeHeight);
        }
    }

    LONG scrnX = newPosX + (game_PortalWidth >> 1) + EDGE_OFF_X;
    LONG scrnY = newPosY + (game_PortalHeight >> 1) + EDGE_OFF_Y;
    if (SetViewHexNum(scrnX, scrnY)) {
        pGameArea_Current->SetPosition((float)-newPosX, (float)-newPosY);
        rcGame_PORTAL = { newPosX, newPosY, newPosX + game_PortalWidth, newPosY + game_PortalHeight };
        PS_UpdatePortalDimensions();
        ResetZoomLevel();
        DrawObjOutlines(*pMAP_LEVEL);
    }

    if (flags & 0x4)// & 0x1) {//draw flag was previously 0x1 but is generally unnecessary, so most calls from exe can be ignored.
        DrawMapChanges(nullptr, *pMAP_LEVEL, FLG_Floor | FLG_Obj | FLG_Roof | FLG_Hud);

    return 0;
}


//______________________________________
LONG MapScroller(LONG xMove, LONG yMove) {

    static ULONGLONG mapScrollerTickCount = 0;
    ULONGLONG tickCount = GetTickCount64();
    ULONGLONG numticksPast = tickCount - mapScrollerTickCount;

    if (numticksPast < 33)
        return -2;//not time to scroll yet
    mapScrollerTickCount = tickCount;
    //xMove *= -1;

    bool no_x_movment = false;
    bool no_y_movment = false;
    if (xMove == 0)
        no_x_movment = true;
    if (yMove == 0)
        no_y_movment = true;

    if (no_x_movment && no_y_movment)
        return -1;//no movement possible

    xMove *= 32;
    yMove *= 24;

    LONG newPosX = rcGame_PORTAL.left;
    LONG newPosY = rcGame_PORTAL.top;

    if (pGameArea_Current) {
        if (*pSCROLL_BLOCK_FLAG) {
            if (xMove) {
                if (pGameArea_Current->width < game_PortalWidth)
                    no_x_movment = true;
                else if (xMove < 0 && rcGame_PORTAL.left == pGameArea_Current->rect.left)
                    no_x_movment = true;
                else if (xMove > 0 && rcGame_PORTAL.left == pGameArea_Current->rect.right - game_PortalWidth)
                    no_x_movment = true;
                else {
                    newPosX += xMove;
                    if (newPosX < pGameArea_Current->rect.left) {
                        newPosX = pGameArea_Current->rect.left;
                        EDGE_OFF_X = -EDGE_X_DIFF;
                    }
                    else if (newPosX > pGameArea_Current->rect.right - game_PortalWidth) {
                        newPosX = pGameArea_Current->rect.right - game_PortalWidth;
                        EDGE_OFF_X = EDGE_X_DIFF;
                    }
                }
            }

            if (yMove) {
                if (pGameArea_Current->height < game_PortalHeight)
                    no_y_movment = true;
                else if (yMove < 0 && rcGame_PORTAL.top == pGameArea_Current->rect.top)
                    no_y_movment = true;
                else if (yMove > 0 && rcGame_PORTAL.top == pGameArea_Current->rect.bottom - game_PortalHeight)
                    no_y_movment = true;
                else {
                    newPosY += yMove;
                    if (newPosY < pGameArea_Current->rect.top) {
                        newPosY = pGameArea_Current->rect.top;
                        EDGE_OFF_Y = -EDGE_Y_DIFF;
                    }
                    else if (newPosY > pGameArea_Current->rect.bottom - game_PortalHeight) {
                        newPosY = pGameArea_Current->rect.bottom - game_PortalHeight;
                        EDGE_OFF_Y = EDGE_Y_DIFF;
                    }
                }
            }

            if (no_x_movment && no_y_movment)
                return -1;
        }
        else {
            newPosX += xMove;
            newPosY += yMove;
        }

        LONG scrnX = newPosX + (game_PortalWidth >> 1) + EDGE_OFF_X;
        LONG scrnY = newPosY + (game_PortalHeight >> 1) + EDGE_OFF_Y;
        if (SetViewHexNum(scrnX, scrnY)) {
            pGameArea_Current->SetPosition((float)-newPosX, (float)-newPosY);
            rcGame_PORTAL = { newPosX, newPosY, newPosX + game_PortalWidth, newPosY + game_PortalHeight };
            PS_UpdatePortalDimensions();
            DrawObjOutlines(*pMAP_LEVEL);
            return 0;
        }
    }

    return -1;
    //return -1;//no movement possible
    //return -2;//not time to scroll yet
}


//_______________________________________
void __declspec(naked) map_scroller(void) {

    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call MapScroller
        add esp, 0x8

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_01(void) {

    __asm {
        mov ecx, dword ptr ds : [eax + 0x6C]//pObj->lightradius
        and ecx, 0x000000FF
        cmp ecx, 0x4
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_02(void) {

    __asm {
        mov eax, dword ptr ds : [esi + 0x6C]//pObj->lightradius
        mov edx, dword ptr ds : [edi + 0x6C]//pObj2->lightradius
        and eax, 0x000000FF
        and edx, 0x000000FF
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_03(void) {
    //eax safe to use
    __asm {
        mov eax, dword ptr ds : [esi + 0x6C]//pObj->lightradius
        and eax, 0xFFFFFF00
        mov dword ptr ds : [esi + 0x6C] , eax
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_04(void) {
    //eax safe to use
    __asm {
        mov dword ptr ds : [esi + 0x70] , ebx//pObj->lightIntensity
        mov eax, dword ptr ds : [esi + 0x6C]//pObj->lightradius
        and eax, 0xFFFFFF00
        and ebp, 0x000000FF
        or eax, ebp
        mov dword ptr ds : [esi + 0x6C] , eax
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_05(void) {
    //edx safe to use
    __asm {
        mov edx, dword ptr ds : [eax + 0x6C]//pObj->lightradius
        and edx, 0x000000FF
        cmp edx, 0x8
        jle endFunc
        mov edx, dword ptr ds : [eax + 0x6C]
        and edx, 0xFFFFFF00
        or edx, 0x00000008
        mov dword ptr ds : [eax + 0x6C] , edx
        endFunc :
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_06(void) {

    __asm {
        mov edx, dword ptr ds : [edx + 0x70]
        mov ebx, dword ptr ds : [ebx + 0x6C]
        and ebx, 0x000000FF
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_07(void) {

    __asm {
        mov eax, dword ptr ds : [esp + 0xC8]
        mov esi, dword ptr ds : [edx + 0x6C]
        and esi, 0x000000FF
        ret
    }
}


//______________________________________________
void __declspec(naked) obj_light_colour_08(void) {

    __asm {
        mov esi, dword ptr ds : [esp + 0xE4]
        mov esi, dword ptr ds : [esi + 0x6C]
        and esi, 0x000000FF
        ret
    }
}


//_____________________________
void Modifications_Dx_Game_CH() {

    //To-Do Modifications_Dx_Game_CH

    MemWrite16(0x502CC2, 0x7674, 0x9090);
    MemWrite16(0x502CCB, 0x6D74, 0x9090);

    MemWrite8(0x502D40, 0x6A, 0xC3);

    MemWrite16(0x485D7E, 0x128B, 0xC031);

    MemWrite16(0x485E77, 0x90FF, 0xC483);
    MemWrite32(0x485E79, 0x80, 0x90909008);

    MemWrite16(0x485E74, 0x8B, 0xC031);

    MemWrite16(0x4863A5, 0x92FF, 0xC483);
    FuncWrite32(0x4863A7, 0x80, 0x90909008);

    //prevent new_window function from checking window size against screen size.
    MemWrite16(0x4DC0F2, 0x64A1, 0x24EB);
    MemWrite16(0x4DC0F4, 0x6BCF, 0x9090);
    MemWrite8(0x4DC0F6, 0x00, 0x90);
}


//________________________________
void Modifications_Dx_Game_MULTI() {

    // in function - 0044B684 PROCESS_MAP_MOUSE()
    MemWrite8(0x44B91A, 0xE8, 0x90);
    MemWrite32(0x44B91B, 0x0659A1, 0x90909090);

    //in function 004D6FD8 DDRAW_STUFF(EAX* winStruct, EDX* rect, EBX* tBuff)
    MemWrite8(0x4D71E6, 0xE8, 0x90);
    MemWrite32(0x4D71E7, 0x03C5, 0x90909090);

    //prevent new_window function from checking window size against screen size.
    MemWrite16(0x4D625D, 0x2D8B, 0x22EB);
    MemWrite32(0x4D625F, FixAddress(0x6AC9F0), 0x90909090);


    //DRAW_SCENE_IF_CURRENT_LEVEL
    MemWrite8(0x44B7B3, 0xE8, 0x90);
    MemWrite32(0x44B7B4, 0x065B08, 0x90909090);

    //force draw of obj only when set to draw obj + mouse
    MemWrite8(0x44BD0B, 0x01, 0x00);

    MemWrite8(0x44C68D, 0xE8, 0x90);
    MemWrite32(0x44C68E, 0x064C2E, 0x90909090);

    MemWrite8(0x44C74B, 0xE8, 0x90);
    MemWrite32(0x44C74C, 0x064B70, 0x90909090);

    MemWrite8(0x44CCE3, 0xE8, 0x90);
    MemWrite32(0x44CCE4, 0x0645D8, 0x90909090);

    MemWrite8(0x44CEA4, 0xE8, 0x90);
    MemWrite32(0x44CEA5, 0x064417, 0x90909090);


    //draw changes to map
    MemWrite32(0x4B11D5, 0x004B1554, (DWORD)&map_draw_hud_obj);
    MemWrite32(0x51D964, 0x004B15E8, (DWORD)&map_draw_hud_obj);


    MemWrite8(0x4826C0, 0x53, 0xE9);
    FuncWrite32(0x4826C1, 0x55575651, (DWORD)&map_scroller);

    //add extra space in object struct to store pointer to dx obj--------------
    MemWrite8(0x488F77, 0x84, 0x88);
    FuncReplace32(0x488F7C, 0x03CB50, (DWORD)&allocate_mem_obj);
    MemWrite8(0x48D782, 0x84, 0x88);
    MemWrite8(0x48D792, 0x84, 0x88);

    //delete obj
    FuncReplace32(0x48DAFA, 0x038126, (DWORD)&delete_obj_dx_ptr);

    FuncReplace32(0x47A962, 0x036972, (DWORD)&set_ambient_light);

    MemWrite16(0x48DC72, 0xFE81, 0xE890);
    FuncWrite32(0x48DC74, 0x9C40, (DWORD)&map_light);

    MemWrite8(0x47AA88, 0xB9, 0xE8);
    FuncWrite32(0x47AA89, 0x27100, (DWORD)&map_light_reset);

    FuncReplace32(0x48EA99, 0x03817B, (DWORD)&draw_light_changes);


    FuncReplace32(0x48A8E5, 0x03C32F, (DWORD)&map_draw_roof);


    //latest sfall is over taking this/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //00482AEC  |.  E8 83000000   CALL 00482B74
    //FuncReplace32(0x482AED, 0x83,  (DWORD)&loading_map_data);

    //latest sfall work aroung
    MemWrite8(0x482B82, 0xB8, 0xE8);
    FuncWrite32(0x482B83, 0x1, (DWORD)&loading_map_data_first);
    MemWrite8(0x483153, 0xB8, 0xE8);
    FuncWrite32(0x483154, 0xFFFFFFFF, (DWORD)&loading_map_data_last);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 ///----------------prevent screen redraw when opening and closing doors------------------------------------------------------------------------------------------------------------MAY CAUSE PROBLEMS--------------------------
    MemWrite8(0x49CB42, 0xE8, 0x90);
    MemWrite32(0x49CB43, 0x014791, 0x90909090);

    MemWrite8(0x49CC03, 0xE8, 0x90);
    MemWrite32(0x49CC04, 0x0146D0, 0x90909090);

    FuncReplace32(0x49CB3E, 0xFFFEE112, (DWORD)&redraw_door_light);
    FuncReplace32(0x49CBFF, 0xFFFEE051, (DWORD)&redraw_door_light);

    //draw door opening
    FuncReplace32(0x49CBDF, 0x0146DD, (DWORD)&map_draw_obj);
    //draw door closing
    FuncReplace32(0x49CCA0, 0x01461C, (DWORD)&map_draw_obj);
    ///----------------------------------------------------------------------------------------

    ///add 4 bytes to allocate mem for d3d obj inside text struct. here
    MemWrite32(0x4B03BF, 0x30, 0x34);
    ///add 4 bytes to clear mem for d3d obj inside text struct. here
    MemWrite32(0x4B03D5, 0x30, 0x34);

    ///create and draw text buff to d3d surface. here
    FuncReplace32(0x4B06B8, 0x025220, (DWORD)&floating_text_create_frame);

    ///destroy text d3d surface. here
    MemWrite16(0x4B023E, 0x408B, 0x9090);
    MemWrite8(0x4B0240, 0x2C, 0x90);
    FuncReplace32(0x4B0242, 0x0159DE, (DWORD)&floating_text_destroy_frame);
    ///and. here
    MemWrite16(0x4B08DD, 0x408B, 0x9090);
    MemWrite8(0x4B08DF, 0x2C, 0x90);
    FuncReplace32(0x4B08E1, 0x01533F, (DWORD)&floating_text_destroy_frame);

    //floating text
    FuncReplace32(0x412885, 0x0009EA37, (DWORD)&map_draw_hud);
    FuncReplace32(0x42B858, 0x00085A64, (DWORD)&map_draw_hud);
    FuncReplace32(0x459492, 0x00057E2A, (DWORD)&map_draw_hud);
    FuncReplace32(0x495E4D, 0x0001B46F, (DWORD)&map_draw_hud);
    FuncReplace32(0x4B0945, 0x00000977, (DWORD)&map_draw_hud);
    FuncReplace32(0x44BD9E, 0x06551E, (DWORD)&map_draw_hud_obj);


    FuncReplace32(0x44D927, 0x0861B5, (DWORD)&redraw_hex_mouse);
    FuncReplace32(0x44DE7C, 0xFFFCB3E0, (DWORD)&destroy_hex_mouse);

    FuncReplace32(0x44D84C, 0x086290, (DWORD)&redraw_hit_mouse);
    FuncReplace32(0x44DDEB, 0xFFFCB471, (DWORD)&destroy_hit_mouse);
    FuncReplace32(0x44DE9D, 0xFFFCB3BF, (DWORD)&destroy_hit_mouse);

    MemWrite16(0x4B12ED, 0x15FF, 0xE890);
    FuncWrite32(0x4B12EF, 0x051D964, (DWORD)&map_draw_all);

    //draw objects only as default draw rect
    MemWrite16(0x4B12D1, 0x15FF, 0xE890);
    FuncWrite32(0x4B12D3, 0x051D964, (DWORD)&map_draw_obj);

    FuncReplace32(0x49B707, 0x00015BB5, (DWORD)&map_draw_floor_obj);//pickup_obj
    FuncReplace32(0x49B992, 0x0001592A, (DWORD)&map_draw_floor_obj);//drop_obj

    //draw after kill from obj to flat corpse
    FuncReplace32(0x410F37, 0x000A0385, (DWORD)&map_draw_floor_obj);

    //draw critter animating while moving hexes
    FuncReplace32(0x417794, 0x00099B28, (DWORD)&map_draw_hud_obj);

    FuncReplace32(0x41798D, 0x0009992F, (DWORD)&map_draw_hud_obj);

    //draw critter animating while not moving
    FuncReplace32(0x417E5F, 0x0009945D, (DWORD)&map_draw_hud_obj);

    FuncReplace32(0x417EE7, 0x000993D5, (DWORD)&map_draw_hud_obj);

    //draw critter animating after moving hexes
    FuncReplace32(0x418568, 0x00098D54, (DWORD)&map_draw_hud_obj);


    FuncReplace32(0x422B7B, 0x0008E741, (DWORD)&map_draw_hud_obj);
    FuncReplace32(0x422C0C, 0x0008E6B0, (DWORD)&map_draw_hud_obj);

    ///00410F36  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw after kill from obj to flat corpse
    //004115F2  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// in throw_change_fid
    ///00412884  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after floating text create----------------------
    //00412B0F  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw mouse2 rect------------- in SELECT_HEXPOS() MAPPER
    //00412B55  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw mouse2 rect------------- in SELECT_HEXPOS() MAPPER
    //0041581A  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0x6
    //004158C3  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0xA
    //00415940  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0xE
    //00415966  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0xE
    //004159A3  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0xF
    //004159C9  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0xF
    //00415A03  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0x10
    //00415A5B  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0x13
    //00415AC9  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//animations case 0x18
    //00415C50  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after delete obj
    ///00417793  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter moving hexes animate
    //0041798C  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    ///00417E5E  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter animating while not moving
    ///00417EE6  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter animating while not moving
    ///00418567  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter animating after move hexes.
    //00418645  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    //004186B4  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    ///00422B7A  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter standing still after animating after move hexes.
    ///00422C0B  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// critter standing still after animating after move hexes.
    //0042B5C2  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after set combat flags
    ///0042B857  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//floating text  - hud only//----------------
    //0042DC8F  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//redraw after critter kill ?
    ///0044B7B3  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse move
    ///0044B91A  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse draw hex msg
    //0044B9D1  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//obj is item under mouse - outline
    ///0044BAB2  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse single menu
    ///0044BC61  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse to hit frm with percent
    ///0044BC89  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse not hitable x?
    ///0044BD81  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw mouse and obj - after combat flags cleared -- item mouse hover highlight?
    ///0044BD9D  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after combat flags cleared -- item mouse hover highlight?
    //0044C174  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//obj under mouse
    ///0044C627  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -multimenu
    ///0044C68D  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -multimenu
    ///0044C6DD  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -multimenu after
    ///0044C74B  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -menu selection?
    ///0044CB46  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse
    ///0044CCE3  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -in set_mouse_frm()
    ///0044CE0F  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse -list related
    ///0044CEA4  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//mouse draw mouse and mouse2 rect
    //0044E56A  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw after deleting obj
    //004541B8  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// drawing obj
    //00454F82  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script func move_to()
    //004550C3  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script func create_obj()
    //00455365  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script func destroy_obj()
    //004566F6  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script func add_obj_to_inventory()     --after light check
    //004567F9  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script func remove_obj_from_inventory()
    //004570FD  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script? draw objs in rect?
    //004572DC  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script?
    //0045754C  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script set_obj_visibility()
    //00457E73  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script kill_critter_type()
    //00459161  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script obj_set_light_level()           --floor and objs
    ///00459491  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// script float_msg------------------------------
    //00459987  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script anim()
    //004599B7  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script anim()
    //0045A2C5  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script add_multi_objs_to_inventory()    --after light check
    //0045A3B7  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script remove_multi_objs_to_inventory()
    //0045C24B  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//script destroy_mult_objs()
    //0046EC00  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//inv item list? //drop obj
    //00470DAA  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// set light radius of obj to 4hexes at max brightness - PC?   --floor and objs
    //00472950  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// inventory wield - change appearence?
    //0047747A  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// in func ADD_ITEM_TO_INVENTORY(EAX *obj_to, EDX *obj_item, EBX numItems)
    //00477681  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// inv place obj onto map?
    //004779DF  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// inv place obj onto map?
    //004797D9  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    //00479984  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    //004799EA  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    ///00495E4C  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after floating text create----------------------
    //0049B706  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR> //pickup_obj
    //0049B885  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>
    //0049B991  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR> // drop_obj
    //0049B9E1  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after deleting obj
    //0049C203  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after placing obj onto map
    //0049C94B  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after move obj
    //0049C9EF  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after move obj
    //0049CA93  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after move obj
    //0049CBDE  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw door opening
    //0049CC9F  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// draw door closing
    //0049D617  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>// after move obj
    ///004B0944  CALL DRAW_SCENE_IF_CURRENT_LEVEL(EAX *pR>//floating txt destroy

    //obj light_dist refs
    MemWrite16(0x470D74, 0x488B, 0xE890);
    FuncWrite32(0x470D76, 0x4F9836C, (DWORD)&obj_light_colour_01);

    MemWrite16(0x472927, 0x468B, 0xE890);
    FuncWrite32(0x472929, 0x6C578B6C, (DWORD)&obj_light_colour_02);

    MemWrite16(0x48ACB2, 0x46C7, 0x9090);
    MemWrite8(0x48ACB4, 0x6C, 0xE8);
    FuncWrite32(0x48ACB5, 0x00000000, (DWORD)&obj_light_colour_03);

    MemWrite16(0x48ACC5, 0x5E89, 0xE890);
    FuncWrite32(0x48ACC7, 0x6C6E8970, (DWORD)&obj_light_colour_04);

    MemWrite32(0x48DCD9, 0x086C7883, 0x90909090);
    MemWrite16(0x48DCDD, 0x077E, 0x9090);
    MemWrite16(0x48DCDF, 0x40C7, 0x9090);
    MemWrite8(0x48DCE1, 0x6C, 0xE8);
    FuncWrite32(0x48DCE2, 0x00000008, (DWORD)&obj_light_colour_05);

    MemWrite16(0x48DD37, 0x528B, 0xE890);
    FuncWrite32(0x48DD39, 0x6C5B8B70, (DWORD)&obj_light_colour_06);

    MemWrite16(0x48DE12, 0x848B, 0x9090);
    MemWrite8(0x48DE14, 0x24, 0xE8);
    FuncWrite32(0x48DE15, 0x000000C4, (DWORD)&obj_light_colour_07);
    MemWrite16(0x48DE19, 0x728B, 0x9090);
    MemWrite8(0x48DE1B, 0x6C, 0x90);

    MemWrite16(0x48E9C0, 0xB48B, 0x9090);
    MemWrite8(0x48E9C2, 0x24, 0xE8);
    FuncWrite32(0x48E9C3, 0x000000E0, (DWORD)&obj_light_colour_08);
    MemWrite16(0x48E9C7, 0x768B, 0x9090);
    MemWrite8(0x48E9C9, 0x6C, 0x90);

    //protos
    //create pros mapper
    //0049EB7A  |> \C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //item
    //0049EDD1  |.  C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //critter
    //0049FC12  |> \C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //scenery
    //0049FD75  |.  C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //wall
    //0049FDEF  |.  C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //tile
    //0049FE67  |.  C746 0C 00000 MOV DWORD PTR DS:[ESI+0C],0                                  ; pro->lightRadius //misc

    //read
    //004A0FF8  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case item,    case 0 of switch fallout2.4A0FE7
    //004A10FB  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case critter, case 1 of switch fallout2.4A0FE7
    //004A11A6  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case scenery, case 2 of switch fallout2.4A0FE7
    //004A1256  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case walls,   case 3 of switch fallout2.4A0FE7
    //004A12CD  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case tiles,   case 4 of switch fallout2.4A0FE7
    //004A1326  |> \8D51 0C       LEA EDX,[ECX+0C]                                             ; pro.lightRadius  case misc,    case 5 of switch fallout2.4A0FE7
    //write
    //004A180A  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case item
    //004A18F7  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case critters
    //004A199C  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case sceney
    //004A1A3B  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case walls
    //004A1AA2  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case tiles
    //004A1AE7  |.  8B51 0C       MOV EDX,DWORD PTR DS:[ECX+0C]                                ; pro->lightRadius  case misc

    //pfall_load_map_data = (void*)FixAddress(0x482B74);

    //sets the draw flag in setViewPosition_Hex, needed to redraw when moving between maps
    MemWrite32(0x48371F, 0x1, 0x00000004);

    //in move object function - pc movement
    MemWrite16(0x48A6B2, 0x053B, 0xE890);
    FuncWrite32(0x48A6B4, FixAddress(0x6610B8), (DWORD)&check_pc_obj_position);

    //pre defined rect structures defining effected area when a light is added or moved from ranges 1 to 9 hex radiuses
    //check light rect
        //0048E9DB | .  8DB6 4C965100 LEA ESI, [ESI + 51964C]
/*
        0051964C  00 00 00 00 | 00 00 00 00 | 60 00 00 00 | 2A 00 00 00 |
        0051965C  00 00 00 00 | 00 00 00 00 | A0 00 00 00 | 4A 00 00 00 |
        0051966C  00 00 00 00 | 00 00 00 00 | E0 00 00 00 | 6A 00 00 00 |
        0051967C  00 00 00 00 | 00 00 00 00 | 20 01 00 00 | 8A 00 00 00 |
        0051968C  00 00 00 00 | 00 00 00 00 | 60 01 00 00 | AA 00 00 00 |
        0051969C  00 00 00 00 | 00 00 00 00 | A0 01 00 00 | CA 00 00 00 |
        005196AC  00 00 00 00 | 00 00 00 00 | E0 01 00 00 | EA 00 00 00 |
        005196BC  00 00 00 00 | 00 00 00 00 | 20 02 00 00 | 0A 01 00 00 |
        005196CC  00 00 00 00 | 00 00 00 00 | 60 02 00 00 | 2A 01 00 00 |


      3  96, 42
      5  160, 74
      7  224, 106
      9  288, 138
      11  352, 170
      13  416, 202
      15  480, 234
      17  544, 266
      19  608, 298
*/
//adjust the rect.bottom element to fix artifacts when redrawing some lit scenery
/*    UINT light_rect_height_add = LIT_OBJ_HEIGHT_ADJUSTMENT * 2;
    MemWrite32(0x519658, 42, 42 + light_rect_height_add);
    MemWrite32(0x519668, 74, 74 + light_rect_height_add);
    MemWrite32(0x519678, 106, 106 + light_rect_height_add);
    MemWrite32(0x519688, 138, 138 + light_rect_height_add);
    MemWrite32(0x519698, 170, 170 + light_rect_height_add);
    MemWrite32(0x5196A8, 202, 202 + light_rect_height_add);
    MemWrite32(0x5196B8, 234, 234 + light_rect_height_add);
    MemWrite32(0x5196C8, 266, 266 + light_rect_height_add);
    MemWrite32(0x5196D8, 298, 298 + light_rect_height_add);*/


    //in function - SET_HEX_LIGHT_INTENSITY(EAX level, EDX hexPos, EBX lightLevel)
    MemWrite16(0x47AA36, 0xFA81, 0xE890);
    FuncWrite32(0x47AA38, 0x9C40, (DWORD)&add_hex_light);
    //in function - SUBTRACT_FROM_LIGHT_MAP(EAX level, EDX hexPos, EBX light) light 0-65536
    MemWrite16(0x47AA72, 0xFA81, 0xE890);
    FuncWrite32(0x47AA74, 0x9C40, (DWORD)&sub_hex_light);
}


//__________________________
void Modifications_Dx_Game() {

    if (SfallReadInt(L"Misc", L"MoreTiles", 0) != 0)
        SFALL_MoreTiles = true;

    //lightColour_PC
    DWORD colour = ConfigReadInt(L"MAPS", L"PC_LIGHT_COLOUR", 0xFFFFFF);// ; R00 G00 B00");//display as a hexadecimal number in the ini, so as to more intuitively modify colour values.
    lightColour_PC.x = (float)((colour & 0x00FF0000) >> 16) / 256.0f;
    lightColour_PC.y = (float)((colour & 0x0000FF00) >> 8) / 256.0f;
    lightColour_PC.z = (float)(colour & 0x000000FF) / 256.0f;
    lightColour_PC.w = 1.0f;
    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Dx_Game_CH();
    else
        Modifications_Dx_Game_MULTI();
}
