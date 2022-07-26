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
#include "Dx_RenderTarget.h"
#include "Dx_Graphics.h"

#include "Fall_Windows.h"
#include "win_fall.h"
#include "text.h"

#define FLG_BUTT_UP         0x00000001
#define FLG_BUTT_DN         0x00000002
#define FLG_BUTT_HV         0x00000004
#define FLG_BUTT_UP_DIS     0x00000008
#define FLG_BUTT_DN_DIS     0x00000010
#define FLG_BUTT_HV_DIS     0x00000020


#define FLG_BUTT_OVERLAY_ENABLED     0x00000001
//#define FLG_BUTT_OVERLAY_CLIPPED     0x00000002
//#define FLG_BUTT_OVERLAY_CENTRED     0x00000004
/*
enum BUTTON_STATES : LONG{
    up,
    down,
    hover,
    disabled
};
*/

class Window_DX;


class Button_DX : public BASE_VERTEX_DX, public BASE_POSITION_DX, public BASE_TEXTURE_STAGING {
public:
    Button_DX(Window_DX* p_winDxParent, float inX, float inY, DWORD width, DWORD height, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, bool inHasOverlay);
    ~Button_DX();
    bool Set_Disabled_Button_Frms(DWORD frmID_UP_Dis, DWORD frmID_Dn_Dis, DWORD frmID_Hv_Dis);
    void SetCurrentFrm(DWORD flags);
    BYTE* SetPointerMask(DWORD frmID, RECT* pRect);
    void Display();

    DirectX::XMFLOAT4 GetButtonColour() {
        return *p_buttonColour;
    };
    bool Overlay_CreateTexture(float in_offset_x, float in_offset_y, float in_down_offset_x, float in_down_offset_y, DWORD in_width, DWORD in_height, bool is32bit, bool isRenderTarget, bool hasStagingTexture);
    bool Overlay_CreateTexture(bool is32bit, bool isRenderTarget, bool hasStagingTexture);//Assume dimensions match that of the button.
    bool Overlay_CreateTexture(float in_offset_x, float in_offset_y, float in_down_offset_x, float in_down_offset_y);//Assume width and height match that of the button, assume this is a 32bit render target wiyh no staging.

    bool Overlay_SetFrm(DWORD frmID_Item, int ori, int frameNum, float inX, float inY, DWORD maxWidth, DWORD maxHeight);//set maxWidth and maxHeight to zero to ignore scaling.
    bool Overlay_SetFrm(DWORD frmID_Item, float inX, float inY, DWORD maxWidth, DWORD maxHeight);//Assume orientation and frame number are zero, set maxWidth and maxHeight to zero to ignore scaling.
    void SetOverlayRenderTarget() {
        SetRenderTargetAndViewPort(nullptr);
    };
    void OverLay_SetPixelShader(ID3D11PixelShader* in_pd3d_PS_shader, void (*in_pOverlayShader_Setup)(Button_DX* butt)) {
        pd3d_PS_OverlayShader = in_pd3d_PS_shader;
        pOverlayShader_Setup = in_pOverlayShader_Setup;
    };
    bool OverLay_SetAsColouredButton(DWORD flags) {
        if (pixelWidth != 1)
            return false;
        buttonColour_flags = flags;
        pd3d_PS_OverlayShader = pd3d_PS_Colour_32_Brightness_ZeroMasked;
        return true;
    };
    void OverLay_SetColours(DirectX::XMFLOAT4* up, DirectX::XMFLOAT4* down, DirectX::XMFLOAT4* hover, DirectX::XMFLOAT4* disabled) {
        if (up)
            buttonColour[0] = *up;
        if (down)
            buttonColour[1] = *down;
        if (hover)
            buttonColour[2] = *hover;
        if (disabled)
            buttonColour[3] = *disabled;
    };
    //Set/Un-set overlay to be clipped at button dimensions.
    void OverLay_SetClippingRect(bool set);

    bool OverLay_Frm_SetFrameNum(int frameNum) {
        if (cfrm_Overlay && cfrm_Overlay->GetFrame(overlay_frm_ori, frameNum)) {
            overlay_frm_frameNum = frameNum;
            return true;
        }
        return false;
    };
    bool OverLay_Frm_SetOri(int ori) {
        if (cfrm_Overlay && cfrm_Overlay->GetFrame(ori, overlay_frm_frameNum)) {
            overlay_frm_ori = ori;
            return true;
        }
        return false;
    };
    bool GetWidthHeight(DWORD* pWidth, DWORD* pHeight) {
        if (!pWidth)
            return false;
        if (!pHeight)
            return false;
        if (!cfrm_Up)
            return false;
        FRMframeDx* frameDx = cfrm_Up->GetFrame(0, 0);
        if (!frameDx)
            return false;
        *pWidth = frameDx->GetWidth();
        *pHeight = frameDx->GetHeight();
        return true;
    }
protected:
    virtual void SetMatrices();
private:
    DirectX::XMFLOAT4X4* pOrtho2D_Win;
    FRMCached* cfrm_Up;
    FRMCached* cfrm_Dn;
    FRMCached* cfrm_Hv;
    FRMCached* cfrm_Up_Dis;
    FRMCached* cfrm_Dn_Dis;
    FRMCached* cfrm_Hv_Dis;
    FRMCached* cfrm_Current;
    BYTE* pointerMask;//To-Do button pointer mask - find a better way to do this, needs to be scalable.

    DWORD overlay_Flags;
    float overlay_down_offset_x;
    float overlay_down_offset_y;
    float overlay_tex_x;
    float overlay_tex_y;
    float overlay_frm_x;
    float overlay_frm_y;
    float overlay_frm_scale_x;
    float overlay_frm_scale_y;
    int overlay_frm_ori;
    int overlay_frm_frameNum;
    D3D11_RECT* p_rc_overLay_clip;
    D3D11_RECT* p_rc_parentWindow_clip;
    FRMCached* cfrm_Overlay;

    ID3D11PixelShader* pd3d_PS_OverlayShader;
    void (*pOverlayShader_Setup)(Button_DX* butt);

    DirectX::XMFLOAT4 buttonColour[4];
    DirectX::XMFLOAT4* p_buttonColour;
    DWORD buttonColour_flags;
};


//Modified fallout button structure, additional pointer to Button_DX class.
struct ButtonStruct_DX {
	  LONG ref;                     //0x00 // ref number
	  DWORD flags;                  //0x04 // flags - see Fall_General.h
	  RECT rect;                    //left 0x08, top 0x0C, right 0x10, bottom 0x14
	  LONG refHvOn;                 //0x18  mouse hover over
	  LONG refHvOff;                //0x1C  mouse hover off
	  LONG refDn;                   //0x20  left mouse down
	  LONG refUp;                   //0x24  left mouse up
	  LONG refDnRht;                //0x28  right mouse down
      LONG refUpRht;                //0x2C  right mouse up
      BYTE *buffUp;                 //0x30  buffer - button up
      BYTE *buffDn;                 //0x34  buffer - button down
	  BYTE *buffHv;                 //0x38  buffer - button hover
	  BYTE *buffUpDis;              //0x3C  buffer - button up Disabled
	  BYTE *buffDnDis;              //0x40  buffer - button down Disabled
	  BYTE *buffHvDis;              //0x44  buffer - button hover Disabled
	  BYTE *buffCurrent;            //0x48  buffer - button current
	  BYTE *buffPointerMask;        //0x4C  buffer - button default // mask buffer check if pixel under mouse
	  void (*funcHvOn)();           //0x50  function pointer for mouse hover over
	  void (*funcHvOff)();          //0x54  function pointer for mouse hover off
	  void (*funcDn)();             //0x58  function pointer for left mouse down
	  void (*funcUp)();             //0x5C  function pointer for left mouse up
	  void (*funcDnRht)();          //0x60  function pointer for right mouse down
	  void (*funcUpRht)();          //0x64  function pointer for right mouse up
	  void (*funcDnSnd)();          //0x68  function pointer for push sound effect
	  void (*funcUpSnd)();          //0x6C  function pointer for lift sound effect
	  DWORD unknown70;              //0x70  don't know - a pointer to something //related to toggle array
	  ButtonStruct_DX *nextButton;  //0x74  pointer to next button in list
	  ButtonStruct_DX *prevButton;  //0x78  pointer to previous button in list
	  Button_DX *buttDx;            //0x7C   //struct size 0x80
};



struct WinStructDx;


class Window_DX : public BASE_VERTEX_DX, public BASE_TEXTURE_DX, public BASE_POSITION_DX {
public:
    Window_DX(float inX, float inY, DWORD inWidth, DWORD inHeight, DWORD in_bgColour, Window_DX* in_parent, int* pRetSubWinNum) :
        BASE_VERTEX_DX(),
        BASE_TEXTURE_DX(in_bgColour),
        BASE_POSITION_DX(inX, inY, 1, false),
        winNum(0),
        p_fall_parent(nullptr),
        parent(in_parent),
        draw_flag(true),
        clip_at_parent(false),
        pRect_clip(nullptr),
        pRect_clip_pre_scale(nullptr),
        winPrev(nullptr),
        winNext(nullptr),
        winFirst(nullptr),
        pOnScreenResize(nullptr),
        pOnDisplay_Pre(nullptr),
        pOnDisplay_Post(nullptr),
        pOnDisplay_Instead(nullptr)
    {
        mxWorldPrjection = {};
        SetBaseDimensions(inWidth, inHeight);
        if (!CreateVerticies())
            Fallout_Debug_Error("Window_DX - CreateVerticies Failed.");
        if (!Texture_Initialize(DXGI_FORMAT_B8G8R8A8_UNORM, true, false))
            Fallout_Debug_Error("Window_DX - CreateTexture Failed.");
        SetMatrices();
        if (parent) {//if this is a sub window
            winPrev = parent->GetLastSubWin();
            winPrev->winNext = this;
            winNum = GetLastWinSubNum() + 1;
            if (winNum == 0)
                Fallout_Debug_Error("Window_DX - Invalid window ref number in sub window creation");
        }
        else
            winFirst = this;//if window has no parent set first in sub window list

        if (pRetSubWinNum)
            *pRetSubWinNum = winNum;
    };
    ~Window_DX() {
        if (!parent)//delete all sub windows when deleting parent
            DeleteSubWindows();
        else {//remove this window from the list if this is a sub window.
            if (winPrev)
                winPrev->winNext = winNext;//point to the sub after parent.
            if (winNext)
                winNext->winPrev = winPrev;//point to the sub before parent.
            winPrev = nullptr;
            winNext = nullptr;
            //Fallout_Debug_Info("~Window_DX %d", winNum);
        }

        if (pRect_clip)
            delete pRect_clip;
        pRect_clip = nullptr;
        if (pRect_clip_pre_scale)
            delete pRect_clip_pre_scale;
        pRect_clip_pre_scale = nullptr;

    };
    void Set_FalloutParentWindow(WinStructDx* in_fall_parent) {
        if (!parent) {
            p_fall_parent = in_fall_parent;
            return;
        }
        parent->Set_FalloutParentWindow(in_fall_parent);
    };
    WinStructDx* Get_FalloutParent() {
        if (!parent)
            return p_fall_parent;
        return parent->Get_FalloutParent();
    }
    void Set_OnScreenResizeFunction(void (*pIN_OnScreenResize)(Window_DX*)) {
        if (parent)//this is for parent windows only
            return;
        pOnScreenResize = pIN_OnScreenResize;
    };
    bool Run_OnScreenResizeFunction() {
        if (!pOnScreenResize)
            return false;
        else
            pOnScreenResize(this);
        return true;
    };
    void Set_OnDisplayFunctions(void (*pIN_OnDisplay_Pre)(Window_DX*), void (*pIN_OnDisplay_Post)(Window_DX*), void (*pIN_OnDisplay_Instead)(Window_DX*)) {
        pOnDisplay_Pre = pIN_OnDisplay_Pre;
        pOnDisplay_Post = pIN_OnDisplay_Post;
        pOnDisplay_Instead = pIN_OnDisplay_Instead;
    };
    void Display();

    void Clear(DWORD colour);
    void Clear() {
        Clear(bg_Colour);
    };

    void SetDrawFlag(bool flag) {
        draw_flag = flag;
    }
    //Set the sub window clipping rect if required, if (in_rect = nullptr) window will be clipped at parent window rect. Dimensions are relative to parent window.
    void SetClippingRect(D3D11_RECT* in_rect) {
        if (parent) {//only for sub windows - parent rect always its window dimensions

            D3D11_RECT* parentRect = parent->GetClippingRect();
            if (!in_rect)
                clip_at_parent = true;
            else {
                if (!pRect_clip)
                    pRect_clip = new D3D11_RECT{ 0,0,0,0 };
                if (!pRect_clip_pre_scale)
                    pRect_clip_pre_scale = new D3D11_RECT{ in_rect->left,in_rect->top,in_rect->right,in_rect->bottom };
                else
                    *pRect_clip_pre_scale = { in_rect->left, in_rect->top,in_rect->right,in_rect->bottom };

                pRect_clip->left = (LONG)pRect_clip_pre_scale->left * scaleLevel_GUI + parentRect->left;
                pRect_clip->top = (LONG)pRect_clip_pre_scale->top * scaleLevel_GUI + parentRect->top;
                pRect_clip->right = pRect_clip_pre_scale->right * scaleLevel_GUI + parentRect->left;
                pRect_clip->bottom = pRect_clip_pre_scale->bottom * scaleLevel_GUI + parentRect->top;
            }

        }
    }
    D3D11_RECT* GetClippingRect() {
        return pRect_clip;
    }
    int GetSubWinNum() {
        return winNum;
    };
    Window_DX* GetSubWin(int num) {
        if (parent != nullptr) {
            if (parent->winFirst)
                return parent->winFirst->GetSubWin_Internal(num);
            return nullptr;
        }
        if (winFirst)
            return winFirst->GetSubWin_Internal(num);
        return nullptr;
    }
    Window_DX* GetLastSubWin() {
        if (winNext)
            return winNext->GetLastSubWin();
        else
            return this;
    }
    Window_DX* GetFirstSubWin() {
        if (parent)
            return parent->winFirst;
        return winFirst;
    }
    //get the sub window number of the last sub window created.
    int GetLastWinSubNum() {
        if (parent) {
            if (parent->winFirst)
                return parent->winFirst->GetLastWinSubNum_Internal(parent->winFirst->winNum);
            return -1;
        }
        if (winFirst)
            return winFirst->GetLastWinSubNum_Internal(winFirst->winNum);
        return -1;
    }
    bool DeleteSubWin(int num) {
        if (parent != nullptr) {
            //Fallout_Debug_Error("DeleteSubWin can't be called from a sub window");
            return false;
        }
        if (num <= 0)//window numbers can't be negative, dont delete the parent window "0";
            return false;
        Window_DX* theSubToDelete = GetSubWin(num);

        if (theSubToDelete) {
            delete theSubToDelete;
            return true;
        }
        return false;
    }
    BOOL ArrangeWin(BOOL toBack) {
        Window_DX* nextSub = this->winNext;
        Window_DX* prevSub = this->winPrev;
        if (toBack == TRUE) {
            Window_DX* pCurrentFirstWin = GetFirstSubWin();
            if (pCurrentFirstWin == this)//already at the back
                return TRUE;
            if (pCurrentFirstWin) {
                if (prevSub)
                    prevSub->winNext = nextSub;//point to the sub after this sub.
                if (nextSub)
                    nextSub->winPrev = prevSub;//point to the sub before this sub.
                this->winPrev = nullptr;
                this->winNext = pCurrentFirstWin;
                pCurrentFirstWin->winPrev = this;

                if (parent)
                    parent->winFirst = this;
                else
                    winFirst = this;

                return TRUE;
            }
        }
        else if (toBack == FALSE) {//move to front;
            Window_DX* pCurrentLastWin = GetLastSubWin();
            if (pCurrentLastWin == this)//already at front
                return TRUE;
            if (pCurrentLastWin) {
                if (prevSub)
                    prevSub->winNext = nextSub;//point to the sub after this sub.
                if (nextSub)
                    nextSub->winPrev = prevSub;//point to the sub before this sub.
                this->winPrev = pCurrentLastWin;
                this->winNext = nullptr;
                pCurrentLastWin->winNext = this;

                if (!prevSub && nextSub) {//if no prevSub than this is the first, set nextSub to the new first
                    if (parent)
                        parent->winFirst = nextSub;
                    else
                        winFirst = nextSub;
                }
                return TRUE;
            }
        }
        else if (toBack == 2) {//move just behind parent
            if (parent && nextSub != parent) {//if this is not the parent and this is not already just behind parent.
                if (this == GetFirstSubWin())
                    parent->winFirst = nextSub;
                if (prevSub)
                    prevSub->winNext = nextSub;//point to the sub after this sub.
                if (nextSub)
                    nextSub->winPrev = prevSub;//point to the sub before this sub.

                this->winPrev = parent->winPrev;
                this->winNext = parent;

                if (parent->winPrev)//if parent already has a window behind it, set its next window to this.
                    parent->winPrev->winNext = this;
                else//else if parent is the first window set this to the first window
                    parent->winFirst = this;
                parent->winPrev = this;// finally set this as the parents previous window.


            }
            return TRUE;
        }
        return FALSE;
    }
    DirectX::XMFLOAT4X4* GetProjectionMatrix() {
        return &mxWorldPrjection;
    };
    void SetOpaqueness(float inOpaqueness) {
        opaqueness = inOpaqueness;
    };
    void Draw(BYTE* fBuff, DWORD* fpal, bool is_fBuff_32bit, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY);
    void Draw_Text(const char* txt, LONG xPos, LONG yPos, DWORD colour, DWORD colourBG, TextEffects effects);
    LONG Draw_Text_Formated(const char* txt, RECT* pMargins, DWORD colour, DWORD colourBG, DWORD textFlags, TextEffects effects);//The height of the drawn text is returned to pMargins->height, setting dest_buff no NULL will allow you to get the height without drawing anything.
    void RenderTargetDrawFrame(float xPos, float yPos, FRMframeDx* pFrame, ID3D11PixelShader* pd3d_PixelShader, RECT* pScissorRect);
    void ClearRect(RECT* pRect);

    void ResizeWindow(DWORD inWidth, DWORD inHeight) {
        DestroyVerticies();
        DestroyTexture();
        SetBaseDimensions(inWidth, inHeight);
        if (!CreateVerticies())
            Fallout_Debug_Error("Window_DX::ResizeWindow - CreateVerticies Failed .");
        if (!CreateTexture())
            Fallout_Debug_Error("Window_DX::ResizeWindow - CreateTexture Failed.");
        SetMatrices();
    };
protected:
    virtual void SetMatrices() {
        MATRIX_DATA posData{};
        DirectX::XMMATRIX xmManipulation{};
        DirectX::XMMATRIX xmScaling{};

        xmManipulation = DirectX::XMMatrixTranslation(x, y, 0);

        DirectX::XMMATRIX* pOrtho2D;
        DirectX::XMMATRIX Ortho2D_PARENT{};
        if (parent != nullptr) {//if a sub window, get parent 
            Ortho2D_PARENT = XMLoadFloat4x4(parent->GetProjectionMatrix());
            pOrtho2D = &Ortho2D_PARENT;
        }
        else//if a parent win, get screen ortho matrix
            pOrtho2D = GetScreenProjectionMatrix_XM();


        xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);
        DirectX::XMStoreFloat4x4(&mxWorldPrjection, posData.WorldViewProjection);
        UpdatePositionData(0, &posData);

        if (parent == nullptr) {//if this is the parent window being ajusted then adjust its sub windows to match

            if (!pRect_clip)//always setup the parent clipping rect
                pRect_clip = new D3D11_RECT{ 0,0,0,0 };
            pRect_clip->left = (LONG)x * scaleLevel_GUI;
            pRect_clip->top = (LONG)y * scaleLevel_GUI;
            pRect_clip->right = pRect_clip->left + (LONG)width * scaleLevel_GUI;
            pRect_clip->bottom = pRect_clip->top + (LONG)height * scaleLevel_GUI;

            Window_DX* winSub = nullptr;
            if (winPrev && winFirst != this) {//if the parent window is not the first window, temporarily remove the parent window from the list or this loop will go on forever. 
                winPrev->winNext = winNext;//in the previous window in the list - point winNext to the sub after parent.
                winSub = winFirst;
            }
            else
                winSub = winNext;

            while (winSub) {
                winSub->SetMatrices();
                winSub = winSub->winNext;
            }

            if (winPrev && winFirst != this) {//if the parent window is not the first window, re-insert the parent window into the list. 
                winPrev->winNext = this;//in the previous window in the list - point winNext back to the parent.
            }
        }
        else {
            if (pRect_clip) {
                D3D11_RECT* parentRect = parent->GetClippingRect();
                if (pRect_clip_pre_scale) {
                    pRect_clip->left = (LONG)pRect_clip_pre_scale->left * scaleLevel_GUI + parentRect->left;
                    pRect_clip->top = (LONG)pRect_clip_pre_scale->top * scaleLevel_GUI + parentRect->top;
                    pRect_clip->right = pRect_clip_pre_scale->right * scaleLevel_GUI + parentRect->left;
                    pRect_clip->bottom = pRect_clip_pre_scale->bottom * scaleLevel_GUI + parentRect->top;
                }
            }
        }
    };
    void DisplayWindow();
private:
    float opaqueness;
    DirectX::XMFLOAT4X4 mxWorldPrjection;
    int winNum;
    bool draw_flag;//set false to prevent old window draw function from drawing to tex;
    bool clip_at_parent;

    D3D11_RECT* pRect_clip;
    D3D11_RECT* pRect_clip_pre_scale;

    WinStructDx* p_fall_parent;

    Window_DX* parent;
    Window_DX* winFirst;
    Window_DX* winPrev;
    Window_DX* winNext;

    void (*pOnScreenResize)(Window_DX*);

    void (*pOnDisplay_Pre)(Window_DX*);
    void (*pOnDisplay_Post)(Window_DX*);
    void (*pOnDisplay_Instead)(Window_DX*);

    void Draw8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD toWidth);
    void Draw32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, DWORD* toBuff, LONG tX, LONG tY, DWORD toWidth);
    void DrawPal32(BYTE* fBuff, DWORD* pal, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, DWORD* toBuff, LONG tX, LONG tY, DWORD toWidth);

    Window_DX* GetSubWin_Internal(int num) {
        if (num < 0)
            return nullptr;
        else if (num == winNum)
            return this;
        else if (winNext != nullptr)
            return winNext->GetSubWin_Internal(num);
        else
            return nullptr;
    }
    int GetLastWinSubNum_Internal(int lastNum) {
        if (winNum > lastNum)
            lastNum = winNum;
        if (winNext)
            return winNext->GetLastWinSubNum_Internal(lastNum);
        else
            return lastNum;
    }
    void DeleteSubWindows() {
        if (parent != nullptr) {
            //Fallout_Debug_Error("DeleteSubWindows can't be called from a sub window");
            return;
        }
        if (!winPrev) //if parent is at the top of the list, move winFirst to the next sub.
            winFirst = winNext;
        else {//remove the parent window from the sub list.
            winPrev->winNext = winNext;//point to the sub after parent.
            if (winNext)
                winNext->winPrev = winPrev;//point to the sub before parent.
        }
        winPrev = nullptr;
        winNext = nullptr;

        int count = 0;
        Window_DX* nextSub = nullptr;
        while (winFirst) {//delete the sub list
            nextSub = winFirst->winNext;
            delete winFirst;
            winFirst = nextSub;
            count++;
        }
        winFirst = nullptr;
        nextSub = nullptr;
        //Fallout_Debug_Info("DeleteSubWindows %d", count);
        return;
    }

};


//Modified fallout window structure, additional pointer to Window_DX class.
struct WinStructDx {
	  LONG ref;                     //0x00
	  DWORD flags;                  //0x04
	  RECT rect;                    //0x08 window rect
	  DWORD width;                  //0x18
      DWORD height;                 //0x1C
      DWORD colour;                 //0x20 colour index offset
	  DWORD unknown24;              //0x24 x random?
	  DWORD unknown28;              //0x28 y random?
	  BYTE *buff;                   //0x2C byte buffer, palette offsets
	  ButtonStruct_DX *ButtonList;  //0x30 button struct list
	  ButtonStruct_DX *Button34;    //0x34
	  DWORD unknown38;              //0x38
	  DWORD unknown3C;              //0x3C
	  void (*pBlit)(BYTE *fBuff, LONG subWidth, LONG subHeight, LONG fWidth, BYTE *LONG, DWORD tWidth);//0x40 drawing func address
      Window_DX *winDx;             //0x44
};


void Display_Buttons(WinStructDx* p_win);

void SetDxWin(WinStruct* p_win);

void MoveWindowX(WinStructDx* p_win, LONG x, LONG y);
void ResizeWindowX(WinStructDx* p_win, LONG x, LONG y, DWORD width, DWORD height);

void OnScreenResize_Windows(LONG prevScreenWidth, LONG prevScreenHeight);

bool ScaleWindowToScreen(Window_DX* subwin, LONG scaleType, float* pPoint_W, float* pPoint_H, RECT* pRectOnScreen);

LONG CreateButtonX(LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, DWORD flags);
LONG CreateButtonX_Overlay(LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, DWORD flags);
LONG SetButtonSoundsX(LONG buttRef, void (*funcDnSnd)(), void (*funcUpSnd)());
LONG SetButtonPointerMaskX(LONG buttRef, DWORD frmID);
LONG SetButtonRightClickX(LONG buttRef, LONG dnCode, LONG upCode, void (*funcDn)(), void (*funcUp)());
LONG SetButtonDisabledX(LONG buttRef);
LONG SetButtonEnabledX(LONG buttRef);
LONG SetButtonDisabledFrmsX(LONG buttRef, DWORD frmID_UP_Dis, DWORD frmID_Dn_Dis, DWORD frmID_Hv_Dis);
LONG ButtonDx_DrawX(ButtonStruct_DX* pButton, WinStructDx* pWin, BYTE* fBuff, DWORD drawFlag, RECT* areaRect, DWORD soundFlag);

bool Subtitles_Set(const char* in_txt, LONG width);
void Subtitles_Destroy();
void Subtitles_Display();

void OnScreenResize_Centred_On_GameWin(Window_DX* pWin_This);
void OnScreenResize_Centred_On_Screen(Window_DX* pWin_This);
LONG Get_GameWin_Width();
LONG Get_GameWin_Height();

//Creates a fallout window centred on the game window, if the game window exists and is not hidden.
LONG Win_Create_CenteredOnGame(DWORD width, DWORD height, DWORD colour, DWORD winFlags);

void Draw_Window_OLD(WinStructDx* p_win, RECT* p_rect, BYTE* toBuff);

void Draw_Text_Frame(DirectX::XMMATRIX* p_Ortho2D, const char* txt, float xPos, float yPos, DWORD colour, DWORD colourBG, TextEffects effects);
//Returns the number of characters processed, the height of the drawn text is returned to pMargins->height.
LONG Draw_Text_Frame_Formated(DirectX::XMMATRIX* p_Ortho2D, const char* txt, RECT* pMargins, DWORD colour, DWORD colourBG, DWORD textFlags, TextEffects effects);

