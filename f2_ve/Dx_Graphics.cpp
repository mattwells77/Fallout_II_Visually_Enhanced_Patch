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
#include "Dx_Graphics.h"
#include "Fall_General.h"

//________________________________________________________________________________________________________________________________________________
FRMframeDx::FRMframeDx(FRMdx* p_in_frm_parent, DWORD inWidth, DWORD inHeight, BYTE* indexBuff, DWORD* pAltPal, bool is32bit, BOOL widenForOutline) :
    BASE_VERTEX_DX(),
    BASE_TEXTURE_STAGING(0x00000000)
{
    p_frm_parent = p_in_frm_parent;
    isAnimated = 0;
    ani_TimeZones = 0;
    ani_Rect = { 0,0,0,0 };
    lockType = 0;

    x_offset_from_previous_frame = 0;
    y_offset_from_previous_frame = 0;
    x_offset_from_first_frame = 0;
    y_offset_from_first_frame = 0;
    x_offset_ori_centre = 0;
    y_offset_ori_centre = 0;

    pTex_Base = nullptr;
    pTex_Base_shaderResourceView = nullptr;

    tex_widened = widenForOutline;

    //widened frames by +2 pixels and centered -1, to allow for shader outlining.
    if (tex_widened) {
        inWidth += 2;
        inHeight += 2;
    }
    SetBaseDimensions(inWidth, inHeight);

    pixelSize.x = 1.0f / (float)width;
    pixelSize.y = 1.0f / (float)height;

    DXGI_FORMAT dxgi_Format = DXGI_FORMAT_R8_UNORM;
    if (pAltPal || is32bit)
        dxgi_Format = DXGI_FORMAT_B8G8R8A8_UNORM;

    frameCount++;
    if (!CreateVerticies())
        Fallout_Debug_Error("FRMframeDx - Failed FRMframeDx CreateVerticies.");
    if (!Texture_Initialize(dxgi_Format, false, true))
        Fallout_Debug_Error("FRMframeDx - Failed FRMframeDx CreateTexture.");
    Clear_Staging();
    if (indexBuff)
        Draw(indexBuff, pAltPal);
}


//____________________________________
void FRMframeDx::DestroyBaseTextures() {
    if (pTex_Base)
        pTex_Base->Release();
    pTex_Base = nullptr;
    if (pTex_Base_shaderResourceView)
        pTex_Base_shaderResourceView->Release();
    pTex_Base_shaderResourceView = nullptr;
}


//______________________________________________________________________________________________________________________________________________________________________
void FRMframeDx::Draw8(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD to_pitchBytes) {
    toBuff += tY * to_pitchBytes + tX;
    fBuff += fY * fWidth + fX;

    for (LONG y = 0; y < (LONG)subHeight; y++) {
        for (LONG x = 0; x < (LONG)subWidth; x++) {

            toBuff[x] = fBuff[x];

            if (fBuff[x] > PAL_ANI_START_1 && fBuff[x] <= PAL_ANI_START_6) {
                if (!isAnimated)
                    ani_Rect = { x,y,x + 1,y + 1 };
                if (x < ani_Rect.left)
                    ani_Rect.left = x;
                else if (x > ani_Rect.right)
                    ani_Rect.right = x;
                if (y < ani_Rect.top)
                    ani_Rect.top = y;
                else if (y > ani_Rect.bottom)
                    ani_Rect.bottom = y;

                isAnimated = true;
                if (fBuff[x] == PAL_ANI_START_6)//red/black - alarm
                    ani_TimeZones |= PAL_TIME_ZONE_4;
                else if (fBuff[x] >= PAL_ANI_START_5)//brown - shoreline
                    ani_TimeZones |= PAL_TIME_ZONE_1;
                else if (fBuff[x] >= PAL_ANI_START_4)//orang/red - fast fire
                    ani_TimeZones |= PAL_TIME_ZONE_2;
                else if (fBuff[x] >= PAL_ANI_START_3)//orang/red - slow fire
                    ani_TimeZones |= PAL_TIME_ZONE_1;
                else if (fBuff[x] >= PAL_ANI_START_2)//blue/grey - monitors | water
                    ani_TimeZones |= PAL_TIME_ZONE_3;
                else if (fBuff[x] >= PAL_ANI_START_1)//green - slime
                    ani_TimeZones |= PAL_TIME_ZONE_1;
            }
        }
        toBuff += to_pitchBytes;
        fBuff += fWidth;
    }
}


//______________________________________________________________________________________________________________________________________________________________________
void FRMframeDx::Draw32(BYTE* fBuff, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD to_pitchBytes) {
    toBuff += tY * to_pitchBytes + tX * 4;
    DWORD* frBuff;
    frBuff = (DWORD*)fBuff;
    frBuff += fY * fWidth + fX;

    for (DWORD y = 0; y < subHeight; y++) {
        for (DWORD x = 0; x < subWidth; x++)
            ((DWORD*)toBuff)[x] = frBuff[x];
        toBuff += to_pitchBytes;
        frBuff += fWidth;
    }
}


//______________________________________________________________________________________________________________________________________________________________________________________
void FRMframeDx::DrawPal32(BYTE* fBuff, DWORD* pal, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, BYTE* toBuff, LONG tX, LONG tY, DWORD to_pitchBytes) {
    toBuff += tY * to_pitchBytes + tX * 4;
    fBuff += fY * fWidth + fX;

    for (DWORD y = 0; y < subHeight; y++) {
        for (DWORD x = 0; x < subWidth; x++) {
            ((DWORD*)toBuff)[x] = pal[fBuff[x]];
            /// alpha of mask colour now set in function "LoadFrmFromID" in "F_Art.cpp".
        }
        toBuff += to_pitchBytes;
        fBuff += fWidth;
    }
}


//________________________________________________
void FRMframeDx::Draw(BYTE* indexBuff, DWORD* pal) {

    if (pTex == nullptr)
        return;
    DWORD buff_width = width;
    DWORD buff_height = height;
    LONG to_Offset = 0;

    //if texture size has been increase by 2 pixels - to allow for shader outline
    if (tex_widened) {
        buff_width -= 2;
        buff_height -= 2;
        to_Offset = 1;
    }

    BYTE* tbuff = nullptr;
    UINT pitchBytes = 0;
    Lock((void**)&tbuff, &pitchBytes, D3D11_MAP_WRITE);
    if (pixelWidth == 1)
        Draw8(indexBuff, buff_width, buff_height, 0, 0, buff_width, buff_height, tbuff, to_Offset, to_Offset, pitchBytes);
    else if (pixelWidth == 4) {
        if (pal)
            DrawPal32(indexBuff, pal, buff_width, buff_height, 0, 0, buff_width, buff_height, tbuff, to_Offset, to_Offset, pitchBytes);
        else
            Draw32(indexBuff, buff_width, buff_height, 0, 0, buff_width, buff_height, tbuff, to_Offset, to_Offset, pitchBytes);
    }
    RECT rect{ to_Offset , to_Offset , to_Offset + (LONG)buff_width , to_Offset + (LONG)buff_height };
    Unlock(&rect);
}


//___________________________________________________________________________________________________________________________________________________________________________
void FRMframeDx::DrawToFrame(BYTE* fBuff, DWORD* fpal, bool is_fBuff_32bit, DWORD fWidth, DWORD fHeight, LONG fX, LONG fY, DWORD subWidth, DWORD subHeight, LONG tX, LONG tY) {

    if (pTex == nullptr)
        return;
    if (fBuff == nullptr)
        return;
    if (tX < 0 || tX >(LONG)width - 1)
        return;
    if (tY < 0 || tY >(LONG)height - 1)
        return;
    if (fX < 0 || fX + subWidth > fWidth)
        return;
    if (fY < 0 || fY + subHeight > fHeight)
        return;

    if (tX + subWidth > width)
        subWidth = width - tX;
    if (tY + subHeight > height)
        subHeight = height - tY;

    RECT rect = { tX, tY,static_cast<long>(tX + subWidth),static_cast<long>(tY + subHeight) };

    if ((LONG)subWidth - fX > rect.right - rect.left) {
        Fallout_Debug_Error("FRMframeDx::DrawToFrame - draw fail x");
        return;
    }
    if ((LONG)subHeight - fY > rect.bottom - rect.top) {
        Fallout_Debug_Error("FRMframeDx::DrawToFrame - draw fail y");
        return;
    }

    BYTE* tbuff;
    UINT pitchBytes = 0;
    Lock((void**)&tbuff, &pitchBytes, D3D11_MAP_WRITE);
    if (pixelWidth == 1)
        Draw8(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, tbuff, tX, tY, pitchBytes);
    else if (pixelWidth == 4) {
        if (!fpal && !is_fBuff_32bit) {
            if (color_pal) {
                fpal = nullptr;// (DWORD*)color_pal->GetBuff();
                if (SUCCEEDED(color_pal->Lock((DWORD**)&fpal, D3D11_MAP_READ))) {
                    DrawPal32(fBuff, fpal, fWidth, fHeight, fX, fY, subWidth, subHeight, tbuff, tX, tY, pitchBytes);
                    color_pal->Unlock(0, 0);
                }
                fpal = nullptr;
            }
        }
        else if (fpal) {
            DrawPal32(fBuff, fpal, fWidth, fHeight, fX, fY, subWidth, subHeight, tbuff, tX, tY, pitchBytes);
        }
        else
            Draw32(fBuff, fWidth, fHeight, fX, fY, subWidth, subHeight, tbuff, tX, tY, pitchBytes);
    }
    Unlock(&rect);
}


//__________________________________________________
//create a texture which roughly holds the base "feet" of the frame, used for creating shadows.
//reduceFauxShadows - effects default pal frms, attemps to exclude the shadow already present at the base of most sprites by ignoring dark pal colours.
void FRMframeDx::DrawBaseTex(bool reduceFauxShadows) {

    if (pD3DDev == nullptr)
        return;
    if (pTex == nullptr)
        return;
    if (pTex_Base != nullptr)
        return;

    LONG* bHeight = new LONG[width];
    for (DWORD i = 0; i < width; i++)
        bHeight[i] = 0;

    BYTE* mainbuff = nullptr;

    UINT pitch = 0;
    Lock((void**)&mainbuff, &pitch, D3D11_MAP_READ);
    bool ignorePixel = false;

    //list of dark colours, default pal offsets.
    BYTE darkPixel[] = { 0x00,
                        0x0C, 0x0D, 0x0E, 0x0F,
                        0x1F,
                        0x2F,
                        0x35, 0x36, 0x37,
                        0x44,
                        0x63, 0x64,
                        0xCA, 0xCF,
                        0xE3, 0xE4 };
    int darkSize = 1;//always check first item in list, it is the mask offset 0x00;
    if (reduceFauxShadows)
        darkSize = sizeof(darkPixel);

    //get height of lowest pixel for each column in frame
    if (pixelWidth == 1) {
        for (LONG yPos = 0; yPos < (int)height; yPos++) {

            for (DWORD xPos = 0; xPos < width; xPos++) {
                ignorePixel = false;
                for (int i = 0; i < darkSize; i++) {
                    if (darkPixel[i] == mainbuff[xPos])
                        ignorePixel = true, i = darkSize;
                }
                if (!ignorePixel) {
                    if (bHeight[xPos] < yPos)
                        bHeight[xPos] = yPos;
                }
            }
            mainbuff += pitch;
        }
    }
    else if (pixelWidth == 4) {
        for (LONG yPos = 0; yPos < (int)height; yPos++) {

            for (DWORD xPos = 0; xPos < width; xPos++) {
                if (!(((DWORD*)mainbuff)[xPos] & 0xFF000000)) {
                    if (bHeight[xPos] < yPos)
                        bHeight[xPos] = yPos;
                }
            }
            mainbuff += pitch;
        }
    }
    Unlock(nullptr);

    D3D11_TEXTURE2D_DESC textureDesc;
    HRESULT result;
    // Initialize the render target texture description.
    ZeroMemory(&textureDesc, sizeof(textureDesc));

    // Setup the render target texture description.
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the render target texture.
    result = pD3DDev->CreateTexture2D(&textureDesc, nullptr, &pTex_Base);
    if (FAILED(result))
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    // Setup the description of the shader resource view.
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    // Create the shader resource view.
    result = pD3DDev->CreateShaderResourceView(pTex_Base, &shaderResourceViewDesc, &pTex_Base_shaderResourceView);
    if (FAILED(result))
        return;

    BYTE* pbaseBuff = new BYTE[width * height];
    memset(pbaseBuff, 0, width * height);

    BYTE* baseBuff = nullptr;
    baseBuff = (BYTE*)pbaseBuff;


    LONG minHeight = height - height / 3;
    //try to reduce the height of base for slim sprites, looks better.
    if (reduceFauxShadows) {
        int left = -1, right = width - 1;

        for (int xPos = 0; xPos < (int)width; xPos++) {
            if (bHeight[xPos] > minHeight && left == -1)
                left = xPos;
            else if (bHeight[xPos] > minHeight && left != -1)
                right = xPos;

        }
        int roughWidth = right - left;
        if (roughWidth < 4)
            roughWidth = 4;
        if (minHeight < (int)height - roughWidth)
            minHeight = (int)height - roughWidth;
    }

    //join the lowest pixels into a line across the base texture
    for (int xPos = 0; xPos < (int)width; xPos++) {
        if (bHeight[xPos] > minHeight)
            baseBuff[bHeight[xPos] * width + xPos] = 0xFF;
        if (xPos < (int)width - 1 && bHeight[xPos]>minHeight && bHeight[xPos + 1] > minHeight) {
            int diff = abs((int)bHeight[xPos] - (int)bHeight[xPos + 1]);
            if (diff > 1) {
                int xLow = xPos, xHigh = xPos + 1;
                if (bHeight[xPos] > bHeight[xPos + 1])
                    xLow = xPos + 1, xHigh = xPos;
                for (int d = 0; d < diff; d++) {
                    baseBuff[(bHeight[xLow] + d) * width + xHigh] = 0xFF;
                }
            }
        }
    }

    D3D11_BOX destRegion;
    destRegion.left = 0;
    destRegion.right = width;
    destRegion.top = 0;
    destRegion.bottom = height;
    destRegion.front = 0;
    destRegion.back = 1;
    //ID3D11DeviceContext* pd3dDeviceContext = GetD3dDeviceContext();
    pD3DDevContext->UpdateSubresource(pTex_Base, 0, &destRegion, pbaseBuff, width, 0);

    delete[] bHeight;
    delete[] pbaseBuff;
}


//_____________________________________________________________________________
void FRMframeDx::DrawFrame(DirectX::XMMATRIX* pOrtho2D, float xPos, float yPos) {
    MATRIX_DATA posData;

    posData.World = DirectX::XMMatrixTranslation(xPos, yPos, 0.0f);
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D);

    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();
    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

    //set pixel shader stuff
    if (pixelWidth == 1) {
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);
    }
    else {
        pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);
    }

    pD3DDevContext->DrawIndexed(4, 0, 0);
}


//__________________________________________________________________________________________________________________
void FRMframeDx::DrawFrame(DirectX::XMMATRIX* pOrtho2D, float xPos, float yPos, ID3D11PixelShader* pd3d_PixelShader) {
    MATRIX_DATA posData;

    posData.World = DirectX::XMMatrixTranslation(xPos, yPos, 0.0f);
    posData.WorldViewProjection = XMMatrixMultiply(posData.World, *pOrtho2D);

    pPS_BuffersFallout->UpdatePositionBuff(&posData);
    pPS_BuffersFallout->SetPositionRender();
    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PixelShader, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);
}



///--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//_______FOR_LOADING_UNLISTED_FRM_ONLY_-_MANUALY_DELETE_AFTER_USE_________
FRMdx::FRMdx(const char* FrmName, LONG in_type, LONG in_palOffset_Mask) {//
    for (int ori = 0; ori < 6; ori++) {
        xCentreShift[ori] = 0;
        yCentreShift[ori] = 0;
        lpFrame[ori] = nullptr;
    }
    numFrames = 0;
    version = 0;//version num
    FPS = 0;//frames per sec
    actionFrame = 0;
    tex_widened = FALSE;
    uniformly_lit = false;
    palOffset_Mask = in_palOffset_Mask;
    type = in_type;
    UNLSTDfrm* frm = LoadUnlistedFrm(FrmName, type, palOffset_Mask);
    bool framesAdded = false;
    if (!frm) {
        ///imonitorInsertText("could not load frm");
        return;
    }
    if (!frm->numFrames)
        return;
    if (numFrames == 0)
        numFrames = frm->numFrames;
    else if (numFrames != frm->numFrames) {
        ///imonitorInsertText("numFrames mismatch");
        return;
    }
    version = frm->version;
    FPS = frm->FPS;
    actionFrame = frm->actionFrame;

    if (frm->uniformly_lit)
        uniformly_lit = true;

    int numOri = 6;
    if (frm->oriOffset[0] == frm->oriOffset[1])
        numOri = 1;
    for (int ori = 0; ori < numOri; ori++) {
        xCentreShift[ori] = frm->xCentreShift[ori];
        yCentreShift[ori] = frm->yCentreShift[ori];
        if (frm->frames[ori]) {
            if (AddFrames(frm, ori, ori))
                framesAdded = true;
        }
    }

    delete frm;
    frm = nullptr;
    return;
}


//_______________________
FRMdx::FRMdx(DWORD frmID) {

    for (int ori = 0; ori < 6; ori++) {
        xCentreShift[ori] = 0;
        yCentreShift[ori] = 0;
        lpFrame[ori] = nullptr;
    }
    numFrames = 0;
    version = 0;//version num
    FPS = 0;//frames per sec
    actionFrame = 0;
    type = (frmID & 0x0F000000) >> 24;
    if (type == ART_HEADS) {
        DWORD num = frmID & 0x00000FFF;
        DWORD ID2 = (frmID & 0x00FF0000) >> 16;
        DWORD ID1 = (frmID & 0x0000F000) >> 12;
        //Fallout_Debug_Info("talking head num_%d id1_%d id2_%d", num, ID2, ID1);
    }
    tex_widened = FALSE;
    if (type <= 5)//widen map frms so they can be outlined in pixel shader, types 5 and below.
        tex_widened = TRUE;
    if (frmID == 0x06000001)//widen mouse hex frm as well, which is of the interface type.
        tex_widened = TRUE;

    uniformly_lit = false;
    palOffset_Mask = 0;

    Add(frmID);
    return;
}


//_______________________________________________
FRMdx::FRMdx(DWORD frmID, LONG in_palOffset_Mask) {

    for (int ori = 0; ori < 6; ori++) {
        xCentreShift[ori] = 0;
        yCentreShift[ori] = 0;
        lpFrame[ori] = nullptr;
    }
    numFrames = 0;
    version = 0;//version num
    FPS = 0;//frames per sec
    actionFrame = 0;
    type = (frmID & 0x0F000000) >> 24;
    tex_widened = FALSE;
    if (type <= 5)//widen map frms so they can be outlined in pixel shader, types 5 and below.
        tex_widened = TRUE;
    if (frmID == 0x06000001)//widen mouse hex frm as well, which is of the interface type.
        tex_widened = TRUE;

    uniformly_lit = false;
    palOffset_Mask = in_palOffset_Mask;

    Add(frmID);
    return;
}


//_________________________________________________________
bool FRMdx::AddFrames(UNLSTDfrm* frm, int fileOri, int ori) {
    if (lpFrame[ori] != nullptr)
        return false;
    lpFrame[ori] = new FRMframeDx * [numFrames];
    UNLSTDframe* frame = nullptr;

    bool is32bitColour = false;
    if (frm->version == FRM_VER_32BIT)
        is32bitColour = true;

    LONG offset_frm_first_frame_X = 0;
    LONG offset_frm_first_frame_Y = 0;

    for (WORD n = 0; n < numFrames; n++) {
        frame = frm->frames[fileOri][n];
        if (frame) {
            offset_frm_first_frame_X += frame->x;
            offset_frm_first_frame_Y += frame->y;

            lpFrame[ori][n] = new FRMframeDx(this, frame->width, frame->height, frame->buff, frm->pPal, is32bitColour, tex_widened);

            lpFrame[ori][n]->SetOffsets_FromPreviousFrame(frame->x, frame->y);
            lpFrame[ori][n]->SetOffsets_FromFirstFrame(offset_frm_first_frame_X, offset_frm_first_frame_Y);
            
            lpFrame[ori][n]->SetOffsets_Centre(frm->xCentreShift[fileOri] - frame->width / 2 + 1 - tex_widened, frm->yCentreShift[fileOri] - frame->height + 1 - tex_widened);
        }
        else
            return false;
    }
    frame = nullptr;
    return true;
}


//__________________________
bool FRMdx::Add(DWORD frmID) {// returns false is frames already added or could not add frames.
    DWORD ID3 = (frmID & 0x70000000) >> 28;
    if (numFrames && !ID3)//only add frames to an existing(allready has numFrames) frm if has ID3
        return false;
    if (ID3) {
        int ori = (int)ID3 - 1;
        if (ori < 0 || ori >= 6)
            return false;
        if (lpFrame[ori] != nullptr)// if frames for this ori are already loaded
            return false;
    }

    UNLSTDfrm* frm = LoadFrmFromID(frmID, palOffset_Mask);

    bool framesAdded = false;
    if (!frm) {
        ///imonitorInsertText("could not load frm");
        return false;
    }
    if (!frm->numFrames)
        return false;
    if (numFrames == 0)
        numFrames = frm->numFrames;
    else if (numFrames != frm->numFrames) {
        ///imonitorInsertText("numFrames mismatch");
        return false;
    }
    version = frm->version;
    FPS = frm->FPS;
    actionFrame = frm->actionFrame;

    if (frm->uniformly_lit)
        uniformly_lit = true;

    if (ID3 != 0) {
        int ori = (int)ID3 - 1;
        if (ori >= 0 && ori < 6) {
            xCentreShift[ori] = frm->xCentreShift[0];
            yCentreShift[ori] = frm->yCentreShift[0];
            framesAdded = AddFrames(frm, 0, ori);
        }
        ///if(framesAdded)
        ///   imonitorInsertText("added id3 frames");
        ///else
        ///   imonitorInsertText("could not add id3 frames");
    }
    else {
        int numOri = 6;
        if (frm->oriOffset[0] == frm->oriOffset[1])
            numOri = 1;
        for (int ori = 0; ori < numOri; ori++) {
            xCentreShift[ori] = frm->xCentreShift[ori];
            yCentreShift[ori] = frm->yCentreShift[ori];
            if (frm->frames[ori]) {
                if (AddFrames(frm, ori, ori))
                    framesAdded = true;
            }
        }
    }
    delete frm;
    frm = nullptr;
    return framesAdded;
}


//___________________________________________________
FRMframeDx* FRMdx::GetFrame(DWORD ori, WORD frameNum) {
    if (lpFrame[ori] && frameNum < numFrames)
        return lpFrame[ori][frameNum];
    return nullptr;
}


///--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

FRMDXcache* frmDxCache = nullptr;

//_________________________
void Destroy_FRM_DX_CACHE() {
    if (frmDxCache)
        delete frmDxCache;
    frmDxCache = nullptr;
}
