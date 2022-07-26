/*
The MIT License (MIT)
Copyright © 2022 Matt Wells

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


cbuffer FadeColour : register(b4)
{
    float4 fadeColour;
};


cbuffer PortalDimensions : register(b5)
{
    float4 portalDim;
};


struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoords : TEXCOORD0;
    float3 Normal : NORMAL0;
    float3 WorldPos : POSITION0;
};


static const float KernelOffsets[3] = { 0.0f, 1.3846153846f, 3.2307692308f };
static const float BlurWeights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };

// gaussian blur in the vertical direction___
float4 PS_GaussianBlurV(PS_INPUT input) : SV_TARGET
{
    float3 textureColor = float3(1.0f, 0.0f, 0.0f);
    float2 uv = input.TexCoords;

    textureColor = Texture_Main.Sample(Sampler_Main, uv).xyz * BlurWeights[0];
    for (int i = 1; i < 3; i++) {
        float2 normalizedOffset = float2(0.0f, KernelOffsets[i]) / genData4_1.y;
        textureColor += Texture_Main.Sample(Sampler_Main, uv + normalizedOffset).xyz * BlurWeights[i];
        textureColor += Texture_Main.Sample(Sampler_Main, uv - normalizedOffset).xyz * BlurWeights[i];
    }
    return float4(textureColor, 1.0);
}

// gaussian blur in the horizontal direction___
float4 PS_GaussianBlurU(PS_INPUT input) : SV_TARGET
{
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


//________________________________
float4 PS_ObjFlat8(in PS_INPUT IN) : SV_TARGET {

    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    return texel;
};


//_________________________________
float4 PS_ObjFlat32(in PS_INPUT IN) : SV_TARGET{
    
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    return texel;
};


//___________________________________
float4 PS_ObjUpright8(in PS_INPUT IN) : SV_TARGET{


    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);

    if (texel.a == 0) {
        return float4(0, 0, 0, 0);
    }

    texel.a = clamp(texel.a * Opaqueness, 0.0, 1.0);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    if (flags.z != 0) {//is egg effect enabled
        float x = distance(IN.WorldPos.x, PcEgg_Pos.x);
        float y = distance(IN.WorldPos.y, PcEgg_Pos.y);
        float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
        if (objDist < 60) {
            texel.a = texel.a * objDist / 60;
        }

    }
    float4 lightTexel = clamp(Texture_Light_Obj.Sample(Sampler_Main, IN.TexCoords), 0, 1.0);
    
    float3 lightColourMain = clamp(lightTexel.rgb, AmbientLight.rgb, 1.0);
    return float4(clamp(texel.rgb * lightColourMain.rgb, 0.0, 1.0f), texel.a);
}

//____________________________________
float4 PS_ObjUpright32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    if (texel.a == 0) {
        return float4(0, 0, 0, 0);
    }

    texel.a = clamp(texel.a * Opaqueness, 0.0, 1.0);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    if (flags.z != 0) {//is egg effect enabled
        float x = distance(IN.WorldPos.x, PcEgg_Pos.x);
        float y = distance(IN.WorldPos.y, PcEgg_Pos.y);
        float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
        if (objDist < 60) {
            texel.a = texel.a * objDist / 60;
        }

    }
    float4 lightTexel = clamp(Texture_Light_Obj.Sample(Sampler_Main, IN.TexCoords), 0, 1.0);

    float3 lightColourMain = clamp(lightTexel.rgb, AmbientLight.rgb, 1.0);
    return float4(clamp(texel.rgb * lightColourMain.rgb, 0.0, 1.0f), texel.a);
}


//____________________________________________________
float4 PS_ObjUpright8_OriginalLighting(in PS_INPUT IN) : SV_TARGET{


    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);

    if (texel.a == 0) {
        return float4(0, 0, 0, 0);
    }

    texel.a = clamp(texel.a * Opaqueness, 0.0, 1.0);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    if (flags.z != 0) {//is egg effect enabled
        float x = distance(IN.WorldPos.x, PcEgg_Pos.x);
        float y = distance(IN.WorldPos.y, PcEgg_Pos.y);
        float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
        if (objDist < 60) {
            texel.a = texel.a * objDist / 60;
        }

    }

    float lightIntensity = lightColour.w;
    float3 lightColourMain = max(lightColour.rgb * lightIntensity, AmbientLight.rgb);
    return float4(clamp((texel.rgb * lightColourMain.rgb), 0, 1.0), texel.a);
}

//_____________________________________________________
float4 PS_ObjUpright32_OriginalLighting(in PS_INPUT IN) : SV_TARGET{


    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    if (texel.a == 0) {
        return float4(0, 0, 0, 0);
    }

    texel.a = clamp(texel.a * Opaqueness, 0.0, 1.0);
    if (flags.w != 0) {//is trans effect enabled
        float greyscale = dot(texel.rgb, float3(0.3, 0.59, 0.11));
        float4 texel_trans = Texture_Pal.Sample(Sampler_Main, palColour_Trans.x);
        texel.rgb = (greyscale * texel_trans.rgb).rgb;
        texel.a = clamp(texel.a * palColour_Trans.y, 0.0, 1.0);
    }
    if (flags.z != 0) {//is egg effect enabled
        float x = distance(IN.WorldPos.x, PcEgg_Pos.x);
        float y = distance(IN.WorldPos.y, PcEgg_Pos.y);
        float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
        if (objDist < 60) {
            texel.a = texel.a * objDist / 60;
        }

    }
    float lightIntensity = lightColour.w;
    float3 lightColourMain = max(lightColour.rgb * lightIntensity, AmbientLight.rgb);
    return float4(clamp((texel.rgb * lightColourMain.rgb), 0, 1.0), texel.a);
}


//_____________________________________________________
float4 PS_DrawHexLight_OriginalLighting(in PS_INPUT IN) : SV_TARGET{

    float x = distance(0.5, IN.TexCoords.x);
    float y = distance(0.5, IN.TexCoords.y);
    float dist = sqrt(abs(x * x) + abs(y * y));
    float attenuation = clamp(lightColour.w - (dist * dist) / 0.5, 0.0, 1.0);

    return float4(clamp(lightColour.rgb * attenuation, 0, 1.0f), attenuation);
}


//__________________________________
float4 PS_DrawHexFog(in PS_INPUT IN) : SV_TARGET{

    float x = distance(0.5, IN.TexCoords.x)*2;
    float y = distance(0.5, IN.TexCoords.y)*2;
    float dist = sqrt(abs(x * x) + abs(y * y));
    float attenuation = clamp(genData4_1.w * (1.0 - dist), 0.0, 1.0);
    return float4(attenuation, attenuation, attenuation, attenuation);
}


//____________________________________
float4 PS_RenderRoof32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    float x = distance(IN.WorldPos.x, (PcEgg_Pos.z - portalDim.x) * pixelSize.x);
    float y = distance(IN.WorldPos.y, (PcEgg_Pos.w - portalDim.y) * pixelSize.y);
    float objDist = sqrt(abs(x * x) + abs(y * y / 0.77));
    objDist /= pixelSize.x;
    if (objDist < 60) {
        texel.a = texel.a * objDist / 60;
    }
    return float4(clamp(texel.rgb * AmbientLight.rgb, 0, 1.0f), clamp(Opaqueness, 0, texel.a));
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

//__________________________________________
float4 PS_Outline_OuterEdge8(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    if (palOffset == 0.0f) {
        float2 offset = float2(pixelSize.x, 0);
        //sample the left and the right neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        offset = float2(0, pixelSize.y);
        //sample the up and the down neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        if (palOffset !=0.0 )
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, palColour_Outline.x).rgb, 0.0f, 1.0f), palColour_Outline.y);
    }
    return texel;
}


//___________________________________________
float4 PS_Outline_OuterEdge32(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float4 texel_in = Texture_Main.Sample(Sampler_Main, IN.TexCoords);

    if (texel_in.a == 0.0f) {

        float4 rect;
        float2 offset = float2(pixelSize.x, 0);
        //sample the left and the right neighbouring pixels
        rect.x = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.z = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;
        offset = float2(0, pixelSize.y);
        //sample the up and the down neighbouring pixels
        rect.y = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.w = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;

        if (rect.x + rect.y + rect.z + rect.w != 0.0)
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, palColour_Outline.x).rgb, 0.0f, 1.0f), palColour_Outline.y);

    }
    return texel;
}


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


//______________________________________
float4 PS_DrawObjLight32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    if (texel.a == 0)
        return float4(0, 0, 0, 0);
    return  PS_DRAW_OBJ_LIGHT(IN,texel);
}


//_____________________________________
float4 PS_DrawObjLight8(in PS_INPUT IN) : SV_TARGET{
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).w, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);
    if (palOffset == 0.0f)
        return float4(0, 0, 0, 0);
    else
        texel.a = 1.0;
    return  PS_DRAW_OBJ_LIGHT(IN,texel);
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


//_______________________________________
float4 PS_DrawWallLight32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    if (texel.a == 0)
        return float4(0, 0, 0, 0);
    return  PS_DRAW_WALL_LIGHT(IN,texel);
}


//______________________________________
float4 PS_DrawWallLight8(in PS_INPUT IN) : SV_TARGET{
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).w, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);
    if (palOffset == 0.0f)
        return float4(0, 0, 0, 0);
    else
        texel.a = 1.0;
    return  PS_DRAW_WALL_LIGHT(IN,texel);
}


//____draw object bases "feet" to light texture__________
float4 PS_ShadowsStep1_DrawBase(in PS_INPUT IN) : SV_TARGET{

    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    if (palOffset == 0)
        return float4(0, 0, 0, 0);
    return float4(1, 1, 1, 1);
}


//_____run several times cycling the 2 shadow textures, fills out the shadowed area cast by objects_____
float4 PS_ShadowsStep2_Build(in PS_INPUT IN) : SV_TARGET{

    //if the currect pixel is shadowed then leave as is;
    float currentColor = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    if (currentColor == 1.0) {
        return float4(1, 1, 1, 1);
    }

    //vector between light source(centre of texture) and the current pixel
    float2 lightvec = float2(0.5, 0.5).xy - IN.TexCoords.xy;


    //check points ever closer to the light source, marking the current pixel as shadowed if any others are marked.
    float div = 3;
    float2 dist = float2(lightvec.x / div, lightvec.y / div);
    float2 samplePos = float2(IN.TexCoords.x + dist.x, IN.TexCoords.y + dist.y);

    float sampleColor = clamp(Texture_Main.Sample(Sampler_Main, samplePos).x, 0, 1.0);
    if (sampleColor == 1.0) {
        return float4(1, 1, 1, 1);
    }

    for (int i = 0; i < 4; i++) {
        dist = float2(dist.x / div, dist.y / div);
        samplePos = float2(IN.TexCoords.x + dist.x, IN.TexCoords.y + dist.y);
        sampleColor = clamp(Texture_Main.Sample(Sampler_Main, samplePos).x, 0, 1.0);
        if (sampleColor == 1.0) {
            return float4(1, 1, 1, 1);
        }
    }

    return float4(0, 0, 0, 0);
}


//______run several times cycling the 2 shadow textures, blur the shadows____
float4 PS_ShadowsStep3_Blur(in PS_INPUT IN) : SV_TARGET{

    float pixel = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    float2 newCoords = float2(IN.TexCoords.x + pixelSize.x, IN.TexCoords.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0);//tex2D(ColorTextureSampler, IN.TexCoords.x + (0.01));
    newCoords = float2(IN.TexCoords.x - pixelSize.x, IN.TexCoords.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0);//tex2D(ColorTextureSampler, IN.TexCoords.x - (0.01));
    newCoords = float2(IN.TexCoords.x, IN.TexCoords.y + pixelSize.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0);//tex2D(ColorTextureSampler, IN.TexCoords.y + (0.01));
    newCoords = float2(IN.TexCoords.x, IN.TexCoords.y - pixelSize.y);
    pixel += clamp(Texture_Main.Sample(Sampler_Main, newCoords).x, 0, 1.0);//tex2D(ColorTextureSampler, IN.TexCoords.y - (0.01));
    pixel = pixel / 5;

    return float4(pixel, pixel, pixel, pixel);
}


//______run several times cycling the 2 shadow textures, create a faux penumbra for shadows____
float4 PS_ShadowsStep4_RadialBlur(in PS_INPUT IN) : SV_TARGET{

    float2 vector1 = IN.TexCoords - float2(0.5, 0.5);

    float dist = sqrt(vector1.x * vector1.x + vector1.y * vector1.y);
    float angle = atan2(vector1.y, vector1.x);


    //check a few pixels close to and along the same arc around the light source as the current pixel and blend
    float2 vector2;
    float pixel = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    
    for (int i = 0; i < 2; i++) {
        angle += 0.01;// 0.0175;
        vector2 = float2(dist*cos(angle)+0.5, dist*sin(angle)+0.5);
        pixel += clamp(Texture_Main.Sample(Sampler_Main, vector2).x, 0, 1.0);
    }
    pixel /= 3;

    return float4(pixel, pixel, pixel, pixel);
}


//___combine all shadows into a single texture for applying to the floor_________
float4 PS_ShadowsStep5_Combine(in PS_INPUT IN) : SV_TARGET{

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


//____draw flat lights and shadows to main render target_____________
float4  PS_RenderFloorLight32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    float4 lightTexel = clamp(Texture_Light.Sample(Sampler_Main, IN.TexCoords), 0, 1.0);

    float3 lightColourMain = max(lightTexel.rgb, AmbientLight.rgb);

    return float4(clamp(texel.rgb * lightColourMain.rgb, 0, 1.0f), texel.a);
}


//____________________________________
float4 PS_Basic_Tex_32(in PS_INPUT IN) : SV_TARGET{
    float4 texel = Texture_Main.Sample(Sampler_Main, IN.TexCoords);
    return texel;
}


//___________________________________
float4 PS_Basic_Tex_8(in PS_INPUT IN) : SV_TARGET{
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float4 texel = Texture_Pal.Sample(Sampler_Main, palOffset);
    return texel;
}


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


//______________________________________
float4 PS_Fade_Colour_32(in PS_INPUT IN) : SV_TARGET{
    return fadeColour;
}


//_________________________________
float4 PS_Colour_32(in PS_INPUT IN) : SV_TARGET{
    return genData4_1;
}


//_______________________________________
float4 PS_Colour_32_Alpha(in PS_INPUT IN) : SV_TARGET{
    return float4(genData4_1.rgb, clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0));
}


//_____________________________________________
//float4 PS_Colour_32_RevAlpha(in PS_INPUT IN) : SV_TARGET{
//    float alpha = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
//    return float4(genData4_1.rgb, clamp(1.0 - alpha, 0, 1.0));
//}


//_____________________________________________________
float4 PS_Colour_32_RevAlpha_ZeroMasked(in PS_INPUT IN) : SV_TARGET{
    float alpha = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    if (alpha == 0)
        return float4(0,0,0,0);
    return float4(genData4_1.rgb, clamp(1.0 - alpha, 0, 1.0));
}


//_______________________________________________________
float4 PS_Colour_32_Brightness_ZeroMasked(in PS_INPUT IN) : SV_TARGET{
    float brightness = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);
    float4 texel = float4(genData4_1.rgb, 1.0);
    if (brightness == 0)
        texel.a = 0;
    return float4(clamp(texel.rgb * brightness, 0.0f, 1.0f), texel.a);
}


//________________________________________
float4 PS_Outline_Colour_8(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    if (palOffset == 0.0f) {
        float2 offset = float2(genData4_2.x, 0);
        //sample the left and the right neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        offset = float2(0, genData4_2.y);
        //sample the up and the down neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        if (palOffset != 0.0)
            texel = genData4_1;
    }
    return texel;
}


//_________________________________________
float4 PS_Outline_Colour_32(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float4 texel_in = Texture_Main.Sample(Sampler_Main, IN.TexCoords);

    if (texel_in.a == 0.0f) {

        float4 rect;
        float2 offset = float2(genData4_2.x, 0);
        //sample the left and the right neighbouring pixels
        rect.x = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.z = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;
        offset = float2(0, genData4_2.y);
        //sample the up and the down neighbouring pixels
        rect.y = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.w = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;

        if (rect.x + rect.y + rect.z + rect.w != 0.0)
            texel = genData4_1;
    }
    return texel;
}


//_________________________________________
float4 PS_Outline_Palette_8(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float palOffset = clamp(Texture_Main.Sample(Sampler_Main, IN.TexCoords).x, 0, 1.0);

    if (palOffset == 0.0f) {
        float2 offset = float2(genData4_2.x, 0);
        //sample the left and the right neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        offset = float2(0, genData4_2.y);
        //sample the up and the down neighbouring pixels
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).x;
        palOffset += Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).x;

        if (palOffset != 0.0)
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.x).rgb, 0.0f, 1.0f), genData4_1.y);
}
return texel;
}


//__________________________________________
float4 PS_Outline_Palette_32(in PS_INPUT IN) : SV_TARGET{

    float4 texel = float4(0, 0, 0, 0);
    float4 texel_in = Texture_Main.Sample(Sampler_Main, IN.TexCoords);

    if (texel_in.a == 0.0f) {

        float4 rect;
        float2 offset = float2(genData4_2.x, 0);
        //sample the left and the right neighbouring pixels
        rect.x = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.z = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;
        offset = float2(0, genData4_2.y);
        //sample the up and the down neighbouring pixels
        rect.y = Texture_Main.Sample(Sampler_Main, IN.TexCoords - offset).a;
        rect.w = Texture_Main.Sample(Sampler_Main, IN.TexCoords + offset).a;

        if (rect.x + rect.y + rect.z + rect.w != 0.0)
            texel = float4(clamp(Texture_Pal.Sample(Sampler_Main, genData4_1.x).rgb, 0.0f, 1.0f), genData4_1.y);
    }
    return texel;
}
