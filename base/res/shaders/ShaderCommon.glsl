#version 430

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5

// Shader constants
layout(binding = 0) uniform PerFrame
{
    mat4 um4View;
    mat4 um4Persp;
    float ufFar;
    float ufNear;
    float ufInvScrHeight;
    float ufInvScrWidth;
    float ufMouseTexX;
    float ufMouseTexY;
    float ufGlowmask;
    int uiDisplayType;
    int uiScreenWidth;
    int uiScreenHeight;
    bool ubBloomOn;
    bool ubToonOn;
    bool ubDOFOn;
    bool ubDOFDebug;
};

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth)
{
    return (2 * ufNear) / (ufFar + ufNear -  exp_depth * (ufFar - ufNear)); 
}

vec3 SampleTexture(sampler2D textureSampler, vec2 texCoord) 
{
    return texture(textureSampler, texCoord).xyz;
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(sampler2D randomNormalTex, vec2 texCoord) 
{
    ivec2 sz = textureSize(randomNormalTex, 0);
    return texture(randomNormalTex, vec2(texCoord.s* (uiScreenWidth)/sz.x, (texCoord.t)*(uiScreenHeight)/sz.y)).rgb;
}

//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(sampler2D randomScalarTex, vec2 texCoord) 
{
    ivec2 sz = textureSize(randomScalarTex,0);
    return texture(randomScalarTex, vec2(texCoord.s*uiScreenWidth/sz.x, texCoord.t*uiScreenHeight/sz.y)).r;
}