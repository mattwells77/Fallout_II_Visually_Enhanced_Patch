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
//______run several times cycling the 2 shadow textures, create a faux penumbra for shadows____
float4 PS_ShadowsStep4_RadialBlur(in PS_INPUT IN) : SV_TARGET {

    float2 vector1 = IN.TexCoords - float2(0.5, 0.5);

    float dist = sqrt(vector1.x * vector1.x + vector1.y * vector1.y);
    float angle = atan2(vector1.y, vector1.x);

    //check a few pixels close to and along the same arc around the light source as the current pixel and blend
    float2 vector2;
    float pixel = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    
    for (int i = 0; i < 2; i++) {
        angle += 0.01; // 0.0175;
        vector2 = float2(dist * cos(angle) + 0.5, dist * sin(angle) + 0.5);
        pixel += clamp(Texture_Main.Sample(Sampler_Main, vector2).x, 0, 1.0);
    }
    pixel /= 3;

    return float4(pixel, pixel, pixel, pixel);
}
