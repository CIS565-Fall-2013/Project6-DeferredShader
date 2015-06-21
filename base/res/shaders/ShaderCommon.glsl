#version 430

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5
#define	DISPLAY_GLOWMASK 6

// Shader constants
layout(binding = 0) uniform PerFrame
{
    mat4 u_View;
    mat4 u_Persp;
    float u_Far;
    float u_Near;
    float u_InvScrHeight;
    float u_InvScrWidth;
    float u_mouseTexX;
    float u_mouseTexY;
    float glowmask;
    int u_OcclusionType;
    int u_DisplayType;
    int u_ScreenWidth;
    int u_ScreenHeight;
    bool u_BloomOn;
    bool u_toonOn;
    bool u_DOFOn;
    bool u_DOFDebug;
};

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) 
{
    return (2 * near) / (far + near -  exp_depth * (far - near)); 
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
    return texture(randomNormalTex, vec2(texCoord.s* (u_ScreenWidth)/sz.x, (texCoord.t)*(u_ScreenHeight)/sz.y)).rgb;
}

//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(sampler2D randomScalarTex, vec2 texCoord) 
{
    ivec2 sz = textureSize(randomScalarTex,0);
    return texture(randomScalarTex, vec2(texCoord.s*u_ScreenWidth/sz.x, texCoord.t*u_ScreenHeight/sz.y)).r;
}