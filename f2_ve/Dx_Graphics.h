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

#include "Dx_General.h"
#include "Fall_Graphics.h"
#include "win_fall.h"
#include "modifications.h"
#include "graphics.h"


//for debuging - to keep track of number of frames currently loaded
extern int frameCount;

class FRMdx;

class FRMframeDx : public BASE_VERTEX_DX, public BASE_TEXTURE_STAGING {

public:
    FRMframeDx(FRMdx* p_in_frm_parent, DWORD inWidth, DWORD inHeight, BYTE* indexBuff, DWORD* pAltPal, bool is32bit, BOOL widenForOutline);
    ~FRMframeDx() {
        DestroyBaseTextures();
        p_frm_parent = nullptr;
        frameCount--;
    };
    void DestroyBaseTextures();

    bool GetRect(RECT* pRect) {
        if (!pRect)
            return false;
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = 0 + width - 1;
        pRect->bottom = 0 + height - 1;
        return true;
    };
    //Get the rectangle around the palette animated area of the frame. 
    bool GetAniRect(RECT* pRect) {
        if (!pRect)
            return false;
        pRect->left = ani_Rect.left;
        pRect->top = ani_Rect.top;
        pRect->right = ani_Rect.right;
        pRect->bottom = ani_Rect.bottom;
        return true;
    };
    void DrawToFrame(BYTE* fBuff, DWORD* fpal, bool is_fBuff_32bit, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY);
    /*ID3D11ShaderResourceView* GetBaseTex(bool reduceFauxShadows) {
        if (pTex_Base_shaderResourceView)
            return pTex_Base_shaderResourceView;
        DrawBaseTex(reduceFauxShadows);
        return pTex_Base_shaderResourceView;
    };*/
    ID3D11ShaderResourceView* GetBaseTex(bool create_from_frame) {
        if (pTex_Base_shaderResourceView)
        return pTex_Base_shaderResourceView;
        if (create_from_frame)
            DrawBaseTex_From_Frame(false);
        return pTex_Base_shaderResourceView;
    };
    void DrawBaseTex(BYTE* indexBuff);
    bool Is32bitColour() {
        if (pixelWidth == 4)
            return true;
        else
            return false;
    };
    void DrawFrame(DirectX::XMMATRIX* pOrtho2D, float xPos, float yPos);
    void DrawFrame(DirectX::XMMATRIX* pOrtho2D, float xPos, float yPos, ID3D11PixelShader* pd3d_PixelShader);
    //Set the x and y offsets incorporating the frm orientation offset plus frame centring for map objects. 
    void SetOffsets_Centre(LONG x, LONG y) {
        x_offset_ori_centre = x;
        y_offset_ori_centre = y;
    };
    void SetOffsets_FromPreviousFrame(LONG x, LONG y) {
        x_offset_from_previous_frame = x;
        y_offset_from_previous_frame = y;
    };
    void SetOffsets_FromFirstFrame(LONG x, LONG y) {
        x_offset_from_first_frame = x;
        y_offset_from_first_frame = y;
    };
    //Get the pixel width and height of the frame texture mapped to a quad, 0.0f to 1.0f.
    void GetTexturePixelSize(float* p_x, float* p_y) {
        *p_x = pixelSize.x;
        *p_y = pixelSize.y;
    };
    LONG GetOffset_FromPreviousFrame_X() { return x_offset_from_previous_frame; };
    LONG GetOffset_FromPreviousFrame_Y() { return y_offset_from_previous_frame; };
    LONG GetOffset_FromFirstFrame_X() { return x_offset_from_first_frame; };
    LONG GetOffset_FromFirstFrame_Y() { return y_offset_from_first_frame; };
    //Get the x offset incorporating the frm orientation offset plus frame "-width/2+1", for map objects centring. 
    LONG GetOffset_OriCentre_X() { return x_offset_ori_centre; };
    //Get the y offset incorporating the frm orientation offset plus frame "-height+1", for map objects centring.
    LONG GetOffset_OriCentre_Y() { return y_offset_ori_centre; };
    //Check if frame has palette animated pixels.
    bool IsAnimated() { return isAnimated; };
    DWORD Animation_ZoneFlags() { return ani_TimeZones; };
    FRMdx* GetParent() { return p_frm_parent; };
protected:

private:
    FRMdx* p_frm_parent;
    LONG x_offset_from_previous_frame;
    LONG y_offset_from_previous_frame;
    LONG x_offset_from_first_frame;
    LONG y_offset_from_first_frame;
    LONG x_offset_ori_centre; //x offset incorporating the frm orientation offset plus frame "-width/2+1", for map objects centring. 
    LONG y_offset_ori_centre;//y offset incorporating the frm orientation offset plus frame "-height+1", for map objects centring. 

    bool isAnimated;
    DWORD ani_TimeZones;
    RECT ani_Rect;

    DirectX::XMFLOAT2 pixelSize;
    ID3D11Texture2D* pTex_Base;
    ID3D11ShaderResourceView* pTex_Base_shaderResourceView;
    
    BOOL tex_widened;//for widening textures by 2 pixels for use with the outline shader;
    int lockType;
    void Draw8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD toWidth);
    void Draw32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD toWidth);
    void DrawPal32(BYTE* fBuff, DWORD* pal, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD toWidth);
    void Draw(BYTE* indexBuff, DWORD* pal);
    void DrawBaseTex_From_Frame(bool reduceFauxShadows);
};



//32 bit frms - version will equal 5  - size still num bytes in buff
class FRMdx {
public:
    FRMdx(DWORD frmID);//(UNLSTDfrm *frm);
    FRMdx(DWORD frmID, LONG in_palOffset_Mask);
    FRMdx(const char* FrmName, LONG type, LONG in_palOffset_Mask);///for loading unlisted frms - remember to delete after use.
    ~FRMdx() {
        for (int ori = 0; ori < 6; ori++) {
            if (lpFrame[ori]) {
                for (int t = 0; t < numFrames; t++) {
                    if (lpFrame[ori][t]) {
                        delete lpFrame[ori][t];
                        lpFrame[ori][t] = nullptr;
                    }
                }
                delete[] lpFrame[ori];
                lpFrame[ori] = nullptr;
            }
        }
    }
    WORD GetFPS() { return FPS; };
    INT16 GetNumFrames() { return numFrames; };
    INT16 GetShift_X(DWORD ori) { if (ori < 6)return xCentreShift[ori]; else return 0; };
    INT16 GetShift_Y(DWORD ori) { if (ori < 6)return yCentreShift[ori]; else return 0; };
    FRMframeDx* GetFrame(DWORD ori, WORD frameNum);
    bool Add(DWORD frmID);
    bool AddFrames(UNLSTDfrm* frm, int fileOri, int ori);
    bool isUniformlyLit() { return uniformly_lit; };
    BOOL IsTex_Widened() { return tex_widened; };
protected:
private:
    DWORD version;//version num
    WORD FPS;//frames per sec
    WORD actionFrame;
    INT16 numFrames;//number of frames per direction
    INT16 xCentreShift[6];//offset from frm centre +=right -=left
    INT16 yCentreShift[6];//offset from frm centre +=down -=up
    FRMframeDx** lpFrame[6];
    DWORD type;
    BOOL tex_widened;
    bool uniformly_lit;//true if the frm should be lit uniformly as originally intend. Don't apply the new lighting.
    LONG palOffset_Mask;//palette offset of mask colour - this is 0 by default call FRMdx(DWORD frmID, , LONG in_palOffset_Mask)
};


#define NUM_TYPES 11//8//7              //number of frm art types
#define CLEAR_FRM_CACHE_DELAY   30000   //milliseconds to wait before deleting an unused frm
#define CLEAR_FRM_CACHE_MAX_FRMS   8    //max number of frm's to delete at a time

extern DWORD nodeCount;

class FRMDXnode {
public:
    FRMDXnode() {
        inUse = 0;
        discard_time = 0;
        frmDX = nullptr;
        nodeCount++;
    };
    ~FRMDXnode() {
        if (frmDX)
            delete frmDX;
        inUse = 0;
        frmDX = nullptr;
        nodeCount--;
    };
    FRMdx* Create(DWORD frmID) {
        if (!frmDX)
            frmDX = new FRMdx(frmID);
        else
            frmDX->Add(frmID);
        inUse++;
        return frmDX;
    };
    FRMdx* Create(DWORD frmID, LONG palOffset_Mask) {
        if (!frmDX)
            frmDX = new FRMdx(frmID, palOffset_Mask);
        else
            frmDX->Add(frmID);
        inUse++;
        return frmDX;
    };
    bool Discard() {
        bool isAlreadyBeingDiscarded = false;
        if (discard_time != 0)
            isAlreadyBeingDiscarded = true;
        if (inUse > 0) {
            inUse--;
            if (inUse == 0)
                discard_time = GetTickCount64() + CLEAR_FRM_CACHE_DELAY;
        }

        return isAlreadyBeingDiscarded;
    };
    void Discard_Reset() {
        discard_time = 0;
    };
    FRMdx* GetFrm() { return frmDX; };
    FRMframeDx* GetFrame(DWORD ori, WORD frameNum) {
        if (frmDX)
            return frmDX->GetFrame(ori, frameNum);
        else
            return nullptr;
    };
    bool IsInUse() {
        if (inUse > 0)
            return true;
        else
            return false;
    };
    ULONGLONG GetDiscardTime() {
        return discard_time;
    };
protected:
private:
    LONG inUse;
    FRMdx* frmDX;
    ULONGLONG discard_time;
};


struct FRMDX_DELETE_node {
    DWORD type;
    DWORD num;
    FRMDX_DELETE_node* next;
};


///Hopefully these are right
#define ID1_SIZE_CRITTER    12      //max number of weapons
#define ID2_SIZE_CRITTER    66      //max number of animations last one seems to be ANIM_max_anim 65
#define ID1_SIZE_HEAD       4//2      //suffix1 max 3 fidgits + 1 for none
#define ID2_SIZE_HEAD       12//3    //suffix2 + suffix3 max

class FRMDXcache {
public:
    FRMDXcache() {
        node_to_delete_count = 0;
        node_count = 0;
        nodes_in_use = 0;
        nodes_added_since = 0;

        id2_Block_critter = 0;
        id2_Block_head = 0;

        frmDxDeleteNode = nullptr;
        for (DWORD type = 0; type < NUM_TYPES; type++) {
            size[type] = GetArtTypeSize(type);
            if (type == ART_CRITTERS) {
                id2_Block_critter = size[type] * ID1_SIZE_CRITTER;
                node[type] = new FRMDXnode * [size[type] * ID1_SIZE_CRITTER * ID2_SIZE_CRITTER];
                for (DWORD num = 0; num < size[type] * ID1_SIZE_CRITTER * ID2_SIZE_CRITTER; num++)
                    node[type][num] = nullptr;
            }
            else if (type == ART_HEADS) {

                id2_Block_head = size[type] * ID1_SIZE_HEAD;
                node[type] = new FRMDXnode * [size[type] * ID1_SIZE_HEAD * ID2_SIZE_HEAD];
                for (DWORD num = 0; num < size[type] * ID1_SIZE_HEAD * ID2_SIZE_HEAD; num++)
                    node[type][num] = nullptr;
            }
            else {
                node[type] = new FRMDXnode * [size[type]];
                for (DWORD num = 0; num < size[type]; num++)
                    node[type][num] = nullptr;
            }
        }
    }
    ~FRMDXcache() {
        for (DWORD type = 0; type < NUM_TYPES; type++) {
            if (node[type]) {
                if (type == ART_CRITTERS) {
                    for (DWORD num = 0; num < size[type] * ID1_SIZE_CRITTER * ID2_SIZE_CRITTER; num++) {
                        if (node[type][num])
                            delete  node[type][num];
                    }
                }
                else if (type == ART_HEADS) {
                    for (DWORD num = 0; num < size[type] * ID1_SIZE_HEAD * ID2_SIZE_HEAD; num++) {
                        if (node[type][num])
                            delete  node[type][num];
                    }
                }
                else {
                    for (DWORD num = 0; num < size[type]; num++) {
                        if (node[type][num])
                            delete node[type][num];
                    }
                }
                delete[] node[type];
                node[type] = nullptr;
                size[type] = 0;
            }
        }
    }
    FRMDXnode* Get(DWORD frmID) {
        if (!frmID)
            return nullptr;
        DWORD type = (frmID & 0x0F000000) >> 24;
        DWORD num = frmID & 0x00000FFF;
        DWORD ID2 = (frmID & 0x00FF0000) >> 16;
        DWORD ID1 = (frmID & 0x0000F000) >> 12;

        if (type < 0 || type >= NUM_TYPES)
            return nullptr;
        if (num > size[type])
            return nullptr;
        if (type == ART_CRITTERS)
            num = ID2 * id2_Block_critter + ID1 * size[type] + num;
        else if (type == ART_HEADS) {
            if (ID2 != 1 && ID2 != 4 && ID2 != 7) {
                //if (ID1) {
                    //char msg[64];
                    //sprintf_s(msg, "frmCache false figet Get %d", ID1);
                    //imonitorInsertText(msg);
                //}
                ID1 = 0;
            }
            else
                ID1 += 1;
            num = ID2 * id2_Block_head + ID1 * size[type] + num;
        }
        if (node[type][num] == nullptr) {
            node[type][num] = new FRMDXnode;
            node_count++;
        }
        node[type][num]->Create(frmID);
        nodes_in_use++;
        return node[type][num];
    };
    FRMDXnode* Get(DWORD frmID, LONG palOffset_Mask) {
        if (!frmID)
            return nullptr;
        DWORD type = (frmID & 0x0F000000) >> 24;
        DWORD num = frmID & 0x00000FFF;
        DWORD ID2 = (frmID & 0x00FF0000) >> 16;
        DWORD ID1 = (frmID & 0x0000F000) >> 12;

        if (type < 0 || type >= NUM_TYPES)
            return nullptr;
        if (num > size[type])
            return nullptr;
        if (type == ART_CRITTERS)
            num = ID2 * id2_Block_critter + ID1 * size[type] + num;
        else if (type == ART_HEADS) {
            if (ID2 != 1 && ID2 != 4 && ID2 != 7) {
                if (ID1) {
                    char msg[64];
                    sprintf_s(msg, "frmCache false figet Get %d", ID1);
                    imonitorInsertText(msg);
                }
                ID1 = 0;
            }
            else
                ID1 += 1;
            num = ID2 * id2_Block_head + ID1 * size[type] + num;
        }

        if (node[type][num] == nullptr) {
            node[type][num] = new FRMDXnode;
            node_count++;
        }
        node[type][num]->Create(frmID, palOffset_Mask);
        nodes_in_use++;
        return node[type][num];
    };
    void Forget(DWORD frmID) {
        if (!frmID)
            return;
        DWORD type = (frmID & 0x0F000000) >> 24;
        DWORD num = frmID & 0x00000FFF;
        DWORD ID2 = (frmID & 0x00FF0000) >> 16;
        DWORD ID1 = (frmID & 0x0000F000) >> 12;

        if (type < 0 || type >= NUM_TYPES)
            return;
        if (num > size[type])
            return;
        if (type == ART_CRITTERS)
            num = ID2 * id2_Block_critter + ID1 * size[type] + num;
        else if (type == ART_HEADS) {
            if (ID2 != 1 && ID2 != 4 && ID2 != 7) {
                if (ID1) {
                    char msg[64];
                    sprintf_s(msg, "frmCache false figet Forget %d", ID1);
                    imonitorInsertText(msg);
                }
                ID1 = 0;
            }
            else
                ID1 += 1;
            num = ID2 * id2_Block_head + ID1 * size[type] + num;
        }

        if (node[type][num] != nullptr) {
            if (!node[type][num]->Discard() && !node[type][num]->IsInUse()) // if discard time already set before this dicard than frm should already be in the deletion list, so no need to add a new deletion node - deletion time is updated in Discard function.
                AddFrmForDeletion(type, num);
        }
        nodes_in_use--;
    };
    void ClearFrmCache() {
        if (!frmDxDeleteNode)
            return;
        FRMDX_DELETE_node* thisNode = frmDxDeleteNode;
        FRMDX_DELETE_node* tempNode = nullptr;
        FRMDX_DELETE_node* lastNode = frmDxDeleteNode;
        int count = 0;
        ULONGLONG time = GetTickCount64();
        while (thisNode && count < CLEAR_FRM_CACHE_MAX_FRMS) {
            if (!node[thisNode->type][thisNode->num] || node[thisNode->type][thisNode->num]->GetDiscardTime() < time) {
                if (thisNode == frmDxDeleteNode) {
                    frmDxDeleteNode = thisNode->next;
                    lastNode = thisNode->next;
                }
                else
                    lastNode->next = thisNode->next;
                tempNode = thisNode;
                thisNode = thisNode->next;

                if (node[tempNode->type][tempNode->num] && !node[tempNode->type][tempNode->num]->IsInUse()) {
                    //imonitorInsertText("frmCache deleting");
                    delete node[tempNode->type][tempNode->num];
                    node[tempNode->type][tempNode->num] = nullptr;
                    node_count--;
                    count++;
                }
                else if (node[tempNode->type][tempNode->num]) //if frm is in use reset the discard time before deleting node
                    node[tempNode->type][tempNode->num]->Discard_Reset();
                tempNode->next = nullptr;
                delete tempNode;
                node_to_delete_count--;
            }
            else {
                lastNode = thisNode;
                thisNode = thisNode->next;
            }
        }
    };
    void AddFrmForDeletion(DWORD type, DWORD num) {
        if (type < 0 || type >= NUM_TYPES)
            return;
        FRMDX_DELETE_node* nextNode = nullptr;
        if (frmDxDeleteNode)
            nextNode = frmDxDeleteNode;
        frmDxDeleteNode = new FRMDX_DELETE_node;
        frmDxDeleteNode->type = type;
        frmDxDeleteNode->num = num;
        frmDxDeleteNode->next = nextNode;
        node_to_delete_count++;
        nodes_added_since++;
    };
    void PrintStats() {
        char msg[64];
        sprintf_s(msg, "frmCache nodes_added_since %d", nodes_added_since);
        imonitorInsertText(msg);
        sprintf_s(msg, "frmCache nodes to delete %d", node_to_delete_count);
        imonitorInsertText(msg);
        sprintf_s(msg, "frmCache node count %d", node_count);
        imonitorInsertText(msg);
        sprintf_s(msg, "frmCache nodes in use %d", nodes_in_use);
        imonitorInsertText(msg);
        if (frmDxDeleteNode == nullptr)
            imonitorInsertText("frmDxDeleteNode = null");
        nodes_added_since = 0;
    };
protected:
private:
    DWORD size[NUM_TYPES];
    FRMDXnode** node[NUM_TYPES];

    DWORD id2_Block_critter;// size[ART_CRITTERS] * ID1_SIZE_CRITTER. - ID2 offset into cache for critters.
    DWORD id2_Block_head;// size[ART_HEADS] * ID1_SIZE_HEAD. - ID2 offset into cache for talking heads.

    FRMDX_DELETE_node* frmDxDeleteNode;
    LONG node_to_delete_count;
    LONG node_count;
    LONG nodes_in_use;
    LONG nodes_added_since;
};


extern FRMDXcache *frmDxCache;
void Destroy_FRM_DX_CACHE();


class FRMCached {
public:
    FRMCached(DWORD inFrmID) {
        frmID = inFrmID;
        frmNode = nullptr;
        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        frmNode = frmDxCache->Get(frmID);
        if (!frmNode)
            frmID = 0;
    }
    FRMCached(DWORD inFrmID, LONG palOffset_Mask) {
        frmID = inFrmID;
        frmNode = nullptr;
        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        frmNode = frmDxCache->Get(frmID, palOffset_Mask);
        if (!frmNode)
            frmID = 0;
    }
    ~FRMCached() {
        if (frmID && frmDxCache)
            frmDxCache->Forget(frmID);
        frmNode = nullptr;
        frmID = 0;
    }
    FRMframeDx* GetFrame(int ori, int frameNum) {
        if (!frmDxCache)
            return nullptr;
        if (!frmNode)
            return nullptr;
        return frmNode->GetFrame(ori, frameNum);
    }
    FRMdx* GetFrm() {
        if (!frmDxCache)
            return nullptr;
        if (!frmNode)
            return nullptr;
        return frmNode->GetFrm();
    }
protected:
private:
    DWORD frmID;
    FRMDXnode* frmNode;
};


extern FRMdx* mouseInfoframeDx_Hit;
