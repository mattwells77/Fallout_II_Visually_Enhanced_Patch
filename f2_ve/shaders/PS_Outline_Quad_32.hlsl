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

float4 PS_Outline_Quad_32(in PS_INPUT IN) : SV_TARGET {
	
	float4 texel = float4(1.0, 1.0, 1.0, 1.0);
	/*
	float2 offset = float2(genData4_2.x, 0);
	
	if(IN.TexCoords.x -	offset <= 0.0) {
			texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.x).rgb, 0.0f, 1.0f), 1.0f);
	}
	if(IN.TexCoords.x + offset >= 1.0f) {
		texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.y).rgb, 0.0f, 1.0f), 1.0f);
	}
	
	offset = float2(0, genData4_2.y);
	 
	if(IN.TexCoords.y -	offset <= 0.0) {
			texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.z).rgb, 0.0f, 1.0f), 1.0f);
	}
	if(IN.TexCoords.y + offset >= 1.0f) {
		texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.w).rgb, 0.0f, 1.0f), 1.0f);
	}*/
	
    return texel;
}
