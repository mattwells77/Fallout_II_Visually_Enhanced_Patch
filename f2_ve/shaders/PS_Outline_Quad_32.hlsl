#include "shaderPixel_Main.hlsli"


//____________________________________________________
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
