#version 330

////////////////////////////
//       ENUMERATIONS
////////////////////////////

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_GlowMask;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform float u_InvScrHeight;
uniform float u_InvScrWidth;

uniform bool u_BloomOn;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
}

float sampleGlowMask (vec2 texcoords)
{
	return	texture (u_GlowMask, texcoords).x;
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
vec3 getRandomNormal(vec2 texcoords) {
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
const mat3 GaussianMat = mat3 (vec3 (1,2,1), 
							   vec3 (2,4,2),
							   vec3 (1,2,1)) / 16.0;
void main() 
{
    vec3 color = sampleCol(fs_Texcoord);
	if (u_BloomOn)
	{
//		if (sampleGlowMask (fs_Texcoord))
//		{
			vec3 bloomColour = vec3(0);
			for (int i = -1; i < 2; ++i)
			{
				int j = -1;
				bloomColour += (texture (u_GlowMask, vec2 (fs_Texcoord.x + i*u_InvScrWidth, fs_Texcoord.y + j*u_InvScrHeight), 3).xyz * GaussianMat [i+1].x);
				++ j;
				bloomColour += (texture (u_GlowMask, vec2 (fs_Texcoord.x + i*u_InvScrWidth, fs_Texcoord.y + j*u_InvScrHeight), 3).xyz * GaussianMat [i+1].y);
				++ j;
				bloomColour += (texture (u_GlowMask, vec2 (fs_Texcoord.x + i*u_InvScrWidth, fs_Texcoord.y + j*u_InvScrHeight), 3).xyz * GaussianMat [i+1].z);
				++ j;
			}  
			color += color*bloomColour;  
//		}
	}
//    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
//    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    out_Color = vec4(color,1.0);//vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
    return;
}

