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

#include <d3d11.h>
#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include <iostream>
#include <string>
#include <algorithm>

// Link library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")


#include <DirectXMath.h>
#include <DirectXPackedVector.h>


__declspec(align(16))struct VERTEX_BASE {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 texUV;
    DirectX::XMFLOAT3 Normal;
};


__declspec(align(16)) struct MATRIX_DATA {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX WorldViewProjection;
};


__declspec(align(16)) struct MAP_GLOBAL_BUFF_DATA {
    DirectX::XMFLOAT4 AmbientLight;
};


__declspec(align(16)) struct OBJECT_BUFF_DATA {
    DirectX::XMFLOAT4 objPos;   // float2 objPosISO;
    DirectX::XMFLOAT4 PalEffects;//x,y = palOffset,Alpha for colour_outline. z,w = palOffset,Alpha for colour_Trans;
    DirectX::XMFLOAT4 PixelData;//size xy, w = Opaqueness
    DirectX::XMFLOAT4 lightColour;
    DirectX::XMFLOAT4 lightDetails;
    DirectX::XMFLOAT4 flags;//x is East/West Wall, y is North/South Wall, z is egg, w is TransEffect
};


__declspec(align(16)) struct PC_POS_BUFF_DATA {
    DirectX::XMFLOAT4 PcEgg_Pos;//(x,y) = pos offset to game area, (z,w)  = pos on game map
};


__declspec(align(16)) struct GEN_SURFACE_BUFF_DATA {
    DirectX::XMFLOAT4 genData4_1;
    DirectX::XMFLOAT4 genData4_2;
};


__declspec(align(16)) struct PORTAL_DIMENSIONS_DATA {
    DirectX::XMFLOAT4 portal_dim;
};


// Shader data
extern ID3D11VertexShader* pd3dVertexShader_Main;
extern ID3D11InputLayout* pd3dVS_InputLayout_Main;

extern ID3D11PixelShader* pd3d_PS_Basic_Tex_32;
extern ID3D11PixelShader* pd3d_PS_Basic_Tex_8;

extern ID3D11PixelShader* pd3d_PS_Fader;

extern ID3D11PixelShader* pd3d_PS_Colour32;


extern ID3D11PixelShader* pd3d_PS_Shadow1_DrawBase;
extern ID3D11PixelShader* pd3d_PS_Shadow2_Build;
extern ID3D11PixelShader* pd3d_PS_Shadow3_Blur;
extern ID3D11PixelShader* pd3d_PS_Shadow4_RadialBlur;
extern ID3D11PixelShader* pd3d_PS_Shadow5_Combine;

extern ID3D11PixelShader* pd3d_PS_Outline_OuterEdge8;
extern ID3D11PixelShader* pd3d_PS_Outline_OuterEdge32;

extern ID3D11PixelShader* pd3d_PS_Outline_Colour8;
extern ID3D11PixelShader* pd3d_PS_Outline_Colour32;
extern ID3D11PixelShader* pd3d_PS_Outline_Palette8;
extern ID3D11PixelShader* pd3d_PS_Outline_Palette32;



extern ID3D11PixelShader* pd3d_PS_ObjFlat8;
extern ID3D11PixelShader* pd3d_PS_ObjFlat32;
extern ID3D11PixelShader* pd3d_PS_ObjUpright8;
extern ID3D11PixelShader* pd3d_PS_ObjUpright32;

extern ID3D11PixelShader* pd3d_PS_DrawHexFog;

extern ID3D11PixelShader* pd3d_PS_DrawHexLight_OriginalLighting;

extern ID3D11PixelShader* pd3d_PS_ObjUpright8_OriginalLighting;
extern ID3D11PixelShader* pd3d_PS_ObjUpright32_OriginalLighting;

extern ID3D11PixelShader* pd3d_PS_DrawObjLight8;
extern ID3D11PixelShader* pd3d_PS_DrawObjLight32;

extern ID3D11PixelShader* pd3d_PS_DrawWallLight8;
extern ID3D11PixelShader* pd3d_PS_DrawWallLight32;

extern ID3D11PixelShader* pd3d_PS_GaussianBlurV;
extern ID3D11PixelShader* pd3d_PS_GaussianBlurU;

extern ID3D11PixelShader* pd3d_PS_RenderFloorLight32;

extern ID3D11PixelShader* pd3d_PS_RenderRoof32;

extern ID3D11PixelShader* pd3d_PS_Colour_32_Alpha;
extern ID3D11PixelShader* pd3d_PS_Colour_32_Brightness_ZeroMasked;

extern ID3D11PixelShader* pd3d_PS_Colour_32_RevAlpha_ZeroMasked;

extern ID3D11PixelShader* pd3d_PS_Outline_Quad_32;

extern ID3D11SamplerState* pd3dPS_SamplerState_Point;
extern ID3D11SamplerState* pd3dPS_SamplerState_Linear;

extern ID3D11BlendState* pBlendState_Zero;
extern ID3D11BlendState* pBlendState_One;
extern ID3D11BlendState* pBlendState_Two;
extern ID3D11BlendState* pBlendState_Three;
extern ID3D11BlendState* pBlendState_Four;


extern ID3D11Buffer* pVB_Quad_IndexBuffer;
extern ID3D11Buffer* pVB_Quad_Line_IndexBuffer;

//The maximum supported texture width or height for your graphics card.
extern UINT max_texDim;


extern LARGE_INTEGER refreshTime;
extern LARGE_INTEGER Frequency;
extern float refreshTime_ms;

ID3D11Device* GetD3dDevice();
ID3D11DeviceContext* GetD3dDeviceContext();
ID3D11RenderTargetView* GetRenderTargetView();
ID3D11DepthStencilView* GetDepthStencilView();
ID3D11DepthStencilState* GetDepthStencilState();

void Set_ViewPort(long width, long height);

extern DWORD Dx_PresentSkip;
void Dx_Present();
void ReSizeDisplayEx();

bool SetScreenProjectionMatrix_XM(DWORD width, DWORD height);
DirectX::XMMATRIX *GetScreenProjectionMatrix_XM();

bool CreateQuadVB(ID3D11Device* pD3DDev, unsigned int width, unsigned int height, ID3D11Buffer** plpVB);
bool CreateQuadVB_LineStrip(ID3D11Device* pD3DDev, unsigned int width, unsigned int height, ID3D11Buffer** lpVB);
bool CreateQuadrilateralVB_LineStrip(ID3D11Device* pD3DDev, POINT* p_lb, POINT* p_lt, POINT* p_rt, POINT* p_rb, ID3D11Buffer** lpVB);

bool Shader_Main_Setup();
void Shader_Main_Destroy();


class BUFFER_DX {
public:
    BUFFER_DX(UINT in_numConstantBuffers, bool in_isUpdatedOften, UINT in_ByteWidth) {
        ByteWidth = in_ByteWidth;
        isUpdatedOften = in_isUpdatedOften;
        Create_ConstantBuffers(in_numConstantBuffers);

    }
    BUFFER_DX(bool in_isUpdatedOften, UINT in_ByteWidth) {
        ByteWidth = in_ByteWidth;
        isUpdatedOften = in_isUpdatedOften;
        numConstantBuffers = 0;
        lpd3dConstantBuffers = nullptr;
    }
    ~BUFFER_DX() {
        if (lpd3dConstantBuffers == nullptr)
            return;
        ID3D11Buffer* pTemp = nullptr;
        for (UINT i = 0; i < numConstantBuffers; i++) {
            pTemp = lpd3dConstantBuffers[i];
            if (pTemp != nullptr)
                pTemp->Release();
            pTemp = nullptr;
        }
        delete[] lpd3dConstantBuffers;
        lpd3dConstantBuffers = nullptr;

    }
    void UpdateData(ID3D11DeviceContext* pD3DDevContext, UINT buffer_num, const void* data) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        if (lpd3dConstantBuffers[buffer_num] == nullptr)
            return;
        if (isUpdatedOften) {

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT result;
            result = pD3DDevContext->Map(lpd3dConstantBuffers[buffer_num], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (FAILED(result))
            {
                Fallout_Debug_Error("BUFFER_DX - Failed to Map Constant buffers.");
                return;
            }
            memcpy(mappedResource.pData, data, ByteWidth);
            pD3DDevContext->Unmap(lpd3dConstantBuffers[buffer_num], 0);
        }
        else
            pD3DDevContext->UpdateSubresource(lpd3dConstantBuffers[buffer_num], 0, nullptr, data, 0, 0);

    };
    ID3D11Buffer* GetBuffer(UINT buffer_num) {
        if (lpd3dConstantBuffers == nullptr)
            return nullptr;
        if (buffer_num >= numConstantBuffers)
            return nullptr;
        return lpd3dConstantBuffers[buffer_num];
    };
    void SetForRenderPS(ID3D11DeviceContext* pD3DDevContext, UINT buffer_num, UINT constPos) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        pD3DDevContext->PSSetConstantBuffers(constPos, 1, &lpd3dConstantBuffers[buffer_num]);
    }
    void SetForRenderVS(ID3D11DeviceContext* pD3DDevContext, UINT buffer_num, UINT constPos) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        pD3DDevContext->VSSetConstantBuffers(constPos, 1, &lpd3dConstantBuffers[buffer_num]);
    }
protected:
    bool Create_ConstantBuffers(UINT in_numConstantBuffers) {
        if (lpd3dConstantBuffers != nullptr)
            return false;
        lpd3dConstantBuffers = new ID3D11Buffer * [in_numConstantBuffers];
        ID3D11Device* pD3DDev = GetD3dDevice();
        D3D11_BUFFER_DESC constantBufferDesc;
        ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.ByteWidth = ByteWidth;

        if (isUpdatedOften) {
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        }
        else {
            constantBufferDesc.CPUAccessFlags = 0;
            constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        }
        constantBufferDesc.MiscFlags = 0;
        constantBufferDesc.StructureByteStride = 0;

        for (UINT i = 0; i < in_numConstantBuffers; i++) {
            HRESULT hr = pD3DDev->CreateBuffer(&constantBufferDesc, nullptr, &lpd3dConstantBuffers[i]);
            if (FAILED(hr)) {
                Fallout_Debug_Error("BUFFER_DX - Failed to CreateConstantBuffer_MatrixData.");
                return false;
            }
            numConstantBuffers = i + 1;
        }
        return true;
    };
    void DestroyConstantBuffers() {
        if (lpd3dConstantBuffers != nullptr) {
            ID3D11Buffer* pTemp = nullptr;
            for (UINT i = 0; i < numConstantBuffers; i++) {
                pTemp = lpd3dConstantBuffers[i];
                if (pTemp != nullptr)
                    pTemp->Release();
                pTemp = nullptr;
            }
            delete[] lpd3dConstantBuffers;
            lpd3dConstantBuffers = nullptr;
        }
    };
    UINT ByteWidth;
    UINT numConstantBuffers;
    bool isUpdatedOften;
    ID3D11Buffer** lpd3dConstantBuffers;
private:
};


class PS_BUFFERS_FALLOUT {
public:
    PS_BUFFERS_FALLOUT() {
        pD3DDevContext = GetD3dDeviceContext();
        pPosition = new BUFFER_DX(1, true, sizeof(MATRIX_DATA));
        pMapBuff = new BUFFER_DX(1, true, sizeof(MAP_GLOBAL_BUFF_DATA));
        pObjBuff = new BUFFER_DX(1, true, sizeof(OBJECT_BUFF_DATA));
        pPCObjBuff = new BUFFER_DX(1, true, sizeof(PC_POS_BUFF_DATA));
        pBaseBuff = new BUFFER_DX(1, true, sizeof(GEN_SURFACE_BUFF_DATA));
        pFadeColourBuff = new BUFFER_DX(1, true, sizeof(DirectX::XMFLOAT4));
        pPortalDimensions = new BUFFER_DX(1, true, sizeof(PORTAL_DIMENSIONS_DATA));
    };
    ~PS_BUFFERS_FALLOUT() {
        if (pPosition)
            delete pPosition;
        pPosition = nullptr;
        if (pMapBuff)
            delete pMapBuff;
        pMapBuff = nullptr;
        if (pObjBuff)
            delete pObjBuff;
        pObjBuff = nullptr;
        if (pPCObjBuff)
            delete pPCObjBuff;
        pPCObjBuff = nullptr;
        if (pBaseBuff)
            delete pBaseBuff;
        pBaseBuff = nullptr;
        if (pFadeColourBuff)
            delete pFadeColourBuff;
        pFadeColourBuff = nullptr;
        if (pPortalDimensions)
            delete pPortalDimensions;
        pPortalDimensions = nullptr;

        pD3DDevContext = nullptr;
    };
    void UpdatePositionBuff(const void* data) {
        pPosition->UpdateData(pD3DDevContext, 0, data);
        pPosition->SetForRenderVS(pD3DDevContext, 0, 0);
    };
    void UpdateMapBuff(const void* data) {
        pMapBuff->UpdateData(pD3DDevContext, 0, data);
    };
    void UpdatePCObjBuff(const void* data) {
        pPCObjBuff->UpdateData(pD3DDevContext, 0, data);
    };
    void UpdateObjBuff(const void* data) {
        pObjBuff->UpdateData(pD3DDevContext, 0, data);
    };
    void UpdateBaseBuff(const void* data) {
        pBaseBuff->UpdateData(pD3DDevContext, 0, data);
    };
    void UpdateFadeColour(const void* data) {
        pFadeColourBuff->UpdateData(pD3DDevContext, 0, data);
    };
    void UpdatePortalDimensions(const void* data) {
        pPortalDimensions->UpdateData(pD3DDevContext, 0, data);
    };
    void SetForRender() {
        pMapBuff->SetForRenderPS(pD3DDevContext, 0, 0);
        pPCObjBuff->SetForRenderPS(pD3DDevContext, 0, 1);
        pObjBuff->SetForRenderPS(pD3DDevContext, 0, 2);
        pBaseBuff->SetForRenderPS(pD3DDevContext, 0, 3);
        pFadeColourBuff->SetForRenderPS(pD3DDevContext, 0, 4);
        pPortalDimensions->SetForRenderPS(pD3DDevContext, 0, 5);
    };
    void SetPositionRender() {
        pPosition->SetForRenderVS(pD3DDevContext, 0, 0);
    }
protected:
private:
    BUFFER_DX* pPosition;
    BUFFER_DX* pMapBuff;
    BUFFER_DX* pObjBuff;
    BUFFER_DX* pPCObjBuff;
    BUFFER_DX* pBaseBuff;
    BUFFER_DX* pFadeColourBuff;
    BUFFER_DX* pPortalDimensions;
    ID3D11DeviceContext* pD3DDevContext;
};

extern PS_BUFFERS_FALLOUT* pPS_BuffersFallout;


class POSITION_DX : public BUFFER_DX {
public:
    POSITION_DX(UINT in_numConstantBuffers, bool in_isUpdatedOften) :
        BUFFER_DX(in_numConstantBuffers, in_isUpdatedOften, sizeof(MATRIX_DATA)) {
    };
    ~POSITION_DX() {
    };
    void UpdatePositionData(ID3D11DeviceContext* pD3DDevContext, UINT buffer_num, MATRIX_DATA* posData) {
        UpdateData(pD3DDevContext, buffer_num, posData);
    }
    void SetPositionRender(ID3D11DeviceContext* pD3DDevContext, UINT buffer_num) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        pD3DDevContext->VSSetConstantBuffers(0, 1, &lpd3dConstantBuffers[buffer_num]);
    }
protected:
private:
};


class BASE_BASE_DX {
public:
    BASE_BASE_DX() {
        width = 0;
        height = 0;
        size = 0;
        pD3DDev = GetD3dDevice();
        pD3DDevContext = GetD3dDeviceContext();
    }
    ~BASE_BASE_DX() {
        pD3DDev = nullptr;
        pD3DDevContext = nullptr;
    };
    void SetBaseDimensions(DWORD in_width, DWORD in_height) {
        width = in_width;
        height = in_height;
        size = width * height;
    }
    DWORD GetWidth() {
        return width;
    };
    DWORD GetHeight() {
        return height;
    };
protected:
    ID3D11Device* pD3DDev;
    ID3D11DeviceContext* pD3DDevContext;
    DWORD width;
    DWORD height;
    DWORD size;
private:
};


class BASE_POSITION_DX : virtual public BASE_BASE_DX, public BUFFER_DX {
public:
    BASE_POSITION_DX(float inX, float inY, UINT in_numConstantBuffers, bool in_isUpdatedOften) :
        BUFFER_DX(in_numConstantBuffers, in_isUpdatedOften, sizeof(MATRIX_DATA)),
        x(inX),
        y(inY),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        isPositionScaled(false)
    {

    };
    BASE_POSITION_DX(float inX, float inY, bool in_isUpdatedOften) :
        BUFFER_DX(in_isUpdatedOften, sizeof(MATRIX_DATA)),
        x(inX),
        y(inY),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        isPositionScaled(false)
    {

    };
    ~BASE_POSITION_DX() {
    };
    void SetPosition(float inX, float inY) {
        x = inX, y = inY;
        SetMatrices();
    };
    void GetPosition(float* pX, float* pY) {
        if (pX)
            *pX = x;
        if (pY)
            *pY = y;
    };
    void SetScale(float inScaleX, float inScaleY) {
        if (scaleX == inScaleX && scaleY == inScaleY)
            return;
        scaleX = inScaleX;
        scaleY = inScaleY;
        SetMatrices();
    };
    void RefreshMatrices() {
        SetMatrices();
    };
    void ScalePosition(bool in_isPositionScaled) {
        isPositionScaled = in_isPositionScaled;
    };
protected:
    float x;
    float y;
    float z;
    float scaleX;
    float scaleY;
    bool isPositionScaled;

    virtual void SetMatrices() {
        MATRIX_DATA posData{};
        DirectX::XMMATRIX xmManipulation{};
        DirectX::XMMATRIX xmScaling{};
        if (isPositionScaled)
            xmManipulation = DirectX::XMMatrixTranslation(x * scaleX, y * scaleY, 0);
        else
            xmManipulation = DirectX::XMMatrixTranslation(x, y, 0);
        xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);
        DirectX::XMMATRIX* pOrtho2D_SCRN = GetScreenProjectionMatrix_XM();
        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D_SCRN);
        UpdatePositionData(0, &posData);
    };
    void UpdatePositionData(UINT buffer_num, MATRIX_DATA* posData) {
        UpdateData(pD3DDevContext, buffer_num, posData);
    }
    void SetPositionRender(UINT buffer_num) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        pD3DDevContext->VSSetConstantBuffers(0, 1, &lpd3dConstantBuffers[buffer_num]);
    }
private:
};


enum class BuffData_Pixel {
    pixelSize,
    colour,

};


class BASE_VERTEX_DX : virtual public BASE_BASE_DX {
public:
    BASE_VERTEX_DX() :
        pd3dVB(nullptr)
    {
    };
    ~BASE_VERTEX_DX() {
        DestroyVerticies();
    };
    void GetVertexBuffer(ID3D11Buffer** ppVB) {
        if (ppVB)
            *ppVB = pd3dVB;
    };
    bool isVertex() {
        if (pd3dVB)
            return true;
        return false;
    }
protected:
    ID3D11Buffer* pd3dVB;

    virtual bool CreateVerticies() {
        if (width == 0 || height == 0)
            return false;
        return CreateQuadVB(pD3DDev, width, height, &pd3dVB);
    };
    virtual bool CreateVerticies(DWORD in_width, DWORD in_height) {
        if (in_width == 0 || in_height == 0)
            return false;
        return CreateQuadVB(pD3DDev, in_width, in_height, &pd3dVB);
    };
    virtual void DestroyVerticies() {
        if (pd3dVB != nullptr) {
            pd3dVB->Release();
            pd3dVB = nullptr;
        }
    };
private:
};


class BASE_TEXTURE_DX : virtual public BASE_BASE_DX {
public:
    BASE_TEXTURE_DX(DWORD in_bgColour) :
        pTex(nullptr),
        pTex_Staging(nullptr),
        pTex_shaderResourceView(nullptr),
        pTex_renderTargetView(nullptr),
        isRenderTarget(false),
        hasStagingTexture(false),
        dxgiFormat(DXGI_FORMAT_UNKNOWN),
        pixelWidth(0)
    {
        SetBackGroungColour(in_bgColour);
    };
    ~BASE_TEXTURE_DX() {
        DestroyTexture();
    };
    void SetBackGroungColour(DWORD in_bgColour) {
        bg_Colour = in_bgColour;
        bg_Colour_f[0] = ((bg_Colour & 0xFF000000) >> 24) / 256.0f;
        bg_Colour_f[1] = ((bg_Colour & 0x00FF0000) >> 16) / 256.0f;
        bg_Colour_f[2] = ((bg_Colour & 0x0000FF00) >> 8) / 256.0f;
        bg_Colour_f[3] = ((bg_Colour & 0x000000FF)) / 256.0f;
    }
    ID3D11ShaderResourceView* GetShaderResourceView() {
        return pTex_shaderResourceView;
    };
    void SetRenderTarget(ID3D11DepthStencilView* depthStencilView) {
        pD3DDevContext->OMSetRenderTargets(1, &pTex_renderTargetView, depthStencilView);
        Set_ViewPort(width, height);
        return;
    };
    void ClearRenderTarget(ID3D11DepthStencilView* depthStencilView) {
        pD3DDevContext->ClearRenderTargetView(pTex_renderTargetView, bg_Colour_f);
        pD3DDevContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        return;
    }
    void ClearRenderTarget(ID3D11DepthStencilView* depthStencilView, float color[4]) {
        pD3DDevContext->ClearRenderTargetView(pTex_renderTargetView, color);
        pD3DDevContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        return;
    }
    void SetRenderTargetAndViewPort(ID3D11DepthStencilView* depthStencilView) {
        pD3DDevContext->OMSetRenderTargets(1, &pTex_renderTargetView, depthStencilView);
        Set_ViewPort(width, height);
    }
    bool isTexture() {
        if (pTex)
            return true;
        return false;
    }
    bool GetOrthoMatrix(DirectX::XMMATRIX* pOrtho2D) {
        if (!pOrtho2D)
            return false;
        *pOrtho2D = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)(width), (float)(height), 0.0f, -1000.5f, 1000.5f);
        return true;
    };
    ID3D11Texture2D* GetTexture() {
        return pTex;
    };
    UINT GetPixelWidth() {
        return pixelWidth;
    };
protected:
    ID3D11Texture2D* pTex;
    ID3D11Texture2D* pTex_Staging;
    ID3D11ShaderResourceView* pTex_shaderResourceView;
    ID3D11RenderTargetView* pTex_renderTargetView;
    UINT pixelWidth;
    DWORD bg_Colour;
    float bg_Colour_f[4];
    bool isRenderTarget;
    bool hasStagingTexture;
    virtual bool CreateTexture() {

        if (width == 0 || height == 0)
            return false;
        if (pTex)
            return true;
        D3D11_TEXTURE2D_DESC textureDesc;
        HRESULT result = S_OK;
        ZeroMemory(&textureDesc, sizeof(textureDesc));
        //Setup the render target texture description.
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = dxgiFormat;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (isRenderTarget)
            textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        //Create the texture.
        result = pD3DDev->CreateTexture2D(&textureDesc, nullptr, &pTex);
        if (FAILED(result))
            return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        //Setup the description of the shader resource view.
        shaderResourceViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;

        //Create the shader resource view.
        result = pD3DDev->CreateShaderResourceView(pTex, &shaderResourceViewDesc, &pTex_shaderResourceView);
        if (FAILED(result)) {
            DestroyTexture();
            return false;
        }
        if (isRenderTarget) {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
            //Setup the description of the render target view.
            renderTargetViewDesc.Format = dxgiFormat;
            renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            //Create the render target view.
            result = pD3DDev->CreateRenderTargetView(pTex, &renderTargetViewDesc, &pTex_renderTargetView);
            if (FAILED(result)) {
                DestroyTexture();
                return false;
            }
        }
        if (hasStagingTexture) {
            textureDesc.Usage = D3D11_USAGE_STAGING;
            textureDesc.BindFlags = 0;
            textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;// 0;
            result = pD3DDev->CreateTexture2D(&textureDesc, nullptr, &pTex_Staging);
            if (FAILED(result)) {
                DestroyTexture();
                return false;
            }
        }
        return true;
    };
    virtual bool Texture_Initialize(DXGI_FORMAT in_dxgiFormat, bool in_isRenderTarget, bool in_hasStagingTexture) {
        SetPixelFormat(in_dxgiFormat);
        isRenderTarget = in_isRenderTarget;
        hasStagingTexture = in_hasStagingTexture;
        return CreateTexture();

    };
    virtual void DestroyTexture() {
        if (pTex)
            pTex->Release();
        pTex = nullptr;
        if (pTex_Staging)
            pTex_Staging->Release();
        pTex_Staging = nullptr;
        if (pTex_shaderResourceView)
            pTex_shaderResourceView->Release();
        pTex_shaderResourceView = nullptr;
        if (pTex_renderTargetView)
            pTex_renderTargetView->Release();
        pTex_renderTargetView = nullptr;
    };
    void SetPixelFormat(DXGI_FORMAT in_dxgiFormat) {
        dxgiFormat = in_dxgiFormat;
        if (dxgiFormat == DXGI_FORMAT_R8_UNORM)
            pixelWidth = 1;
        else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM)
            pixelWidth = 2;
        else {
            dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            pixelWidth = 4;
        }
    }
private:
    DXGI_FORMAT dxgiFormat;

};


class BASE_TEXTURE_STAGING : public BASE_TEXTURE_DX {
public:
    BASE_TEXTURE_STAGING(DWORD in_bgColour) :
        BASE_TEXTURE_DX(in_bgColour),
        isLocked(false),
        lockType(D3D11_MAP_READ)
    {

    };
    ~BASE_TEXTURE_STAGING() {

    };
    HRESULT Lock(void** data, UINT* p_pitch, D3D11_MAP MapType) {
        if (!pTex_Staging || !pTex)
            return -1;
        if (isLocked)//already locked
            return -1;
        if (width == 0 || height == 0)
            return -1;
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        lockType = MapType;
        result = pD3DDevContext->Map(pTex_Staging, 0, lockType, 0, &mappedResource);
        *data = (void*)mappedResource.pData;
        *p_pitch = mappedResource.RowPitch;
        if (SUCCEEDED(result))
            isLocked = true;
        return result;
    };
    void Unlock(RECT* pRect) {
        if (!pTex_Staging || !pTex)
            return;
        if (!isLocked)
            return;
        pD3DDevContext->Unmap(pTex_Staging, 0);
        if (lockType == D3D11_MAP_READ) {//reading only
            isLocked = false;//reset lockType to unlocked
            return;
        }
        //update main texture
        isLocked = false;//reset lockType to unlocked
        if (pRect) {
            if (pRect->left > (LONG)width || pRect->right < 0 || pRect->top >(LONG)height || pRect->bottom < 0)
                return;

            D3D11_BOX sourceRegion;

            sourceRegion.left = pRect->left;
            sourceRegion.right = pRect->right;
            sourceRegion.top = pRect->top;
            sourceRegion.bottom = pRect->bottom;
            sourceRegion.front = 0;
            sourceRegion.back = 1;
            pD3DDevContext->CopySubresourceRegion(pTex, 0, pRect->left, pRect->top, 0, pTex_Staging, 0, &sourceRegion);
        }
        else
            pD3DDevContext->CopyResource(pTex, pTex_Staging);

    };
    void Clear_Staging(DWORD colour) {
        if (!pTex_Staging || !pTex)
            return;
        BYTE* pBackBuff = nullptr;
        UINT pitch = 0;
        if (SUCCEEDED(Lock((void**)&pBackBuff, &pitch, D3D11_MAP_WRITE))) {
            if (pixelWidth == 4) {
                DWORD size = pitch / 4 * height;
                DWORD colourOffset = 0;
                while (colourOffset < size) {
                    ((DWORD*)pBackBuff)[colourOffset] = colour;
                    colourOffset++;
                }
            }
            else
                memset(pBackBuff, colour, pitch * height);
            Unlock(nullptr);
        }
    };
    void Clear_Staging() {
        Clear_Staging(bg_Colour);
    };
protected:
    virtual bool Texture_Initialize(DXGI_FORMAT in_dxgiFormat, bool in_isRenderTarget, bool in_hasStagingTexture) {
        SetPixelFormat(in_dxgiFormat);
        isRenderTarget = in_isRenderTarget;
        hasStagingTexture = in_hasStagingTexture;
        return CreateTexture();
    };
private:
    D3D11_MAP lockType;
    bool isLocked;
};


class BASE_BASE_DX_TILED {
public:
    BASE_BASE_DX_TILED() {
        width = 0;
        height = 0;
        tile_width = 0;
        tile_height = 0;
        tiles_x = 0;
        tiles_y = 0;
        tiles_num = 0;
        pD3DDev = GetD3dDevice();
        pD3DDevContext = GetD3dDeviceContext();
    }
    ~BASE_BASE_DX_TILED() {
        pD3DDev = nullptr;
        pD3DDevContext = nullptr;
    };
    DWORD GetWidth() {
        return width;
    };
    DWORD GetHeight() {
        return height;
    };
    void SetBaseDimensions(DWORD in_width, DWORD in_height) {
        width = in_width;
        height = in_height;

        if (width > max_texDim) {
            tile_width = max_texDim;
            tiles_x = width / max_texDim;
            tiles_x += 1;
        }
        else {
            tiles_x = 1;
            tile_width = width;
        }
        if (height > max_texDim) {
            tile_height = max_texDim;
            tiles_y = height / max_texDim;
            tiles_y += 1;
        }
        else {
            tiles_y = 1;
            tile_height = height;
        }
        tiles_num = tiles_x * tiles_y;
    }
protected:
    ID3D11Device* pD3DDev;
    ID3D11DeviceContext* pD3DDevContext;
    DWORD width;
    DWORD height;

    DWORD tile_width;
    DWORD tile_height;
    UINT tiles_x;
    UINT tiles_y;
    UINT tiles_num;
private:
};


class BASE_POSITION_DX_TILED : virtual public BASE_BASE_DX_TILED, public BUFFER_DX {
public:
    BASE_POSITION_DX_TILED(float inX, float inY, UINT in_numConstantBuffers, bool in_isUpdatedOften) :
        BUFFER_DX(in_numConstantBuffers, in_isUpdatedOften, sizeof(MATRIX_DATA)),
        x(inX),
        y(inY),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        isPositionScaled(false),
        pOrtho2D(nullptr)
    {

    };
    BASE_POSITION_DX_TILED(float inX, float inY, bool in_isUpdatedOften) :
        BUFFER_DX(in_isUpdatedOften, sizeof(MATRIX_DATA)),
        x(inX),
        y(inY),
        z(0.0f),
        scaleX(1.0f),
        scaleY(1.0f),
        isPositionScaled(false),
        pOrtho2D()
    {

    };
    ~BASE_POSITION_DX_TILED() {
        if (pOrtho2D)
            delete pOrtho2D;
        pOrtho2D = nullptr;
    };
    void SetPosition(float inX, float inY) {
        if (x == inX && y == inY)
            return;
        x = inX, y = inY;
        SetMatrices();
    };
    void GetPosition(float* pX, float* pY) {
        if (pX)
            *pX = x;
        if (pY)
            *pY = y;
    };
    void SetScale(float inScaleX, float inScaleY) {
        if (scaleX == inScaleX && scaleY == inScaleY)
            return;
        scaleX = inScaleX;
        scaleY = inScaleY;
        SetMatrices();
    };
    void SetProjectionMatrix(DirectX::XMMATRIX* pOrtho2D_XM) {
        if (!pOrtho2D_XM)
            return;
        if (!pOrtho2D)
            pOrtho2D = new DirectX::XMFLOAT4X4();
        DirectX::XMStoreFloat4x4(pOrtho2D, *pOrtho2D_XM);
        SetMatrices();
    };
    DirectX::XMMATRIX GetProjectionMatrix() {
        return XMLoadFloat4x4(pOrtho2D);
    };
    void RefreshMatrices() {
        SetMatrices();
    };
    void ScalePosition(bool in_isPositionScaled) {
        isPositionScaled = in_isPositionScaled;
    };
protected:
    float x;
    float y;
    float z;
    float scaleX;
    float scaleY;
    bool isPositionScaled;
    DirectX::XMFLOAT4X4* pOrtho2D;

    virtual void SetMatrices() {
        MATRIX_DATA posData{};
        DirectX::XMMATRIX xmManipulation{};
        DirectX::XMMATRIX xmScaling{};
        if (isPositionScaled)
            xmManipulation = DirectX::XMMatrixTranslation(x * scaleX, y * scaleY, 0);
        else
            xmManipulation = DirectX::XMMatrixTranslation(x, y, 0);
        xmScaling = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
        posData.World = DirectX::XMMatrixMultiply(xmScaling, xmManipulation);

        DirectX::XMMATRIX* pOrtho2D_XM = nullptr;
        DirectX::XMMATRIX Ortho2D_XM{};
        if (pOrtho2D != nullptr) {
            Ortho2D_XM = XMLoadFloat4x4(pOrtho2D);
            pOrtho2D_XM = &Ortho2D_XM;
        }
        else//if no custom matrix get screen ortho matrix
            pOrtho2D_XM = GetScreenProjectionMatrix_XM();

        posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, *pOrtho2D_XM);
        UpdatePositionData(0, &posData);
    };
    void UpdatePositionData(UINT buffer_num, MATRIX_DATA* posData) {
        UpdateData(pD3DDevContext, buffer_num, posData);
    }
    void SetPositionRender(UINT buffer_num) {
        if (lpd3dConstantBuffers == nullptr)
            return;
        if (buffer_num >= numConstantBuffers)
            return;
        pD3DDevContext->VSSetConstantBuffers(0, 1, &lpd3dConstantBuffers[buffer_num]);
    }
private:
};


class BASE_VERTEX_DX_TILED : virtual public BASE_BASE_DX_TILED {
public:
    BASE_VERTEX_DX_TILED() :
        pd3dVB(nullptr),
        pd3dVB_Tile(nullptr)
    {
    };
    ~BASE_VERTEX_DX_TILED() {
        DestroyVerticies();
    };
    void GetVertexBuffer(ID3D11Buffer** lpVB) {
        *lpVB = pd3dVB;
    };
    void GetVertexBuffers_Tile(ID3D11Buffer** lpVB) {
        *lpVB = pd3dVB_Tile;
    };
protected:
    ID3D11Buffer* pd3dVB;
    ID3D11Buffer* pd3dVB_Tile;

    virtual bool CreateVerticies() {
        if (width == 0 || height == 0)
            return false;
        if (tiles_num > 1)
            CreateQuadVB(pD3DDev, tile_width, tile_height, &pd3dVB_Tile);
        return CreateQuadVB(pD3DDev, width, height, &pd3dVB);
    };
    virtual void DestroyVerticies() {
        if (pd3dVB != nullptr) {
            pd3dVB->Release();
            pd3dVB = nullptr;
        }
        if (pd3dVB_Tile != nullptr) {
            pd3dVB_Tile->Release();
            pd3dVB_Tile = nullptr;
        }
    };
private:
};


class BASE_TEXTURE_DX_TILED : virtual public BASE_BASE_DX_TILED {
public:
    BASE_TEXTURE_DX_TILED(DXGI_FORMAT in_dxgiFormat, DWORD in_bgColour, bool in_isRenderTarget) {
        dxgiFormat = in_dxgiFormat;
        if (dxgiFormat == DXGI_FORMAT_R8_UNORM)
            pixelWidth = 1;
        else
            pixelWidth = 4;
        bg_Colour = in_bgColour;
        bg_Colour_f[0] = ((bg_Colour & 0x00FF0000) >> 16) / 256.0f;
        bg_Colour_f[1] = ((bg_Colour & 0x0000FF00) >> 8) / 256.0f;
        bg_Colour_f[2] = ((bg_Colour & 0x000000FF)) / 256.0f;
        bg_Colour_f[3] = ((bg_Colour & 0xFF000000) >> 24) / 256.0f;

        ppTex = nullptr;
        ppTex_shaderResourceView = nullptr;
        ppTex_renderTargetView = nullptr;
        isRenderTarget = in_isRenderTarget;
        tile_current = 0;
    };
    ~BASE_TEXTURE_DX_TILED() {
        DestroyTexture();
    };
    ID3D11ShaderResourceView* GetShaderResourceView(UINT tile_num) {
        if (!ppTex_shaderResourceView || tile_num >= tiles_num)
            return nullptr;
        return ppTex_shaderResourceView[tile_num];
    };
    void DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation, ID3D11DepthStencilView* depthStencilView, float index_x, float index_y, DWORD index_width, DWORD index_height, RECT* p_scissorRect) {
        if (index_x > width || index_x + index_width < 0 || index_y > height || index_y + index_height < 0)
            return;

        RECT scissorRect = { 0,0,(LONG)width, (LONG)height };
        if (p_scissorRect) {
            scissorRect = { p_scissorRect->left, p_scissorRect->top, p_scissorRect->right, p_scissorRect->bottom };
            if (scissorRect.right > (LONG)width)
                scissorRect.right = (LONG)width;
            if (scissorRect.bottom > (LONG)height)
                scissorRect.bottom = (LONG)height;
        }
        if (scissorRect.left >= scissorRect.right || scissorRect.top >= scissorRect.bottom)
            return;


        D3D11_VIEWPORT Viewport{};
        Viewport.Width = static_cast<float>(tile_width);
        Viewport.Height = static_cast<float>(tile_height);
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;
        pD3DDevContext->RSSetViewports(1, &Viewport);

        DirectX::XMMATRIX Ortho2D = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)(tile_width), (float)(tile_height), 0.0f, -1000.5f, 1000.5f);
        MATRIX_DATA posData;
        posData.World = DirectX::XMMatrixTranslation((float)index_x, (float)index_y, (float)0);

        RECT rt_rect = { 0, 0, (LONG)tile_width, (LONG)tile_height };
        RECT rt_scissorRect = { 0, 0, 0, 0 };
        UINT tile_row = 0;

        for (UINT tileY = 0; tileY < tiles_y; tileY++) {
            rt_rect.left = 0;
            rt_rect.right = tile_width;

            for (UINT tileX = 0; tileX < tiles_x; tileX++) {
                if (!(index_x > rt_rect.right) && !(index_x + (LONG)index_width < rt_rect.left) && !(index_y > rt_rect.bottom) && !(index_y + (LONG)index_height < rt_rect.top)) {
                    pD3DDevContext->OMSetRenderTargets(1, &ppTex_renderTargetView[tile_row + tileX], depthStencilView);

                    Ortho2D = DirectX::XMMatrixOrthographicOffCenterLH((float)rt_rect.left, (float)rt_rect.right, (float)rt_rect.bottom, (float)rt_rect.top, -1.0f, 1.0f);
                    posData.WorldViewProjection = DirectX::XMMatrixMultiply(posData.World, Ortho2D);
                    pPS_BuffersFallout->UpdatePositionBuff(&posData);
                    pPS_BuffersFallout->SetPositionRender();

                    rt_scissorRect = { scissorRect.left - rt_rect.left, scissorRect.top - rt_rect.top, scissorRect.right - rt_rect.left,  scissorRect.bottom - rt_rect.top };
                    pD3DDevContext->RSSetScissorRects(1, &rt_scissorRect);

                    pD3DDevContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
                }
                rt_rect.left += tile_width;
                rt_rect.right += tile_width;
            }
            rt_rect.top += tile_height;
            rt_rect.bottom += tile_height;
            tile_row += tiles_x;
        }
    };
    void ClearRenderTarget(ID3D11DepthStencilView* depthStencilView) {
        for (UINT tile = 0; tile < tiles_num; tile++)
            pD3DDevContext->ClearRenderTargetView(ppTex_renderTargetView[tile], bg_Colour_f);
        pD3DDevContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        return;
    };
    void ClearRenderTarget(ID3D11DepthStencilView* depthStencilView, float color[4]) {
        for (UINT tile = 0; tile < tiles_num; tile++)
            pD3DDevContext->ClearRenderTargetView(ppTex_renderTargetView[tile], color);
        pD3DDevContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        return;
    };
    bool isTexture() {
        if (ppTex)
            return true;
        return false;
    }
    ID3D11Texture2D* GetTexture_Start() {
        tile_current = 0;
        return ppTex[tile_current];
    };
    ID3D11Texture2D* GetTexture_Next(LONG *p_x, LONG* p_y) {
        tile_current ++;
        if (tile_current < tiles_num) {
            if(p_y)
                *p_y = tile_current / tiles_x * tile_height;
            if(p_x)
                *p_x = tile_current % tiles_x * tile_width;
            return ppTex[tile_current];
        }
        else
            return nullptr;
    };
    bool GetOrthoMatrix(DirectX::XMMATRIX* pOrtho2D) {
        if (!pOrtho2D)
            return false;
        *pOrtho2D = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)(width), (float)(height), 0.0f, -1000.5f, 1000.5f);
        return true;
    };
protected:
    ID3D11Texture2D** ppTex;
    ID3D11ShaderResourceView** ppTex_shaderResourceView;
    ID3D11RenderTargetView** ppTex_renderTargetView;
    DXGI_FORMAT dxgiFormat;
    UINT pixelWidth;
    DWORD bg_Colour;
    float bg_Colour_f[4];
    bool isRenderTarget;
    virtual bool CreateTexture() {
        if (ppTex)
            return true;

        ppTex = new ID3D11Texture2D * [tiles_num];
        ppTex_shaderResourceView = new ID3D11ShaderResourceView * [tiles_num];
        ppTex_renderTargetView = new ID3D11RenderTargetView * [tiles_num];
        for (UINT tile = 0; tile < tiles_num; tile++) {
            ppTex[tile] = nullptr;
            ppTex_shaderResourceView[tile] = nullptr;
            ppTex_renderTargetView[tile] = nullptr;
        }

        for (UINT tile = 0; tile < tiles_num; tile++) {
            D3D11_TEXTURE2D_DESC textureDesc;
            HRESULT result = S_OK;
            //Initialize the render target texture description.
            ZeroMemory(&textureDesc, sizeof(textureDesc));

            //Setup the render target texture description.
            textureDesc.Width = tile_width;
            textureDesc.Height = tile_height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = dxgiFormat;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            if (isRenderTarget)
                textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            //Create the texture.
            result = pD3DDev->CreateTexture2D(&textureDesc, nullptr, &ppTex[tile]);
            if (FAILED(result)) {
                DestroyTexture();
                return false;
            }
            D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
            //Setup the description of the shader resource view.
            shaderResourceViewDesc.Format = textureDesc.Format;
            shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
            shaderResourceViewDesc.Texture2D.MipLevels = 1;

            //Create the shader resource view.
            result = pD3DDev->CreateShaderResourceView(ppTex[tile], &shaderResourceViewDesc, &ppTex_shaderResourceView[tile]);
            if (FAILED(result)) {
                DestroyTexture();
                return false;
            }
            if (isRenderTarget) {
                D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
                //Setup the description of the render target view.
                renderTargetViewDesc.Format = dxgiFormat;
                renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                renderTargetViewDesc.Texture2D.MipSlice = 0;

                //Create the render target view.
                HRESULT result = pD3DDev->CreateRenderTargetView(ppTex[tile], &renderTargetViewDesc, &ppTex_renderTargetView[tile]);
                if (FAILED(result)) {
                    DestroyTexture();
                    return false;
                }
            }
        }
        return true;
    };
    virtual void DestroyTexture() {
        for (UINT tile = 0; tile < tiles_num; tile++) {
            if (ppTex && ppTex[tile]) {
                ppTex[tile]->Release();
                ppTex[tile] = nullptr;
            }
            if (ppTex_shaderResourceView && ppTex_shaderResourceView[tile]) {
                ppTex_shaderResourceView[tile]->Release();
                ppTex_shaderResourceView[tile] = nullptr;
            }
            if (ppTex_renderTargetView && ppTex_renderTargetView[tile]) {
                ppTex_renderTargetView[tile]->Release();
                ppTex_renderTargetView[tile] = nullptr;
            }
        }
        if (ppTex)
            delete[]ppTex;
        ppTex = nullptr;
        if (ppTex_shaderResourceView)
            delete[]ppTex_shaderResourceView;
        ppTex_shaderResourceView = nullptr;
        if (ppTex_renderTargetView)
            delete[]ppTex_renderTargetView;
        ppTex_renderTargetView = nullptr;
    };
private:
    UINT tile_current;
};


class STORED_VIEW {//if in_on_create_and_destroy = true view will be stored on creation of this class and restored on destruction, otherwise call  StoreView and ReStoreView separately;
public:
    STORED_VIEW(bool in_on_create_and_destroy) {
        pRenderTargetViews = nullptr;
        pDepthStencilView = nullptr;
        pCViewPorts = nullptr;
        pScissorRects = nullptr;
        numViewPorts = 0;
        on_create_and_destroy = in_on_create_and_destroy;
        if (on_create_and_destroy)
            StoreView();
    }
    ~STORED_VIEW() {
        if (on_create_and_destroy)
            ReStoreView();
        if (pRenderTargetViews)
            pRenderTargetViews->Release();
        pRenderTargetViews = nullptr;
        if (pDepthStencilView)
            pDepthStencilView->Release();
        pDepthStencilView = nullptr;
        if (pCViewPorts != nullptr) {
            delete[] pCViewPorts;
            pCViewPorts = nullptr;
        }
        if (pScissorRects != nullptr) {
            delete[] pScissorRects;
            pScissorRects = nullptr;
        }
        numViewPorts = 0;
    }
    void StoreView();
    void ReStoreView();
protected:
private:

    ID3D11RenderTargetView* pRenderTargetViews;
    ID3D11DepthStencilView* pDepthStencilView;
    UINT numViewPorts;
    D3D11_VIEWPORT* pCViewPorts;
    D3D11_RECT* pScissorRects;
    bool on_create_and_destroy;
};


//slow fire, shoreline, slime
#define PAL_TIME_1     200
//fast fire
#define PAL_TIME_2     142
//monitors | water
#define PAL_TIME_3     100
//alarm
#define PAL_TIME_4     33


//slow fire, shoreline, slime
#define PAL_TIME_ZONE_1     0x00000001
//fast fire
#define PAL_TIME_ZONE_2     0x00000002
//monitors | water
#define PAL_TIME_ZONE_3     0x00000004
//alarm
#define PAL_TIME_ZONE_4     0x00000008

//green - slime
#define PAL_ANI_START_1     229
//blue/grey - monitors | water
#define PAL_ANI_START_2     233
//orang/red - slow fire
#define PAL_ANI_START_3     238
//orang/red - fast fire
#define PAL_ANI_START_4     243
//brown - shoreline
#define PAL_ANI_START_5     248
//red/black - alarm
#define PAL_ANI_START_6     254

#define RGBA_32BIT(a,r,g,b) (a<<24) | (r<<16) | (g<<8) | (b)


class PAL_DX : public BASE_TEXTURE_DX {
public:
    PAL_DX(UINT inSize, bool in_isVolatile) :
        BASE_TEXTURE_DX(0x00000000),
        isLocked(false),
        lockType(D3D11_MAP_READ)
    {
        SetBaseDimensions(inSize, 1);
        trans_offset = 0xFFFFFFFF;//set out of range by default;
        if (!Texture_Initialize(DXGI_FORMAT_B8G8R8A8_UNORM, false, true))
            Fallout_Debug_Error("PAL_DX - Failed BASE_TEXTURE_DX CreateTexture.");
    };
    ~PAL_DX() {
    };
    void SetPalEntries(BYTE* palette, DWORD offset, DWORD dwNumEntries) {
        if (offset < 0)
            return;
        if (offset + dwNumEntries >= size)
            return;
        DWORD* paldata = nullptr;
        DWORD colour = 0;
        if (FAILED(Lock(&paldata, D3D11_MAP_WRITE)))
            return;

        DWORD colourOffset = offset;
        while (colourOffset < offset + dwNumEntries) {
            paldata[colourOffset] = RGBA_32BIT(0xFF, palette[0] << 2, palette[1] << 2, palette[2] << 2);
            if (colourOffset == trans_offset)//set palette[trans_offset] alpha to 0 - transparent
                paldata[colourOffset] = (paldata[colourOffset] & 0x00FFFFFF);
            palette += 3;
            colourOffset++;
        }
        Unlock(offset, dwNumEntries);
    };
    void SetPalEntry(int offset, BYTE r, BYTE g, BYTE b) {
        BYTE palette[3] = { r,g,b };
        SetPalEntries(palette, offset, 1);
    };
    bool GetPalEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) {
        if (lpEntries == nullptr)
            return false;
        if (dwBase > size - 1)
            return false;
        if (dwBase + dwNumEntries > size)
            return false;

        DWORD* paldata = nullptr;
        DWORD colour = 0;
        if (FAILED(Lock(&paldata, D3D11_MAP_READ)))
            return false;

        DWORD offset = 0;
        while (dwNumEntries) {
            lpEntries[offset].peFlags = (paldata[dwBase] >> 24) & 0x000000FF;
            lpEntries[offset].peBlue = (paldata[dwBase] >> 16) & 0x000000FF;
            lpEntries[offset].peGreen = (paldata[dwBase] >> 8) & 0x000000FF;
            lpEntries[offset].peRed = (paldata[dwBase]) & 0x000000FF;
            offset++;
            dwBase++;
            dwNumEntries--;
        }
        Unlock(0, 0);
        return true;
    };
    void SetTransparentOffset(DWORD offset) {
        trans_offset = offset;
    };
    DWORD GetColour(UINT offset) {
        if (offset < size) {
            DWORD* paldata = nullptr;
            DWORD colour = 0;
            if (SUCCEEDED(Lock(&paldata, D3D11_MAP_READ)) && paldata != nullptr)
                colour = paldata[offset];
            Unlock(0, 0);
            return colour;
        }
        return 0;
    };
    HRESULT Lock(DWORD** paldata, D3D11_MAP MapType) {
        if (!pTex_Staging)
            return -1;
        if (isLocked)//already locked
            return -1;
        HRESULT result;
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        result = pD3DDevContext->Map(pTex_Staging, 0, MapType, 0, &mappedResource);
        *paldata = (DWORD*)mappedResource.pData;
        lockType = MapType;
        if (SUCCEEDED(result))
            isLocked = true;
        return result;
        
    };
    void Unlock(DWORD offset, DWORD dwNumEntries) {
        if (!pTex_Staging)
            return;
        if (!isLocked)
            return;
        pD3DDevContext->Unmap(pTex_Staging, 0);
        if (lockType == D3D11_MAP_READ) {//reading only
            isLocked = false;//reset lockType to unlocked
            return;
        }
        //update main texture
        isLocked = false;//reset lockType to unlocked
        if (offset < 0)
            return;
        if (offset + dwNumEntries > size)
            return;
        D3D11_BOX destRegion;
        destRegion.left = offset;
        destRegion.right = offset + dwNumEntries;
        destRegion.top = 0;
        destRegion.bottom = 1;
        destRegion.front = 0;
        destRegion.back = 1;
        pD3DDevContext->CopySubresourceRegion(pTex, 0, offset, 0, 0, pTex_Staging, 0, &destRegion);
    };
protected:
private:
    DWORD trans_offset;

    D3D11_MAP lockType;
    bool isLocked;
};

extern PAL_DX* color_pal;
extern PAL_DX* main_pal;
extern DWORD* pIsPaletteCyclingEnabled;

HRESULT __stdcall GetPalEntriesX(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries);

bool GetDefaultPalette(DWORD *palette);
bool GetColorPalette(DWORD *palette);

void Dx_Present_Main();
int DxSetup();

void Display_Release_RenderTargets();

void TransitionalFader_SetColour(DirectX::XMFLOAT4 inColour);
//This only effects the next fade - will return to default "transitional_fade_seconds_default" when complete.
void TransitionalFader_SetTime(float f_seconds);
void TransitionalFader_Start(bool fade_in);
void TransitionalFader_Complete();

void FadeToPalette(BYTE* pPal);
void SetPalette_Fading(BYTE* pPal);

HRESULT SaveTextureToFile(ID3D11Texture2D* pTex_Source, const char* pfile_path, bool isFalloutDataPath);


