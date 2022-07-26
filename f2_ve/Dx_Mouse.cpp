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
#include "Fall_GameMap.h"
#include "Fall_General.h"
#include "win_fall.h"
#include "modifications.h"
#include "memwrite.h"


#include "Dx_General.h"
#include "Dx_Mouse.h"
#include "Dx_Windows.h"
#include "Dx_Graphics.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

LONG* pIsMouseAquired = nullptr;
LONG* pIsMouseHidden = nullptr;
LONG* pMouseX = nullptr;
LONG* pMouseY = nullptr;
LONG* pMousePicXOffset = nullptr;
LONG* pMousePicYOffset = nullptr;

DWORD* pINV_FRM_LST_NUMS = nullptr;
WORD* pMenu_FRM_LST_NUMS = nullptr;
DWORD* pINV_MOUSE_FRM_LST_NUM_CURRENT = nullptr;

LONG* pfrmObjRef_Mouse = nullptr;


class MouseMenuDX : public POSITION_DX {
public:
    MouseMenuDX(DWORD* pInFrmID, LONG inNumItems) :
        POSITION_DX(inNumItems, false),
        x(0),
        y(0),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        ori(0),
        frameNum(0),
        numItems(0),
        selectedItem(0),
        lp_cfrm(nullptr),
        pfrmID(nullptr)
    {
        pD3DDevContext = GetD3dDeviceContext();
        if (inNumItems <= 1)
            numItems = 1;
        else
            numItems = inNumItems;
        pfrmID = new DWORD[numItems];
        lp_cfrm = new FRMCached * [numItems];

        if (!frmDxCache)
            frmDxCache = new FRMDXcache;
        for (int i = 0; i < numItems; i++) {
            pfrmID[i] = pInFrmID[i];
            //highlighted item if frm list num -1
            if (i == selectedItem && numItems > 1)//only highlight item in multi item menu
                lp_cfrm[i] = new FRMCached(pfrmID[i] - 1);
            else
                lp_cfrm[i] = new FRMCached(pfrmID[i]);

        }
        pmxWorld = new XMFLOAT4X4[numItems];
        pmxWorldPrjection = new XMFLOAT4X4[numItems];
        SetMatrices();
    };
    ~MouseMenuDX() {
        if (pfrmID)
            delete[] pfrmID;
        pfrmID = nullptr;
        if (lp_cfrm)
            delete[] lp_cfrm;
        lp_cfrm = nullptr;
        if (pmxWorld)
            delete[] pmxWorld;
        pmxWorld = nullptr;
        if (pmxWorldPrjection)
            delete[] pmxWorldPrjection;
        pmxWorldPrjection = nullptr;

        pD3DDevContext = nullptr;
    };
    void SetPosition(float inX, float inY) {
        float xNow = inX;
        float yNow = inY;
        FRMframeDx* pFrame = lp_cfrm[0]->GetFrame(ori, frameNum);
        if (pFrame) {
            //xNow += (pFrame->x2);
            //yNow += (pFrame->y2);
        }
        pFrame = nullptr;
        if (xNow == x && yNow == y)
            return;
        x = xNow;
        y = yNow;
        SetMatrices();
    };
    FRMframeDx* GetFrame() {
        return lp_cfrm[0]->GetFrame(0, 0);
    };
    DWORD GetWidth() {
        FRMframeDx* pFrame = lp_cfrm[0]->GetFrame(0, 0);
        return pFrame->GetWidth();
    };
    DWORD GetHeight() {
        FRMframeDx* pFrame = lp_cfrm[0]->GetFrame(0, 0);
        return pFrame->GetHeight() * numItems;
    };
    LONG GetNumItems() {
        return numItems;
    };
    bool SetSelectItem(LONG itemNum) {
        if (itemNum < 0)
            return false;
        if (itemNum >= numItems)
            return false;
        if (itemNum == selectedItem)
            return true;
        //the highlighted image list num, is the un-highlighted list num - 1.
        delete lp_cfrm[selectedItem];//delete highlighted image
        lp_cfrm[selectedItem] = new FRMCached(pfrmID[selectedItem]);//replace with un-highlighted 
        selectedItem = itemNum;//set new highlighted item
        delete lp_cfrm[selectedItem];//delete un-highlighted image of new selected item
        lp_cfrm[selectedItem] = new FRMCached(pfrmID[selectedItem] - 1);//replace with highlighted image

        return true;
    };
    void Display();

private:
    virtual void SetMatrices() {
        MATRIX_DATA posData = {};
        XMMATRIX xmManipulation;
        XMMATRIX xmScaling;

        xmScaling = XMMatrixScaling(scaleX, scaleY, 1.0f);
        XMMATRIX* pOrtho2D_SCRN = GetScreenProjectionMatrix_XM();

        FRMframeDx* pFrame = lp_cfrm[0]->GetFrame(ori, frameNum);
        float yNow = y;

        for (int i = 0; i < numItems; i++) {
            xmManipulation = XMMatrixTranslation(x, yNow, 0);
            posData.World = XMMatrixMultiply(xmScaling, xmManipulation);
            posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D_SCRN);
            yNow += pFrame->GetHeight();

            UpdatePositionData(pD3DDevContext, i, &posData);
        }
    };
    ID3D11DeviceContext* pD3DDevContext;
    float x;
    float y;
    float z;
    float scaleX;
    float scaleY;
    DWORD ori;
    WORD frameNum;
    LONG numItems;
    LONG selectedItem;
    DWORD* pfrmID;
    FRMCached** lp_cfrm;
    XMFLOAT4X4* pmxWorld;
    XMFLOAT4X4* pmxWorldPrjection;
};


//_________________________
void MouseMenuDX::Display() {

    if (pD3DDevContext == nullptr)
        return;
    if (numItems != numConstantBuffers)
        return;

    FRMframeDx* pFrame = nullptr;
    for (int i = 0; i < numItems; i++) {
        pFrame = lp_cfrm[i]->GetFrame(ori, frameNum);
        if (!pFrame)
            return;

        UINT stride = sizeof(VERTEX_BASE);
        UINT offset = 0;

        //set vertex stuff
        ID3D11Buffer* frame_pd3dVB = nullptr;
        pFrame->GetVertexBuffer(&frame_pd3dVB);// , nullptr);
        pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
        //set texture stuff
        ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
        //set shader constant buffers 
        SetPositionRender(pD3DDevContext, i);
        //set pixel shader stuff
        if (pFrame->GetPixelWidth() == 1) {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
        }
        else {
            pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);
        }

        pD3DDevContext->DrawIndexed(4, 0, 0);
    }
    pFrame = nullptr;
}


class MouseDX : public POSITION_DX {
public:
    MouseDX(DXGI_FORMAT inD3dFormat, DWORD inFrmID) :
        POSITION_DX(1, true),
        x(0),
        y(0),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        ori(0),
        frameNum(0),
        numFrames(0),
        frameTime(0),//num ms between frames
        isAnimated(0),
        lastFrameTime(0),
        menu(nullptr),
        xOffset_Menu(0),
        yOffset_Menu(0),
        xMax_Menu(0),
        yMax_Menu(0),
        isMirrored(false),
        frmID(inFrmID)
    {
        pD3DDevContext = GetD3dDeviceContext();
        SetMatrices();
        cfrm = new FRMCached(frmID);
        FRMdx* frmDX = cfrm->GetFrm();
        numFrames = frmDX->GetNumFrames();
        WORD FPS = frmDX->GetFPS();
        if (FPS)
            frameTime = 1000 / FPS;
        if (numFrames > 1)
            isAnimated = true;
        else
            isAnimated = false;
    };
    ~MouseDX() {
        if (cfrm)
            delete cfrm;
        cfrm = nullptr;
        frmID = 0;
        MouseMenu_Destroy();
        pD3DDevContext = nullptr;
    };
    bool SetFrame(DWORD inFrmID, DWORD in_ori, WORD in_frameNum) {
        if (frmID != inFrmID) {
            frmID = inFrmID;
            if (cfrm)
                delete cfrm;
            cfrm = new FRMCached(frmID);

            lastFrameTime = 0;
            FRMdx* frmDX = cfrm->GetFrm();
            numFrames = frmDX->GetNumFrames();
            WORD FPS = frmDX->GetFPS();
            if (FPS)
                frameTime = 1000 / FPS;


            if (numFrames > 1)
                isAnimated = true;
            else
                isAnimated = false;
            ori = in_ori;
            frameNum = in_frameNum;

            FRMframeDx* pFrame = cfrm->GetFrame(ori, frameNum);
            if (!pFrame) {
                frmID = 0;
                return false;
            }
        }
        return true;
    };
    void SetFRMframeDx(DWORD inFrmID, DWORD in_ori, WORD in_frameNum, bool isCentred) {
        SetFrame(inFrmID, in_ori, in_frameNum);
        MouseMenu_Destroy();
        SetPositionOffsets(isCentred);
    };
    DWORD FrmID() {
        return frmID;
    };
    void UpdateAnimation() {
        if (isAnimated) {
            ULONGLONG count = GetTickCount64();
            if (lastFrameTime == 0)
                lastFrameTime = count;
            else {
                if (count > lastFrameTime + frameTime) {
                    if (frameNum + 1 < numFrames)
                        frameNum += 1;
                    else
                        frameNum = 0;
                    lastFrameTime = count;
                }
            }

        }

    };
    void SetPositionOffsets(bool isCentred) {
        FRMframeDx* pFrame = cfrm->GetFrame(ori, frameNum);
        if (pFrame) {
            if (isCentred) {
                xOffset = -(float)pFrame->GetWidth() / 2;
                yOffset = -(float)pFrame->GetHeight() / 2;
            }
            else {
                if (isMirrored)
                    xOffset = -(float)(pFrame->GetOffset_OriCentre_X()) + pFrame->GetOffset_FromFirstFrame_X();
                else
                    xOffset = (float)(pFrame->GetOffset_OriCentre_X()) + pFrame->GetOffset_FromFirstFrame_X();
                yOffset = (float)(pFrame->GetOffset_OriCentre_Y()) + pFrame->GetOffset_FromFirstFrame_Y();
            }
        }
        SetMatrices();
    };
    void SetPosition(float inX, float inY) {
        if (menu && menu->GetNumItems() > 1)//dont redraw mouse when browsing mouse menu
            return;
        float xNow = inX;
        float yNow = inY;

        if (xNow == x && yNow == y)
            return;
        x = xNow;
        y = yNow;
        SetMatrices();
        MouseMenu_SetPosition();

    };
    void Display();
    bool MouseMenu_Create(DWORD* pfIdLstNum, LONG numItems, LONG xMax, LONG yMax);
    void MouseMenu_Destroy();
    void MouseMenu_SetPosition();
    bool MouseMenu_SetSelectedItem(LONG itemNum);
    bool IsMenu() {
        if (menu)
            return true;
        else
            return false;
    };
private:
    void SetMatrices() {
        MATRIX_DATA posData{};
        XMMATRIX xmManipulation;
        XMMATRIX xmScaling;


        xmManipulation = XMMatrixTranslation(x + xOffset, y + yOffset, z);
        if (isMirrored) {
            XMMATRIX mxRotation;
            mxRotation = XMMatrixRotationY(3.14f);
            xmManipulation = XMMatrixMultiply(mxRotation, xmManipulation);
        }


        xmScaling = XMMatrixScaling(scaleX, scaleY, 1.0f);
        posData.World = XMMatrixMultiply(xmScaling, xmManipulation);
        XMMATRIX* pOrtho2D_SCRN = GetScreenProjectionMatrix_XM();
        posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D_SCRN);
        UpdatePositionData(pD3DDevContext, 0, &posData);
    };
    ID3D11DeviceContext* pD3DDevContext;
    float x;
    float y;
    float z;
    float scaleX;
    float scaleY;
    float xOffset;
    float yOffset;
    DWORD ori;
    WORD frameNum;
    WORD numFrames;
    WORD frameTime;
    bool isAnimated;
    ULONGLONG lastFrameTime;
    DWORD frmID;
    FRMCached* cfrm;
    MouseMenuDX* menu;
    LONG xOffset_Menu;
    LONG yOffset_Menu;
    LONG xMax_Menu;
    LONG yMax_Menu;
    bool isMirrored;
};

MouseDX* mouseDX = nullptr;


//_____________________
void MouseDX::Display() {

    OBJStruct* pObj = ((OBJStruct*)*ppObj_Mouse);
    if (*pIsGameMouse && !(pObj->flags & FLG_Disabled) && isAnimated)
        return;
    if (pD3DDevContext == nullptr)
        return;

    FRMframeDx* pFrame = cfrm->GetFrame(ori, frameNum);

    if (frmID == 0x0600011C && mouseInfoframeDx_Hit != nullptr)
        pFrame = mouseInfoframeDx_Hit->GetFrame(0, 0);

    if (!pFrame)
        return;

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    ID3D11Buffer* frame_pd3dVB = nullptr;
    pFrame->GetVertexBuffer(&frame_pd3dVB);
    pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);
    //set texture stuff
    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
    pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);

    if (pFrame->GetPixelWidth() == 1) {
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
    }
    else
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

    //set shader constant buffers 
    SetPositionRender(pD3DDevContext, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);

    if (menu)
        menu->Display();
}


//___________________________________
void MouseDX::MouseMenu_SetPosition() {
    if (!menu)
        return;
    DWORD fidArrow = fall_GetFrmID(ART_INTRFACE, 0xFA, 0, 0, 0);
    SetFrame(fidArrow, 0, 0);
    FRMframeDx* frameDX_Arrow = cfrm->GetFrame(ori, frameNum);
    DWORD frame_width = frameDX_Arrow->GetWidth();
    xOffset_Menu = frame_width + 2;
    yOffset_Menu = 2;

    LONG yMaxMenu = (LONG)y + menu->GetHeight();
    LONG yMaxMenuDiff = yMaxMenu - yMax_Menu + 2;

    if (x + (frame_width + menu->GetWidth()) >= xMax_Menu) {
        xOffset_Menu = -xOffset_Menu - menu->GetWidth();
        isMirrored = true;
    }
    else
        isMirrored = false;
    SetPositionOffsets(false);

    if (yMaxMenu >= yMax_Menu)
        yOffset_Menu = yOffset_Menu - yMaxMenuDiff;
    menu->SetPosition(x + xOffset_Menu, y + yOffset_Menu);
}


//_______________________________
void MouseDX::MouseMenu_Destroy() {
    if (menu) {
        delete menu;
        isMirrored = false;
    }
    menu = nullptr;
}


//____________________________________________________________________________________
bool MouseDX::MouseMenu_Create(DWORD* pfIdLstNum, LONG numItems, LONG xMax, LONG yMax) {
    if (!pfIdLstNum)
        return false;
    if (numItems <= 0 || numItems >= 10)
        return false;
    if (menu)
        MouseMenu_Destroy();

    xMax_Menu = xMax;
    yMax_Menu = yMax;

    DWORD* pfidMenu = new DWORD[numItems];

    for (LONG i = 0; i < numItems; i++) {
        pfidMenu[i] = fall_GetFrmID(ART_INTRFACE, (pMenu_FRM_LST_NUMS[pfIdLstNum[i]] & 0x0000FFFF), 0, 0, 0);
        if (fall_CheckFrmFileExists(pfidMenu[i]) == 0) {
            delete[] pfidMenu;
            pfidMenu = nullptr;
            return false;
        }
    }

    menu = new MouseMenuDX(pfidMenu, numItems);
    MouseMenu_SetPosition();

    delete[] pfidMenu;
    pfidMenu = nullptr;

    return true;
}


//___________________________________________________
bool MouseDX::MouseMenu_SetSelectedItem(LONG itemNum) {
    if (!menu)
        return false;
    if (!menu->SetSelectItem(itemNum))
        return false;
    return true;
}


//______________________________________________
void MouseDX_SetFrm(DWORD frmID, bool isCentred) {
    if (mouseDX == nullptr)
        mouseDX = new MouseDX(DXGI_FORMAT_B8G8R8A8_UNORM, frmID);

    if (mouseDX != nullptr)
        mouseDX->SetFRMframeDx(frmID, 0, 0, isCentred);

    return;
}


//_____________________________________________________________________________
FRMhead* MouseDX_SetFrm_fall_GetFrm(DWORD frmID, DWORD* frmObj, LONG isCentred) {
    FRMhead* pfrm = fall_GetFrm(frmID, frmObj);
    if (isCentred == 0)
        MouseDX_SetFrm(frmID, false);
    else
        MouseDX_SetFrm(frmID, true);
    return pfrm;
}


//______________________________________
void __declspec(naked) mouseDX_set(void) {

    __asm {
        push esi
        push edi
        push ebp
        push ebx
        push ecx

        push 0
        push edx
        push eax
        call MouseDX_SetFrm_fall_GetFrm
        add esp, 0xC

        pop ecx
        pop ebx
        pop ebp
        pop edi
        pop esi
        ret
    }
}


//inv items
//______________________________________________
void __declspec(naked) mouseDX_set_centred(void) {

    __asm {
        push esi
        push edi
        push ebp
        push ebx
        push ecx

        push 1
        push edx
        push eax
        call MouseDX_SetFrm_fall_GetFrm
        add esp, 0xC

        pop ecx
        pop ebx
        pop ebp
        pop edi
        pop esi
        ret
    }
}


//____________________________________________________________
LONG MouseDX_MenuSingle(DWORD fIdLstNum, LONG xMax, LONG yMax) {
    if (!mouseDX)
        return -1;

    if (!mouseDX->MouseMenu_Create(&fIdLstNum, 1, xMax, yMax))
        return -1;

    return 0;
}


//______________________________________________
void __declspec(naked) mouseDX_menu_single(void) {

    __asm {
        mov eax, dword ptr ss : [esp + 0x04]
        push esi
        push edi
        push ebp

        push eax
        push ecx
        push ebx
        call MouseDX_MenuSingle
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        ret 0x4
    }
}


//____________________________________________________________________________
LONG MouseDX_MenuMulti(DWORD* pfIdLstNum, LONG numItems, LONG xMax, LONG yMax) {
    if (!mouseDX)
        return -1;

    if (!mouseDX->MouseMenu_Create(pfIdLstNum, numItems, xMax, yMax))
        return -1;

    fall_PlayAcm("iaccuxx1");
    return 0;
}


//_____________________________________________
void __declspec(naked) mouseDX_menu_multi(void) {

    __asm {
        mov edx, dword ptr ss : [esp + 0x04]//xMax
        mov eax, dword ptr ss : [esp + 0x08]//yMax
        push esi
        push edi
        push ebp

        push eax
        push edx
        push ecx
        push ebx
        call MouseDX_MenuMulti
        add esp, 0x10

        pop ebp
        pop edi
        pop esi

        ret 0x8
    }
}


//___________________________________
LONG MouseDX_MenuSelect(LONG itemNum) {
    if (!mouseDX)
        return -1;
    if (!mouseDX->MouseMenu_SetSelectedItem(itemNum))
        return -1;

    return 0;
}


//_______________________________________________________
void __declspec(naked) mouseDX_menu_update_selected(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push eax
        call MouseDX_MenuSelect
        add esp, 0x4

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//_____________________
LONG MouseDX_MenuExit() {
    if (!mouseDX)
        return 0;
    mouseDX->MouseMenu_Destroy();
    return 0;
}


//__________________________________________
void __declspec(naked) mouse_menu_exit(void) {

    __asm {
        pushad
        call MouseDX_MenuExit
        popad
        mov eax, 0x0
        ret 0xC
    }
}


//_________________
void MouseDX_Show() {
    if (*pIsMouseAquired && mouseDX) {
        mouseDX->SetPosition((float)*pMouseX + *pMousePicXOffset, (float)*pMouseY + *pMousePicYOffset);
        mouseDX->UpdateAnimation();
        *pIsMouseHidden = 0;

        Dx_Present_Main();
    }
}


//______________________________________________
void __declspec(naked) mouseDX_update_show(void) {

    __asm {
        pushad
        call MouseDX_Show
        popad
        ret
    }
}


//_________________________________________________
void __declspec(naked) mouseDX_set_mouse_buff(void) {

    __asm {
        pushad

        mov eax, pIsMouseHidden
        cmp dword ptr ds : [eax] , 0
        jne dontShow

        call MouseDX_Show

        dontShow :
        popad
        xor eax, eax
        ret 0xC
    }
}


//__________________________________________
void MouseDX_Inv_SetFrmFromLst(DWORD lstNum) {
    DWORD frmID = fall_GetFrmID(6, pINV_FRM_LST_NUMS[lstNum], 0, 0, 0);
    if (mouseDX == nullptr)
        mouseDX = new MouseDX(DXGI_FORMAT_B8G8R8A8_UNORM, frmID);
    if (mouseDX != nullptr)
        mouseDX->SetFRMframeDx(frmID, 0, 0, false);

    return;
}


//_______________________________________________________
void __declspec(naked) mouseDX_inv_set_frm_from_lst(void) {

    __asm {
        pushad

        push eax
        call MouseDX_Inv_SetFrmFromLst
        add esp, 0x4

        popad

        mov eax, pIsMouseHidden
        cmp dword ptr ds : [eax] , 0
        jne dontShow

        call MouseDX_Show

        dontShow :
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________________
void MouseDx_UpdateAnimation() {
    if (mouseDX != nullptr)
        mouseDX->UpdateAnimation();
}


//____________________
void MouseDx_Display() {
    if (mouseDX && !IsMouseHidden())// && isMainPalActive)
        mouseDX->Display();
}


//____________________
void MouseDx_Destroy() {
    if (mouseDX)
        delete mouseDX;
    mouseDX = nullptr;
}


//________________________
bool MouseDx_IsScrolling() {
    if (mouseDX) {
        DWORD frmID = mouseDX->FrmID();
        if ((frmID & 0x00000FFF) >= 0x10E && (frmID & 0x00000FFF) <= 0x115)
            return true;
    }
    return false;
}


//_____________________________________________________________
void SetGameMouseFID(OBJStruct* pObj, DWORD frmID, RECT* rcOut) {
    // 0x060000F9 == MSEF003.FRM    ; Action move  hex
    // 0x0600010A == BLANK.FRM    ; Mouse cursor - none ( 1 x 1 transparent)
    if (!mouseDX)
        return;
    //game mouse already set and mouseDX frmID already set to blank
    if (frmID == 0x060000F9 && mouseDX->FrmID() == 0x0600010A)
        return;

    //mouseDX frmID already set and no menu to close
    if (mouseDX->FrmID() == frmID && !mouseDX->IsMenu())
        return;

    if (frmID == 0x060000F9) {//if change to hex mouse set gui mouse to blank
        MouseDX_SetFrm(0x0600010A, false);
        fall_Obj_GetRect(pObj, rcOut);
    }
    else {// set gui to new frm
        MouseDX_SetFrm(frmID, false);
        fall_Obj_GetRect(pObj, rcOut);
    }
}


//_____________________________________________
void __declspec(naked) set_game_mouse_fid(void) {

    __asm {
        pushad

        push ebx//*outRect
        push edx//FID
        push eax//*obj
        call SetGameMouseFID
        add esp, 0xC

        popad
        xor eax, eax
        ret
    }
}


//___________________________
void Modifications_Dx_Mouse() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        //To-Do Modifications_Dx_Mouse Chinese
        pfrmObjRef_Mouse = (LONG*)FixAddress(0x528A00);

        FuncReplace32(0x44B1EB, 0x03EC4D, (DWORD)&set_game_mouse_fid);
        FuncReplace32(0x44B39C, 0x03EA9C, (DWORD)&set_game_mouse_fid);
        FuncReplace32(0x44BD43, 0x03E0F5, (DWORD)&set_game_mouse_fid);
        FuncReplace32(0x44C1D4, 0x03DC64, (DWORD)&set_game_mouse_fid);
        FuncReplace32(0x44D67B, 0x03C7BD, (DWORD)&set_game_mouse_fid);
        FuncReplace32(0x44D6C5, 0x03C773, (DWORD)&set_game_mouse_fid);
    }
    else {
        //list of 6 intrface.lst numbers at address 0x5190FC
        pINV_FRM_LST_NUMS = (DWORD*)FixAddress(0x5190FC);

        pINV_MOUSE_FRM_LST_NUM_CURRENT = (DWORD*)FixAddress(0x059E940);

        //mouse_menu_fid_list
        pMenu_FRM_LST_NUMS = (WORD*)0x518D1E;

        pIsMouseAquired = (LONG*)FixAddress(0x6AC7BC);
        pIsMouseHidden = (LONG*)FixAddress(0x6AC790);
        pMouseX = (LONG*)FixAddress(0x6AC7A8);
        pMouseY = (LONG*)FixAddress(0x6AC7A4);
        pMousePicXOffset = (LONG*)FixAddress(0x6AC7D0);
        pMousePicYOffset = (LONG*)FixAddress(0x6AC7CC);

        pfrmObjRef_Mouse = (LONG*)FixAddress(0x518C10);


        //in function SET_MOUSE_PIC(EAX mousePicNum) addr - 0x44C840
        FuncReplace32(0x44C893, 0xFFFCC8C9, (DWORD)&mouseDX_set);

        //inv items
        FuncReplace32(0x471061, 0xFFFA80FB, (DWORD)&mouseDX_set_centred);
        FuncReplace32(0x474897, 0xFFFA48C5, (DWORD)&mouseDX_set_centred);
        FuncReplace32(0x474EB8, 0xFFFA42A4, (DWORD)&mouseDX_set_centred);
        FuncReplace32(0x475183, 0xFFFA3FD9, (DWORD)&mouseDX_set_centred);

        MemWrite8(0x470BF2, 0x89, 0xE9);
        FuncWrite32(0x470BF3, 0x03E0C1C2, (DWORD)&mouseDX_inv_set_frm_from_lst);

        MemWrite16(0x44CFA0, 0x5756, 0xE990);
        FuncWrite32(0x44CFA2, 0x34EC8355, (DWORD)&mouseDX_menu_single);


        MemWrite8(0x44D214, 0x56, 0xE9);
        FuncWrite32(0x44D215, 0xEC815557, (DWORD)&mouseDX_menu_multi);
        MemWrite32(0x44D219, 0x90, 0x90909090);

        MemWrite8(0x44D630, 0x53, 0xE9);
        FuncWrite32(0x44D631, 0x57565251, (DWORD)&mouseDX_menu_update_selected);


        //don't create inv mouse menu button - not needed - causes mouse to be drawn to inv
        MemWrite8(0x47331D, 0xE8, 0x90);
        MemWrite32(0x47331E, 0x064F3E, 0x9024C483);
        //don't destroy un-created button
        MemWrite8(0x4733C4, 0xE8, 0x90);
        MemWrite32(0x4733C5, 0x065EF3, 0x90909090);

        //don't draw button area on inv window
        MemWrite8(0x47332F, 0xE8, 0x90);
        MemWrite32(0x473330, 0x063C4C, 0x90909090);
        MemWrite8(0x4733B4, 0xE8, 0x90);
        MemWrite32(0x4733B5, 0x063BC7, 0x90909090);

        //don't draw mouse menu changes to inv window
        MemWrite8(0x47344E, 0xE8, 0x90);
        MemWrite32(0x47344F, 0x060281, 0x90909090);
        MemWrite8(0x4734CE, 0xE8, 0x90);
        MemWrite32(0x4734CF, 0x060201, 0x90909090);

        //destroy single mouse menu when move beyond item
        FuncReplace32(0x470D00, 0x0593A8, (DWORD)&mouse_menu_exit);
        FuncReplace32(0x470D4B, 0x05935D, (DWORD)&mouse_menu_exit);


        MemWrite8(0x4CA34C, 0x53, 0xE9);
        FuncWrite32(0x4CA34D, 0x57565251, (DWORD)&mouseDX_update_show);

        MemWrite16(0x4CA0AC, 0x5756, 0xE990);
        FuncWrite32(0x4CA0AE, 0x18EC8355, (DWORD)&mouseDX_set_mouse_buff);


        //don't draw single mouse menu here
        //ACTPICK.FRM    ; Action pick default
        MemWrite8(0x44BA9A, 0xE8, 0x90);
        MemWrite32(0x44BA9B, 0x0003EF9D, 0x9090C031);
        //don't need to redraw scene when setting interface mouse
        MemWrite8(0x44BAB2, 0xE8, 0x90);
        MemWrite32(0x44BAB3, 0x065809, 0x90909090);

        //don't draw multi mouse menu here
        //ACTMENU.FRM    ; Action menu
        MemWrite8(0x44C5F2, 0xE8, 0x90);
        MemWrite32(0x44C5F3, 0x0003E445, 0x9090C031);
        //don't need to redraw scene when setting interface mouse
        MemWrite8(0x44C627, 0xE8, 0x90);
        MemWrite32(0x44C628, 0x064C94, 0x90909090);


        //ACTTOHIT.FRM   ; Action to hit
        FuncReplace32(0x44BC4C, 0x03EDEC, (DWORD)&set_game_mouse_fid);
        //don't need to redraw scene when setting interface mouse
        MemWrite8(0x44BC61, 0xE8, 0x90);
        MemWrite32(0x44BC62, 0x06565A, 0x90909090);

        //from mouse ID list @ 0x518D3C
        FuncReplace32(0x44CA84, 0x03DFB4, (DWORD)&set_game_mouse_fid);
        //don't need to redraw scene when setting interface mouse
        MemWrite8(0x44CB46, 0xE8, 0x90);
        MemWrite32(0x44CB47, 0x064775, 0x90909090);

        //not called ?
        FuncReplace32(0x44DF2B, 0x03CB0D, (DWORD)&set_game_mouse_fid);

        //from mouse ID list @ 0x518D3C
        FuncReplace32(0x44DF75, 0x03CAC3, (DWORD)&set_game_mouse_fid);

        //don't need to redraw scene when setting interface mouse
        MemWrite8(0x44BC89, 0xE8, 0x90);
        MemWrite32(0x44BC8A, 0x065632, 0x90909090);
        MemWrite8(0x44C6DD, 0xE8, 0x90);
        MemWrite32(0x44C6DE, 0x064BDE, 0x90909090);
        MemWrite8(0x44CE0F, 0xE8, 0x90);
        MemWrite32(0x44CE10, 0x0644AC, 0x90909090);
    }
}
