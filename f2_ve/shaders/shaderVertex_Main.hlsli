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


#pragma pack_matrix( row_major )
cbuffer ModelViewProjectionConstantBuffer  : register(b0)
{
	float4x4 WorldMatrix;
	float4x4 WorldViewProjectionMatrix;
}


struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoords : TEXCOORD0;
	float3 Normal : NORMAL0;
};


struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TexCoords : TEXCOORD0;
	float3 Normal : NORMAL0;
	float3 WorldPos : POSITION0;
};
