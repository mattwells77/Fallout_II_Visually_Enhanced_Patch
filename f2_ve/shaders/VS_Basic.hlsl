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

#include "shaderVertex_Main.hlsli"

VS_OUTPUT VS_Basic(VS_INPUT IN)
{
    VS_OUTPUT OUT;
	//float4 pos = float4(IN.Position.xyz, 1.0f); 
    IN.Position.w = 1.0f;
	//IN.Position.z = 0.0f;
    OUT.Position = mul(IN.Position, WorldViewProjectionMatrix);
    OUT.TexCoords = IN.TexCoords;
    OUT.Normal = mul(IN.Normal, (float3x3) WorldMatrix);
    OUT.WorldPos = mul(IN.Position, WorldMatrix).xyz;

	//OUT.Position = mul(IN.Position, WorldMatrix);
	//OUT.Position = mul(OUT.Position, viewMatrix);
	//OUT.Position = mul(OUT.Position, WorldViewProjectionMatrix);
	//OUT.Position = IN.Position;
    return OUT;
}