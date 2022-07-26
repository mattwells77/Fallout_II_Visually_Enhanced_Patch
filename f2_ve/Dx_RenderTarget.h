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

class RenderTarget : public BASE_VERTEX_DX, public BASE_TEXTURE_DX, public BASE_POSITION_DX {
public:
    RenderTarget(float inX, float inY, DWORD inWidth, DWORD inHeight, DXGI_FORMAT in_dxgiFormat, DWORD in_bgColour) :
        BASE_VERTEX_DX(),
        BASE_TEXTURE_DX(in_bgColour),
        BASE_POSITION_DX(inX, inY, 1, false)
    {
        SetBaseDimensions(inWidth, inHeight);
        opaqueness = 1.0f;
        if (!CreateVerticies())
            Fallout_Debug_Error("Failed RenderTarget CreateVerticies.");
        if(!Texture_Initialize(in_dxgiFormat, true, false))
            Fallout_Debug_Error("Failed RenderTarget CreateTexture.");
        SetMatrices();
    };
    ~RenderTarget() {
    };
    void ClearRect(DirectX::XMFLOAT4 colour, RECT* pRect);
    void SetOpaqueness(float inOpaqueness) {
        opaqueness = inOpaqueness;
    };
    float GetOpaqueness() {
        return opaqueness;
    };
    void Display();
    void Display(ID3D11PixelShader* pd3d_PixelShader);
protected:
private:
    float opaqueness;
};


class RenderTarget2 : public BASE_VERTEX_DX_TILED, public BASE_TEXTURE_DX_TILED, public BASE_POSITION_DX_TILED {
public:
    RenderTarget2(float inX, float inY, DWORD inWidth, DWORD inHeight, DXGI_FORMAT in_dxgiFormat, DWORD in_bgColour) :
        BASE_VERTEX_DX_TILED(),
        BASE_TEXTURE_DX_TILED(in_dxgiFormat, in_bgColour, true),
        BASE_POSITION_DX_TILED(inX, inY, false)
    {
        SetBaseDimensions(inWidth, inHeight);

        opaqueness = 1.0f;
        if (!CreateTexture())
            Fallout_Debug_Error("Failed RenderTarget CreateTexture.");
        if (!CreateVerticies())
            Fallout_Debug_Error("Failed RenderTarget CreateVerticies.");
        if (!Create_ConstantBuffers(tiles_num))
            Fallout_Debug_Error("Failed RenderTarget Create_ConstantBuffers.");
        SetMatrices();
    };
    ~RenderTarget2() {
    };
    void SetOpaqueness(float inOpaqueness) {
        opaqueness = inOpaqueness;
    };
    float GetOpaqueness() {
        return opaqueness;
    };
    void Display(ID3D11PixelShader* pd3d_PixelShader, RenderTarget2* plightRT);
    void ClearRect(DirectX::XMFLOAT4 colour, RECT* pRect);
protected:

    virtual void SetMatrices() {
        MATRIX_DATA posData{};
        DirectX::XMMATRIX xmManipulation{};
        DirectX::XMMATRIX xmScaling{};

        DirectX::XMMATRIX* pOrtho2D_XM = nullptr;
        DirectX::XMMATRIX Ortho2D_XM{};
        if (pOrtho2D != nullptr) {
            Ortho2D_XM = XMLoadFloat4x4(pOrtho2D);
            pOrtho2D_XM = &Ortho2D_XM;
        }
        else//if no custom matrix get screen ortho matrix
            pOrtho2D_XM = GetScreenProjectionMatrix_XM();

        xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);

        float yPos = y;
        UINT tileLine = 0;
        for (UINT tileY = 0; tileY < tiles_y; tileY++) {
            float xPos = x;
            for (UINT tileX = 0; tileX < tiles_x; tileX++) {

                if (isPositionScaled)
                    xmManipulation = DirectX::XMMatrixTranslation(xPos * scaleX, yPos * scaleY, 0);
                else
                    xmManipulation = DirectX::XMMatrixTranslation(xPos, yPos, 0);
               
                posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
                posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D_XM);
                UpdatePositionData(tileLine + tileX, &posData);
                xPos += tile_width;
            }
            yPos += tile_height;
            tileLine += tiles_x;
        }
    };

private:
    float opaqueness;
};
