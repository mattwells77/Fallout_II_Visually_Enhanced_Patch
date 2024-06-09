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
//___combine all shadows into a single texture for applying to the floor_________
float4 PS_ShadowsStep5_Combine(in PS_INPUT IN) : SV_TARGET {

    float lightRadius = lightDetails.z;
    float shadows = 1.0 - clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float3 lightPos = float3(0.5 / pixelSize.x, 0.5 / pixelSize.y, -100);
    float3 worldpos = float3(IN.TexCoords.x / pixelSize.x, IN.TexCoords.y / pixelSize.y, 0);

    float2 lightvec = float2(0.5, 0.5).xy - IN.TexCoords.xy;
    lightvec.x = lightvec.x / pixelSize.x;
    lightvec.y = lightvec.y / pixelSize.y / 0.44444;
    float lightTexelDist = length(lightvec);

    float attenuation = clamp(1.0 - lightTexelDist * lightTexelDist / (lightRadius * lightRadius), 0.0, 1.0);

    return float4(clamp(lightColour.rgb * shadows * attenuation * saturate(dot(IN.Normal, normalize(lightPos - worldpos))), 0, 1.0), 1.0);
}
