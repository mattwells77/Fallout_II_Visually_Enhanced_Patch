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

float4 PS_RenderRoof32(in PS_INPUT IN) : SV_TARGET {
    
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    float x = distance(IN.WorldPos.x, (PcEgg_Pos.z - portalDim.x) * pixelSize.x);
    float y = distance(IN.WorldPos.y, (PcEgg_Pos.w - portalDim.y) * pixelSize.y);
    float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
    objDist /= pixelSize.x;
    if (objDist < 60)
        texel.a = texel.a * objDist / 60;
    
    return float4(clamp(texel.rgb * AmbientLight.rgb, 0, 1.0f), clamp(Opaqueness, 0, texel.a));
}
