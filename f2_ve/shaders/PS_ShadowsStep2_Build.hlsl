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
//_____run several times cycling the 2 shadow textures, fills out the shadowed area cast by objects_____
float4 PS_ShadowsStep2_Build(in PS_INPUT IN) : SV_TARGET {

    //if the currect pixel is shadowed then leave as is;
    float currentColor = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    if (currentColor == 1.0)
        return float4(1, 1, 1, 1);

    //vector between light source(centre of texture) and the current pixel
    float2 lightvec = float2(0.5, 0.5).xy - IN.TexCoords.xy;

    //check points ever closer to the light source, marking the current pixel as shadowed if any others are marked.
    float div = 3;
    float2 dist = float2(lightvec.x / div, lightvec.y / div);
    float2 samplePos = float2(IN.TexCoords.x + dist.x, IN.TexCoords.y + dist.y);

    float sampleColor = clamp(Texture_Main.Sample(Sampler_Main, samplePos).x, 0, 1.0);
    if (sampleColor == 1.0)
        return float4(1, 1, 1, 1);
    
    [unroll]
    for (int i = 0; i < 4; i++) {
        dist = float2(dist.x / div, dist.y / div);
        samplePos = float2(IN.TexCoords.x + dist.x, IN.TexCoords.y + dist.y);
        sampleColor = clamp(Texture_Main.Sample(Sampler_Main, samplePos).x, 0, 1.0);
        if (sampleColor == 1.0)
            return float4(1, 1, 1, 1);
    }

    return float4(0, 0, 0, 0);
}
