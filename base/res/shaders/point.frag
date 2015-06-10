#version 430

////////////////////////////
//       ENUMERATIONS
////////////////////////////

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5


// Textures
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

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

layout(binding = 1) uniform PerDraw_Light
{
    vec4 u_Light;
    vec3 u_LightCol;
    float u_LightIl;
};

in vec2 fs_Texcoord;
out vec4 out_Color;

/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) 
{
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) 
{
    return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) 
{
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) 
{
    return texture(u_Colortex,texcoords).xyz;
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(vec2 texcoords) 
{
    ivec2 sz = textureSize(u_RandomNormaltex,0);
    return texture(u_RandomNormaltex,vec2(texcoords.s* (u_ScreenWidth)/sz.x,
                (texcoords.t)*(u_ScreenHeight)/sz.y)).rgb;
}


//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomScalartex,0);
    return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
                texcoords.t*u_ScreenHeight/sz.y)).r;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() 
{
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth, u_Near, u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float lightRadius = u_Light.w;
    out_Color = vec4(0, 0, 0, 1.0);

    if(u_DisplayType == DISPLAY_LIGHTS)
    {
        //Put some code here to visualize the fragment associated with this point light
		out_Color = vec4 (u_LightCol, 1.0);
    }
    else
    {
		float distLight = length (light-position);
		float decay = max(1 - (distLight / lightRadius), 0);
		float clampedDotPdt = clamp (dot (normalize(normal), (light-position)/distLight), 0.0, 1.0);

		if (u_toonOn)
		{
			if (clampedDotPdt == 1.0)
				clampedDotPdt = 1.0;
			else if (clampedDotPdt >= 0.8)
				clampedDotPdt = 0.8;
			else if (clampedDotPdt >= 0.6)
				clampedDotPdt = 0.6;
			else if (clampedDotPdt >= 0.4)
				clampedDotPdt = 0.4;
			else if (clampedDotPdt >= 0.2)
				clampedDotPdt = 0.2;
			else
				clampedDotPdt = 0.0;
		}
		vec3 finalColour = (color * u_LightCol * u_LightIl * clampedDotPdt) * decay;
		out_Color = vec4(finalColour, 1.0);		// Because light and normal are both in view space.
    }
}

