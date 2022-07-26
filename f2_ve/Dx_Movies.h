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

//#include <ddraw.h>
///ddraw dummy structures, for the basic requirements here.-----------------------------------
typedef struct _DUMMY_DDSURFACEDESC   FAR* LPDDSURFACEDESC;
typedef void FAR* LPDDBLTFX;//unused here.

typedef struct _DDPIXELFORMAT {
    DWORD       dwSize;                 // size of the structure
    DWORD       dwFlags;                // unused here.
    DWORD       dwFourCC;               // unused here.
    DWORD       dwRGBBitCount;          // how many bits per pixel
    DWORD       dwRBitMask;             // unused here.
    DWORD       dwGBitMask;             // unused here.
    DWORD       dwBBitMask;             // unused here.
    DWORD       dwRGBAlphaBitMask;      // unused here.
} DDPIXELFORMAT;


typedef struct _DUMMY_DDSURFACEDESC {
    DWORD           dwSize;                 // size of the structure
    DWORD           dwFlags;                // unused here.
    DWORD           dwHeight;               // height of surface to be created
    DWORD           dwWidth;                // width of input surface
    LONG            lPitch;                 // distance to start of next line (return value only)
    DWORD           dwBackBufferCount;      // unused here.
    DWORD           dwMipMapCount;          // unused here.
    DWORD           dwAlphaBitDepth;        // unused here.
    DWORD           dwReserved;             // reserved
    LPVOID          lpSurface;              // pointer to the associated surface memory
    DWORD           colorKeySec1a;          // color key section filler
    DWORD           colorKeySec1b;          // color key section filler
    DWORD           colorKeySec2a;          // color key section filler
    DWORD           colorKeySec2b;          // color key section filler
    DWORD           colorKeySec3a;          // color key section filler
    DWORD           colorKeySec3b;          // color key section filler
    DWORD           colorKeySec4a;          // color key section filler
    DWORD           colorKeySec4b;          // color key section filler
    DDPIXELFORMAT   ddpfPixelFormat;        // pixel format description of the surface
    DWORD           dwCaps;                 // dwCaps section filler
} DDSURFACEDESC;
//------------------------------------------------------------------------------------------------


class MVE_SURFACE : public BASE_VERTEX_DX, public BASE_TEXTURE_DX, public BASE_POSITION_DX {
public:
    MVE_SURFACE(DDSURFACEDESC* DDSurfaceDesc, MVE_SURFACE** plp_MVE_SURFACE) :
        BASE_VERTEX_DX(),
        BASE_TEXTURE_DX(0x00000000),
        BASE_POSITION_DX(0, 0, 1, false)
    {
        DXGI_FORMAT dxgi_Format = DXGI_FORMAT_UNKNOWN;
        if (DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8)
            dxgi_Format = DXGI_FORMAT_R8_UNORM;
        else if (DDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 16)
            dxgi_Format = DXGI_FORMAT_B5G5R5A1_UNORM;

        SetBaseDimensions(DDSurfaceDesc->dwWidth, DDSurfaceDesc->dwHeight);
        if (!CreateVerticies())
            Fallout_Debug_Error("Failed MVE_SURFACE CreateVerticies.");
        if (!Texture_Initialize(dxgi_Format, false, true))
            Fallout_Debug_Error("Failed MVE_SURFACE CreateTexture.");
        ScaleToScreen();
        *plp_MVE_SURFACE = this;
    };
    ~MVE_SURFACE() {
    };
    HRESULT Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc) {
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        result = pD3DDevContext->Map(pTex_Staging, 0, D3D11_MAP_WRITE, 0, &mappedResource);
        lpDDSurfaceDesc->lpSurface = mappedResource.pData;
        lpDDSurfaceDesc->lPitch = mappedResource.RowPitch;
        return result;
    };
    HRESULT Unlock(LPRECT lpRect) {
        pD3DDevContext->Unmap(pTex_Staging, 0);
        pD3DDevContext->CopyResource(pTex, pTex_Staging);
        return 0;
    };
    void Display();
    void ScaleToScreen();
private:
};

void MVE_Surface_Display();
void MVE_ResetMatrices();
