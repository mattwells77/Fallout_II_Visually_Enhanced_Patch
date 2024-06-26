/*
The MIT License (MIT)
Copyright � 2022 to 2024 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the �Software�), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "shaderPixel_Main.hlsli"
// gaussian blur in the horizontal direction___
float4 PS_GaussianBlurU(PS_INPUT input) : SV_TARGET {
    
    float3 textureColor = float3(1.0f, 0.0f, 0.0f);
    float2 uv = input.TexCoords;

    textureColor = Texture_Main.Sample(Sampler_Main, uv).xyz * BlurWeights[0];
    
        for (int i = 1; i < 3; i++) {
        float2 normalizedOffset = float2(KernelOffsets[i], 0.0f) / genData4_1.x;
        textureColor += Texture_Main.Sample(Sampler_Main, uv + normalizedOffset).xyz * BlurWeights[i];
        textureColor += Texture_Main.Sample(Sampler_Main, uv - normalizedOffset).xyz * BlurWeights[i];
    }
    return float4(textureColor, 1.0);
}
