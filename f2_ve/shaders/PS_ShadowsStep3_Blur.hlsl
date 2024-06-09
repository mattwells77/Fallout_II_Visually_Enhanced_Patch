/*
The MIT License (MIT)
Copyright © 2022 to 2024 Matt Wells

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

#include "shaderPixel_Main.hlsli"
//______run several times cycling the 2 shadow textures, blur the shadows____
float4 PS_ShadowsStep3_Blur(in PS_INPUT IN) : SV_TARGET {

    float pixel = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    float2 newCoords = float2(IN.TexCoords.x + pixelSize.x, IN.TexCoords.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0); //tex2D(ColorTextureSampler, IN.TexCoords.x + (0.01));
    newCoords = float2(IN.TexCoords.x - pixelSize.x, IN.TexCoords.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0); //tex2D(ColorTextureSampler, IN.TexCoords.x - (0.01));
    newCoords = float2(IN.TexCoords.x, IN.TexCoords.y + pixelSize.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0); //tex2D(ColorTextureSampler, IN.TexCoords.y + (0.01));
    newCoords = float2(IN.TexCoords.x, IN.TexCoords.y - pixelSize.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0); //tex2D(ColorTextureSampler, IN.TexCoords.y - (0.01));
    pixel = pixel / 5;

    return float4(pixel, pixel, pixel, pixel);
}
