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

Texture2D Texture_Main : register(t0);
Texture2D Texture_Pal : register(t1);
Texture2D Texture_Light : register(t2);
Texture2D Texture_Light_Obj : register(t3);

sampler Sampler_Main : register(s0);


cbuffer MapGlobals : register(b0) {
    float4 AmbientLight;//r,g,b,opaqueness
};


cbuffer Object_PC : register(b1) {
    float4 PcEgg_Pos;//x,y = xy pos on game area for wall egg. z,w  = xy pos on screen for roof egg.
};


cbuffer Object : register(b2) {
    float4 objPos;   //x, y = xy obj position on map. z = max light strengh for distance from light source, w = light y shape modifier for walls;
    float2 palColour_Outline;
    float2 palColour_Trans;
    float3 pixelSize;//size xy, pixel Width, Pixel height
    float Opaqueness;
    float4 lightColour;
    float4 lightDetails;//x y = positon, z = radius, w = intensity.
    float4 flags;//x & y not used currently, z is egg, w is TransEffect
};


cbuffer GeneralSurface : register(b3) {
    //genData4 = ...
    //colour for fading;
    //.w for general trans;
    //.xy=textureDimensions .z=blurXOffset .w=loopCount for GaussianBlur
    float4 genData4_1;
    float4 genData4_2;
};


cbuffer FadeColour : register(b4) {
    float4 fadeColour;
};


cbuffer PortalDimensions : register(b5) {
    float4 portalDim;
};


struct PS_INPUT {
    float4 Position : SV_Position;
    float2 TexCoords : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 WorldPos : POSITION0;
};

static const float KernelOffsets[3] = { 0.0f, 1.3846153846f, 3.2307692308f };
static const float BlurWeights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };


//_________draw_obj_lights____________________________
float4 PS_DRAW_OBJ_LIGHT(in PS_INPUT IN, float4 texel) {

    float3 lightPos = float3(lightDetails.x, lightDetails.y, -100);
    float lightRadius = lightDetails.z;

    float attMain = objPos.z;//get max brightness for obj from distance from floor light

    float2 floorDist = float2(distance(IN.WorldPos.x, lightPos.x), distance(IN.WorldPos.y, lightPos.y));
    float lightTexelDist = sqrt(abs(floorDist.x * floorDist.x) + abs(floorDist.y * floorDist.y));//shape of light on upright objs

    float attenuation = clamp(attMain - lightTexelDist * lightTexelDist / (lightRadius * lightRadius), 0.0, 1.0);

    return float4(clamp(lightColour.rgb * attenuation * saturate(dot(IN.Normal, normalize(lightPos - IN.WorldPos))), 0, 1.0), 1.0);
}


//_________draw_obj_Wall_lights________________________
float4 PS_DRAW_WALL_LIGHT(in PS_INPUT IN, float4 texel) {

    float3 lightPos = float3(lightDetails.x, lightDetails.y, -100);
    float lightRadius = lightDetails.z;

    float attMain = objPos.z;//get max brightnes for obj from distance from floor light
    float light_y_shape_modifier = objPos.w;//
    float2 floorDist = float2(distance(IN.WorldPos.x, lightPos.x) / light_y_shape_modifier, distance(IN.WorldPos.y, lightPos.y));
    float lightTexelDist = sqrt(abs(floorDist.x * floorDist.x) + abs(floorDist.y * floorDist.y));//shape of light on upright objs

    float attenuation = clamp(attMain - lightTexelDist * lightTexelDist / (lightRadius * lightRadius), 0.0, 1.0);

    return float4(clamp(lightColour.rgb * attenuation * saturate(dot(IN.Normal, normalize(lightPos - IN.WorldPos))), 0, 1.0), 1.0);
}

/*
//____________________________________________
float4 PS_Outline_InnerEdge_8(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    if (palOffset != 0.0) {

        float4 rect;/// = float4(0,0,0,0);
        float2 offset = float2(pixelSize.x, 0);
        //sample the left and the right neighbouring pixels
        rect.x = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x, 0, 1.0);
        rect.z = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x, 0, 1.0);
        offset = float2(0, pixelSize.y);
        //sample the up and the down neighbouring pixels
        rect.y = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x, 0, 1.0);
        rect.w = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x, 0, 1.0);
        if (rect.x * rect.y * rect.z * rect.w == 0.0f)
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, palColour_Outline.x).rgb, 0.0f, 1.0f), palColour_Outline.y);
    }
    return texel;
}


//_____________________________________________
float4 PS_Outline_InnerEdge_32(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float4 texel_in = Texture_Main.Sample(Sampler_Main, IN.TexCoords);

    if (texel_in.a != 0.0) {

        float4 rect;
        float2 offset = float2(pixelSize.x, 0);
        //sample the left and the right neighbouring pixels
        rect.x = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.z = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;
        offset = float2(0, pixelSize.y);
        //sample the up and the down neighbouring pixels
        rect.y = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.w = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;

        if (rect.x * rect.y * rect.z * rect.w == 0.0f)
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, palColour_Outline.x).rgb, 0.0f, 1.0f), palColour_Outline.y);
    }
    return texel;
}
*/

//________________________________________
//float4 BASIC_IN_8_OUT_8(in PS_INPUT IN) : SV_TARGET{
//    return = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
//}


//________________________________________________
//float4 BASIC_IN_32_OUT_32_TRANS(in PS_INPUT IN) : SV_TARGET{
//    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
//    return float4(clamp(texel.rgb, 0, 1.0f), clamp(genData4_1.a, 0, texel.a));
//}


//_______________________________________________
//float4 BASIC_IN_8_OUT_32_TRANS(in PS_INPUT IN) : SV_TARGET{
//    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
//    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);
//    return float4(clamp(texel.rgb, 0, 1.0f), clamp(genData4_1.a, 0, texel.a));
//}


//_____________________________________________
//float4 PS_Colour_32_RevAlpha(in PS_INPUT IN) : SV_TARGET{
//    float alpha = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
//    return float4(genData4_1.rgb, clamp(1.0 - alpha, 0, 1.0));
//}
