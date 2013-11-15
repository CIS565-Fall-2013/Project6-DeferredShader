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
#define DISPLAY_BLOOM 6
#define DISPLAY_SIL 7
/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform mat4 u_Persp;

uniform sampler2D u_Bluredtex;
uniform sampler2D u_Blurtex;
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_Bloomtex;

uniform float u_Far;
uniform float u_Near;
uniform int u_OcclusionType;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform float u_texelSizeX;
uniform float u_texelSizeY;
uniform int u_blurAmount;
uniform float u_blurScale;
uniform float u_blurStrength;
uniform int u_blurStage;// 0 means horizontal, 1 means vertical

uniform vec4 u_Light;
uniform float u_LightIl;

in vec2 fs_Texcoord;

out vec4 out_Color;
out vec4 out_verBlured;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Blurtex,texcoords).xyz;
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

float Gaussian1D(float x, float dev)
{
	return (1.0/sqrt(2.0*3.14159 * dev)) * exp(-(x*x)/(2.0*dev));			
}
float Gaussian2D(float x, float y, float dev)
{
	return ( 1.0 / (2.0*3.14159 * dev * dev)) * exp(-((x*x+y*y)/(2.0*dev*dev)));
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
    vec3 color = sampleCol(fs_Texcoord);
	vec4 otherColor = vec4(color,1.0);
	vec4 glowColor = vec4(0.0);
	/////////2D /////////////////////////////////////
	/*if(u_DisplayType == DISPLAY_BLOOM)
	{

		float dev = float(u_blurAmount) * 0.3;
		for(int i = -u_blurAmount/2; i<=u_blurAmount/2; ++i)
		{
			for(int j = -u_blurAmount/2; j<=u_blurAmount/2; ++j)
			{
				glowColor += texture(u_Bloomtex,fs_Texcoord + vec2(u_texelSizeX * i * u_blurScale , u_texelSizeY * j* u_blurScale))
					* Gaussian2D(float(i)*u_blurStrength,float(j)*u_blurStrength,dev);
			
			}
		}
		out_Color = clamp((glowColor + otherColor)-(glowColor * otherColor),0.0,1.0);
		
		return;		
	}*/
	////////////////////////separable convolution////////////////////////////////
	if(u_DisplayType == DISPLAY_BLOOM)
	{
		float dev = float(u_blurAmount) * 0.3;
		if(u_blurStage == 0)
		{
			//horizontal
			for(int i = -u_blurAmount/2; i<=u_blurAmount/2; ++i)
			{
				glowColor += texture(u_Bloomtex,fs_Texcoord + vec2(u_texelSizeX * i * u_blurScale , 0))
					* Gaussian1D(float(i)*u_blurStrength,dev);
			}
			out_Color = clamp((glowColor + otherColor)-(glowColor * otherColor),0.0,1.0);
			out_bluredTex = glowColor;
			return;	
		}
		else if(u_blurStage == 1)
		{
			//horizontal
			for(int i = -u_blurAmount/2; i<=u_blurAmount/2; ++i)
			{
				glowColor += texture(u_Bluredtex,fs_Texcoord + vec2(0, u_texelSizeY * i * u_blurScale))
					* Gaussian1D(float(i)*u_blurStrength,dev);
			}
			//out_Color = clamp((glowColor + otherColor)-(glowColor * otherColor),0.0,1.0);
			out_Color = glowColor;
			return;	
		}
		
	}
	out_Color = otherColor;
    return;
}

