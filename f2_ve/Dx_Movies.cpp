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
#include "Dx_Movies.h"
#include "Dx_Windows.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

#include "win_fall.h"
#include "modifications.h"
#include "memwrite.h"
#include "configTools.h"

PAL_DX *mve_pal = nullptr;
//movies are buffered
MVE_SURFACE *mve_surface_front=nullptr;
MVE_SURFACE *mve_surface_back=nullptr;

//loaded from mve cfg files
struct MVE_EFFECT_STRUCT {
    DWORD start_frame;
    DWORD end_frame;
    DWORD num_steps;
    DWORD colour;//b,g,r,type  //type 1 = fadein, type 2 = fadeout
    MVE_EFFECT_STRUCT* next;
};
MVE_EFFECT_STRUCT **lp_mve_effect=nullptr;

LONG* p_mve_pitch = nullptr;
LONG mve_width_pixels = 0;


//__________________________________________________________________________________________________________________
void MvePrintSub(LONG winRef, char* txt, DWORD width, DWORD height, LONG winX, LONG winY, DWORD colour, DWORD flags) {
    Subtitles_Set(txt, 560);
}


//__________________________________________
void __declspec(naked) h_mve_print_sub(void) {

    __asm {
        push esi
        push edi
        push ebp

        mov esi, dword ptr ds : [esp + 0x1C]
        push esi
        mov esi, dword ptr ds : [esp + 0x1C]
        push esi
        mov esi, dword ptr ds : [esp + 0x1C]
        push esi
        mov esi, dword ptr ds : [esp + 0x1C]
        push esi

        push ecx
        push ebx
        push edx
        push eax
        call MvePrintSub
        add esp, 0x20

        pop ebp
        pop edi
        pop esi
        ret 0x10
    }
}


//________________________
void MVE_Surface_Display() {
    if (mve_surface_front)
        mve_surface_front->Display();
}


//_________________________
void MVE_SURFACE::Display() {

    if (pD3DDevContext == nullptr)
        return;

    if (*lp_mve_effect == nullptr) {//if no movie fader effect enabled, make sure the fade colour is transparent.
        XMFLOAT4 colour = { 0.0f,0.0f,0.0f,0.0f };
        TransitionalFader_SetColour(colour);
    }

    UINT stride = sizeof(VERTEX_BASE);
    UINT offset = 0;
    //set vertex stuff
    pD3DDevContext->IASetVertexBuffers(0, 1, &pd3dVB, &stride, &offset);

    //set texture stuff
    pD3DDevContext->PSSetShaderResources(0, 1, &pTex_shaderResourceView);

    if (mve_pal) {//switch to the movie palette
        ID3D11ShaderResourceView* palTex = mve_pal->GetShaderResourceView();
        pD3DDevContext->PSSetShaderResources(1, 1, &palTex);
    }

    //set shader constant buffers
    SetPositionRender(0);

    //set pixel shader stuff
    pD3DDevContext->PSSetShader(pd3d_PS_Basic_Tex_8, nullptr, 0);

    pD3DDevContext->DrawIndexed(4, 0, 0);

    //switch back to color.pal
    ID3D11ShaderResourceView* palTex = color_pal->GetShaderResourceView();
    pD3DDevContext->PSSetShaderResources(1, 1, &palTex);
}


//_______________________________
void MVE_SURFACE::ScaleToScreen() {

    DWORD display_w = 0;
    DWORD display_h = 0;

    int MOVIE_SIZE = ConfigReadInt(L"MOVIES", L"MOVIE_SIZE", 0);

    if (MOVIE_SIZE == 1 || SCR_WIDTH < width || SCR_HEIGHT < height) {//scale - maintain aspect RO
        float movieRO = (float)width / height;
        float winRO = (float)SCR_WIDTH / SCR_HEIGHT;

        if (movieRO >= winRO) {
            x = 0;
            display_w = SCR_WIDTH;
            display_h = (DWORD)(display_w / movieRO);
            y = (float)(SCR_HEIGHT - display_h) / 2;
        }
        else {
            y = 0;
            display_h = SCR_HEIGHT;
            display_w = (DWORD)(display_h * movieRO);
            x = (float)((LONG)SCR_WIDTH - (LONG)display_w) / 2;
        }
    }
    else if (MOVIE_SIZE == 0 || MOVIE_SIZE != 2) {//original size
        display_w = width;
        display_h = height;
        x = (float)((LONG)SCR_WIDTH - (LONG)display_w) / 2;
        y = (float)((LONG)SCR_HEIGHT - (LONG)display_h) / 2;
    }

    scaleX = (float)display_w / width;
    scaleY = (float)display_h / height;
    SetMatrices();
}


//Re-position and scale, when the O.S. window resizes during movie playback
//______________________
void MVE_ResetMatrices() {
    if (mve_surface_front)
        mve_surface_front->ScaleToScreen();
    if (mve_surface_back)
        mve_surface_back->ScaleToScreen();
}


//______________________________________________________________
void MVE_SetPalEntries(BYTE* palette, DWORD offset, DWORD count) {
    if (!mve_pal)
        mve_pal = new PAL_DX(256, true);
    mve_pal->SetPalEntries(palette, offset, count);
}


/*
//_________________________________
bool MVE_GetPalette(DWORD *palette) {
   if(palette==nullptr)
      return false;

    if(mve_pal==nullptr)
        return false;
    mve_pal->GetPalEntries(0,0,256,(LPPALETTEENTRY)palette);

   return true;
}
*/


//_________________________________________________________________________________________________________________________________
HRESULT _stdcall MVE_Surface_Set(LPRECT lpDestRect, MVE_SURFACE* mve_surface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFX) {

    if (mve_surface == nullptr)
        return -1;
    mve_surface_back = mve_surface_front;
    mve_surface_front = mve_surface;//set the surface to display
    Dx_Present();
    return 0;
}


//_________________________________________________________
void _stdcall MVE_Surface_Release(MVE_SURFACE* mve_surface) {

    if (mve_surface)
        delete mve_surface;
    mve_surface = nullptr;
    mve_surface_front = nullptr;
    mve_surface_back = nullptr;

    if (mve_pal)
        delete mve_pal;
    mve_pal = nullptr;

    Subtitles_Destroy();
}


//___________________________________________________________________________________________________________________________________________
HRESULT _stdcall MVE_Surface_Lock(MVE_SURFACE* mve_surface, LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {

    if (!mve_surface)
        return -1;
    if (mve_surface->Lock(lpDestRect, lpDDSurfaceDesc) != S_OK) {
        Fallout_Debug_Error("MVE_Surface_Lock failed");
        return -1;
    }
    return 0;
}


//__________________________________________________________________________
HRESULT _stdcall MVE_Surface_Unlock(MVE_SURFACE* mve_surface, LPRECT lpRect) {

    if (!mve_surface)
        return -1;
    if (mve_surface->Unlock(lpRect) != S_OK) {
        Fallout_Debug_Error("MVE_Surface_Unlock failed");
        return -1;
    }
    return 0;
}


//_____________________________________________
void __declspec(naked) h_mve_surface_lock(void) {

    __asm {
        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        push dword ptr ss : [esp + 0x14]
        call MVE_Surface_Lock

        test eax, eax
        ret 0x14
    }
}


//_______________________________________________________________________________________________________________________
HRESULT _stdcall MVE_Surface_Create(DDSURFACEDESC* DDSurfaceDesc, MVE_SURFACE** plp_MVE_SURFACE, IUnknown FAR* pUnkOuter) {

    MVE_SURFACE* mve_surface = new MVE_SURFACE(DDSurfaceDesc, plp_MVE_SURFACE);
    HRESULT hresult = S_OK;

    //set the pitch variable
    if (SUCCEEDED(mve_surface->Lock(nullptr, DDSurfaceDesc))) {
        *p_mve_pitch = DDSurfaceDesc->lPitch;
        mve_surface->Unlock(nullptr);
    }

    return hresult;
}


//function for fadeing movies in and out - data read from movie cfg files stored in MVE_EFFECT_STRUCT structure.
//____________________________
void MVE_FADER(DWORD frameNum) {

    if (*lp_mve_effect == nullptr)
        return;
    MVE_EFFECT_STRUCT* effect = *lp_mve_effect;
    while (frameNum < effect->start_frame || frameNum > effect->end_frame) {
        effect = effect->next;
        if (effect == nullptr)
            return;
    }

    XMFLOAT4 colour = { ((effect->colour >> 24) & 0x000000FF) / 255.0f,((effect->colour >> 16) & 0x000000FF) / 255.0f,((effect->colour >> 8) & 0x000000FF) / 255.0f, 0.0f };
    int type = (effect->colour) & 0x000000FF;
    float alpha = (1.0f / effect->num_steps);
    if (type == 1) //fade in
        colour.w = alpha * (effect->end_frame - frameNum);
    else  //fade out
        colour.w = alpha * (frameNum - effect->start_frame);

    TransitionalFader_SetColour(colour);
}


//______________________________________
void __declspec(naked) h_mve_fader(void) {

    __asm {
        pushad
        push eax
        call MVE_FADER
        add esp, 0x4
        popad
        ret
    }
}


//____________________________
void Modifications_Dx_Movies() {

    if (fallout_exe_region == EXE_Region::Chinese) {
        //To-Do Modifications_Dx_Movie Chinese
        //ignore max movie size check
        MemWrite16(0x501E94, 0x870F, 0x9090);
        MemWrite32(0x501E96, 0x033D, 0x90909090);

        MemWrite16(0x501EB1, 0x870F, 0x9090);
        MemWrite32(0x501EB3, 0x0320, 0x90909090);
    }
    else {
        lp_mve_effect = (MVE_EFFECT_STRUCT**)FixAddress(0x5195F4);

        MemWrite32(0x4880A5, 0x00488144, (DWORD)&h_mve_fader);

        FuncReplace32(0x4F51A0, 0x0000106C, (DWORD)&MVE_SetPalEntries);

        MemWrite16(0x4868D7, 0x188B, 0xE890);
        FuncWrite32(0x4868D9, 0x1453FF50, (DWORD)&MVE_Surface_Set);

        MemWrite16(0x4F5DD1, 0x8B51, 0xE890);
        FuncWrite32(0x4F5DD3, 0x1850FF01, (DWORD)&MVE_Surface_Create);

        MemWrite16(0x4F5DF6, 0x8B51, 0xE890);
        FuncWrite32(0x4F5DF8, 0x1850FF01, (DWORD)&MVE_Surface_Create);

        //release movie surfaces
        ///in MVE_INIT_VIDEO_BUFFERS
        MemWrite8(0x4F5CC5, 0x8B, 0xE8);
        FuncWrite32(0x4F5CC6, 0x0853FF18, (DWORD)&MVE_Surface_Release);

        MemWrite8(0x4F5CE3, 0x8B, 0xE8);
        FuncWrite32(0x4F5CE4, 0x0853FF18, (DWORD)&MVE_Surface_Release);

        ///in RELEASE_MOVIE_BUFFERS()
        MemWrite8(0x4F639F, 0x8B, 0xE8);
        FuncWrite32(0x4F63A0, 0x0850FF00, (DWORD)&MVE_Surface_Release);

        MemWrite8(0x4F63BD, 0x8B, 0xE8);
        FuncWrite32(0x4F63BE, 0x0850FF00, (DWORD)&MVE_Surface_Release);

        //in mve lock buffers
        MemWrite8(0x4F5E8F, 0x8B, 0xE8);
        FuncWrite32(0x4F5E90, 0x6450FF01, (DWORD)&MVE_Surface_Lock);

        MemWrite8(0x4F5EB9, 0x8B, 0xE8);
        FuncWrite32(0x4F5EBA, 0x6450FF02, (DWORD)&MVE_Surface_Lock);

        MemWrite16(0x4F5ECC, 0x90FF, 0xE890);
        FuncWrite32(0x4F5ECE, 0x80, (DWORD)&MVE_Surface_Unlock);

        ///in mve unlock buffers
        MemWrite16(0x4F5EFA, 0x90FF, 0xE890);
        FuncWrite32(0x4F5EFC, 0x80, (DWORD)&MVE_Surface_Unlock);

        MemWrite16(0x4F5F0B, 0x90FF, 0xE890);
        FuncWrite32(0x4F5F0D, 0x80, (DWORD)&MVE_Surface_Unlock);

        //in draw movie direct
        MemWrite8(0x486861, 0xFF, 0xE8);
        FuncWrite32(0x486862, 0xC0856450, (DWORD)&h_mve_surface_lock);

        MemWrite16(0x4868BA, 0x90FF, 0xE890);
        FuncWrite32(0x4868BC, 0x80, (DWORD)&MVE_Surface_Unlock);

        //in draw movie buffered
        MemWrite8(0x48699D, 0xFF, 0xE8);
        FuncWrite32(0x48699E, 0xC0856452, (DWORD)&h_mve_surface_lock);

        MemWrite16(0x486A87, 0x90FF, 0xE890);
        FuncWrite32(0x486A89, 0x80, (DWORD)&MVE_Surface_Unlock);

        //?
        MemWrite8(0x486F4D, 0xFF, 0xE8);
        FuncWrite32(0x486F4E, 0xC0856453, (DWORD)&h_mve_surface_lock);

        MemWrite16(0x486FB5, 0x92FF, 0xE890);
        FuncWrite32(0x486FB7, 0x80, (DWORD)&MVE_Surface_Unlock);


        FuncReplace32(0x487688, 0x031270, (DWORD)&h_mve_print_sub);

        MemWrite8(0x4F4BFC, 0x75, 0xEB);

        MemWrite16(0x4F6491, 0x840F, 0x9090);
        MemWrite32(0x4F6493, 0x000000B0, 0x90909090);

        MemWrite16(0x487BD9, 0x0E74, 0x9090);

        //Stop clearing and redrawing movie window buffer when drawing subtitles as they are now drawn independently.
        MemWrite8(0x487620, 0xE8, 0x90);
        MemWrite32(0x487621, 0x04F6A3, 0x9008C483);

        MemWrite8(0x4876B0, 0xE8, 0x90);
        MemWrite32(0x4876B1, 0x04F8CB, 0x90909090);

        //skip creating and drawing movie to a frame buffer - seems unnecessary
        MemWrite16(0x486F2E, 0x840F, 0xE990);

        //ignore max movie size check
        MemWrite16(0x4F5044, 0x870F, 0x9090);
        MemWrite32(0x4F5046, 0x033D, 0x90909090);

        MemWrite16(0x4F5061, 0x870F, 0x9090);
        MemWrite32(0x4F5063, 0x0320, 0x90909090);


        //most references to mve width should be equal to the pitch of the dx buffer-----------------------------

        //the old mve width variable address will be set to the dx surface pitch 
        p_mve_pitch = (LONG*)FixAddress(0x6B3CFC);

        //created a new variable "mve_width_pixels" for the few references that require it.

        MemWrite32(0x4F5012, 0x6B3CFC, (DWORD)&mve_width_pixels);

        //MVE_INIT_VIDEO_BUFFERS(Arg1 width_blocks, Arg2 height_blocks, Arg3 block_mul, Arg4 is_true_colour)
        MemWrite32(0x4F5D19, 0x6B3CFC, (DWORD)&mve_width_pixels);
        MemWrite32(0x4F5D48, 0x6B3CFC, (DWORD)&mve_width_pixels);
        MemWrite32(0x4F5E0A, 0x6B3CFC, (DWORD)&mve_width_pixels);
        MemWrite32(0x4F5E25, 0x6B3CFC, (DWORD)&mve_width_pixels);

        //MVE_SEND_BUFFER_TO_DISPLAY()
        MemWrite32(0x4F5F48, 0x6B3CFC, (DWORD)&mve_width_pixels);

        //?
        MemWrite32(0x4F65D9, 0x6B3CFC, (DWORD)&mve_width_pixels);
        MemWrite32(0x4F6696, 0x6B3CFC, (DWORD)&mve_width_pixels);
        MemWrite32(0x4F689A, 0x6B3CFC, (DWORD)&mve_width_pixels);
        //--------------------------------------------------------------------------------------------------------
    }
}




