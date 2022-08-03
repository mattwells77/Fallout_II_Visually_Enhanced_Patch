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
#include "Dx_Windows.h"
#include "Dx_Game.h"

#include "modifications.h"
#include "memwrite.h"
#include "Fall_General.h"

#include <memory.h>
using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

//DWORD *winStack=nullptr;
//int maxWindows=50;
//int *p_num_windows=nullptr;

bool isDDrawing = false;

Window_DX* pWindow_Dx_Subtitles = nullptr;


//___________________________________________________________________________________________________________________________________
void Draw_Text_Frame(XMMATRIX* p_Ortho2D, const char* txt, float xPos, float yPos, DWORD colour, DWORD colourBG, TextEffects effects) {
    if (!txt)
        return;
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    if (!font)
        return;

    LONG txtHeight = font->GetFontHeight();
    LONG txtWidth = font->GetTextWidth(txt);
    if (txtWidth <= 0 || txtHeight <= 0)
        return;

    FRMframeDx* pFrame = new FRMframeDx(nullptr, txtWidth, txtHeight, nullptr, nullptr, false, true);

    BYTE* tBuff=nullptr;
    UINT pitchBytes = 0;
    if (SUCCEEDED(pFrame->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
        font->PrintText(tBuff + 1 + pitchBytes, txt, pitchBytes, txtWidth);
        pFrame->Unlock(nullptr);


        XMFLOAT4 colour_text_BG = {
         ((colourBG & 0x00FF0000) >> 16) / 256.0f,
         ((colourBG & 0x0000FF00) >> 8) / 256.0f,
         ((colourBG & 0x000000FF)) / 256.0f,
         ((colourBG & 0xFF000000) >> 24) / 256.0f };

        GEN_SURFACE_BUFF_DATA genSurfaceData;
        if (effects == TextEffects::dropShadow) {
            genSurfaceData.genData4_1 = colour_text_BG;
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
            float dropDist = (float)txtHeight / 16;
            pFrame->DrawFrame(p_Ortho2D, xPos + dropDist, yPos + dropDist, pd3d_PS_Colour_32_Alpha);
        }
        XMFLOAT4 colour_text = {
         ((colour & 0x00FF0000) >> 16) / 256.0f,
         ((colour & 0x0000FF00) >> 8) / 256.0f,
         ((colour & 0x000000FF)) / 256.0f,
         ((colour & 0xFF000000) >> 24) / 256.0f };
        genSurfaceData.genData4_1 = colour_text;
        pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

        if (effects == TextEffects::outlined) {
            pFrame->DrawFrame(p_Ortho2D, xPos, yPos, pd3d_PS_Colour_32_Brightness_ZeroMasked);

            genSurfaceData.genData4_1 = colour_text_BG;
            //genSurfaceData.genData4_2.x = pFrame->pixelSize.x;
            //genSurfaceData.genData4_2.y = pFrame->pixelSize.y;
            pFrame->GetTexturePixelSize(&genSurfaceData.genData4_2.x, &genSurfaceData.genData4_2.y);
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
            pFrame->DrawFrame(p_Ortho2D, xPos, yPos, pd3d_PS_Outline_Colour8);
        }
        else
            pFrame->DrawFrame(p_Ortho2D, xPos, yPos, pd3d_PS_Colour_32_Alpha);
    }

    tBuff = nullptr;
    delete pFrame;
    pFrame = nullptr;
}


//_____________________________________________________________________________________________________________________________________________________
LONG Draw_Text_Frame_Formated(XMMATRIX* p_Ortho2D, const char* txt, RECT* pMargins, DWORD colour, DWORD colourBG, DWORD textFlags, TextEffects effects) {
    if (!txt)
        return 0;
    FONT_FUNC_STRUCT* font = GetCurrentFont();
    LONG char_count = 0;
    if (!font)
        return char_count;
    if (!pMargins)
        return char_count;

    if (pMargins->right <= pMargins->left)
        return char_count;
    if (pMargins->bottom <= pMargins->top)
        return char_count;

    DWORD txtWidth = pMargins->right - pMargins->left + 2;;
    DWORD txtHeight = pMargins->bottom - pMargins->top + 2;;

    FRMframeDx* pFrame = new FRMframeDx(nullptr, txtWidth, txtHeight, nullptr, nullptr, false, true);

    BYTE* tBuff = nullptr;
    UINT pitchBytes = 0;
    RECT rect = { 1, 1, pMargins->right - pMargins->left + 1, pMargins->bottom - pMargins->top + 1 };

    if (SUCCEEDED(pFrame->Lock((void**)&tBuff, &pitchBytes, D3D11_MAP_WRITE))) {
        char_count = font->PrintText_Formated(tBuff, txt, pitchBytes, txtHeight, &rect, textFlags);
        pFrame->Unlock(nullptr);

        XMFLOAT4 colour_text_BG = {
         ((colourBG & 0x00FF0000) >> 16) / 256.0f,
         ((colourBG & 0x0000FF00) >> 8) / 256.0f,
         ((colourBG & 0x000000FF)) / 256.0f,
         ((colourBG & 0xFF000000) >> 24) / 256.0f };

        GEN_SURFACE_BUFF_DATA genSurfaceData;
        if (effects == TextEffects::dropShadow) {
            genSurfaceData.genData4_1 = colour_text_BG;
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
            float dropDist = (float)txtHeight / 16;
            pFrame->DrawFrame(p_Ortho2D, (float)pMargins->left - 1 + dropDist, (float)pMargins->top - 1 + dropDist, pd3d_PS_Colour_32_Alpha);
        }
        XMFLOAT4 colour_text = {
         ((colour & 0x00FF0000) >> 16) / 256.0f,
         ((colour & 0x0000FF00) >> 8) / 256.0f,
         ((colour & 0x000000FF)) / 256.0f,
         ((colour & 0xFF000000) >> 24) / 256.0f };
        genSurfaceData.genData4_1 = colour_text;
        pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

        if (effects == TextEffects::outlined) {
            pFrame->DrawFrame(p_Ortho2D, (float)pMargins->left - 1, (float)pMargins->top - 1, pd3d_PS_Colour_32_Brightness_ZeroMasked);

            genSurfaceData.genData4_1 = colour_text_BG;
            //genSurfaceData.genData4_2.x = pFrame->pixelSize.x;
            //genSurfaceData.genData4_2.y = pFrame->pixelSize.y;
            pFrame->GetTexturePixelSize(&genSurfaceData.genData4_2.x, &genSurfaceData.genData4_2.y);
            pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
            pFrame->DrawFrame(p_Ortho2D, (float)pMargins->left - 1, (float)pMargins->top - 1, pd3d_PS_Outline_Colour8);
        }
        else
            pFrame->DrawFrame(p_Ortho2D, (float)pMargins->left - 1, (float)pMargins->top - 1, pd3d_PS_Colour_32_Alpha);

        pMargins->top += rect.top;
    }

    tBuff = nullptr;
    delete pFrame;
    pFrame = nullptr;
    return char_count;
}


//_________________________________________________
void OnScreenResize_Subtitles(Window_DX* pWin_This) {

    if (!pWin_This)
        return;
    pWin_This->SetPosition((float)(((LONG)SCR_WIDTH - (LONG)pWin_This->GetWidth()) / 2), (float)((LONG)SCR_HEIGHT - (LONG)pWin_This->GetHeight() - 8));

}


//________________________________________________
bool Subtitles_Set(const char* in_txt, LONG width) {
    if (in_txt == nullptr)
        return false;

    FONT_FUNC_STRUCT* font = GetCurrentFont();

    RECT margins = { 1,1,561,0 };
    font->PrintText_Formated(nullptr, in_txt, 562, 0, &margins, 0);
    margins.bottom = margins.top;
    margins.top = 1;


    DWORD fontHeight = font->GetFontHeight();

    DWORD subWin_Height = margins.bottom - margins.top + 2;
    DWORD subWin_Width = margins.right - margins.left + 2;
    DWORD textWidth = 0;

    if (!pWindow_Dx_Subtitles) {
        pWindow_Dx_Subtitles = new Window_DX((float)(((LONG)SCR_WIDTH - (LONG)subWin_Width) / 2), (float)((LONG)SCR_HEIGHT - (LONG)subWin_Height - 8), subWin_Width, subWin_Height, 0x00000000, nullptr, nullptr);
        pWindow_Dx_Subtitles->Set_OnScreenResizeFunction(&OnScreenResize_Subtitles);
    }
    else {
        pWindow_Dx_Subtitles->ResizeWindow(subWin_Width, subWin_Height);
        pWindow_Dx_Subtitles->SetPosition((float)(((LONG)SCR_WIDTH - (LONG)subWin_Width) / 2), (float)((LONG)SCR_HEIGHT - (LONG)subWin_Height - 8));
    }

    pWindow_Dx_Subtitles->ClearRenderTarget(nullptr);
    DWORD colour = 0xFFFFFFFF;//for holding colour extracted from palette "color.pal"

    pWindow_Dx_Subtitles->Draw_Text_Formated(in_txt, &margins, colour, 0xFF000000, FLG_TEXT_CENTRED, TextEffects::outlined);
    return true;
}


//______________________
void Subtitles_Destroy() {
    if (pWindow_Dx_Subtitles)
        delete pWindow_Dx_Subtitles;
    pWindow_Dx_Subtitles = nullptr;
}


//______________________
void Subtitles_Display() {
    if (pWindow_Dx_Subtitles)
        pWindow_Dx_Subtitles->Display();
}


//________________________________________________________________________________________________________________________________________________________________
Button_DX::Button_DX(Window_DX* p_winDxParent, float inX, float inY, DWORD width, DWORD height, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, bool inHasOverlay) :
    BASE_VERTEX_DX(),
    BASE_POSITION_DX(inX, inY, false),
    BASE_TEXTURE_STAGING(0x00000000),//(DXGI_FORMAT_B8G8R8A8_UNORM, 0x00000000, true, false),
    cfrm_Up(nullptr),
    cfrm_Dn(nullptr),
    cfrm_Hv(nullptr),
    cfrm_Up_Dis(nullptr),
    cfrm_Dn_Dis(nullptr),
    cfrm_Hv_Dis(nullptr),
    cfrm_Current(nullptr),
    pointerMask(nullptr),

    overlay_Flags(0),
    overlay_down_offset_x(0),
    overlay_down_offset_y(0),
    overlay_tex_x(0),
    overlay_tex_y(0),
    overlay_frm_x(0),
    overlay_frm_y(0),
    overlay_frm_scale_x(1.0f),
    overlay_frm_scale_y(1.0f),
    overlay_frm_ori(0),
    overlay_frm_frameNum(0),

    cfrm_Overlay(nullptr),

    pd3d_PS_OverlayShader(nullptr),
    pOverlayShader_Setup(nullptr),
    buttonColour_flags(0),

    pOrtho2D_Win(nullptr),
    p_rc_parentWindow_clip(nullptr)

{
    if (p_winDxParent) {
        pOrtho2D_Win = p_winDxParent->GetProjectionMatrix();
        p_rc_parentWindow_clip = p_winDxParent->GetClippingRect();
    }
    buttonColour[0] = { 0, 0, 0, 0 };
    buttonColour[1] = { 0, 0, 0, 0 };
    buttonColour[2] = { 0, 0, 0, 0 };
    buttonColour[3] = { 0, 0, 0, 0 };
    p_buttonColour = &buttonColour[0];
    SetBaseDimensions(width, height);
    if (frmID_UP)
        cfrm_Up = new FRMCached(frmID_UP);
    if (frmID_Dn)
        cfrm_Dn = new FRMCached(frmID_Dn);
    if (frmID_Hv)
        cfrm_Hv = new FRMCached(frmID_Hv);

    if (inHasOverlay) {//set up constant buffers for BASE_POSITION_DX
        overlay_Flags |= FLG_BUTT_OVERLAY_ENABLED;
        Create_ConstantBuffers(5);
    }
    else
        Create_ConstantBuffers(1);

    if (cfrm_Up) {
        FRMframeDx* frameDx = cfrm_Up->GetFrame(0, 0);
        if (frameDx)
            SetBaseDimensions(frameDx->GetWidth(), frameDx->GetHeight());
        frameDx = nullptr;
    }
    SetMatrices();
}


//_____________________
Button_DX::~Button_DX() {
    if (cfrm_Up)
        delete cfrm_Up;
    cfrm_Up = nullptr;
    if (cfrm_Dn)
        delete cfrm_Dn;
    cfrm_Dn = nullptr;
    if (cfrm_Hv)
        delete cfrm_Hv;
    cfrm_Hv = nullptr;
    if (cfrm_Up_Dis)
        delete cfrm_Up_Dis;
    cfrm_Up_Dis = nullptr;
    if (cfrm_Dn_Dis)
        delete cfrm_Dn_Dis;
    cfrm_Dn_Dis = nullptr;
    if (cfrm_Hv_Dis)
        delete cfrm_Hv_Dis;
    cfrm_Hv_Dis = nullptr;
    cfrm_Current = nullptr;

    if (pointerMask)
        delete[] pointerMask;
    pointerMask = nullptr;

    if (cfrm_Overlay)
        delete cfrm_Overlay;
    cfrm_Overlay = nullptr;

    if (p_rc_overLay_clip)
        delete p_rc_overLay_clip;
    p_rc_overLay_clip = nullptr;
    p_rc_parentWindow_clip = nullptr;

}


//__________________________________________________________________________________________________
bool Button_DX::Set_Disabled_Button_Frms(DWORD frmID_UP_Dis, DWORD frmID_Dn_Dis, DWORD frmID_Hv_Dis) {
    if (frmID_UP_Dis != 0 && cfrm_Up_Dis == nullptr)
        cfrm_Up_Dis = new FRMCached(frmID_UP_Dis);
    if (frmID_Dn_Dis != 0 && cfrm_Dn_Dis == nullptr)
        cfrm_Dn_Dis = new FRMCached(frmID_Dn_Dis);
    if (frmID_Hv_Dis != 0 && cfrm_Hv_Dis == nullptr)
        cfrm_Hv_Dis = new FRMCached(frmID_Hv_Dis);
    return true;
}


//________________________________________
void Button_DX::SetCurrentFrm(DWORD flags) {
    FRMCached* cfrm_Last = cfrm_Current;
    if (flags & FLG_BUTT_UP) {
        cfrm_Current = cfrm_Up;
        if (buttonColour_flags & FLG_BUTT_UP)
            p_buttonColour = &buttonColour[0];
    }
    else if (flags & FLG_BUTT_DN) {
        cfrm_Current = cfrm_Dn;
        if (buttonColour_flags & FLG_BUTT_DN)
            p_buttonColour = &buttonColour[1];
    }
    else if (flags & FLG_BUTT_HV) {
        cfrm_Current = cfrm_Hv;
        if (buttonColour_flags & FLG_BUTT_HV)
            p_buttonColour = &buttonColour[2];
    }
    else if (flags & FLG_BUTT_UP_DIS) {
        cfrm_Current = cfrm_Up_Dis;
        if (buttonColour_flags & FLG_BUTT_UP_DIS | FLG_BUTT_DN_DIS | FLG_BUTT_HV_DIS)
            p_buttonColour = &buttonColour[3];
    }
    else if (flags & FLG_BUTT_DN_DIS) {
        cfrm_Current = cfrm_Dn_Dis;
        if (buttonColour_flags & FLG_BUTT_UP_DIS | FLG_BUTT_DN_DIS | FLG_BUTT_HV_DIS)
            p_buttonColour = &buttonColour[3];
    }
    else if (flags & FLG_BUTT_HV_DIS) {
        cfrm_Current = cfrm_Hv_Dis;
        if (buttonColour_flags & FLG_BUTT_UP_DIS | FLG_BUTT_DN_DIS | FLG_BUTT_HV_DIS)
            p_buttonColour = &buttonColour[3];
    }
    cfrm_Last = nullptr;

}


//_______________________
void Button_DX::Display() {

    if (pD3DDev == nullptr)
        return;

    ID3D11Buffer* frame_pd3dVB = nullptr;
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    ID3D11ShaderResourceView* pframe_Tex_shaderResourceView = nullptr;

    if (cfrm_Current) {
        FRMframeDx* pFrame = cfrm_Current->GetFrame(0, 0);
        if (pFrame) {
            SetPositionRender(0);
            //set vertex stuff
            pFrame->GetVertexBuffer(&frame_pd3dVB);
            pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

            //set texture stuff
            pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
            pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
            //set pixel shader stuff
            if (pFrame->GetPixelWidth() == 1) {
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
            }
            else
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

            pD3DDevContext->DrawIndexed(4, 0, 0);
        }
    }
    if (overlay_Flags & FLG_BUTT_OVERLAY_ENABLED) {

        UINT numScissorRects_Current = 0;
        D3D11_RECT* pScissorRects_Current = nullptr;
        if (p_rc_overLay_clip) {//set the clipping rect if enabled
            pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, nullptr);
            if (numScissorRects_Current > 0) {
                pScissorRects_Current = new D3D11_RECT[numScissorRects_Current];
                pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, pScissorRects_Current);
            }
            pD3DDevContext->RSSetScissorRects(1, p_rc_overLay_clip);
        }

        if (cfrm_Overlay) {
            FRMframeDx* pFrame = cfrm_Overlay->GetFrame(overlay_frm_ori, overlay_frm_frameNum);
            if (pFrame) {
                if (cfrm_Current == cfrm_Dn)
                    SetPositionRender(4);
                else
                    SetPositionRender(3);

                //set vertex stuff
                pFrame->GetVertexBuffer(&frame_pd3dVB);
                pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

                //set texture stuff
                pframe_Tex_shaderResourceView = pFrame->GetShaderResourceView();
                pD3DDevContext->PSSetShaderResources(0, 1, &pframe_Tex_shaderResourceView);
                //set pixel shader stuff
                if (pFrame->GetPixelWidth() == 1) {
                    pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
                }
                else
                    pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

                pD3DDevContext->DrawIndexed(4, 0, 0);
            }
        }
        if (isTexture()) {
            if (cfrm_Current == cfrm_Dn)
                SetPositionRender(2);
            else
                SetPositionRender(1);
            //set vertex stuff
            if (isVertex()) {
                ID3D11Buffer* pd3dVB;
                GetVertexBuffer(&pd3dVB);
                pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
            }
            else
                pD3DDevContext->IASetVertexBuffers(0, 1, &frame_pd3dVB, &stride, &offset);

            //set texture stuff
            pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

            //set pixel shader stuff
            if (pd3d_PS_OverlayShader) {
                if (buttonColour_flags) {
                    GEN_SURFACE_BUFF_DATA genSurfaceData;
                    genSurfaceData.genData4_1 = *p_buttonColour;
                    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
                    pD3DDevContext->PSSetShader(pd3d_PS_OverlayShader, nullptr, 0);
                }
                else if (pOverlayShader_Setup)
                    pOverlayShader_Setup(this);
                pD3DDevContext->PSSetShader(pd3d_PS_OverlayShader, nullptr, 0);
            }
            else
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

            pD3DDevContext->DrawIndexed(4, 0, 0);
        }

        if (pScissorRects_Current != nullptr) {//restore the old clipping rect if the overlays clipping rect was enabled.
            pD3DDevContext->RSSetScissorRects(numScissorRects_Current, pScissorRects_Current);
            delete[] pScissorRects_Current;
            pScissorRects_Current = nullptr;
            numScissorRects_Current = 0;
        }
    }

}


//_______________________________________________________
BYTE* Button_DX::SetPointerMask(DWORD frmID, RECT* pRect) {
    FRMCached* frm = new FRMCached(frmID);
    if (!frm)
        return nullptr;

    FRMframeDx* frame = frm->GetFrame(0, 0);
    if (!frame) {
        delete frm;
        return nullptr;
    }
    if (pointerMask != nullptr) {
        delete frm;
        return nullptr;
    }
    DWORD maskWidth = pRect->right - pRect->left + 1;
    DWORD maskHeight = pRect->bottom - pRect->top + 1;

    pointerMask = new BYTE[maskWidth * maskHeight];
    memset(pointerMask, 0, maskWidth * maskHeight);

    BYTE* pointerMaskTemp = pointerMask;
    bool is32bit = frame->Is32bitColour();

    BYTE* tbuff = nullptr;
    UINT pitch = 0;
    frame->Lock((void**)&tbuff, &pitch, D3D11_MAP_READ);

    if (!tbuff) {
        delete frm;
        return nullptr;
    }

    DWORD dpitch = pitch;
    DWORD xMax = maskWidth;
    DWORD yMax = maskHeight;

    if (xMax > dpitch)
        xMax = dpitch;
    if (yMax > frame->GetHeight())
        yMax = frame->GetHeight();

    tbuff += dpitch;
    for (DWORD y = 0; y < yMax; y++) {
        for (DWORD x = 0; x < xMax; x++) {
            if (is32bit) {
                if (((DWORD*)tbuff)[x + 1] & 0xFF000000)
                    pointerMaskTemp[x] = 0xFF;
            }
            else {
                if (tbuff[x + 1] != 0x00)
                    pointerMaskTemp[x] = 0xFF;
            }
        }
        tbuff += dpitch;
        pointerMaskTemp += maskWidth;
    }
    frame->Unlock(nullptr);

    pointerMaskTemp = nullptr;
    delete frm;
    return pointerMask;
}


//_________________________________________________________________________________________________________________________
bool Button_DX::Overlay_CreateTexture(float in_offset_x, float in_offset_y, float in_down_offset_x, float in_down_offset_y) {
    if (!(overlay_Flags & FLG_BUTT_OVERLAY_ENABLED))
        return false;

    if (!isTexture()) {
        if (!isVertex())
            CreateVerticies();
        Texture_Initialize(DXGI_FORMAT_B8G8R8A8_UNORM, true, false);

    }
    overlay_tex_x = in_offset_x;
    overlay_tex_y = in_offset_y;
    overlay_down_offset_x = in_down_offset_x;
    overlay_down_offset_y = in_down_offset_y;
    SetMatrices();
    return true;
}


//______________________________________________________________________________________________
bool Button_DX::Overlay_CreateTexture(bool is32bit, bool isRenderTarget, bool hasStagingTexture) {
    if (!(overlay_Flags & FLG_BUTT_OVERLAY_ENABLED))
        return false;

    if (!isTexture()) {
        if (!isVertex())
            CreateVerticies();
        DXGI_FORMAT dxgi_Format = DXGI_FORMAT_R8_UNORM;
        if (is32bit)
            dxgi_Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        Texture_Initialize(dxgi_Format, isRenderTarget, hasStagingTexture);
    }
    SetMatrices();
    return true;
}


//_____________________________________________________________________________________________________________________________________________________________________________________________________________________
bool Button_DX::Overlay_CreateTexture(float in_offset_x, float in_offset_y, float in_down_offset_x, float in_down_offset_y, DWORD in_width, DWORD in_height, bool is32bit, bool isRenderTarget, bool hasStagingTexture) {
    if (!(overlay_Flags & FLG_BUTT_OVERLAY_ENABLED))
        return false;

    if (!isTexture()) {
        SetBaseDimensions(in_width, in_height);
        if (!isVertex()) {
            CreateVerticies();
        }
        DXGI_FORMAT dxgi_Format = DXGI_FORMAT_R8_UNORM;
        if (is32bit)
            dxgi_Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        Texture_Initialize(dxgi_Format, isRenderTarget, hasStagingTexture);
        //Clear_Staging();
    }
    overlay_tex_x = in_offset_x;
    overlay_tex_y = in_offset_y;
    overlay_down_offset_x = in_down_offset_x;
    overlay_down_offset_y = in_down_offset_y;

    SetMatrices();
    return true;
}


//____________________________________________________________________________________________________________________________
bool Button_DX::Overlay_SetFrm(DWORD frmID_Item, int ori, int frameNum, float inX, float inY, DWORD maxWidth, DWORD maxHeight) {
    if (!(overlay_Flags & FLG_BUTT_OVERLAY_ENABLED))
        return false;

    if (cfrm_Overlay)
        delete cfrm_Overlay;
    cfrm_Overlay = nullptr;

    overlay_frm_ori = ori;
    overlay_frm_frameNum = frameNum;

    overlay_frm_x = inX;
    overlay_frm_y = inY;
    if (frmID_Item == -1)
        return false;

    cfrm_Overlay = new FRMCached(frmID_Item);

    FRMframeDx* p_frame = cfrm_Overlay->GetFrame(overlay_frm_ori, overlay_frm_frameNum);
    if (!p_frame) {
        delete cfrm_Overlay;
        cfrm_Overlay = nullptr;
        return false;
    }

    overlay_frm_scale_x = 1.0f;
    overlay_frm_scale_y = 1.0f;

    if (maxWidth && maxHeight) {
        float point_w = 1.0f;
        float point_h = 1.0f;

        DWORD f_width = p_frame->GetWidth();
        DWORD f_height = p_frame->GetHeight();

        if (f_width > maxWidth || f_height > maxHeight) {

            float frmRO = (float)f_width / f_height;
            float maxRO = (float)maxWidth / maxHeight;
            DWORD width_scaled = f_width;
            DWORD height_scaled = f_height;
            float x_scaled = 0;
            float y_scaled = 0;
            if (frmRO >= maxRO) {
                height_scaled = (DWORD)(width_scaled / maxRO);
                y_scaled = (float)(height_scaled - f_height) / 2;
            }
            else {
                width_scaled = (DWORD)(height_scaled * maxRO);
                x_scaled = (float)(width_scaled - f_width) / 2;
            }
            point_w = (float)width_scaled / maxWidth;
            point_h = (float)width_scaled / maxWidth;
            overlay_frm_x += x_scaled / point_w;
            overlay_frm_y += y_scaled / point_h;
            overlay_frm_scale_x = 1.0f / point_w;
            overlay_frm_scale_y = 1.0f / point_h;
        }
        else {
            overlay_frm_x += (maxWidth - f_width) / 2;
            overlay_frm_y += (maxHeight - f_height) / 2;
        }
    }
    p_frame = nullptr;

    SetMatrices();
    return true;
}



//_____________________________________________________________________________________________________
bool Button_DX::Overlay_SetFrm(DWORD frmID_Item, float inX, float inY, DWORD maxWidth, DWORD maxHeight) {
    return Overlay_SetFrm( frmID_Item, 0, 0, inX, inY, maxWidth, maxHeight);
}


//_______________________________________________
void Button_DX::OverLay_SetClippingRect(bool set) {
    if (set) {
        if (p_rc_parentWindow_clip && !p_rc_overLay_clip) {

            p_rc_overLay_clip = new D3D11_RECT{ 0,0,0,0 };
            p_rc_overLay_clip->left = (LONG)x * scaleLevel_GUI + p_rc_parentWindow_clip->left;
            p_rc_overLay_clip->top = (LONG)y * scaleLevel_GUI + p_rc_parentWindow_clip->top;
            p_rc_overLay_clip->right = p_rc_overLay_clip->left + (LONG)width * scaleLevel_GUI;
            p_rc_overLay_clip->bottom = p_rc_overLay_clip->top + (LONG)height * scaleLevel_GUI;
        }
    }
    else {
        if (p_rc_overLay_clip)
            delete p_rc_overLay_clip;
        p_rc_overLay_clip = nullptr;
   }
}


//___________________________
void Button_DX::SetMatrices() {
    DirectX::XMMATRIX xmManipulation;
    DirectX::XMMATRIX xmScaling;

    DirectX::XMMATRIX* pOrtho2D;
    DirectX::XMMATRIX Ortho2D;

    MATRIX_DATA posData;

    if (pOrtho2D_Win != nullptr) {
        Ortho2D = XMLoadFloat4x4(pOrtho2D_Win);
        pOrtho2D = &Ortho2D;
    }
    else//if no win matrix get screen ortho matrix
        pOrtho2D = GetScreenProjectionMatrix_XM();

    xmManipulation = DirectX::XMMatrixTranslation(x, y, 0);
    xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
    posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
    posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);

    UpdatePositionData(0, &posData);

    if (overlay_Flags & FLG_BUTT_OVERLAY_ENABLED) {
        //up position of overlay texture
        xmManipulation = DirectX::XMMatrixTranslation((x + overlay_tex_x) * scaleX, (y + overlay_tex_y) * scaleY, (float)z);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);
        UpdatePositionData(1, &posData);
        //down position of overlay texture
        xmManipulation = DirectX::XMMatrixTranslation((x + overlay_tex_x + overlay_down_offset_x) * scaleX, (y + overlay_tex_y + overlay_down_offset_y) * scaleY, (float)z);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);
        UpdatePositionData(2, &posData);
        //up position of overlay frm
        xmManipulation = DirectX::XMMatrixTranslation((x + overlay_frm_x) * scaleX, (y + overlay_frm_y) * scaleY, (float)z);
        xmScaling = DirectX::XMMatrixScaling(overlay_frm_scale_x, overlay_frm_scale_y, 1.0f);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);
        UpdatePositionData(3, &posData);
        //down position of overlay frm
        xmManipulation = DirectX::XMMatrixTranslation((x + overlay_frm_x + overlay_down_offset_x) * scaleX, (y + overlay_frm_y + overlay_down_offset_y) * scaleY, (float)z);
        xmScaling = DirectX::XMMatrixScaling(overlay_frm_scale_x, overlay_frm_scale_y, 1.0f);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D);
        UpdatePositionData(4, &posData);

        if (p_rc_overLay_clip && p_rc_parentWindow_clip) {
            p_rc_overLay_clip->left = (LONG)x * scaleLevel_GUI + p_rc_parentWindow_clip->left;
            p_rc_overLay_clip->top = (LONG)y * scaleLevel_GUI + p_rc_parentWindow_clip->top;
            p_rc_overLay_clip->right = p_rc_overLay_clip->left + (LONG)width * scaleLevel_GUI;
            p_rc_overLay_clip->bottom = p_rc_overLay_clip->top + (LONG)height * scaleLevel_GUI;
        }
    }
}


//______________________________________________
void RefreshButtonStateX(ButtonStruct_DX* pButt) {
    if (!pButt)
        return;

    if (pButt->buttDx) {
        DWORD dx_butt_flag = 0;
        if (pButt->buffCurrent == pButt->buffUp)
            dx_butt_flag = FLG_BUTT_UP;
        else if (pButt->buffCurrent == pButt->buffDn)
            dx_butt_flag = FLG_BUTT_DN;
        else if (pButt->buffCurrent == pButt->buffHv)
            dx_butt_flag = FLG_BUTT_HV;
        else if (pButt->buffCurrent == pButt->buffUpDis)
            dx_butt_flag = FLG_BUTT_UP_DIS;
        else if (pButt->buffCurrent == pButt->buffDnDis)
            dx_butt_flag = FLG_BUTT_DN_DIS;
        else if (pButt->buffCurrent == pButt->buffHvDis)
            dx_butt_flag = FLG_BUTT_HV_DIS;
        pButt->buttDx->SetCurrentFrm(dx_butt_flag);
    }
}


//____________________________________________________________________________________________________________________________________________________________________________________________________
LONG CreateButtonX(LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, DWORD flags) {
    WinStructDx* win = (WinStructDx*)fall_Win_Get(winRef);
    if (!win || !win->winDx)
        return -1;
    LONG buttRef = fall_Button_Create(winRef, x, y, width, height, keyHoverOn, keyHoverOff, keyPush, keyLift, nullptr, nullptr, nullptr, flags);
    if (buttRef == -1)
        return buttRef;

    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (pButt) {
        pButt->buttDx = new Button_DX(win->winDx, (float)(pButt->rect.left), (float)(pButt->rect.top), width, height, frmID_UP, frmID_Dn, frmID_Hv, false);
        pButt->buffUp = (BYTE*)1;
        pButt->buffDn = (BYTE*)2;
        pButt->buffCurrent = pButt->buffUp;
        RefreshButtonStateX(pButt);
    }

    win = nullptr;
    pButt = nullptr;

    return buttRef;
}


//____________________________________________________________________________________________________________________________________________________________________________________________________________
LONG CreateButtonX_Overlay(LONG winRef, LONG x, LONG y, DWORD width, DWORD height, LONG keyHoverOn, LONG keyHoverOff, LONG keyPush, LONG keyLift, DWORD frmID_UP, DWORD frmID_Dn, DWORD frmID_Hv, DWORD flags) {
    WinStructDx* win = (WinStructDx*)fall_Win_Get(winRef);
    if (!win || !win->winDx)
        return -1;
    LONG buttRef = fall_Button_Create(winRef, x, y, width, height, keyHoverOn, keyHoverOff, keyPush, keyLift, nullptr, nullptr, nullptr, flags);
    if (buttRef == -1)
        return buttRef;
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (pButt) {
        pButt->buttDx = new Button_DX(win->winDx, (float)(pButt->rect.left), (float)(pButt->rect.top), width, height, frmID_UP, frmID_Dn, frmID_Hv, true);
        pButt->buffUp = (BYTE*)1;
        pButt->buffDn = (BYTE*)2;
        if (frmID_Hv)
            pButt->buffHv = (BYTE*)3;
        pButt->buffCurrent = pButt->buffUp;

        RefreshButtonStateX(pButt);
    }
    win = nullptr;
    pButt = nullptr;

    return buttRef;
}


//___________________________________________________________________________
LONG SetButtonSoundsX(LONG buttRef, void (*funcDnSnd)(), void (*funcUpSnd)()) {
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (!pButt)
        return -1;
    pButt->funcUpSnd = funcUpSnd;
    pButt->funcDnSnd = funcDnSnd;
    return 0;
}


//___________________________________________________
LONG SetButtonPointerMaskX(LONG buttRef, DWORD frmID) {
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (!pButt)
        return -1;
    if (!pButt->buttDx)
        return -1;

    pButt->buffPointerMask = pButt->buttDx->SetPointerMask(frmID, &pButt->rect);
    return 0;
}


//___________________________________________________________________________________________________
LONG SetButtonRightClickX(LONG buttRef, LONG dnCode, LONG upCode, void (*funcDn)(), void (*funcUp)()) {
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (!pButt)
        return -1;
    pButt->refUpRht = upCode;
    pButt->funcUpRht = funcUp;

    pButt->refDnRht = dnCode;
    pButt->funcDnRht = funcDn;

    if (pButt->refUpRht != -1 || pButt->refDnRht != -1 || pButt->funcUpRht != nullptr || pButt->funcDnRht != nullptr)
        pButt->flags |= FLG_ButtRgtClickOn;
    else
        pButt->flags ^= FLG_ButtRgtClickOn;
    return 0;
}


//___________________________________
LONG SetButtonDisabledX(LONG buttRef) {
    WinStructDx* pWin = nullptr;
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, (WinStruct**)&pWin);
    if (!pButt)
        return -1;
    if ((pButt->flags & FLG_ButtDisabled))
        return 0;

    pButt->flags |= FLG_ButtDisabled;

    if (pButt == pWin->Button34) {//not sure what this is about, copied from exe
        if (pButt->refHvOff != -1)
            fall_SendKey(pButt->refHvOff);
        pWin->Button34 = nullptr;
    }
    ButtonDx_DrawX(pButt, pWin, pButt->buffCurrent, 1, nullptr, 0);
    return 0;
}


//__________________________________
LONG SetButtonEnabledX(LONG buttRef) {
    WinStructDx* pWin = nullptr;
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, (WinStruct**)&pWin);
    if (!pButt)
        return -1;

    if (!(pButt->flags & FLG_ButtDisabled))
        return 0;

    pButt->flags ^= FLG_ButtDisabled;
    ButtonDx_DrawX(pButt, pWin, pButt->buffCurrent, 1, nullptr, 0);
    return 0;
}


//___________________________________________________________________________________________________
LONG SetButtonDisabledFrmsX(LONG buttRef, DWORD frmID_UP_Dis, DWORD frmID_Dn_Dis, DWORD frmID_Hv_Dis) {
    ButtonStruct_DX* pButt = (ButtonStruct_DX*)fall_Button_Get(buttRef, nullptr);
    if (!pButt || !pButt->buttDx)
        return -1;

    pButt->buttDx->Set_Disabled_Button_Frms(frmID_UP_Dis, frmID_Dn_Dis, frmID_Hv_Dis);

    if (frmID_UP_Dis)
        pButt->buffUpDis = (BYTE*)4;
    if (frmID_Dn_Dis)
        pButt->buffDnDis = (BYTE*)5;
    if (frmID_Hv_Dis)
        pButt->buffHvDis = (BYTE*)6;

    return 0;
}


//_________________________________________________
void MoveWindowX(WinStructDx* pWin, LONG x, LONG y) {
    pWin->rect.left = x;
    pWin->rect.top = y;
    pWin->rect.right = pWin->rect.left + pWin->width - 1;
    pWin->rect.bottom = pWin->rect.top + pWin->height - 1;
    if (pWin->winDx)
        pWin->winDx->SetPosition((float)pWin->rect.left, (float)pWin->rect.top);
    if (pWin->ButtonList) {
        ButtonStruct_DX* pButton = pWin->ButtonList;
        int count = 0;
        while (pButton) {
            count++;
            if (pButton->buttDx)
                pButton->buttDx->RefreshMatrices();
            pButton = pButton->prevButton;
        }
    }
}


//______________________________________________________________________________
void ResizeWindowX(WinStructDx* p_win, LONG x, LONG y, DWORD width, DWORD height) {
    p_win->width = width;
    p_win->height = height;
    p_win->rect.left = x;
    p_win->rect.top = y;
    p_win->rect.right = p_win->rect.left + p_win->width - 1;
    p_win->rect.bottom = p_win->rect.top + p_win->height - 1;
    p_win->buff = (BYTE*)fall_Mem_Reallocate(p_win->buff, p_win->width * p_win->height);
    if (p_win->winDx) {
        p_win->winDx->SetPosition((float)p_win->rect.left, (float)p_win->rect.top);
        p_win->winDx->ResizeWindow(p_win->width, p_win->height);
        p_win->winDx->ClearRenderTarget(nullptr);
    }
    if (p_win->ButtonList) {
        ButtonStruct_DX* pButton = p_win->ButtonList;
        int count = 0;
        while (pButton) {
            count++;
            if (pButton->buttDx)
                pButton->buttDx->RefreshMatrices();
            pButton = pButton->prevButton;
        }
    }
}


//_______________________
void Window_DX::Display() {
    Window_DX* pWin = nullptr;
    if (!parent)
        pWin = winFirst;
    else
        pWin = parent->winFirst;
    pWin->DisplayWindow();
}


//_____________________________
void Window_DX::DisplayWindow() {

    if (pD3DDev == nullptr)
        return;
    if (pTex == nullptr)
        return;

    if (pOnDisplay_Instead) {
        pOnDisplay_Instead(this);
        if (winNext)
            winNext->DisplayWindow();
        return;
    }
    if (pOnDisplay_Pre)
        pOnDisplay_Pre(this);

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
    //set pixel shader stuff
    if (pixelWidth == 1)
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
    else
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);


    //set shader constant buffers 
    SetPositionRender(0);

    //if clip_at_parent enabled set the clipping rect to the dimensions of the parent window.
    UINT numScissorRects_Current = 0;
    D3D11_RECT* pScissorRects_Current = nullptr;
    if (parent && (pRect_clip || clip_at_parent)) {
        pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, nullptr);
        if (numScissorRects_Current > 0) {
            pScissorRects_Current = new D3D11_RECT[numScissorRects_Current];
            pD3DDevContext->RSGetScissorRects(&numScissorRects_Current, pScissorRects_Current);
        }
        if (clip_at_parent)
            pD3DDevContext->RSSetScissorRects(1, parent->GetClippingRect());
        else
            pD3DDevContext->RSSetScissorRects(1, pRect_clip);
    }

    pD3DDevContext->DrawIndexed(4, 0, 0);

    if (pScissorRects_Current != nullptr) {
        pD3DDevContext->RSSetScissorRects(numScissorRects_Current, pScissorRects_Current);
        delete[] pScissorRects_Current;
        pScissorRects_Current = nullptr;
        numScissorRects_Current = 0;
    }

    if (pOnDisplay_Post)
        pOnDisplay_Post(this);

    if (winNext)
        winNext->DisplayWindow();
}


//_________________________________
void Window_DX::Clear(DWORD colour) {

    BYTE* pTex_BackBuff = new BYTE[width * pixelWidth * height];
    memset(pTex_BackBuff, 0, width * pixelWidth * height);
    BYTE* pdest = nullptr;
    pdest = pTex_BackBuff;

    DWORD size_in_pixels = width * height;
    DWORD colourOffset = 0;
    while (colourOffset < size_in_pixels) {
        if (pixelWidth == 4)
            ((DWORD*)pdest)[colourOffset] = colour;
        else if (pixelWidth == 2)
            ((WORD*)pdest)[colourOffset] = (colour & 0x0000FFFF);
        else if (pixelWidth == 1)
            pdest[colourOffset] = (colour & 0x000000FF);
        colourOffset++;
    }

    D3D11_BOX destRegion;
    destRegion.left = 0;
    destRegion.right = width;
    destRegion.top = 0;
    destRegion.bottom = height;
    destRegion.front = 0;
    destRegion.back = 1;
    pD3DDevContext->UpdateSubresource(pTex, 0, &destRegion, pTex_BackBuff, width * pixelWidth, 0);
    delete[] pTex_BackBuff;
    pTex_BackBuff = nullptr;
    pdest = nullptr;
}


//_________________________________________________________________________________________________________________________________________________________________
void Window_DX::Draw32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, DWORD* toBuff, LONG tX, LONG tY, DWORD toWidth) {
    toBuff += tY * toWidth + tX;
    DWORD* frBuff = (DWORD*)fBuff;
    frBuff += fY * fWidth + fX;

    for (DWORD y = 0; y < subHeight; y++) {
        for (DWORD x = 0; x < subWidth; x++) {
            toBuff[x] = frBuff[x];
        }
        toBuff += toWidth;
        frBuff += fWidth;
    }
}


//________________________________________________________________________________________________________________________________________________________________________________
void Window_DX::DrawPal32(BYTE* fBuff, DWORD* pal, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, DWORD* toBuff, LONG tX, LONG tY, DWORD toWidth) {
    toBuff += tY * toWidth + tX;
    fBuff += fY * fWidth + fX;

    for (DWORD y = 0; y < subHeight; y++) {
        for (DWORD x = 0; x < subWidth; x++) {
            toBuff[x] = pal[fBuff[x]];
            /// alpha of mask colour now set in function "LoadFrmFromID" in "F_Art.cpp".
        }
        toBuff += toWidth;
        fBuff += fWidth;
    }
}


//_______________________________________________________________________________________________________________________________________________________________
void Window_DX::Draw8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD toWidth) {
    toBuff += tY * toWidth + tX;
    fBuff += fY * fWidth + fX;

    for (DWORD y = 0; y < subHeight; y++) {
        for (DWORD x = 0; x < subWidth; x++)
            toBuff[x] = fBuff[x];
        toBuff += toWidth;
        fBuff += fWidth;
    }
}


//___________________________________________________________________________________________________________________________________________________________________
void Window_DX::Draw(BYTE* fBuff, DWORD* fpal, bool is_fBuff_32bit, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY) {

    if (draw_flag == false)
        return;

    if (pTex == nullptr)
        return;

    if (tX < 0 || tX >(LONG)width - 1)
        return;
    if (tY < 0 || tY >(LONG)height - 1)
        return;
    if (fX < 0 || fX + subWidth > fWidth)
        return;
    if (fY < 0 || fY + subHeight > fHeight)
        return;
    if (fBuff == nullptr)
        return;

    if (tX + subWidth > width)
        subWidth = width - tX;
    if (tY + subHeight > height)
        subHeight = height - tY;

    RECT rect = { tX, tY,static_cast<long>(tX + subWidth),static_cast<long>(tY + subHeight) };

    if ((LONG)subWidth - fX > rect.right - rect.left) {
        Fallout_Debug_Error("Window_DX::Draw - windraw fail x");
        return;
    }
    if ((LONG)subHeight - fY > rect.bottom - rect.top) {
        Fallout_Debug_Error("Window_DX::Draw - windraw fail y");
        return;
    }

    UINT buffWidth = subWidth * pixelWidth;
    BYTE* pTex_BackBuff = new BYTE[buffWidth * subHeight];
    memset(pTex_BackBuff, 0, buffWidth * subHeight);

    if (pixelWidth == 1)
        Draw8(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, pTex_BackBuff, 0, 0, subWidth);
    else if (pixelWidth == 4) {
        if (!fpal && !is_fBuff_32bit) {
            if (color_pal) {
                if (SUCCEEDED(color_pal->Lock(&fpal, D3D11_MAP_READ))) {
                    DrawPal32(fBuff, fpal, fWidth, fHeight, fX, fY, subWidth, subHeight, (DWORD*)pTex_BackBuff, 0, 0, subWidth);
                    color_pal->Unlock(0, 0);
                }
                fpal = nullptr;
            }
        }
        else if (fpal)
            DrawPal32(fBuff, fpal, fWidth, fHeight, fX, fY, subWidth, subHeight, (DWORD*)pTex_BackBuff, 0, 0, subWidth);
        else
            Draw32(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, (DWORD*)pTex_BackBuff, 0, 0, subWidth);
    }

    D3D11_BOX destRegion;
    destRegion.left = rect.left;
    destRegion.right = rect.right;
    destRegion.top = rect.top;
    destRegion.bottom = rect.bottom;
    destRegion.front = 0;
    destRegion.back = 1;
    pD3DDevContext->UpdateSubresource(pTex, 0, &destRegion, pTex_BackBuff, buffWidth, 0);

    delete[] pTex_BackBuff;
    pTex_BackBuff = nullptr;
}


//_________________________________________________________________________________________________________________
void Window_DX::Draw_Text(const char* txt, LONG xPos, LONG yPos, DWORD colour, DWORD colourBG, TextEffects effects) {

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    XMMATRIX Ortho2D;
    GetOrthoMatrix(&Ortho2D);
    SetRenderTarget(nullptr);
    Set_ViewPort(width, height);
    Draw_Text_Frame(&Ortho2D, txt, (float)xPos, (float)yPos, colour, colourBG, effects);
}


//_____________________________________________________________________________________________________________________________________
LONG Window_DX::Draw_Text_Formated(const char* txt, RECT* pMargins, DWORD colour, DWORD colourBG, DWORD textFlags, TextEffects effects) {
    if (pMargins->left > (LONG)width - 1)
        return 0;
    if (pMargins->top > (LONG)height - 1)
        return 0;
    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));

    XMMATRIX Ortho2D;
    GetOrthoMatrix(&Ortho2D);
    SetRenderTarget(nullptr);
    Set_ViewPort(width, height);
    return Draw_Text_Frame_Formated(&Ortho2D, txt, pMargins, colour, colourBG, textFlags, effects);;
}


//________________________________________________________________________________________________________________________________________
void Window_DX::RenderTargetDrawFrame(float xPos, float yPos, FRMframeDx* pFrame, ID3D11PixelShader* pd3d_PixelShader, RECT* pScissorRect) {

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    XMMATRIX Ortho2D;
    GetOrthoMatrix(&Ortho2D);
    SetRenderTarget(nullptr);
    Set_ViewPort(width, height);
    if (pScissorRect)
        pD3DDevContext->RSSetScissorRects(1, pScissorRect);

    if (pd3d_PixelShader)
        pFrame->DrawFrame(&Ortho2D, xPos, yPos, pd3d_PixelShader);
    else
        pFrame->DrawFrame(&Ortho2D, xPos, yPos);
}


//____________________________________
void Window_DX::ClearRect(RECT* pRect) {
    if (pRect && (pRect->right <= pRect->left || pRect->bottom <= pRect->top))
        return;
    if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right >= (LONG)width && pRect->bottom >= (LONG)height)) {
        ClearRenderTarget(nullptr);
        return;
    }
    if (pRect->right < 0 || pRect->bottom < 0 || pRect->left >= (LONG)width || pRect->top >= (LONG)height) {
        return;
    }

    unique_ptr<STORED_VIEW> store_view(new STORED_VIEW(true));
    SetRenderTargetAndViewPort(nullptr);

    XMMATRIX Ortho2D;
    GetOrthoMatrix(&Ortho2D);
    MATRIX_DATA posData;
    posData.World = DirectX::XMMatrixTranslation((float)0, (float)0, (float)0);
    posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, Ortho2D);
    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();
    pD3DDevContext->RSSetScissorRects(1, pRect);


    pD3DDevContext->OMSetBlendState(pBlendState_Zero, nullptr, -1);
    //set vertex stuff
    ID3D11Buffer* pd3dVB;
    GetVertexBuffer(&pd3dVB);
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    GEN_SURFACE_BUFF_DATA genSurfaceData;
    genSurfaceData.genData4_1 = { bg_Colour_f[0], bg_Colour_f[1], bg_Colour_f[2], bg_Colour_f[3] };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);

    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
}


//_____________________________________
void Display_Buttons(WinStructDx* pWin) {
    if (!pWin)
        return;
    if (!pWin->ButtonList)
        return;
    ButtonStruct_DX* pButtonListTemp = pWin->ButtonList;

    while (pButtonListTemp) {
        if (pButtonListTemp->buttDx)
            pButtonListTemp->buttDx->Display();
        pButtonListTemp = pButtonListTemp->prevButton;
    }
}


//__________________________________________________________________
void Draw_Window_OLD(WinStructDx* p_win, RECT* p_rect, BYTE* toBuff) {//toBuff seems to be for drawing mouse only or pre-directx

    if (toBuff != nullptr)
        return;
    if (!p_win)
        return;
    if (p_win->flags & FLG_WinHidden)
        return;

    if (p_rect->left > p_win->rect.right)
        return;
    if (p_rect->right < p_win->rect.left)
        return;
    if (p_rect->top > p_win->rect.bottom)
        return;
    if (p_rect->bottom < p_win->rect.top)
        return;

    RECT rcWinDraw;

    if (p_rect->left > p_win->rect.left)
        rcWinDraw.left = p_rect->left - p_win->rect.left;
    else
        rcWinDraw.left = 0;
    if (p_rect->right > p_win->rect.right)
        rcWinDraw.right = p_win->width - 1;
    else
        rcWinDraw.right = p_rect->right - p_win->rect.left;

    if (p_rect->top > p_win->rect.top)
        rcWinDraw.top = p_rect->top - p_win->rect.top;
    else
        rcWinDraw.top = 0;
    if (p_rect->bottom > p_win->rect.bottom)
        rcWinDraw.bottom = p_win->height - 1;
    else
        rcWinDraw.bottom = p_rect->bottom - p_win->rect.top;

    DWORD skipTemp = Dx_PresentSkip;
    Dx_PresentSkip = 1;
    fall_Button_Draw((WinStruct*)p_win, p_rect);
    Dx_PresentSkip = skipTemp;

    DWORD* fpal = nullptr;

    if (p_win->winDx) {
        p_win->winDx->Draw(p_win->buff, fpal, false, p_win->width, p_win->height, rcWinDraw.left, rcWinDraw.top, rcWinDraw.right - rcWinDraw.left + 1, rcWinDraw.bottom - rcWinDraw.top + 1, rcWinDraw.left, rcWinDraw.top);
    }

    isDDrawing = true;
    if (!IsMouseHidden() && IsMouseInRect(p_rect))
        fall_Mouse_Show();
    isDDrawing = false;
}


//__________________________________________
void __declspec(naked) draw_window_old(void) {

    __asm {
        pushad
        push ebx
        push edx
        push eax
        call Draw_Window_OLD
        add esp, 0xC
        popad
        ret;
    }
}

//To-Do whats this all about matt
//____________________________________
void __declspec(naked) ddraw_win_dont(void) {

   __asm {
      pushad
      //mov dxPresentFlag, 1
      //call Dx_Present
      popad
      ret;
   }
}


//____________________________________
void __declspec(naked) ddraw_win_dont2(void) {

   __asm {
      pushad
      //mov dxPresentFlag, 1
      //call Dx_Present
      popad
      ret;
   }
}


//_______________________________________________
void DrawFalloutWindows(RECT* pRect, BYTE* pBuff) {
    Dx_PresentSkip = 1;
    *p_draw_window_flag = 1;

    LONG backWinNum = 1;

    for (LONG i = *p_num_windows - 1; i > 0; i--) {
        if (pWin_Array[i]->rect.left <= pRect->left && pWin_Array[i]->rect.top <= pRect->top && pWin_Array[i]->rect.right >= pRect->right && pWin_Array[i]->rect.bottom >= pRect->bottom)
            backWinNum = i;
    }

    for (LONG i = backWinNum; i < *p_num_windows; i++) {
        if (pWin_Array[i]->ref != *pWinRef_GameArea)
            fall_Windows_Draw_Win(pWin_Array[i], pRect, pBuff);
    }

    *p_draw_window_flag = 0;

    if (!pBuff && !IsMouseHidden()) {
        if (IsMouseInRect(pRect))
            fall_Mouse_Show();
    }
    Dx_PresentSkip = 0;
}


//____________________________________________________
void __declspec(naked) draw_fallout_windows_rect(void) {

    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp

        push edx
        push eax
        call DrawFalloutWindows
        add esp, 0x8

        //pushad
        ///push eax
        //call Dx_Present
        ///add esp, 0x4
        //popad

        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        ret
    }
}


//____________________________
void SetDxWin(WinStruct* pWin) {

    WinStructDx* winStructDx = (WinStructDx*)pWin;
    if (!winStructDx)
        return;

    if (winStructDx->ref == *pWinRef_GameArea) {
        winStructDx->winDx = nullptr;
        //GameAreas_SetScale();
    }
    else if (!winStructDx->winDx) {
        winStructDx->winDx = new Window_DX((float)winStructDx->rect.left, (float)winStructDx->rect.top, winStructDx->width, winStructDx->height, 0x000000FF, nullptr, nullptr);
        winStructDx->winDx->Set_FalloutParentWindow(winStructDx);
    }
    else {
        winStructDx->winDx->SetPosition((float)winStructDx->rect.left, (float)winStructDx->rect.top);
    }
}


//________________________________________________________________________________________________________________
bool ScaleWindowToScreen(Window_DX* subwin, LONG scaleType, float* pPoint_W, float* pPoint_H, RECT* pRectOnScreen) {
    if (!subwin)
        return false;
    //size of a pixel on screen - for scaling
    float point_w = 1.0f;
    float point_h = 1.0f;

    DWORD bgWidth = subwin->GetWidth();
    DWORD bgHeight = subwin->GetHeight();

    RECT rect_bg_scaled = { 0,0,0,0 };
    //set window size and scaling - no longer bothering with "scaleType == 2" - stretch fit
    if (scaleType == 1 || SCR_WIDTH < bgWidth || SCR_HEIGHT < bgHeight) {//scale - maintaining aspect of backgroung image - forced if image is larger than screen size
        float bg_imageRO = (float)bgWidth / bgHeight;
        float screenRO = (float)SCR_WIDTH / SCR_HEIGHT;
        DWORD bgWidth_scaled = 0;
        DWORD bgHeight_scaled = 0;

        if (bg_imageRO >= screenRO) {
            rect_bg_scaled.left = 0;
            bgWidth_scaled = bgWidth;
            bgHeight_scaled = (DWORD)(bgWidth_scaled / screenRO);
            rect_bg_scaled.top = (bgHeight_scaled - bgHeight) / 2;
        }
        else {
            rect_bg_scaled.top = 0;
            bgHeight_scaled = bgHeight;
            bgWidth_scaled = (DWORD)(bgHeight_scaled * screenRO);
            rect_bg_scaled.left = (bgWidth_scaled - bgWidth) / 2;
        }
        rect_bg_scaled.bottom = bgHeight_scaled - rect_bg_scaled.top;
        rect_bg_scaled.right = bgWidth_scaled - rect_bg_scaled.left;

        point_h = (float)bgHeight_scaled / SCR_HEIGHT;
        point_w = (float)bgWidth_scaled / SCR_WIDTH;
    }
    else {//if(scaleType == 0) {//original size
        rect_bg_scaled.left = ((LONG)SCR_WIDTH - (LONG)bgWidth) / 2;
        rect_bg_scaled.top = ((LONG)SCR_HEIGHT - (LONG)bgHeight) / 2;
        if (rect_bg_scaled.left < 0)
            rect_bg_scaled.left = 0;
        if (rect_bg_scaled.top < 0)
            rect_bg_scaled.top = 0;
        rect_bg_scaled.bottom = bgHeight + rect_bg_scaled.top;
        rect_bg_scaled.right = bgWidth + rect_bg_scaled.left;
    }
    subwin->SetScale(1 / point_w, 1 / point_h);
    subwin->SetPosition((float)rect_bg_scaled.left / point_w, (float)rect_bg_scaled.top / point_h);
    if (pRectOnScreen)
        *pRectOnScreen = { (LONG)(rect_bg_scaled.left / point_w), (LONG)(rect_bg_scaled.top / point_h), (LONG)(rect_bg_scaled.right / point_w), (LONG)(rect_bg_scaled.bottom / point_h) };
    if (pPoint_W)
        *pPoint_W = point_w;
    if (pPoint_H)
        *pPoint_H = point_h;

    return true;
}


//______________________
LONG Get_GameWin_Width() {
    WinStruct* gameWin = fall_Win_Get(*pWinRef_GameArea);
    if (gameWin) 
        return rcGame_GUI.right - rcGame_GUI.left;
    return 0;
}


//_______________________
LONG Get_GameWin_Height() {
    WinStruct* gameWin = fall_Win_Get(*pWinRef_GameArea);
    if (gameWin)
        return rcGame_GUI.bottom - rcGame_GUI.top;
    return 0;
}


//_____________________________________________________________________________________
LONG Win_Create_CenteredOnGame(DWORD width, DWORD height, DWORD colour, DWORD winFlags) {
    LONG x = 0, y = 0;

    WinStruct* gameWin = fall_Win_Get(*pWinRef_GameArea);
    if (!gameWin || (gameWin->flags & FLG_WinHidden)) {
        x = ((LONG)SCR_WIDTH - (LONG)width) / 2;
        y = (LONG)(SCR_HEIGHT - (LONG)height) / 2;
    }
    else {
        x = ((rcGame_GUI.right - rcGame_GUI.left) - (LONG)width) / 2;
        y = ((rcGame_GUI.bottom - rcGame_GUI.top) - (LONG)height) / 2;
    }
    if (y < 0)
        y = 0;
    LONG winRef = fall_Win_Create(x, y, width, height, colour, winFlags);

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (pWin && pWin->winDx) {
        pWin->winDx->Set_OnScreenResizeFunction(&OnScreenResize_Centred_On_GameWin);
    }

    return winRef;
}


//__________________________________________________________
void OnScreenResize_Centred_On_GameWin(Window_DX* pWin_This) {
    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;
    LONG winX = 0;
    LONG winY = 0;

    WinStruct* gameWin = fall_Win_Get(*pWinRef_GameArea);
    if (gameWin && !(gameWin->flags & FLG_WinHidden)) {
        winX = ((rcGame_GUI.right - rcGame_GUI.left) - (pWin->rect.right - pWin->rect.left)) / 2;
        winY = ((rcGame_GUI.bottom - rcGame_GUI.top) - (pWin->rect.bottom - pWin->rect.top)) / 2;
    }
    else {
        winX = ((LONG)SCR_WIDTH - (pWin->rect.right - pWin->rect.left)) / 2;
        winY = ((LONG)SCR_HEIGHT - (pWin->rect.bottom - pWin->rect.top)) / 2;
    }
    if (winY < 0)
        winY = 0;
    MoveWindowX(pWin, winX, winY);
}


//_________________________________________________________
void OnScreenResize_Centred_On_Screen(Window_DX* pWin_This) {
    WinStructDx* pWin = (WinStructDx*)pWin_This->Get_FalloutParent();
    if (pWin == nullptr)
        return;

    LONG winX = ((LONG)SCR_WIDTH - (pWin->rect.right - pWin->rect.left)) / 2;
    LONG winY = ((LONG)SCR_HEIGHT - (pWin->rect.bottom - pWin->rect.top)) / 2;

    MoveWindowX(pWin, winX, winY);
}


//________________________________________________________________________________________
void OnScreenResize_Window(WinStructDx* pWin, LONG prevScreenWidth, LONG prevScreenHeight) {

    if (!pWin)
        return;
    if (pWin->ref == -1)
        return;
    //if (pWin->ref == *pWinRef_GameArea)
    //    return;
    if (!pWin->winDx)
        return;
    if (pWin->winDx->Run_OnScreenResizeFunction())
        return;

    int shiftX = 0;
    int shiftY = 0;

    if (pWin->width <= 640 && SCR_WIDTH != prevScreenWidth) {
        shiftX = (LONG)(prevScreenWidth / 2) - (LONG)(SCR_WIDTH / 2);
        pWin->rect.left -= shiftX;
        pWin->rect.right -= shiftX;
    }

    if (pWin->height <= 480 && SCR_HEIGHT != prevScreenHeight) {
        shiftY = (prevScreenHeight / 2) - (SCR_HEIGHT / 2);
        pWin->rect.top -= shiftY;
        pWin->rect.bottom -= shiftY;
    }

    if (pWin->rect.right >= (LONG)SCR_WIDTH) {
        shiftX = pWin->rect.right - (LONG)SCR_WIDTH;
        pWin->rect.left -= shiftX;
        pWin->rect.right -= shiftX;
    }
    else if (pWin->rect.left < 0) {
        pWin->rect.left = 0;
        pWin->rect.right = pWin->rect.left + pWin->width - 1;
    }

    if (pWin->rect.bottom >= (LONG)SCR_HEIGHT) {
        shiftY = pWin->rect.bottom - SCR_HEIGHT;
        pWin->rect.top -= shiftY;
        pWin->rect.bottom -= shiftY;
    }
    else if (pWin->rect.top < 0) {
        pWin->rect.top = 0;
        pWin->rect.bottom = pWin->rect.top + pWin->height - 1;
    }

    MoveWindowX(pWin, pWin->rect.left, pWin->rect.top);
}


//______________________________________________________________________
void OnScreenResize_Windows(LONG prevScreenWidth, LONG prevScreenHeight) {
    //ResizeGameWin();
    for (int i = 1; i < *p_num_windows; i++)
        OnScreenResize_Window((WinStructDx*)pWin_Array[i], prevScreenWidth, prevScreenHeight);
    if (pWindow_Dx_Subtitles)
        pWindow_Dx_Subtitles->Run_OnScreenResizeFunction();
}


//_____________________________________________________
void __declspec(naked) allocate_mem_for_dx_window(void) {

    __asm {
        add eax, 0x4
        push eax
        call fall_Mem_Allocate
        mov dword ptr ds : [eax + 0x44] , 0//set pWin->winDx to null
        add esp, 0x4
        ret
    }
}


//____________________________________________________
void CreateDxWindow(LONG winRef, LONG xPos, LONG yPos) {

    WinStructDx* pWin = (WinStructDx*)fall_Win_Get(winRef);
    if (pWin->winDx)
        return;
    if (pWin->ref == *pWinRef_GameArea)
        return;

    pWin->rect.left = xPos;
    pWin->rect.top = yPos;
    pWin->rect.right = xPos + pWin->width - 1;
    pWin->rect.bottom = yPos + pWin->height - 1;

    pWin->winDx = new Window_DX((float)pWin->rect.left, (float)pWin->rect.top, pWin->width, pWin->height, 0x000000FF, nullptr, nullptr);
    pWin->winDx->Set_FalloutParentWindow(pWin);
}


//____________________________________________
void __declspec(naked) create_dx_window(void) {

    __asm {
        pushad
        //push 1
        push ebx//y
        push edx//x
        push eax//winRef
        call CreateDxWindow
        add esp, 0xC
        popad
        ret
    }
}


//_____________________________________
void DestroyDxWindow(WinStructDx* pWin) {
    if (!pWin)
        return;
    if (pWin->winDx)
        delete pWin->winDx;
    pWin->winDx = nullptr;

    fall_Mem_Deallocate(pWin);
}


//____________________________________________
void __declspec(naked) destroy_dx_window(void) {

    __asm {
        pushad
        push eax//winRef
        call DestroyDxWindow
        add esp, 0x4
        popad
        ret
    }
}


//____________________________________________
void ButtonDX_setup(ButtonStruct_DX* buttonDX) {
    if (!buttonDX)
        return;
    buttonDX->buttDx = nullptr;
    return;
}


//__________________________________________
void __declspec(naked) button_dx_setup(void) {

    __asm {
        mov eax, 0x80///size of ButtonStruct_DX structure
        push eax
        call fall_Mem_Allocate
        add esp, 0x4
        pushad
        push eax//*button
        call ButtonDX_setup
        add esp, 0x4
        popad
        ret
    }
}


//______________________________________________
void ButtonDX_destroy(ButtonStruct_DX* buttonDX) {
    if (!buttonDX)
        return;
    if (buttonDX->buttDx)
        delete buttonDX->buttDx;
    buttonDX->buttDx = nullptr;
    return;
}


//_____________________________________________
void __declspec(naked) button_dx_destroy(void) {

    __asm {
        pushad
        push eax//*button
        call ButtonDX_destroy
        add esp, 0x4
        popad

        mov edx, eax
        test byte ptr ds : [eax + 0x6] , 0x01
        ret
    }
}


//needed to pass vars to button functions
//_________________________________________________________________________
void ButtonDx_FuncRunner(void (*buttFunc)(), DWORD buttRef, DWORD pressRef) {

    __asm {
        pushad
        mov edx, pressRef
        mov eax, buttRef
        call buttFunc
        popad
    }
}


//____________________________________________________________________________________________________________________________
LONG ButtonDx_DrawX(ButtonStruct_DX* pButton, WinStructDx* pWin, BYTE* fBuff, DWORD drawFlag, RECT* areaRect, DWORD soundFlag) {
    if (pWin == nullptr)
        return -1;
    BYTE* pBuff = fBuff;
    BYTE* lastCurrentBuff = nullptr;

    if (pBuff != nullptr) {
        RECT rbutt_win;
        CopyRect(&rbutt_win, &pButton->rect);
        RECT rbutt_scrn = { rbutt_win.left + pWin->rect.left, rbutt_win.top + pWin->rect.top, rbutt_win.right + pWin->rect.left, rbutt_win.bottom + pWin->rect.top };
        if (areaRect != nullptr) {
            if (IntersectRect(&rbutt_scrn, areaRect, &rbutt_scrn) == false)
                return -1;
            rbutt_win.left = rbutt_scrn.left - pWin->rect.left;
            rbutt_win.top = rbutt_scrn.top - pWin->rect.top;
            rbutt_win.right = rbutt_scrn.right - pWin->rect.left;
            rbutt_win.bottom = rbutt_scrn.bottom - pWin->rect.top;
        }
        if (pBuff == pButton->buffUp && (pButton->flags & FLG_ButtTglDn))
            pBuff = pButton->buffDn;
        if ((pButton->flags & FLG_ButtDisabled)) {//set buff to disabled variant if this flag set.
            if (pBuff == pButton->buffUp)
                pBuff = pButton->buffUpDis;
            else if (pBuff == pButton->buffDn)
                pBuff = pButton->buffDnDis;
            else if (pBuff == pButton->buffHv)
                pBuff = pButton->buffHvDis;
        }
        else {//otherwise make sure buff in not set to disabled variant.
            if (pBuff == pButton->buffUpDis)
                pBuff = pButton->buffUp;
            else if (pBuff == pButton->buffDnDis)
                pBuff = pButton->buffDn;
            else if (pBuff == pButton->buffHvDis)
                pBuff = pButton->buffHv;
        }
        if (pBuff != nullptr) {
            if (drawFlag == 0 && !pButton->buttDx) {
                DWORD buttonWidth = pButton->rect.right - pButton->rect.left + 1;
                DWORD fromWidth = rbutt_win.right - rbutt_win.left + 1;
                DWORD fromHeight = rbutt_win.bottom - rbutt_win.top + 1;
                BYTE* toBuff = pWin->buff + rbutt_win.top * pWin->width + rbutt_win.left;
                BYTE* fromBuff = fBuff + ((rbutt_win.top - pButton->rect.top) * buttonWidth) + (rbutt_win.left - pButton->rect.left);
                if ((pButton->flags & FLG_ButtTrans))
                    MemBltMasked8(fromBuff, fromWidth, fromHeight, buttonWidth, toBuff, pWin->width);
                else
                    MemBlt8(fromBuff, fromWidth, fromHeight, buttonWidth, toBuff, pWin->width);
            }
            lastCurrentBuff = pButton->buffCurrent;
            pButton->buffCurrent = pBuff;

            RefreshButtonStateX(pButton);
            if (drawFlag != 0 && !pButton->buttDx)
                Draw_Window_OLD(pWin, &rbutt_scrn, nullptr);
        }
    }
    if (soundFlag) {
        if (pBuff != lastCurrentBuff) {
            if (pBuff == pButton->buffDn) {
                if (pButton->funcDnSnd != nullptr)
                    ButtonDx_FuncRunner(pButton->funcDnSnd, pButton->ref, pButton->refDn);
            }
            else if (pBuff == pButton->buffUp) {
                if (pButton->funcUpSnd != nullptr)
                    ButtonDx_FuncRunner(pButton->funcUpSnd, pButton->ref, pButton->refUp);
            }
        }
    }
    return 0;
}


//________________________________________
void __declspec(naked) buttonDX_draw(void) {

    __asm {
        push esi
        push edi
        push ebp
        push dword ptr ss : [esp + 0x14]//playSoundFlag
        push dword ptr ss : [esp + 0x14]//pRect
        push ecx
        push ebx
        push edx
        push eax
        call ButtonDx_DrawX
        add esp, 0x18
        pop ebp
        pop edi
        pop esi
        ret 0x8
    }
}



//____________________________
void Modifications_Dx_Window() {


    if (fallout_exe_region == EXE_Region::Chinese) {
        FuncReplace32(0x4DC11E, 0xFFFE8679, (DWORD)&allocate_mem_for_dx_window);

        FuncReplace32(0x4DBC34, 0xFFFE8B63, (DWORD)&allocate_mem_for_dx_window);

        FuncReplace32(0x4DC2E2, 0x0E73, (DWORD)&create_dx_window);

        FuncReplace32(0x4DC3DF, 0x1B80, (DWORD)&destroy_dx_window);

        FuncReplace32(0x4DDB70, 0x0349, (DWORD)&draw_fallout_windows_rect);

        MemWrite8(0x4DD35B, 0x51, 0xE9);
        FuncWrite32(0x4DD35C, 0x89555756, (DWORD)&draw_window_old);
        MemWrite8(0x4DD360, 0xE5, 0x90);

        //To-Do Modifications_Dx_Window Chinese
    }
    else {
        //Dx9Windows=(Dx9Win**)FixAddress(0x6ADE58);
        //winStack=(DWORD*)FixAddress(0x6ADD90);
        //p_num_windows=(int*)FixAddress(0x6ADF24);

        //allocate memory for new window
        FuncReplace32(0x4D6287, 0xFFFEF845, (DWORD)&allocate_mem_for_dx_window);

        //allocate memory for the first (primary) window in the window array
        FuncReplace32(0x4D5E6E, 0xFFFEFC5E, (DWORD)&allocate_mem_for_dx_window);

        FuncReplace32(0x4D63CB, 0x0AD1, (DWORD)&create_dx_window);

        FuncReplace32(0x4D654E, 0xFFFEF6D2, (DWORD)&destroy_dx_window);

        FuncReplace32(0x4D75A9, 0x0267, (DWORD)&draw_fallout_windows_rect);

        //sfall stomped on this "GNW_win_refresh_hack"
        //MemWrite8(0x4D6FD8, 0x51, 0xE9);
        //FuncWrite32(0x4D6FD9, 0x83555756,  (DWORD)&draw_window_old);
        //MemWrite16(0x4D6FDD, 0x34EC, 0x9090);

        ///allocate mem for button struct
        FuncReplace32(0x4D88A8, 0xFFFED224, (DWORD)&button_dx_setup);

        ///in DESTROY_BUTTON_MEM(EAX *button);
        MemWrite16(0x4D937A, 0xC289, 0xE890);
        FuncWrite32(0x4D937C, 0x010640F6, (DWORD)&button_dx_destroy);


        //stops sfall from calling its GNW_win_refresh function - 
        MemWrite8(0x4D6FD8, 0x51, 0xC3);
        //sfall work around "GNW_win_refresh_hack" - override all draw win calls
        //not called
        FuncReplace32(0x4D6120, 0x0EB4, (DWORD)&draw_window_old);
        //not called
        FuncReplace32(0x4D6842, 0x0792, (DWORD)&draw_window_old);
        //win print text
        FuncReplace32(0x4D69CD, 0x0607, (DWORD)&ddraw_win_dont);
        //show win
        FuncReplace32(0x4D6DEF, 0x01E5, (DWORD)&ddraw_win_dont);
        //show win
        FuncReplace32(0x4D6E59, 0x017B, (DWORD)&ddraw_win_dont);
        //set win pos
        FuncReplace32(0x4D6F3D, 0x97, (DWORD)&ddraw_win_dont);
        //draw win
        FuncReplace32(0x4D6F76, 0x5E, (DWORD)&draw_window_old);
        //draw win area
        FuncReplace32(0x4D6FCA, 0x0A, (DWORD)&draw_window_old);
        //func called from draw_window_old
        FuncReplace32(0x4D75FC, 0xFFFFF9D8, (DWORD)&draw_window_old);
        //drawing buttons?
        FuncReplace32(0x4D76F3, 0xFFFFF8E1, (DWORD)&ddraw_win_dont);
        //draw windows
        FuncReplace32(0x4D7841, 0xFFFFF793, (DWORD)&ddraw_win_dont);
        //draw button hover off
        FuncReplace32(0x4D9A0D, 0xFFFFD5C7, (DWORD)&ddraw_win_dont);
        //dialog list box - load map list
        FuncReplace32(0x4DB2BA, 0xFFFFBD1A, (DWORD)&draw_window_old);
        //dialog list box - load map list
        FuncReplace32(0x4DB3E0, 0xFFFFBBF4, (DWORD)&draw_window_old);
        //dialog list box - load map list
        FuncReplace32(0x4DB453, 0xFFFFBB81, (DWORD)&draw_window_old);
        //edit text win
        FuncReplace32(0x4DCB38, 0xFFFFA49C, (DWORD)&draw_window_old);
        //edit text win
        FuncReplace32(0x4DCBE9, 0xFFFFA3EB, (DWORD)&draw_window_old);
        //edit text win
        FuncReplace32(0x4DCC74, 0xFFFFA360, (DWORD)&draw_window_old);
        //edit text win
        FuncReplace32(0x4DCD3A, 0xFFFFA29A, (DWORD)&draw_window_old);


        FuncReplace32(0x4C8FB4, 0xE5E4, (DWORD)&ddraw_win_dont);
        FuncReplace32(0x4CA113, 0xD485, (DWORD)&ddraw_win_dont);
        FuncReplace32(0x4CA58E, 0xD00A, (DWORD)&ddraw_win_dont);
        FuncReplace32(0x4CA8A2, 0xCCF6, (DWORD)&ddraw_win_dont2);
        FuncReplace32(0x4CAAFC, 0xCA9C, (DWORD)&ddraw_win_dont);


        MemWrite16(0x4D9808, 0x5756, 0xE990);
        FuncWrite32(0x4D980A, 0x2CEC8355, (DWORD)&buttonDX_draw);
    }

}
