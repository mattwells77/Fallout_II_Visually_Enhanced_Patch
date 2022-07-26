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
#include "Dx_RenderTarget.h"
#include "win_fall.h"
#include "modifications.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;


//____________________________
void RenderTarget::Display() {
    if (pTex_shaderResourceView == nullptr)
        return;

    //set vertex stuff
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
    //set shader constant buffers 
    SetPositionRender(0);
    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);
}


//____________________________
void RenderTarget::Display(ID3D11PixelShader* pd3d_PixelShader) {

    if (pTex_shaderResourceView == nullptr)
        return;

    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    //set vertex stuff
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);
    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);
    //set shader constant buffers 
    SetPositionRender(0);
    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PixelShader, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);

}


//_________________________________________________________
void RenderTarget::ClearRect(XMFLOAT4 colour, RECT* pRect) {
    if (pRect && (pRect->right <= pRect->left || pRect->bottom <= pRect->top))
        return;

    if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right >= (LONG)width && pRect->bottom >= (LONG)height)) {
        float colourf[4] = { colour.x, colour.y, colour.z, colour.w };
        ClearRenderTarget(nullptr, colourf);
        return;
    }
    if (pRect->right < 0 || pRect->bottom < 0 || pRect->left >= (LONG)width || pRect->top >= (LONG)height)
        return;


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
    genSurfaceData.genData4_1 = colour;
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);

    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
}



//_______________________________________________________________________________________
void RenderTarget2::Display(ID3D11PixelShader* pd3d_PixelShader, RenderTarget2* plightRT) {

    if (pD3DDev == nullptr)
        return;
    if (ppTex == nullptr)
        return;

    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    //set vertex stuff
    if (pd3dVB_Tile)
        pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB_Tile, &stride, &offset);
    else
        pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    UINT tileLine = 0;
    for (UINT tileY = 0; tileY < tiles_y; tileY++) {
        for (UINT tileX = 0; tileX < tiles_x; tileX++) {
            if (plightRT) {
                ID3D11ShaderResourceView* lightAreaSRV = plightRT->GetShaderResourceView(tileLine + tileX);
                pD3DDevContext->PSSetShaderResources(2, 1, &lightAreaSRV);
            }
            //set texture stuff
            pD3DDevContext->PSSetShaderResources(0, 1, &ppTex_shaderResourceView[tileLine + tileX]);
            //set shader constant buffers 
            SetPositionRender(tileLine + tileX);
            //set pixel shader stuff
            if (pd3d_PixelShader)
                pD3DDevContext->PSSetShader(pd3d_PixelShader, nullptr, 0);
            else
                pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_32, nullptr, 0);

            pD3DDevContext->DrawIndexed(4, 0, 0);
        }
        tileLine += tiles_x;
    }
}

//_________________________________________________________
void RenderTarget2::ClearRect(XMFLOAT4 colour, RECT* pRect) {
    if (pRect && (pRect->right <= pRect->left || pRect->bottom <= pRect->top))
        return;

    if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right >= (LONG)width && pRect->bottom >= (LONG)height)) {
        float f_colour[4] = { colour.x, colour.y, colour.z, colour.w };
        ClearRenderTarget(nullptr , f_colour);
        return;
    }

    pD3DDevContext->OMSetBlendState(pBlendState_Zero, nullptr, -1);
    //set vertex stuff
    ID3D11Buffer* pd3dVB;

    GetVertexBuffer(&pd3dVB);
    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    GEN_SURFACE_BUFF_DATA genSurfaceData;
    genSurfaceData.genData4_1 = colour;
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);
    pD3DDevContext->PSSetShader(pd3d_PS_Colour32, nullptr, 0);

    DrawIndexed(4, 0, 0, nullptr, (float)pRect->left, (float)pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, pRect);

    pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
}
