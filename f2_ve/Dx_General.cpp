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

#include "Dx_General.h"
#include "Dx_Game.h"
#include "Dx_Windows.h"
#include "Dx_RenderTarget.h"
#include "Dx_Graphics.h"
#include "Dx_Mouse.h"

#include "modifications.h"
#include "memwrite.h"
#include "win_fall.h"
#include "Fall_General.h"
#include "Fall_GameMap.h"
#include "Fall_File.h"
#include "graphics.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

//vertex shaders
#include "shaders\compiled_h\VS_Basic.h"

//pixel shaders
#include "shaders\compiled_h\PS_Basic_Tex_32.h"
#include "shaders\compiled_h\PS_Basic_Tex_8.h"
#include "shaders\compiled_h\PS_Fade_Colour_32.h"

#include "shaders\compiled_h\PS_Colour_32.h"

#include "shaders\compiled_h\PS_ShadowsStep1_DrawBase.h"
#include "shaders\compiled_h\PS_ShadowsStep2_Build.h"
#include "shaders\compiled_h\PS_ShadowsStep3_Blur.h"
#include "shaders\compiled_h\PS_ShadowsStep4_RadialBlur.h"
#include "shaders\compiled_h\PS_ShadowsStep5_Combine.h"

#include "shaders\compiled_h\PS_Outline_OuterEdge8.h"
#include "shaders\compiled_h\PS_Outline_OuterEdge32.h"

#include "shaders\compiled_h\PS_Outline_Colour_8.h"
#include "shaders\compiled_h\PS_Outline_Colour_32.h"
#include "shaders\compiled_h\PS_Outline_Palette_8.h"
#include "shaders\compiled_h\PS_Outline_Palette_32.h"

#include "shaders\compiled_h\PS_ObjFlat8.h"
#include "shaders\compiled_h\PS_ObjFlat32.h"
#include "shaders\compiled_h\PS_ObjUpright8.h"
#include "shaders\compiled_h\PS_ObjUpright32.h"

#include "shaders\compiled_h\PS_DrawHexFog.h"

#include "shaders\compiled_h\PS_DrawHexLight_OriginalLighting.h"

#include "shaders\compiled_h\PS_ObjUpright8_OriginalLighting.h"
#include "shaders\compiled_h\PS_ObjUpright32_OriginalLighting.h"

#include "shaders\compiled_h\PS_DrawObjLight8.h"
#include "shaders\compiled_h\PS_DrawObjLight32.h"

#include "shaders\compiled_h\PS_DrawWallLight8.h"
#include "shaders\compiled_h\PS_DrawWallLight32.h"

#include "shaders\compiled_h\PS_RenderFloorLight32.h"

#include "shaders\compiled_h\PS_RenderRoof32.h"

#include "shaders\compiled_h\PS_GaussianBlurU.h"
#include "shaders\compiled_h\PS_GaussianBlurV.h"

#include "shaders\compiled_h\PS_Colour_32_Alpha.h"
#include "shaders\compiled_h\PS_Colour_32_Brightness_ZeroMasked.h"

#include "shaders\compiled_h\PS_Colour_32_RevAlpha_ZeroMasked.h"

#include "shaders\compiled_h\PS_Outline_Quad_32.h"


//Direct3D device and swap chain.
ID3D11Device* g_d3dDevice = nullptr;
ID3D11Device2* g_d3dDevice2 = nullptr;
ID3D11DeviceContext* g_d3dDeviceContext = nullptr;
ID3D11DeviceContext2* g_d3dDeviceContext2 = nullptr;
IDXGISwapChain* g_d3dSwapChain = nullptr;
IDXGISwapChain1* g_d3dSwapChain1 = nullptr;

//Render target view for the back buffer of the swap chain.
ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr;
//Depth/stencil view for use as a depth buffer.
ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr;
//A texture to associate to the depth stencil view.
ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;

//Define the functionality of the depth/stencil stages.
ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr;
//Define the functionality of the rasterizer stage.
ID3D11RasterizerState* g_d3dRasterizerState = nullptr;
D3D11_VIEWPORT g_Viewport = { 0 };

bool isMainPalActive = false;
DWORD* pIsPaletteCyclingEnabled = nullptr;

//for fade functions
void (**ppfall_Sound_Continue_All_FADE_REF)() = nullptr;
LONG* pIsMusicOnFlag = nullptr;
LONG* pIsSpeechOnFlag = nullptr;
//void (*func_UpdatePlayingSound)() = nullptr;

PAL_DX* main_pal = nullptr;
PAL_DX* color_pal = nullptr;

BYTE* pPalCycle_Slime = nullptr;
BYTE* pPalCycle_Shoreline = nullptr;
BYTE* pPalCycle_FireSlow = nullptr;
BYTE* pPalCycle_FireFast = nullptr;
BYTE* pPalCycle_Monitors = nullptr;
BYTE* pPalCycle_Alarm = nullptr;
int frameCount = 0;
LONG* pKeyValScreenShot = nullptr;
//max texture dimensions defined by driver
UINT max_texDim = 8192;

DWORD Dx_PresentSkip=0;

PS_BUFFERS_FALLOUT* pPS_BuffersFallout = nullptr;

// Shader data
ID3D11VertexShader* pd3dVertexShader_Main = nullptr;
ID3D11InputLayout* pd3dVS_InputLayout_Main = nullptr;

ID3D11PixelShader* pd3d_PS_Basic_Tex_32 = nullptr;
ID3D11PixelShader* pd3d_PS_Basic_Tex_8 = nullptr;

ID3D11PixelShader* pd3d_PS_Fader = nullptr;

ID3D11PixelShader* pd3d_PS_Colour32 = nullptr;


ID3D11PixelShader* pd3d_PS_Shadow1_DrawBase = nullptr;
ID3D11PixelShader* pd3d_PS_Shadow2_Build = nullptr;
ID3D11PixelShader* pd3d_PS_Shadow3_Blur = nullptr;
ID3D11PixelShader* pd3d_PS_Shadow4_RadialBlur = nullptr;
ID3D11PixelShader* pd3d_PS_Shadow5_Combine = nullptr;

ID3D11PixelShader* pd3d_PS_Outline_OuterEdge8 = nullptr;
ID3D11PixelShader* pd3d_PS_Outline_OuterEdge32 = nullptr;

ID3D11PixelShader* pd3d_PS_Outline_Colour8 = nullptr;
ID3D11PixelShader* pd3d_PS_Outline_Colour32 = nullptr;
ID3D11PixelShader* pd3d_PS_Outline_Palette8 = nullptr;
ID3D11PixelShader* pd3d_PS_Outline_Palette32 = nullptr;


ID3D11PixelShader* pd3d_PS_ObjFlat8 = nullptr;
ID3D11PixelShader* pd3d_PS_ObjFlat32 = nullptr;
ID3D11PixelShader* pd3d_PS_ObjUpright8 = nullptr;
ID3D11PixelShader* pd3d_PS_ObjUpright32 = nullptr;

ID3D11PixelShader* pd3d_PS_DrawHexFog = nullptr;

ID3D11PixelShader* pd3d_PS_DrawHexLight_OriginalLighting = nullptr;

ID3D11PixelShader* pd3d_PS_ObjUpright8_OriginalLighting = nullptr;
ID3D11PixelShader* pd3d_PS_ObjUpright32_OriginalLighting = nullptr;

ID3D11PixelShader* pd3d_PS_DrawObjLight8 = nullptr;
ID3D11PixelShader* pd3d_PS_DrawObjLight32 = nullptr;

ID3D11PixelShader* pd3d_PS_DrawWallLight8 = nullptr;
ID3D11PixelShader* pd3d_PS_DrawWallLight32 = nullptr;

ID3D11PixelShader* pd3d_PS_GaussianBlurV = nullptr;
ID3D11PixelShader* pd3d_PS_GaussianBlurU = nullptr;

ID3D11PixelShader* pd3d_PS_RenderFloorLight32 = nullptr;

ID3D11PixelShader* pd3d_PS_RenderRoof32 = nullptr;

ID3D11PixelShader* pd3d_PS_Colour_32_Alpha = nullptr;
ID3D11PixelShader* pd3d_PS_Colour_32_Brightness_ZeroMasked = nullptr;


ID3D11PixelShader* pd3d_PS_Colour_32_RevAlpha_ZeroMasked = nullptr;

ID3D11PixelShader* pd3d_PS_Outline_Quad_32 = nullptr;


ID3D11SamplerState* pd3dPS_SamplerState_Point = nullptr;
ID3D11SamplerState* pd3dPS_SamplerState_Linear = nullptr;

ID3D11BlendState* pBlendState_Zero = nullptr;
ID3D11BlendState* pBlendState_One = nullptr;
ID3D11BlendState* pBlendState_Two = nullptr;
ID3D11BlendState* pBlendState_Three = nullptr;
ID3D11BlendState* pBlendState_Four = nullptr;

//every texture is drawn to a basic quad so uses the same index buffer;
ID3D11Buffer* pVB_Quad_IndexBuffer = nullptr;

ID3D11Buffer* pVB_Quad_Line_IndexBuffer = nullptr;


ID3D11Device* GetD3dDevice() {
    return g_d3dDevice;
}
ID3D11DeviceContext* GetD3dDeviceContext() {
    return g_d3dDeviceContext;
}


//________________________________________
void Set_ViewPort(long width, long height) {
    D3D11_VIEWPORT Viewport{};
    Viewport.Width = static_cast<float>(width);
    Viewport.Height = static_cast<float>(height);
    Viewport.TopLeftX = 0.0f;
    Viewport.TopLeftY = 0.0f;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    g_d3dDeviceContext->RSSetViewports(1, &Viewport);

    D3D11_RECT rect{ 0,0,width ,height };
    g_d3dDeviceContext->RSSetScissorRects(1, &rect);
}


//____________________________________________________________
void Set_ViewPort(float x, float y, float width, float height) {
    D3D11_VIEWPORT Viewport{};
    Viewport.Width = width;
    Viewport.Height = height;
    Viewport.TopLeftX = x;
    Viewport.TopLeftY = y;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    g_d3dDeviceContext->RSSetViewports(1, &Viewport);

    D3D11_RECT rect{ (LONG)x,(LONG)y, (LONG)x + (LONG)width , (LONG)y + (LONG)height };
    g_d3dDeviceContext->RSSetScissorRects(1, &rect);
}


ID3D11RenderTargetView* GetRenderTargetView() {
    return g_d3dRenderTargetView;
}
ID3D11DepthStencilView* GetDepthStencilView() {
    return g_d3dDepthStencilView;
}
ID3D11DepthStencilState* GetDepthStencilState() {
    return g_d3dDepthStencilState;
}


//___________________________
void STORED_VIEW::StoreView() {

    g_d3dDeviceContext->OMGetRenderTargets(1, &pRenderTargetViews, &pDepthStencilView);

    g_d3dDeviceContext->RSGetViewports(&numViewPorts, nullptr);
    if (numViewPorts > 0) {
        if (pCViewPorts)
            delete[] pCViewPorts;

        pCViewPorts = new D3D11_VIEWPORT[numViewPorts];
        g_d3dDeviceContext->RSGetViewports(&numViewPorts, pCViewPorts);

        if (pScissorRects)
            delete[] pScissorRects;

        pScissorRects = new D3D11_RECT[numViewPorts];
        g_d3dDeviceContext->RSGetScissorRects(&numViewPorts, pScissorRects);
    }
}


//_____________________________
void STORED_VIEW::ReStoreView() {

    g_d3dDeviceContext->OMSetRenderTargets(1, &pRenderTargetViews, pDepthStencilView);

    if (pCViewPorts != nullptr) {
        g_d3dDeviceContext->RSSetViewports(numViewPorts, pCViewPorts);
    }
    if (pScissorRects != nullptr) {
        g_d3dDeviceContext->RSSetScissorRects(numViewPorts, pScissorRects);
    }
}


RenderTarget* display01 = nullptr;
RenderTarget* display02 = nullptr;
///RenderTarget* display03 = nullptr;
///RenderTarget* display04 = nullptr;
//void GameAreas_GetScale(float* pScaleX, float* pScaleY);


//__________________________________
void Display_Release_RenderTargets() {
    if (display01)
        delete display01;
    display01 = nullptr;
    if (display02)
        delete display02;
    display02 = nullptr;
    /*if (display03)
        delete display03;
    display03 = nullptr;
    if (display04)
        delete display04;
    display04 = nullptr;*/

}


//_______________
void Dx_Present() {
    if (!Is_WindowActive() || isMapperSizing) 
        return;
    if (!g_d3dDeviceContext) 
        return;
    if (Dx_PresentSkip)
        return;

    float colour[4]{ 0.0f,0.0f,0.0f,0.0f };
    g_d3dDeviceContext->ClearRenderTargetView(g_d3dRenderTargetView, colour);
    g_d3dDeviceContext->ClearDepthStencilView(g_d3dDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    if (!display01) {
        display01 = new RenderTarget(0, 0, SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI, DXGI_FORMAT_B8G8R8A8_UNORM, 0xFFFF0000);
        if (scaleLevel_GUI > 1)
            display01->SetScale(1.0f / scaleLevel_GUI, 1.0f / scaleLevel_GUI);
    }

    colour[0] = 0.0f, colour[1] = 0.0f, colour[2] = 0.0f, colour[3] = 0.0f;
    display01->ClearRenderTarget(g_d3dDepthStencilView, colour);
    display01->SetRenderTarget(g_d3dDepthStencilView);

    //g_d3dDeviceContext->OMSetRenderTargets(1, &g_d3dRenderTargetView, g_d3dDepthStencilView);
    //g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 0);
    //SetScreenViewPort();
    Set_ViewPort(SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI);

    WinStructDx** pwinDxArray = (WinStructDx**)pWin_Array;
    for (LONG i = 1; i < *p_num_windows; i++) {
        //if (pwinDxArray[i]->ref != *pWinRef_GameArea) {
            if (pwinDxArray[i]->winDx) {
                if (!(pwinDxArray[i]->flags & FLG_WinHidden)) {
                    pwinDxArray[i]->winDx->Display();
                    Display_Buttons(pwinDxArray[i]);
                }
            }
        //}
        //else {
         //   GameAreas_Display();
            /*if (display03) {
                float scaleX = 0;
                float scaleY = 0;
                GameAreas_GetScale(&scaleX, &scaleY);
                display03->SetScale(scaleX, scaleY);
                display03->SetPosition(0, 0);
                g_d3dDeviceContext->OMSetBlendState(pBlendState_Three, colour, -1);
                display01->SetRenderTargetAndViewPort(g_d3dDepthStencilView);
                display03->Display(pd3d_PS_Basic_Tex_32);
                g_d3dDeviceContext->OMSetBlendState(pBlendState_One, colour, -1);
            }*/
        //}
    }

    if (isMainPalActive)
        MouseDx_Display();

    if (!display02) {
        display02 = new RenderTarget(0, 0, SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI, DXGI_FORMAT_B8G8R8A8_UNORM, 0xFFFF0000);
        if (scaleLevel_GUI > 1)
            display02->SetScale(1.0f / scaleLevel_GUI, 1.0f / scaleLevel_GUI);
    }
    display02->ClearRenderTarget(g_d3dDepthStencilView, colour);

    display02->SetRenderTarget(g_d3dDepthStencilView);
    display01->Display(pd3d_PS_Basic_Tex_32);

    /*genSurfaceData.genData4_1 = { static_cast<float>(SCR_WIDTH << scaleLevel_GUI) , static_cast<float>(SCR_HEIGHT << scaleLevel_GUI) , 0.0f, 1.0f };
    pPS_BuffersFallout->UpdateBaseBuff(&genSurfaceData);

    g_d3dDeviceContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Linear);
    for (int i = 0; i < 1; i++) {
        display02->SetRenderTarget(g_d3dDepthStencilView);
        //display02->Display(pd3d_PS_GaussianBlurV);
        display01->Display(pd3d_PS_GaussianBlurU);
        display01->SetRenderTarget(g_d3dDepthStencilView);
        display02->Display(pd3d_PS_GaussianBlurV);
        //display02->Display(pd3d_PS_GaussianBlurU);
    }
    g_d3dDeviceContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Point);*/

    display01->SetRenderTarget(g_d3dDepthStencilView);
    display02->Display(pd3d_PS_Fader);
    Subtitles_Display();



    g_d3dDeviceContext->OMSetRenderTargets(1, &g_d3dRenderTargetView, g_d3dDepthStencilView);
    g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 0);
    Set_ViewPort(SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI);

    display01->Display(pd3d_PS_Basic_Tex_32);// (display01PassList, 2);

   // display01->Display(pd3d_PS_Fader);// (display01PassList, 2);
    HRESULT hr = g_d3dSwapChain->Present(1, 0);
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            //To-Do HandleDeviceLost();
            Fallout_Debug_Error("Present - DeviceLost.");
        }
        else
            Fallout_Debug_Error("Present Failed.");
    }

}



//______________________________________________________________________________________________________
//Finds the refresh rate for the primary video card and monitor using the choosen screen width and height.
//This function utilizes code taken from a tutorial at http://www.rastertek.com/, http://www.rastertek.com/dx11s2tut03.html
bool GetRefreshRate(DXGI_FORMAT format, UINT screenWidth, UINT screenHeight, DXGI_RATIONAL* refreshRate) {
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;

    //Create a DirectX graphics interface factory.
    HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result))
        return false;


    //Use the factory to create an adapter for the primary graphics interface (video card).
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result))
        return false;

    //Enumerate the primary adapter output (monitor).
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result))
        return false;

    UINT numModes = 0;
    //Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
    result = adapterOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
    if (FAILED(result))
        return false;

    //Create a list to hold all the possible display modes for this monitor/video card combination.
    DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];


    // Now fill the display mode list structures.
    result = adapterOutput->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    if (FAILED(result))
        return false;


    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    bool matchFound = false;
    for (UINT i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight) {
            refreshRate->Numerator = displayModeList[i].RefreshRate.Numerator;
            refreshRate->Denominator = displayModeList[i].RefreshRate.Denominator;
            matchFound = true;
        }
    }

    //We now have the numeratorand denominator for the refresh rate.The last thing we will retrieve using the adapter is the name of the video card and the amount of memory on the video card.
    /*
        // Get the adapter (video card) description.
        result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result))
    {
        return false;
    }

    // Store the dedicated video card memory in megabytes.
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // Convert the name of the video card to a character array and store it.
    error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0)
    {
        return false;
    }
    */
    // Now that we have stored the numeratorand denominator for the refresh rateand the video card information we can release the structuresand interfaces used to get that information.

     // Release the display mode list.
    delete[] displayModeList;
    displayModeList = 0;

    // Release the adapter output.
    adapterOutput->Release();
    adapterOutput = 0;

    // Release the adapter.
    adapter->Release();
    adapter = 0;

    // Release the factory.
    factory->Release();
    factory = 0;

    return matchFound;
}



LARGE_INTEGER refreshTime;
LARGE_INTEGER Frequency;
LARGE_INTEGER lastPresentTime;
float transitional_fade_unit = 1.0f;
float refreshTime_ms = 1.0f;

LARGE_INTEGER transitional_fade_last_time = { 0LL };
//default transitional fade-time in seconds
float transitional_fade_seconds_default = 2.0f;

bool transitional_fade_in = false;
bool transitional_fade_complete = true;

XMFLOAT4 transitional_fade_colour = { 0.0f,0.0f,0.0f,0.0f };


//_____________________________________________
void TransitionalFader_SetTime(float f_seconds) {
    transitional_fade_unit = refreshTime_ms / f_seconds;
}


//_________________________________________________
void TransitionalFader_SetColour(XMFLOAT4 inColour) {
    transitional_fade_colour = inColour;// {inColour.x, inColour.y, inColour.z, inColour.w};
    pPS_BuffersFallout->UpdateFadeColour(&transitional_fade_colour);
}


//______________________
bool TransitionalFader() {
    if (transitional_fade_complete)
        return false;

    LARGE_INTEGER thisTime = { 0LL };
    LARGE_INTEGER ElapsedMicroseconds = { 0LL };

    QueryPerformanceCounter(&thisTime);
    if (transitional_fade_last_time.QuadPart == 0)
        transitional_fade_last_time.QuadPart = thisTime.QuadPart;

    ElapsedMicroseconds.QuadPart = thisTime.QuadPart - transitional_fade_last_time.QuadPart;
    ElapsedMicroseconds.QuadPart *= 1000000LL;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
    if (ElapsedMicroseconds.QuadPart < 0)
        transitional_fade_last_time.QuadPart = thisTime.QuadPart;
    else if(ElapsedMicroseconds.QuadPart > refreshTime.QuadPart) {
        if (transitional_fade_in) {
            if (transitional_fade_colour.w > 0) {
                transitional_fade_colour.w -= transitional_fade_unit;
                if (transitional_fade_colour.w < 0.0f)
                    transitional_fade_colour.w = 0.0f;
                pPS_BuffersFallout->UpdateFadeColour(&transitional_fade_colour);
                transitional_fade_last_time.QuadPart = thisTime.QuadPart;
            }
            else {
                transitional_fade_complete = true;
                transitional_fade_unit = refreshTime_ms / transitional_fade_seconds_default;

            }
            return true;
        }
        else {
            if (transitional_fade_colour.w < 1.0f) {
                transitional_fade_colour.w += transitional_fade_unit;
                if (transitional_fade_colour.w > 1.0f)
                    transitional_fade_colour.w = 1.0f;
                pPS_BuffersFallout->UpdateFadeColour(&transitional_fade_colour);
                transitional_fade_last_time.QuadPart = thisTime.QuadPart;
            }
            else {
                transitional_fade_complete = true;
                transitional_fade_unit = refreshTime_ms / transitional_fade_seconds_default;
            }
            return true;
        }
    }
    return false;
}


//________________________________________
void TransitionalFader_Start(bool fade_in) {
    transitional_fade_in = fade_in;
    transitional_fade_last_time = { 0LL };
    transitional_fade_complete = false;
}


//_______________________________
void TransitionalFader_Complete() {
    if (transitional_fade_complete)
        return;
    if (*pIsMusicOnFlag != 0 || *pIsSpeechOnFlag != 0) {
        *ppfall_Sound_Continue_All_FADE_REF = fall_Sound_Continue_ALL;
    }
    else
        *ppfall_Sound_Continue_All_FADE_REF = nullptr;

    while (!transitional_fade_complete) {
        if (TransitionalFader())
            Dx_Present();
        if (*ppfall_Sound_Continue_All_FADE_REF)
            (*ppfall_Sound_Continue_All_FADE_REF)();
        CheckMessagesNoWait();
    }
}


//____________________________
void FadeToPalette(BYTE* pPal) {
    TransitionalFader_Complete();//finish last fade if still running
    if (transitional_fade_colour.w == 1.0f)
        transitional_fade_in = true;

    if (pPal == pBLACK_PAL || pPal == pWHITE_PAL) {
        transitional_fade_in = false;
        if (pPal == pBLACK_PAL)
            transitional_fade_colour = { 0.0f,0.0f,0.0f,0.0f };
        else if (pPal == pWHITE_PAL)
            transitional_fade_colour = { 1.0f,1.0f,1.0f,0.0f };
    }
    else
        fall_Palette_SetActive(pPal);

    transitional_fade_last_time = { 0LL };
    transitional_fade_complete = false;
    TransitionalFader_Complete();//run the new fade
}


//__________________________________________
void __declspec(naked) fade_to_palette(void) {

    __asm {
        pushad
        push eax
        call FadeToPalette
        add esp, 0x4
        popad
        ret;
    }
}


//________________________________
void SetPalette_Fading(BYTE* pPal) {
    TransitionalFader_Complete();//finish last fade if still running
    if (pPal == pBLACK_PAL || pPal == pWHITE_PAL) {

        if (pPal == pBLACK_PAL)
            transitional_fade_colour = { 0.0f,0.0f,0.0f,1.0f };
        else if (pPal == pWHITE_PAL)
            transitional_fade_colour = { 1.0f,1.0f,1.0f,1.0f };
        pPS_BuffersFallout->UpdateFadeColour(&transitional_fade_colour);
        Dx_Present();
        //}
        return;//don't change palette to fade
    }
    else {//set fade alpha to 0
        transitional_fade_colour.w = 0.0f;
        pPS_BuffersFallout->UpdateFadeColour(&transitional_fade_colour);
        fall_Palette_SetActive(pPal);
    }

    return;
}


//_____________________________________________
void __declspec(naked) set_palette_fading(void) {

    __asm {
        pushad
        push eax
        call SetPalette_Fading
        add esp, 0x4
        popad
        ret
    }
}


//__________________________
void Dx_Present_Main_Setup() {

    DXGI_RATIONAL refreshRate;
    HMONITOR hmon = MonitorFromWindow(hGameWnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monInfo;
    ZeroMemory(&monInfo, sizeof(MONITORINFO));
    monInfo.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfoA(hmon, &monInfo)) {
        UINT scrWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
        UINT scrHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
        if (!GetRefreshRate(DXGI_FORMAT_B8G8R8A8_UNORM, scrWidth, scrHeight, &refreshRate))
            refreshRate.Numerator = 60, refreshRate.Denominator = 1;
    }
    else
        refreshRate.Numerator = 60, refreshRate.Denominator = 1;
 
    float f_refreshrate = static_cast<float>(refreshRate.Numerator / refreshRate.Denominator);

    if (f_refreshrate > 0) {
        refreshTime.QuadPart = 1000000LL * refreshRate.Denominator / refreshRate.Numerator;
        refreshTime_ms = refreshRate.Numerator / refreshRate.Denominator / 1000.0f;
    }
    else {
        refreshTime.QuadPart = 1000000LL / 60LL;
        refreshTime_ms = 0.06f;
    }
    transitional_fade_unit = refreshTime_ms / transitional_fade_seconds_default;

    QueryPerformanceFrequency(&Frequency);
}


///int negCount = 0;
ULONGLONG frmCacheStatusTime = 0;
DWORD nodeCount;
//char msgCount[64];
//____________________
void Dx_Present_Main() {
    TransitionalFader();
    MouseDx_UpdateAnimation();
    GameAreas_FadingObjects_Draw();

    if (frmDxCache) {
        frmDxCache->ClearFrmCache();
        ULONGLONG time = GetTickCount64();
        if (time > frmCacheStatusTime) {
            //frmDxCache->PrintStats();
            //sprintf_s(msgCount, "nodeCount %d", nodeCount);
            //imonitorInsertText(msgCount);
            frmCacheStatusTime = time + 10000;
        }
    }

    LARGE_INTEGER time = { 0 };
    LARGE_INTEGER ElapsedMicroseconds = { 0 };

    QueryPerformanceCounter(&time);
    ElapsedMicroseconds.QuadPart = time.QuadPart - lastPresentTime.QuadPart;
    ElapsedMicroseconds.QuadPart *= 1000000LL;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
    if (ElapsedMicroseconds.QuadPart < 0) {
        lastPresentTime.QuadPart = time.QuadPart;
        ///negCount++;
    }
    else if (ElapsedMicroseconds.QuadPart > refreshTime.QuadPart) {
       // if (!MouseDx_IsScrolling())//already presenting when scrolling - slows things down to present here
            Dx_Present();
        lastPresentTime.QuadPart = time.QuadPart;
    }

}


//_______________________
void PresentTimingSetup() {
    Dx_Present_Main_Setup();

    fall_Event_Add(&Dx_Present_Main);
    fall_Palette_FadeTo(pLOADED_PAL);
}


//_______________________________________________
void __declspec(naked) present_timing_setup(void) {

    __asm {
        pushad
        call PresentTimingSetup
        popad
        ret;
    }
}


//_____________________________________________________________________________________________________
HRESULT SaveTextureToFile(ID3D11Texture2D* pTex_Source, const char* pfile_path, bool isFalloutDataPath) {
    HRESULT hresult;
    ID3D11Device* pD3DDev = GetD3dDevice();
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();

    if (!pTex_Source)
        return -1;
    D3D11_TEXTURE2D_DESC desc{ 0 };
    D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    pTex_Source->GetType(&resType);
    if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    pTex_Source->GetDesc(&desc);

    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    ID3D11Texture2D* pTex_Dest;
    hresult = pD3DDev->CreateTexture2D(&desc, nullptr, &pTex_Dest);
    if (FAILED(hresult))
        return hresult;

    pD3DDevContext->CopyResource(pTex_Dest, pTex_Source);


    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result;
    result = pD3DDevContext->Map(pTex_Dest, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(result)) {
        Fallout_Debug_Error("SaveTextureToFile - Failed to Map screenshot texture.");
        return hresult;
    }

    WORD pixelSizeBits = 32;

    if (isFalloutDataPath) {//using fallouts data path and file functions
        if (!SaveBMP_DATA(pfile_path, desc.Width, desc.Height, pixelSizeBits, mappedResource.RowPitch, (BYTE*)mappedResource.pData, nullptr, 0, 0, true))// SaveScreenShot(desc.Width, desc.Height, pixelSizeBits, mappedResource.RowPitch, (BYTE*)mappedResource.pData))
            hresult = E_FAIL;
    }
    else {//using local path and regular c file functions
        if (!SaveBMP(pfile_path, desc.Width, desc.Height, pixelSizeBits, mappedResource.RowPitch, (BYTE*)mappedResource.pData, nullptr, 0, 0, true))// SaveScreenShot(desc.Width, desc.Height, pixelSizeBits, mappedResource.RowPitch, (BYTE*)mappedResource.pData))
            hresult = E_FAIL;
    }
    pD3DDevContext->Unmap(pTex_Dest, 0);

    return hresult;
}




//____________________________________________________
HRESULT HandleScreenshot(ID3D11Texture2D* pTex_Source) {
    HRESULT hresult;
    ID3D11Device* pD3DDev = GetD3dDevice();
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();

    if (!pTex_Source)
        return -1;
    D3D11_TEXTURE2D_DESC desc{ 0 };
    D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    pTex_Source->GetType(&resType);
    if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    pTex_Source->GetDesc(&desc);

    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    ID3D11Texture2D* pTex_Dest;
    hresult = pD3DDev->CreateTexture2D(&desc, nullptr, &pTex_Dest);
    if (FAILED(hresult))
        return hresult;

    pD3DDevContext->CopyResource(pTex_Dest, pTex_Source);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result;
    result = pD3DDevContext->Map(pTex_Dest, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(result)) {
        Fallout_Debug_Error("HandleScreenshot - Failed to Map screenshot texture.");
        return hresult;
    }

    WORD pixelSizeBits = 32;

    if (!SaveScreenShot(desc.Width, desc.Height, pixelSizeBits, mappedResource.RowPitch, (BYTE*)mappedResource.pData))
        hresult = E_FAIL;
    pD3DDevContext->Unmap(pTex_Dest, 0);

    return hresult;
}


//___________________
void TakeScreenShot() {
    if (HandleScreenshot(display01->GetTexture()) != S_OK)
        Fallout_Debug_Error("TakeScreenShot - Failed HandleScreenshot.");
}


//____________________________
void TakeScreenShot_GameArea() {
    //To-Do TakeScreenShot_GameArea this isn't going to work if game area is greater than texture max size.
    GAME_AREA* pGame = GameAreas_GetCurrentArea();
    Window_DX* pWinDx = new Window_DX(0, 0, pGame->width, pGame->height, 0, nullptr, nullptr);
    GameAreas_DrawToWindow(pWinDx, *pVIEW_HEXPOS, pGame->width, pGame->height);
    if (HandleScreenshot(pWinDx->GetTexture()) != S_OK)
        Fallout_Debug_Error("TakeScreenShot_GameArea - Failed HandleScreenshot.");
    delete pWinDx;
    pWinDx = nullptr;
}


//__________________________________________
void __declspec(naked)take_screen_shot(void) {

    __asm {
        pushad
        call TakeScreenShot
        popad
        ret
    }
}


//___________________________________________
void __declspec(naked)screen_shot_check(void) {

    __asm {

        cmp eax, 0x18A
        jne exitFunc
        pushad
        call TakeScreenShot_GameArea
        popad
        add dword ptr ss : [esp] , 7
        ret
        exitFunc :
        push ebx
            mov ebx, pKeyValScreenShot
            cmp eax, dword ptr ds : [ebx]
            pop ebx
            ret
    }
}


ULONGLONG cycle_speed_factor = 0;
ULONGLONG cycle_time_1 = 0;
ULONGLONG cycle_time_2 = 0;
ULONGLONG cycle_time_3 = 0;
ULONGLONG cycle_time_4 = 0;

//____________________
void Color_Pal_Cycle() {

    if (!color_pal)
        return;
    ULONGLONG time = GetTickCount64();
    DWORD flags = 0;

    //slow_fire, shoreline, slime
    if (time < cycle_time_1 || time - cycle_time_1 >= cycle_speed_factor * PAL_TIME_1) {
        cycle_time_1 = time;
        flags |= PAL_TIME_ZONE_1;
    }

    //fast_fire
    if (time < cycle_time_2 || time - cycle_time_2 >= cycle_speed_factor * PAL_TIME_2) {
        cycle_time_2 = time;
        flags |= PAL_TIME_ZONE_2;
    }
    //monitors
    if (time < cycle_time_3 || time - cycle_time_3 >= cycle_speed_factor * PAL_TIME_3) {
        cycle_time_3 = time;
        flags |= PAL_TIME_ZONE_3;
    }

    //alarm
    if (time < cycle_time_4 || time - cycle_time_4 >= cycle_speed_factor * PAL_TIME_4) {
        cycle_time_4 = time;
        flags |= PAL_TIME_ZONE_4;

    }

    if (flags & 0x0000000F) {
        DWORD temp = 0;
        DWORD* pBuff = nullptr;
        if (SUCCEEDED(color_pal->Lock(&pBuff, D3D11_MAP_WRITE))) {
            pBuff += 229;
            if (flags & PAL_TIME_ZONE_1) {
                temp = pBuff[3];
                pBuff[3] = pBuff[2];
                pBuff[2] = pBuff[1];
                pBuff[1] = pBuff[0];
                pBuff[0] = temp;

                temp = pBuff[13];
                pBuff[13] = pBuff[12];
                pBuff[12] = pBuff[11];
                pBuff[11] = pBuff[10];
                pBuff[10] = pBuff[9];
                pBuff[9] = temp;

                temp = pBuff[24];
                pBuff[24] = pBuff[23];
                pBuff[23] = pBuff[22];
                pBuff[22] = pBuff[21];
                pBuff[21] = pBuff[20];
                pBuff[20] = pBuff[19];
                pBuff[19] = temp;
            }
            if (flags & PAL_TIME_ZONE_2) {
                temp = pBuff[18];
                pBuff[18] = pBuff[17];
                pBuff[17] = pBuff[16];
                pBuff[16] = pBuff[15];
                pBuff[15] = pBuff[14];
                pBuff[14] = temp;
            }
            if (flags & PAL_TIME_ZONE_3) {
                temp = pBuff[8];
                pBuff[8] = pBuff[7];
                pBuff[7] = pBuff[6];
                pBuff[6] = pBuff[5];
                pBuff[5] = pBuff[4];
                pBuff[4] = temp;
            }
            if (flags & PAL_TIME_ZONE_4) {
                pBuff[25] -= 0xFFFC0000;
            }
            color_pal->Unlock(229, 256 - 229);
        }
        DrawPalAniObjs(*pMAP_LEVEL, flags);
        ///fall_Windows_Draw_Rect(pFALL_RC);///needed for non converted windows for pal effects to show
    }
}


//___________________
bool Color_Pal_Init() {

    if (color_pal == nullptr)
        color_pal = new PAL_DX(256, true);
    if (color_pal == nullptr)
        return false;

    void* palStream = fall_fopen("color.pal", "rb");
    if (palStream) {
        DWORD* pBuff = nullptr;
        if (SUCCEEDED(color_pal->Lock(&pBuff, D3D11_MAP_WRITE))) {
            BYTE r, g, b;
            DWORD offset_tri = 0;
            for (DWORD offset = 0; offset < 256; offset++) {
                fall_fread8(palStream, &r);
                fall_fread8(palStream, &g);
                fall_fread8(palStream, &b);
                pBuff[offset] = RGBA_32BIT(0xFF, r << 2, g << 2, b << 2);


                if (offset == 0)/// //set alpha to 0 - transparent
                    pBuff[offset] = pBuff[offset] & 0x00FFFFFF;//ARGB;
                else if (offset == PAL_ANI_START_6)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_Alarm[0], pPalCycle_Alarm[0 + 1], pPalCycle_Alarm[0 + 2]);
                else if (offset >= PAL_ANI_START_5)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_Shoreline[offset_tri - 744], pPalCycle_Shoreline[offset_tri - 744 + 1], pPalCycle_Shoreline[offset_tri - 744 + 2]);
                else if (offset >= PAL_ANI_START_4)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_FireFast[offset_tri - 729], pPalCycle_FireFast[offset_tri - 729 + 1], pPalCycle_FireFast[offset_tri - 729 + 2]);
                else if (offset >= PAL_ANI_START_3)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_FireSlow[offset_tri - 714], pPalCycle_FireSlow[offset_tri - 714 + 1], pPalCycle_FireSlow[offset_tri - 714 + 2]);
                else if (offset >= PAL_ANI_START_2)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_Monitors[offset_tri - 699], pPalCycle_Monitors[offset_tri - 699 + 1], pPalCycle_Monitors[offset_tri - 699 + 2]);
                else if (offset >= PAL_ANI_START_1)
                    pBuff[offset] = RGBA_32BIT(0xFF, pPalCycle_Slime[offset_tri - 687], pPalCycle_Slime[offset_tri - 687 + 1], pPalCycle_Slime[offset_tri - 687 + 2]);
                offset_tri += 3;
            }
            color_pal->Unlock(0, 256);
        }
        fall_fclose(palStream);
        palStream = nullptr;
    }
    FalloutCfg_Read_Int("system", "cycle_speed_factor", (int*)&cycle_speed_factor);
    return true;
}


//____________________________________________________________
void Main_Pal_SetEntries(BYTE* palette, int offset, int count) {
    if (main_pal == nullptr)
        return;
    main_pal->SetPalEntries(palette, offset, count);
    fall_Windows_Draw_Rect(pFALL_RC);
}


//________________________________________________________
void Main_Pal_SetEntry(int offset, BYTE r, BYTE g, BYTE b) {
    if (main_pal == nullptr)
        return;
    main_pal->SetPalEntry(offset, r, g, b);
    fall_Windows_Draw_Rect(pFALL_RC);
}


//_____________________________________________
void __declspec(naked) main_pal_set_entry(void) {

    __asm {
        push esi
        push edi
        push ebp

        push ecx
        push ebx
        push edx
        push eax
        call Main_Pal_SetEntry
        add esp, 0x10

        pop ebp
        pop edi
        pop esi
        ret
    }
}


//_______________________________________________
void __declspec(naked) main_pal_set_entries(void) {

    __asm {
        push ecx
        push esi
        push edi
        push ebp

        push ebx
        push edx
        push eax
        call Main_Pal_SetEntries
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        ret
    }
}


//___________________________________________________
void __declspec(naked) main_pal_set_all_entries(void) {

    __asm {
        push edx
        push ebx
        push ecx
        push esi
        push edi
        push ebp

        push 256
        push 0
        push eax
        call Main_Pal_SetEntries
        add esp, 0xC

        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop edx
        ret
    }
}


//_________________________________________________________________________________________________________
HRESULT __stdcall GetPalEntriesX(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries) {
    if (lpEntries == nullptr)
        return S_FALSE;
    if (main_pal == nullptr)
        return S_FALSE;
    main_pal->GetPalEntries(dwFlags, dwBase, dwNumEntries, lpEntries);

    return S_OK;
}


//____________________________________
bool GetDefaultPalette(DWORD* palette) {
    if (palette == nullptr)
        return false;

    if (main_pal == nullptr)
        return false;
    main_pal->GetPalEntries(0, 0, 256, (LPPALETTEENTRY)palette);

    return true;
}


//__________________________________
bool GetColorPalette(DWORD* palette) {
    if (palette == nullptr)
        return false;

    if (color_pal == nullptr)
        return false;
    color_pal->GetPalEntries(0, 0, 256, (LPPALETTEENTRY)palette);

    return true;
}


//___________
void DxExit() {
    MouseDx_Destroy();

    if (color_pal)
        delete color_pal;
    color_pal = nullptr;

    if (main_pal)
        delete main_pal;
    main_pal = nullptr;

    if (g_d3dDeviceContext)
        g_d3dDeviceContext->ClearState();

    if (g_d3dDepthStencilView != nullptr) {
        g_d3dDepthStencilView->Release();
        g_d3dDepthStencilView = nullptr;
    }
    if (g_d3dRenderTargetView != nullptr) {
        g_d3dRenderTargetView->Release();
        g_d3dRenderTargetView = nullptr;
    }
    if (g_d3dDepthStencilBuffer != nullptr) {
        g_d3dDepthStencilBuffer->Release();
        g_d3dDepthStencilBuffer = nullptr;
    }
    if (g_d3dDepthStencilState != nullptr) {
        g_d3dDepthStencilState->Release();
        g_d3dDepthStencilState = nullptr;
    }
    if (g_d3dRasterizerState != nullptr) {
        g_d3dRasterizerState->Release();
        g_d3dRasterizerState = nullptr;
    }
    if (g_d3dSwapChain1 != nullptr) {
        g_d3dSwapChain1->Release();
        g_d3dSwapChain1 = nullptr;
    }
    if (g_d3dSwapChain != nullptr) {
        g_d3dSwapChain->Release();
        g_d3dSwapChain = nullptr;
    }
    if (g_d3dDeviceContext2 != nullptr) {
        g_d3dDeviceContext2->Release();
        g_d3dDeviceContext2 = nullptr;
    }
    if (g_d3dDeviceContext != nullptr) {
        g_d3dDeviceContext->Release();
        g_d3dDeviceContext = nullptr;
    }
    if (g_d3dDevice2 != nullptr) {
        g_d3dDevice2->Release();
        g_d3dDevice2 = nullptr;
    }
    if (g_d3dDevice != nullptr) {
        g_d3dDevice->Release();
        g_d3dDevice = nullptr;
    }

    Shader_Main_Destroy();
}



//___________
int DxSetup() {

    HRESULT hr = S_OK;
    HMONITOR hmon = MonitorFromWindow(hGameWnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monInfo;
    ZeroMemory(&monInfo, sizeof(MONITORINFO));
    monInfo.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfoA(hmon, &monInfo))
        Fallout_Debug_Error("DxSetup - Failed to GetMonitorInfoA.");
    UINT scrWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
    UINT scrHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;

    RECT clientRect;
    GetClientRect(hGameWnd, &clientRect);

    //Get the window client width and height.
    unsigned int clientWidth = clientRect.right - clientRect.left;
    unsigned int clientHeight = clientRect.bottom - clientRect.top;

    //Creates a device that supports BGRA formats (DXGI_FORMAT_B8G8R8A8_UNORM and DXGI_FORMAT_B8G8R8A8_UNORM_SRGB).
    UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;// | D3D11_CREATE_DEVICE_SINGLETHREADED;

#if defined(_DEBUG)
    // If the project is in a debug build, enable debugging via SDK Layers.
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        //To-Do DON'T FORGET TO TURN THESE BACK ON-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    //Create the DX 11 device and context.
    hr = D3D11CreateDevice(
        nullptr, //use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &g_d3dDevice, //The created device.
        nullptr,
        &g_d3dDeviceContext //The created device context.
    );
    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed D3D11CreateDevice.");
        return -1;
    }

    //Get the maximum texture size of the supported feature level - large game map render targets are tiled to this limit. 
    D3D_FEATURE_LEVEL currentFeatureLevel = g_d3dDevice->GetFeatureLevel();

    if (currentFeatureLevel == D3D_FEATURE_LEVEL_9_1 || currentFeatureLevel == D3D_FEATURE_LEVEL_9_2)
        max_texDim = 2048;
    else if (currentFeatureLevel == D3D_FEATURE_LEVEL_9_3)
        max_texDim = 4096;
    else if (currentFeatureLevel == D3D_FEATURE_LEVEL_10_1 || currentFeatureLevel == D3D_FEATURE_LEVEL_10_0)
        max_texDim = 8192;
    else max_texDim = 16384;

    //testing
    //max_texDim = 512;


    // Get the DXGI factory from the device.
    IDXGIFactory1* dxgiFactory = nullptr;
  
    IDXGIDevice* dxgiDevice = nullptr;
    hr = g_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (SUCCEEDED(hr)) {
        IDXGIAdapter* adapter = nullptr;
        hr = dxgiDevice->GetAdapter(&adapter);
        if (SUCCEEDED(hr)) {
            hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
            adapter->Release();
        }
        dxgiDevice->Release();
    }
  
    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed Obtain DXGI factory.");
        return -1;
    }

    UINT bufferCount = 2;
    //Create the swap chain.
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2) {
        //DirectX 11.1 or later
        hr = g_d3dDevice->QueryInterface(__uuidof(ID3D11Device2), reinterpret_cast<void**>(&g_d3dDevice2));
        if (SUCCEEDED(hr))
            (void)g_d3dDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext2), reinterpret_cast<void**>(&g_d3dDeviceContext2));


        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = clientWidth;
        sd.Height = clientHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Stereo = false;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = bufferCount;
        sd.Scaling = DXGI_SCALING_NONE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;// DXGI_ALPHA_MODE_IGNORE;
        sd.Flags = 0;
        hr = dxgiFactory2->CreateSwapChainForHwnd(g_d3dDevice, hGameWnd, &sd, nullptr, nullptr, &g_d3dSwapChain1);
        if (SUCCEEDED(hr))
            hr = g_d3dSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_d3dSwapChain));
        dxgiFactory2->Release();
    }
    else {
        //DirectX 11.0
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = bufferCount;
        sd.BufferDesc.Width = clientWidth;
        sd.BufferDesc.Height = clientHeight;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        GetRefreshRate(DXGI_FORMAT_R8G8B8A8_UNORM, scrWidth, scrHeight, &sd.BufferDesc.RefreshRate);
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.OutputWindow = hGameWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_d3dDevice, &sd, &g_d3dSwapChain);
    }

    dxgiFactory->Release();

    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed CreateSwapChainForHwnd.");
        return -1;
    }

    //Initialize the back buffer of the swap chain and associate it to a render target view.
    ID3D11Texture2D* backBuffer = nullptr;
    hr = g_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed GetBuffer.");
        return -1;
    }

    hr = g_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_d3dRenderTargetView);
    if (backBuffer != nullptr)
        backBuffer->Release();
    backBuffer = nullptr;
    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed CreateRenderTargetView.");
        return -1;
    }

    //Create the depth buffer for use with the depth/stencil view.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.Width = clientWidth;
    depthStencilBufferDesc.Height = clientHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.SampleDesc.Count = 1;
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = g_d3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &g_d3dDepthStencilBuffer);
    if (FAILED(hr)) 
        return -1;


    hr = g_d3dDevice->CreateDepthStencilView(g_d3dDepthStencilBuffer, nullptr, &g_d3dDepthStencilView);
    if (FAILED(hr))
        return -1;
  

    //Setup depth/stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    //Create a depth stencil state for 2D rendering.
    depthStencilStateDesc.DepthEnable = FALSE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = TRUE;
    depthStencilStateDesc.StencilReadMask = 0xFF;
    depthStencilStateDesc.StencilWriteMask = 0xFF;
    depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = g_d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, &g_d3dDepthStencilState);


    //Setup rasterizer state.
    D3D11_RASTERIZER_DESC RSDesc;
    memset(&RSDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
    RSDesc.FillMode = D3D11_FILL_SOLID;
    RSDesc.CullMode = D3D11_CULL_NONE;// D3D11_CULL_BACK;//must be none for mouse flip
    RSDesc.FrontCounterClockwise = FALSE;
    RSDesc.DepthBias = 0;
    RSDesc.SlopeScaledDepthBias = 0.0f;
    RSDesc.DepthBiasClamp = 0;
    RSDesc.DepthClipEnable = TRUE;
    RSDesc.ScissorEnable = TRUE;
    RSDesc.AntialiasedLineEnable = FALSE;
    RSDesc.MultisampleEnable = FALSE;

    ID3D11RasterizerState* pRState = nullptr;
    hr = g_d3dDevice->CreateRasterizerState(&RSDesc, &pRState);
    if (FAILED(hr)) {
        Fallout_Debug_Error("DxSetup - Failed CreateRasterizerState.");
        return -1;
    }
    g_d3dDeviceContext->RSSetState(pRState);
    pRState->Release();

    Set_ViewPort(SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI);
    SetScreenProjectionMatrix_XM(SCR_WIDTH, SCR_HEIGHT);

    if (!Shader_Main_Setup()) {
        Fallout_Debug_Error("DxSetup - Shader_Main_Setup Failed.");
        return -1;
    }
    if (main_pal == nullptr)
        main_pal = new PAL_DX(256, true);

    Color_Pal_Init();
    if (color_pal) {
        ID3D11ShaderResourceView* palTex = color_pal->GetShaderResourceView();
        g_d3dDeviceContext->PSSetShaderResources(1, 1, &palTex);
    }
    return 0;
}


//____________________
void ReSizeDisplayEx() {

    if (g_d3dDevice == nullptr) {
        return;
    }

    if (g_d3dSwapChain)
    {
        g_d3dDeviceContext->OMSetRenderTargets(0, 0, 0);
        //Release all outstanding references to its back buffers.
        g_d3dRenderTargetView->Release();

        HRESULT hr;
        //Preserve the existing number of buffers in the swap chain and format.
        //Use the client windows width and height.
        hr = g_d3dSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        if (hr != S_OK)
            Fallout_Debug_Error("ReSizeDisplayEx - Failed ResizeBuffers");

        //Get buffer and create a render-target-view.
        ID3D11Texture2D* pBuffer;
        hr = g_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
        if (hr != S_OK)
            Fallout_Debug_Error("ReSizeDisplayEx - Failed GetBuffer");

        if (pBuffer != nullptr)
            hr = g_d3dDevice->CreateRenderTargetView(pBuffer, nullptr, &g_d3dRenderTargetView);
        else
            Fallout_Debug_Error("ReSizeDisplayEx - Failed pBuffer");
        if (hr != S_OK)
            Fallout_Debug_Error("ReSizeDisplayEx - Failed CreateRenderTargetView");
        pBuffer->Release();

        //Resize the stencil buffer
        if (g_d3dDepthStencilBuffer != nullptr) {
            g_d3dDepthStencilBuffer->Release();
            g_d3dDepthStencilBuffer = nullptr;
        }
        if (g_d3dDepthStencilView != nullptr) {
            g_d3dDepthStencilView->Release();
            g_d3dDepthStencilView = nullptr;
        }
        RECT clientRect;
        GetClientRect(hGameWnd, &clientRect);

        //Get the window client width and height.
        unsigned int clientWidth = clientRect.right - clientRect.left;
        unsigned int clientHeight = clientRect.bottom - clientRect.top;

        //Create the depth buffer for use with the depth/stencil view.
        D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
        ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

        depthStencilBufferDesc.ArraySize = 1;
        depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
        depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilBufferDesc.Width = clientWidth;
        depthStencilBufferDesc.Height = clientHeight;
        depthStencilBufferDesc.MipLevels = 1;
        depthStencilBufferDesc.SampleDesc.Count = 1;
        depthStencilBufferDesc.SampleDesc.Quality = 0;
        depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

        hr = g_d3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &g_d3dDepthStencilBuffer);
        if (FAILED(hr))
            Fallout_Debug_Error("ReSizeDisplayEx - Create DepthStencilBuffer Failed");
        if (g_d3dDepthStencilBuffer != nullptr)
            hr = g_d3dDevice->CreateDepthStencilView(g_d3dDepthStencilBuffer, nullptr, &g_d3dDepthStencilView);
        if (FAILED(hr))
            Fallout_Debug_Error("ReSizeDisplayEx - Create DepthStencilView Failed");

        Set_ViewPort(SCR_WIDTH * scaleLevel_GUI, SCR_HEIGHT * scaleLevel_GUI);
    }
    else
        Fallout_Debug_Error("ReSizeDisplayEx - no g_d3dSwapChain");
    Display_Release_RenderTargets();
    SetScreenProjectionMatrix_XM(SCR_WIDTH, SCR_HEIGHT);
}


//____________________________________
void LoadedPaletteCheck(char* palName) {
    isMainPalActive = CompareCharArray_IgnoreCase(palName, "color.pal", 9);
}


//_______________________________________________
void __declspec(naked) loaded_palette_check(void) {

    __asm {
        pushad
        push eax
        call LoadedPaletteCheck
        add esp, 0x4
        popad

        mov edx, 0x200
        ret
    }
}


//____________________________________________________
void __declspec(naked) loaded_palette_splash_fix(void) {

    __asm {
        mov isMainPalActive, 0
        mov ebx, 0x300
        ret
    }
}


//______________________
bool Shader_Main_Setup() {
    ID3D11Device* pD3DDev = GetD3dDevice();
    ID3D11DeviceContext* pD3DDevContext = GetD3dDeviceContext();
    HRESULT hr = S_OK;

    if (!pd3dVertexShader_Main) {
        hr = pD3DDev->CreateVertexShader(pVS_Basic_mem, sizeof(pVS_Basic_mem), nullptr, &pd3dVertexShader_Main);
        if (FAILED(hr)) {
            Fallout_Debug_Error("CreateVertexShader Failed.");
            return false;
        }
    }
    if (!pd3dVS_InputLayout_Main) {
        // Create the input layout for the vertex shader.
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
            { "POSITION", 0,  DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = pD3DDev->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), pVS_Basic_mem, sizeof(pVS_Basic_mem), &pd3dVS_InputLayout_Main);
        if (FAILED(hr)) {
            Fallout_Debug_Error("CreateInputLayout Failed.");
            return false;
        }
    }
    if (!pVB_Quad_IndexBuffer) {
        WORD Indicies[6];
        //Load the index array with data.
        Indicies[0] = 0;  // top left.
        Indicies[1] = 1;  // bottom left.
        Indicies[2] = 2;  // Bottom right.
        Indicies[3] = 3;  // top right.
        Indicies[4] = 0;  // Top left.
        Indicies[5] = 2;  // Bottom right.

        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));

        //Create and initialize the index buffer.
        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.ByteWidth = sizeof(WORD) * _countof(Indicies);
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        InitData.pSysMem = Indicies;

        HRESULT hr = pD3DDev->CreateBuffer(&indexBufferDesc, &InitData, &pVB_Quad_IndexBuffer);
        if (FAILED(hr)) {
            return false;
        }
    }

        if (!pVB_Quad_Line_IndexBuffer) {
        WORD Indicies[5];
        //Load the index array with data.
        Indicies[0] = 0;  // Bottom left.
        Indicies[1] = 1;  // Top left. 
        Indicies[2] = 2;  // Top right.
        Indicies[3] = 3;  // Bottom Right.
        Indicies[4] = 4;  // Bottom left.


        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));

        //Create and initialize the index buffer.
        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.ByteWidth = sizeof(WORD) * _countof(Indicies);
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        InitData.pSysMem = Indicies;

        HRESULT hr = pD3DDev->CreateBuffer(&indexBufferDesc, &InitData, &pVB_Quad_Line_IndexBuffer);
        if (FAILED(hr)) {
            return false;
        }
    }
    //set vetex shader stuff
    pD3DDevContext->VSSetShader(pd3dVertexShader_Main, nullptr, 0);
    pD3DDevContext->IASetInputLayout(pd3dVS_InputLayout_Main);
    //set index buffer stuff
    pD3DDevContext->IASetIndexBuffer(pVB_Quad_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    pD3DDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    if (!pd3d_PS_Basic_Tex_32) {
        hr = pD3DDev->CreatePixelShader(pPS_Basic_Tex_32_mem, sizeof(pPS_Basic_Tex_32_mem), nullptr, &pd3d_PS_Basic_Tex_32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Basic_Tex_32.");
    }
    if (!pd3d_PS_Basic_Tex_8) {
        hr = pD3DDev->CreatePixelShader(pPS_Basic_Tex_8_mem, sizeof(pPS_Basic_Tex_8_mem), nullptr, &pd3d_PS_Basic_Tex_8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Basic_Tex_8.");
    }
    if (!pd3d_PS_Fader) {
        hr = pD3DDev->CreatePixelShader(pPS_Fade_Colour_32_mem, sizeof(pPS_Fade_Colour_32_mem), nullptr, &pd3d_PS_Fader);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Fader.");
    }
    if (!pd3d_PS_Colour32) {
        hr = pD3DDev->CreatePixelShader(pPS_Colour_32_mem, sizeof(pPS_Colour_32_mem), nullptr, &pd3d_PS_Colour32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Colour32.");
    }
    if (!pd3d_PS_Shadow1_DrawBase) {
        hr = pD3DDev->CreatePixelShader(pPS_ShadowsStep1_DrawBase_mem, sizeof(pPS_ShadowsStep1_DrawBase_mem), nullptr, &pd3d_PS_Shadow1_DrawBase);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Shadow1_DrawBase.");
    }
    if (!pd3d_PS_Shadow2_Build) {
        hr = pD3DDev->CreatePixelShader(pPS_ShadowsStep2_Build_mem, sizeof(pPS_ShadowsStep2_Build_mem), nullptr, &pd3d_PS_Shadow2_Build);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Shadow2_Build.");
    }
    if (!pd3d_PS_Shadow3_Blur) {
        hr = pD3DDev->CreatePixelShader(pPS_ShadowsStep3_Blur_mem, sizeof(pPS_ShadowsStep3_Blur_mem), nullptr, &pd3d_PS_Shadow3_Blur);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Shadow3_Blur.");
    }
    if (!pd3d_PS_Shadow4_RadialBlur) {
        hr = pD3DDev->CreatePixelShader(pPS_ShadowsStep4_RadialBlur_mem, sizeof(pPS_ShadowsStep4_RadialBlur_mem), nullptr, &pd3d_PS_Shadow4_RadialBlur);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Shadow4_RadialBlur.");
    }
    if (!pd3d_PS_Shadow5_Combine) {
        hr = pD3DDev->CreatePixelShader(pPS_ShadowsStep5_Combine_mem, sizeof(pPS_ShadowsStep5_Combine_mem), nullptr, &pd3d_PS_Shadow5_Combine);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Shadow5_Combine.");
    }
    if (!pd3d_PS_Outline_OuterEdge8) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_OuterEdge8_mem, sizeof(pPS_Outline_OuterEdge8_mem), nullptr, &pd3d_PS_Outline_OuterEdge8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_OuterEdge8.");
    }
    if (!pd3d_PS_Outline_OuterEdge32) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_OuterEdge32_mem, sizeof(pPS_Outline_OuterEdge32_mem), nullptr, &pd3d_PS_Outline_OuterEdge32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_OuterEdge32.");
    }
    if (!pd3d_PS_Outline_Colour8) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_Colour_8_mem, sizeof(pPS_Outline_Colour_8_mem), nullptr, &pd3d_PS_Outline_Colour8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_Colour8.");
    }
    if (!pd3d_PS_Outline_Colour32) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_Colour_32_mem, sizeof(pPS_Outline_Colour_32_mem), nullptr, &pd3d_PS_Outline_Colour32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_Colour32.");
    }
    if (!pd3d_PS_Outline_Palette8) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_Palette_8_mem, sizeof(pPS_Outline_Palette_8_mem), nullptr, &pd3d_PS_Outline_Palette8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_Palette8.");
    }
    if (!pd3d_PS_Outline_Palette32) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_Palette_32_mem, sizeof(pPS_Outline_Palette_32_mem), nullptr, &pd3d_PS_Outline_Palette32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Outline_Palette32.");
    }
    if (!pd3d_PS_ObjFlat8) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjFlat8_mem, sizeof(pPS_ObjFlat8_mem), nullptr, &pd3d_PS_ObjFlat8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjFlat8.");
    }
    if (!pd3d_PS_ObjFlat32) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjFlat32_mem, sizeof(pPS_ObjFlat32_mem), nullptr, &pd3d_PS_ObjFlat32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjFlat32.");
    }
    if (!pd3d_PS_ObjUpright8) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjUpright8_mem, sizeof(pPS_ObjUpright8_mem), nullptr, &pd3d_PS_ObjUpright8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjUpright8.");
    }
    if (!pd3d_PS_ObjUpright8_OriginalLighting) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjUpright8_OriginalLighting_mem, sizeof(pPS_ObjUpright8_OriginalLighting_mem), nullptr, &pd3d_PS_ObjUpright8_OriginalLighting);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjUpright8_OriginalLighting.");
    }
    if (!pd3d_PS_ObjUpright32) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjUpright32_mem, sizeof(pPS_ObjUpright32_mem), nullptr, &pd3d_PS_ObjUpright32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjUpright32.");
    }
    if (!pd3d_PS_ObjUpright32_OriginalLighting) {
        hr = pD3DDev->CreatePixelShader(pPS_ObjUpright32_OriginalLighting_mem, sizeof(pPS_ObjUpright32_OriginalLighting_mem), nullptr, &pd3d_PS_ObjUpright32_OriginalLighting);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_ObjUpright32_OriginalLighting.");
    }
    if (!pd3d_PS_DrawHexFog) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawHexFog_mem, sizeof(pPS_DrawHexFog_mem), nullptr, &pd3d_PS_DrawHexFog);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawHexFog.");
    }
    if (!pd3d_PS_DrawHexLight_OriginalLighting) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawHexLight_OriginalLighting_mem, sizeof(pPS_DrawHexLight_OriginalLighting_mem), nullptr, &pd3d_PS_DrawHexLight_OriginalLighting);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawHexLight_OriginalLighting.");
    }
    if (!pd3d_PS_DrawObjLight8) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawObjLight8_mem, sizeof(pPS_DrawObjLight8_mem), nullptr, &pd3d_PS_DrawObjLight8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawObjLight8.");
    }
    if (!pd3d_PS_DrawObjLight32) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawObjLight32_mem, sizeof(pPS_DrawObjLight32_mem), nullptr, &pd3d_PS_DrawObjLight32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawObjLight32.");
    }
    if (!pd3d_PS_DrawWallLight8) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawWallLight8_mem, sizeof(pPS_DrawWallLight8_mem), nullptr, &pd3d_PS_DrawWallLight8);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawWallLight8.");
    }
    if (!pd3d_PS_DrawWallLight32) {
        hr = pD3DDev->CreatePixelShader(pPS_DrawWallLight32_mem, sizeof(pPS_DrawWallLight32_mem), nullptr, &pd3d_PS_DrawWallLight32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawWallLight32.");
    }
    if (!pd3d_PS_RenderFloorLight32) {
        hr = pD3DDev->CreatePixelShader(pPS_RenderFloorLight32_mem, sizeof(pPS_RenderFloorLight32_mem), nullptr, &pd3d_PS_RenderFloorLight32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_RenderFloorLight32.");
    }
    if (!pd3d_PS_RenderRoof32) {
        hr = pD3DDev->CreatePixelShader(pPS_RenderRoof32_mem, sizeof(pPS_RenderRoof32_mem), nullptr, &pd3d_PS_RenderRoof32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_RenderRoof32.");
    }
    if (!pd3d_PS_GaussianBlurV) {
        hr = pD3DDev->CreatePixelShader(pPS_GaussianBlurV_mem, sizeof(pPS_GaussianBlurV_mem), nullptr, &pd3d_PS_GaussianBlurV);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_GaussianBlurV.");
    }
    if (!pd3d_PS_GaussianBlurU) {
        hr = pD3DDev->CreatePixelShader(pPS_GaussianBlurU_mem, sizeof(pPS_GaussianBlurU_mem), nullptr, &pd3d_PS_GaussianBlurU);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_GaussianBlurU.");
    }
    if (!pd3d_PS_Colour_32_Alpha) {
        hr = pD3DDev->CreatePixelShader(pPS_Colour_32_Alpha_mem, sizeof(pPS_Colour_32_Alpha_mem), nullptr, &pd3d_PS_Colour_32_Alpha);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawTextAlpha.");
    }
    if (!pd3d_PS_Colour_32_Brightness_ZeroMasked) {
        hr = pD3DDev->CreatePixelShader(pPS_Colour_32_Brightness_ZeroMasked_mem, sizeof(pPS_Colour_32_Brightness_ZeroMasked_mem), nullptr, &pd3d_PS_Colour_32_Brightness_ZeroMasked);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_DrawTextColour.");
    }
    if (!pd3d_PS_Colour_32_RevAlpha_ZeroMasked) {
        hr = pD3DDev->CreatePixelShader(pPS_Colour_32_RevAlpha_ZeroMasked_mem, sizeof(pPS_Colour_32_RevAlpha_ZeroMasked_mem), nullptr, &pd3d_PS_Colour_32_RevAlpha_ZeroMasked);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Draw_Dialog_HighLight.");
    }
    if (!pd3d_PS_Outline_Quad_32) {
        hr = pD3DDev->CreatePixelShader(pPS_Outline_Quad_32_mem, sizeof(pPS_Outline_Quad_32_mem), nullptr, &pd3d_PS_Outline_Quad_32);
        if (FAILED(hr))
            Fallout_Debug_Error("CreatePixelShader Failed - pd3d_PS_Draw_Dialog_HighLight.");
    }




    //Create sampler states for texture sampling in the pixel shader.
    if (!pd3dPS_SamplerState_Point || !pd3dPS_SamplerState_Linear) {
        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        if (!pd3dPS_SamplerState_Point) {
            hr = pD3DDev->CreateSamplerState(&samplerDesc, &pd3dPS_SamplerState_Point);
            if (FAILED(hr)) {
                char msg[256];
                sprintf_s(msg, 256, "%X", hr);
                Fallout_Debug_Error("CreateSamplerState Point");
                return false;
            }
        }
        if (!pd3dPS_SamplerState_Linear) {
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            hr = pD3DDev->CreateSamplerState(&samplerDesc, &pd3dPS_SamplerState_Linear);
            if (FAILED(hr)) {
                char msg[256];
                sprintf_s(msg, 256, "%X", hr);
                Fallout_Debug_Error("CreateSamplerState Linear");
                return false;
            }
        }
    }
    pD3DDevContext->PSSetSamplers(0, 1, &pd3dPS_SamplerState_Point);

    //Create blend states.
    if (!pBlendState_Zero || !pBlendState_One || !pBlendState_Two || !pBlendState_Three || !pBlendState_Four) {
        D3D11_BLEND_DESC blendStateDesc;
        ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
        blendStateDesc.AlphaToCoverageEnable = FALSE;
        blendStateDesc.IndependentBlendEnable = FALSE;
        blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (!pBlendState_Zero)
            hr = pD3DDev->CreateBlendState(&blendStateDesc, &pBlendState_Zero);
        blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
        if (!pBlendState_One)
            hr = pD3DDev->CreateBlendState(&blendStateDesc, &pBlendState_One);
        if (!pBlendState_Two) {
            blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
            hr = pD3DDev->CreateBlendState(&blendStateDesc, &pBlendState_Two);
        }
        if (!pBlendState_Three) {
            //blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            hr = pD3DDev->CreateBlendState(&blendStateDesc, &pBlendState_Three);
        }
        if (!pBlendState_Four) {
            blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            hr = pD3DDev->CreateBlendState(&blendStateDesc, &pBlendState_Four);
        }
        pD3DDevContext->OMSetBlendState(pBlendState_One, nullptr, -1);
    }

    if (!pPS_BuffersFallout)
        pPS_BuffersFallout = new PS_BUFFERS_FALLOUT();
    pPS_BuffersFallout->SetForRender();

    if (hr != S_OK)
        return false;
    return true;
}


//________________________
void Shader_Main_Destroy() {
    if (pd3dVertexShader_Main)
        pd3dVertexShader_Main->Release();
    pd3dVertexShader_Main = nullptr;

    if (pd3dVS_InputLayout_Main)
        pd3dVS_InputLayout_Main->Release();
    pd3dVS_InputLayout_Main = nullptr;

    if (pd3d_PS_Basic_Tex_32)
        pd3d_PS_Basic_Tex_32->Release();
    pd3d_PS_Basic_Tex_32 = nullptr;

    if (pd3d_PS_Basic_Tex_8)
        pd3d_PS_Basic_Tex_8->Release();
    pd3d_PS_Basic_Tex_8 = nullptr;

    if (pd3d_PS_Fader)
        pd3d_PS_Fader->Release();
    pd3d_PS_Fader = nullptr;

    if (pd3d_PS_Colour32)
        pd3d_PS_Colour32->Release();
    pd3d_PS_Colour32 = nullptr;

    if (pd3dPS_SamplerState_Point)
        pd3dPS_SamplerState_Point->Release();
    pd3dPS_SamplerState_Point = nullptr;
    if (pd3dPS_SamplerState_Linear)
        pd3dPS_SamplerState_Linear->Release();
    pd3dPS_SamplerState_Linear = nullptr;

    if (pBlendState_Zero)
        pBlendState_Zero->Release();
    pBlendState_Zero = nullptr;
    if (pBlendState_One)
        pBlendState_One->Release();
    pBlendState_One = nullptr;
    if (pBlendState_Two)
        pBlendState_Two->Release();
    pBlendState_Two = nullptr;
    if (pBlendState_Three)
        pBlendState_Three->Release();
    pBlendState_Three = nullptr;
    if (pBlendState_Four)
        pBlendState_Four->Release();
    pBlendState_Four = nullptr;
    if (pd3d_PS_Shadow1_DrawBase)
        pd3d_PS_Shadow1_DrawBase->Release();
    pd3d_PS_Shadow1_DrawBase = nullptr;
    if (pd3d_PS_Shadow2_Build)
        pd3d_PS_Shadow2_Build->Release();
    pd3d_PS_Shadow2_Build = nullptr;
    if (pd3d_PS_Shadow3_Blur)
        pd3d_PS_Shadow3_Blur->Release();
    pd3d_PS_Shadow3_Blur = nullptr;
    if (pd3d_PS_Shadow4_RadialBlur)
        pd3d_PS_Shadow4_RadialBlur->Release();
    pd3d_PS_Shadow4_RadialBlur = nullptr;
    if (pd3d_PS_Shadow5_Combine)
        pd3d_PS_Shadow5_Combine->Release();
    pd3d_PS_Shadow5_Combine = nullptr;

    if (pd3d_PS_Outline_OuterEdge8)
        pd3d_PS_Outline_OuterEdge8->Release();
    pd3d_PS_Outline_OuterEdge8 = nullptr;
    if (pd3d_PS_Outline_OuterEdge32)
        pd3d_PS_Outline_OuterEdge32->Release();
    pd3d_PS_Outline_OuterEdge32 = nullptr;

    if (pd3d_PS_Outline_Colour8)
        pd3d_PS_Outline_Colour8->Release();
    pd3d_PS_Outline_Colour8 = nullptr;
    if (pd3d_PS_Outline_Colour32)
        pd3d_PS_Outline_Colour32->Release();
    pd3d_PS_Outline_Colour32 = nullptr;
    if (pd3d_PS_Outline_Palette8)
        pd3d_PS_Outline_Palette8->Release();
    pd3d_PS_Outline_Palette8 = nullptr;
    if (pd3d_PS_Outline_Palette32)
        pd3d_PS_Outline_Palette32->Release();
    pd3d_PS_Outline_Palette32 = nullptr;

    if (pd3d_PS_ObjFlat8)
        pd3d_PS_ObjFlat8->Release();
    pd3d_PS_ObjFlat8 = nullptr;
    if (pd3d_PS_ObjFlat32)
        pd3d_PS_ObjFlat32->Release();
    pd3d_PS_ObjFlat32 = nullptr;
    if (pd3d_PS_ObjUpright8)
        pd3d_PS_ObjUpright8->Release();
    pd3d_PS_ObjUpright8 = nullptr;
    if (pd3d_PS_ObjUpright32)
        pd3d_PS_ObjUpright32->Release();
    pd3d_PS_ObjUpright32 = nullptr;

    
    if (pd3d_PS_DrawHexFog)
        pd3d_PS_DrawHexFog->Release();
    pd3d_PS_DrawHexFog = nullptr;

    if (pd3d_PS_DrawHexLight_OriginalLighting)
        pd3d_PS_DrawHexLight_OriginalLighting->Release();
    pd3d_PS_DrawHexLight_OriginalLighting = nullptr;

    if (pd3d_PS_ObjUpright8_OriginalLighting)
        pd3d_PS_ObjUpright8_OriginalLighting->Release();
    pd3d_PS_ObjUpright8_OriginalLighting = nullptr;
    if (pd3d_PS_ObjUpright32_OriginalLighting)
        pd3d_PS_ObjUpright32_OriginalLighting->Release();
    pd3d_PS_ObjUpright32_OriginalLighting = nullptr;

    if (pd3d_PS_DrawObjLight8)
        pd3d_PS_DrawObjLight8->Release();
    pd3d_PS_DrawObjLight8 = nullptr;
    if (pd3d_PS_DrawObjLight32)
        pd3d_PS_DrawObjLight32->Release();
    pd3d_PS_DrawObjLight32 = nullptr;

    if (pd3d_PS_DrawWallLight8)
        pd3d_PS_DrawWallLight8->Release();
    pd3d_PS_DrawWallLight8 = nullptr;
    if (pd3d_PS_DrawWallLight32)
        pd3d_PS_DrawWallLight32->Release();
    pd3d_PS_DrawWallLight32 = nullptr;

    if (pd3d_PS_GaussianBlurV)
        pd3d_PS_GaussianBlurV->Release();
    pd3d_PS_GaussianBlurV = nullptr;
    if (pd3d_PS_GaussianBlurU)
        pd3d_PS_GaussianBlurU->Release();
    pd3d_PS_GaussianBlurU = nullptr;

    if (pd3d_PS_RenderFloorLight32)
        pd3d_PS_RenderFloorLight32->Release();
    pd3d_PS_RenderFloorLight32 = nullptr;

    if (pd3d_PS_RenderRoof32)
        pd3d_PS_RenderRoof32->Release();
    pd3d_PS_RenderRoof32 = nullptr;

    if (pd3d_PS_Colour_32_Alpha)
        pd3d_PS_Colour_32_Alpha->Release();
    pd3d_PS_Colour_32_Alpha = nullptr;
    if (pd3d_PS_Colour_32_Brightness_ZeroMasked)
        pd3d_PS_Colour_32_Brightness_ZeroMasked->Release();
    pd3d_PS_Colour_32_Brightness_ZeroMasked = nullptr;

    if (pd3d_PS_Colour_32_RevAlpha_ZeroMasked)
        pd3d_PS_Colour_32_RevAlpha_ZeroMasked->Release();
    pd3d_PS_Colour_32_RevAlpha_ZeroMasked = nullptr;

    if (pd3d_PS_Outline_Quad_32)
        pd3d_PS_Outline_Quad_32->Release();
    pd3d_PS_Outline_Quad_32 = nullptr;
    


    if (pPS_BuffersFallout)
        delete pPS_BuffersFallout;
    pPS_BuffersFallout = nullptr;

    if (pVB_Quad_IndexBuffer)
        pVB_Quad_IndexBuffer->Release();
    pVB_Quad_IndexBuffer = nullptr;


    if (pVB_Quad_Line_IndexBuffer)
        pVB_Quad_Line_IndexBuffer->Release();
    pVB_Quad_Line_IndexBuffer = nullptr;
    
}


//____________________________________________________________________________________________________
bool CreateQuadVB(ID3D11Device* pD3DDev, unsigned int width, unsigned int height, ID3D11Buffer** lpVB) {
    ID3D11Buffer* pVB = nullptr;

    float left = 0.0f;
    float top = 0.0f;
    float right = (float)width;
    float bottom = (float)height;

    VERTEX_BASE Vertices[4]{};

    Vertices[0].Position = XMFLOAT3(left, bottom, 0.0f);  // bottom left.
    Vertices[0].texUV = XMFLOAT2(0.0f, 1.0f);
    Vertices[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    Vertices[1].Position = XMFLOAT3(left, top, 0.0f);  // Top left.
    Vertices[1].texUV = XMFLOAT2(0.0f, 0.0f);
    Vertices[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);


    Vertices[2].Position = XMFLOAT3(right, bottom, 0.0f);  // bottom right.
    Vertices[2].texUV = XMFLOAT2(1.0f, 1.0f);
    Vertices[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    Vertices[3].Position = XMFLOAT3(right, top, 0.0f);  // top right.
    Vertices[3].texUV = XMFLOAT2(1.0f, 0.0f);
    Vertices[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    //Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VERTEX_BASE) * _countof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    //Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;

    //Create the vertex buffer.
    HRESULT hr = pD3DDev->CreateBuffer(&bufferDesc, &InitData, &pVB);
    if (FAILED(hr))
        return false;

    *lpVB = pVB;
    pVB = nullptr;
    return true;
}



//______________________________________________________________________________________________________________
bool CreateQuadVB_LineStrip(ID3D11Device* pD3DDev, unsigned int width, unsigned int height, ID3D11Buffer** lpVB) {
    ID3D11Buffer* pVB = nullptr;

    float left = 0.0f;
    float top = 0.0f;
    float right = (float)width;
    float bottom = (float)height;

    VERTEX_BASE Vertices[5];
    ZeroMemory(&Vertices, sizeof(VERTEX_BASE) * _countof(Vertices));

    Vertices[0].Position = XMFLOAT3(left, bottom - 1.0f, 0.0f);  // Bottom left.
    Vertices[1].Position = XMFLOAT3(left, top - 1.0f, 0.0f);  // Top left.
    Vertices[2].Position = XMFLOAT3(right, top, 0.0f);  // Top right.
    Vertices[3].Position = XMFLOAT3(right - 1.0f, bottom, 0.0f);  // Bottom right.
    Vertices[4].Position = XMFLOAT3(left - 1.0f, bottom, 0.0f);  // Bottom left.

    //Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VERTEX_BASE) * _countof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    //Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;

    //Create the vertex buffer.
    HRESULT hr = pD3DDev->CreateBuffer(&bufferDesc, &InitData, &pVB);
    if (FAILED(hr))
        return false;

    *lpVB = pVB;
    pVB = nullptr;
    return true;
}

/*
//__________________________________________________________________________________________________________________
bool CreateQuadVB_LineStrip_ISO(ID3D11Device* pD3DDev, unsigned int width, unsigned int height, ID3D11Buffer** lpVB) {
    ID3D11Buffer* pVB = nullptr;

    float left = 0.0f;
    float top = 0.0f;
    float right = (float)width;
    float bottom = (float)height;

    float x_unit = (float)width / 5.0f;

    float lt_y = (float)height / 3.0f;
    float lb_x = x_unit * 2;
    float rb_y = lt_y * 2;
    float rt_x = x_unit * 3;

    VERTEX_BASE Vertices[5];
    ZeroMemory(&Vertices, sizeof(VERTEX_BASE) * _countof(Vertices));

    Vertices[0].Position = XMFLOAT3(lb_x, bottom - 1.0f, 0.0f);  // Bottom left.
    Vertices[1].Position = XMFLOAT3(left, lt_y - 1.0f, 0.0f);  // Top left.
    Vertices[2].Position = XMFLOAT3(rt_x, top, 0.0f);  // Top right.
    Vertices[3].Position = XMFLOAT3(right - 1.0f, rb_y, 0.0f);  // Bottom right.
    Vertices[4].Position = XMFLOAT3(lb_x - 1.0f, bottom, 0.0f);  // Bottom left.

    //Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VERTEX_BASE) * _countof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    //Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;

    //Create the vertex buffer.
    HRESULT hr = pD3DDev->CreateBuffer(&bufferDesc, &InitData, &pVB);
    if (FAILED(hr))
        return false;

    *lpVB = pVB;
    pVB = nullptr;
    return true;
}
*/

/*
//__________________________________________________________________________________________________________________
bool CreateQuadVB_LineStrip_ISO(ID3D11Device* pD3DDev, unsigned int iso_width, unsigned int iso_height, ID3D11Buffer** lpVB) {
    ID3D11Buffer* pVB = nullptr;

    //float left = 0.0f;
    //float top = 0.0f;
    //float right = (float)width;
    //float bottom = (float)height;

    float rt_x = 0.0f;
    float rt_y = 0.0f;

    float lt_y = rt_y + ((float)iso_width / 2);
    float lt_x = -((float)iso_width * 2 - lt_y);

    float rb_y = (float)iso_height + ((float)rt_x / 2);
    float rb_x = -((float)rt_x * 2 - rb_y);

    float lb_y = (float)iso_height + ((float)iso_width / 2);
    float lb_x = -((float)iso_width * 2 - lb_y);

    //float x_unit = (float)width / 5.0f;

    //float lt_y = (float)height / 3.0f;
    //float lb_x = x_unit * 2;
    //float rb_y = lt_y * 2;
    //float rt_x = x_unit * 3;

    VERTEX_BASE Vertices[5];
    ZeroMemory(&Vertices, sizeof(VERTEX_BASE) * _countof(Vertices));

    Vertices[0].Position = XMFLOAT3(lb_x, lb_y - 1.0f, 0.0f);  // Bottom left.
    Vertices[1].Position = XMFLOAT3(lt_x, lt_y - 1.0f, 0.0f);  // Top left.
    Vertices[2].Position = XMFLOAT3(rt_x, rt_y, 0.0f);  // Top right.
    Vertices[3].Position = XMFLOAT3(rb_x - 1.0f, rb_y, 0.0f);  // Bottom right.
    Vertices[4].Position = XMFLOAT3(lb_x - 1.0f, lb_y, 0.0f);  // Bottom left.

    //Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VERTEX_BASE) * _countof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    //Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;

    //Create the vertex buffer.
    HRESULT hr = pD3DDev->CreateBuffer(&bufferDesc, &InitData, &pVB);
    if (FAILED(hr))
        return false;

    *lpVB = pVB;
    pVB = nullptr;
    return true;
}
*/


//__________________________________________________________________________________________________________________________________
bool CreateQuadrilateralVB_LineStrip(ID3D11Device* pD3DDev, POINT* p_lb, POINT* p_lt, POINT* p_rt, POINT* p_rb, ID3D11Buffer** lpVB) {
    if (!p_lb || !p_lt || !p_rb || !p_rt) {
        *lpVB = nullptr;
        return false;
    }
        
    ID3D11Buffer* pVB = nullptr;

    VERTEX_BASE Vertices[5];
    ZeroMemory(&Vertices, sizeof(VERTEX_BASE) * _countof(Vertices));

    Vertices[0].Position = XMFLOAT3((float)p_lb->x, (float)p_lb->y - 1.0f, 0.0f);  // Bottom left.
    Vertices[1].Position = XMFLOAT3((float)p_lt->x, (float)p_lt->y - 1.0f, 0.0f);  // Top left.
    Vertices[2].Position = XMFLOAT3((float)p_rt->x, (float)p_rt->y, 0.0f);  // Top right.
    Vertices[3].Position = XMFLOAT3((float)p_rb->x - 1.0f, (float)p_rb->y, 0.0f);  // Bottom right.
    Vertices[4].Position = XMFLOAT3((float)p_lb->x - 1.0f, (float)p_lb->y, 0.0f);  // Bottom left.

    //Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(VERTEX_BASE) * _countof(Vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    //Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
    InitData.pSysMem = Vertices;

    //Create the vertex buffer.
    HRESULT hr = pD3DDev->CreateBuffer(&bufferDesc, &InitData, &pVB);
    if (FAILED(hr))
        return false;

    *lpVB = pVB;
    pVB = nullptr;
    return true;
}


__declspec(align(16)) XMMATRIX Ortho2D_SCRN_XM;

//__________________________________________________________
bool SetScreenProjectionMatrix_XM(DWORD width, DWORD height) {
    Ortho2D_SCRN_XM = XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1000.0f);
    return true;
};


//______________________________________
XMMATRIX *GetScreenProjectionMatrix_XM() {
    return &Ortho2D_SCRN_XM;
};


//________________________________
void Modifications_Dx_General_CH() {

    MemWrite8(0x4CCF0C, 0x53, 0xE9);
    FuncWrite32(0x4CCF0D, 0x57565251, (DWORD)&DxExit);


    MemWrite32(0x6BCF84, 0, 0);
    MemWrite32(0x6BCF88, 0, 0);
    MemWrite32(0x6BCD44, 0, 0);
    MemWrite32(0x6BCD48, 0, 0);

    MemWrite8(0x4CCFB2, 0x56, 0xE9);
    FuncWrite32(0x4CCFB3, 0xE5895557, (DWORD)&main_pal_set_entry);

    MemWrite8(0x4CD103, 0x51, 0xE9);
    FuncWrite32(0x4CD104, 0x89555756, (DWORD)&main_pal_set_entries);

    MemWrite8(0x4CD4DD, 0x53, 0xE9);
    FuncWrite32(0x4CD4DE, 0x57565251, (DWORD)&main_pal_set_all_entries);

    MemWrite8(0x4C9944, 0x53, 0xE9);
    FuncWrite32(0x4C9945, 0x57565251, (DWORD)&take_screen_shot);

    pKeyValScreenShot = (LONG*)0x6BCCD0;

    MemWrite16(0x4C9419, 0x053B, 0xE890);
    FuncWrite32(0x4C941B, (DWORD)pKeyValScreenShot, (DWORD)&screen_shot_check);

    MemWrite8(0x4C7C46, 0xBA, 0xE8);
    FuncWrite32(0x4C7C47, 0x200, (DWORD)&loaded_palette_check);

    MemWrite8(0x4927B8, 0xBB, 0xE8);
    FuncWrite32(0x4927B9, 0x300, (DWORD)&loaded_palette_splash_fix);
}


//___________________________________
void Modifications_Dx_General_MULTI() {

    MemWrite8(0x4CB1B0, 0x53, 0xE9);
    FuncWrite32(0x4CB1B1, 0x55575251, (DWORD)&DxExit);

    MemWrite32(0x6ACA18, 0, 0);
    MemWrite32(0x6ACA1C, 0, 0);
    MemWrite32(0x6AC7D8, 0, 0);
    MemWrite32(0x6AC7DC, 0, 0);

    MemWrite8(0x4CB218, 0x56, 0xE9);
    FuncWrite32(0x4CB219, 0xEC835557, (DWORD)&main_pal_set_entry);

    MemWrite8(0x4CB310, 0x51, 0xE9);
    FuncWrite32(0x4CB311, 0x81555756, (DWORD)&main_pal_set_entries);

    MemWrite8(0x4CB568, 0x53, 0xE9);
    FuncWrite32(0x4CB569, 0x57565251, (DWORD)&main_pal_set_all_entries);

    MemWrite8(0x4C8F4C, 0x53, 0xE9);
    FuncWrite32(0x4C8F4D, 0x57565251, (DWORD)&take_screen_shot);

    pKeyValScreenShot = (LONG*)FixAddress(0x6AC760);

    MemWrite16(0x4C8C25, 0x053B, 0xE890);
    FuncWrite32(0x4C8C27, (DWORD)pKeyValScreenShot, (DWORD)&screen_shot_check);

    MemWrite8(0x4C7904, 0xBA, 0xE8);
    FuncWrite32(0x4C7905, 0x200, (DWORD)&loaded_palette_check);

    MemWrite8(0x493A08, 0xBB, 0xE8);
    FuncWrite32(0x493A09, 0x300, (DWORD)&loaded_palette_splash_fix);


    pPalCycle_Slime = (BYTE*)FixAddress(0x518440);
    pPalCycle_Shoreline = (BYTE*)FixAddress(0x51844C);
    pPalCycle_FireSlow = (BYTE*)FixAddress(0x51845E);
    pPalCycle_FireFast = (BYTE*)FixAddress(0x51846D);
    pPalCycle_Monitors = (BYTE*)FixAddress(0x51847C);
    pPalCycle_Alarm = (BYTE*)FixAddress(0x5184A8);


    //CYCLE_INIT()
    MemWrite32(0x42E882, 0x0042E97C, (DWORD)&Color_Pal_Cycle);

    //ADD_EVENT_CYCLE_ANI_COLORS()
    MemWrite32(0x42E8D8, 0x0042E97C, (DWORD)&Color_Pal_Cycle);
    //REMOVE_EVENT_CYCLE_ANI_COLORS()
    MemWrite32(0x42E917, 0x0042E97C, (DWORD)&Color_Pal_Cycle);

    //FADE_TRANSITION_SETUP()
    FuncReplace32(0x44260D, 0x000513EF, (DWORD)&present_timing_setup);

    //sound continue function ptr - called in fade transition
    ppfall_Sound_Continue_All_FADE_REF = (void (**)())FixAddress(0x51DF20);
    pIsMusicOnFlag = (LONG*)FixAddress(0x518E38);
    pIsSpeechOnFlag = (LONG*)FixAddress(0x518E44);

    MemWrite8(0x493AD4, 0x53, 0xE9);
    FuncWrite32(0x493AD5, 0x57565251, (DWORD)&fade_to_palette);

    MemWrite16(0x493B48, 0x5251, 0xE990);
    FuncWrite32(0x493B4A, 0xC2895756, (DWORD)&set_palette_fading);

    pIsPaletteCyclingEnabled = (DWORD*)FixAddress(0x518490);
}


//_____________________________
void Modifications_Dx_General() {

    if (fallout_exe_region == EXE_Region::Chinese)
        Modifications_Dx_General_CH();
    else
        Modifications_Dx_General_MULTI();
}