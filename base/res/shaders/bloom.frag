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

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform mat4 u_Persp;

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

uniform vec4 u_Light;
uniform float u_LightIl;

in vec2 fs_Texcoord;

out vec4 out_Color;
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
    return texture(u_Colortex,texcoords).xyz;
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

float Gaussian1D(float x, float y, float dev)
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
    vec4 color = texture(u_Bloomtex,fs_Texcoord);	
	vec4 texColor = vec4(0.0);
	vec2 texelSize = vec2(1.0/float(u_ScreenWidth),1.0/float(u_ScreenHeight));
	int blurAmount = 10;
	float blurScale = 2.0;
	float dev = float(blurAmount) / 6.0;
	for(int i = -blurAmount/2; i<=blurAmount/2; ++i)
	{
		for(int j = -blurAmount/2; j<=blurAmount/2; ++j)
		{
			//if(i <0 || j<0 || i>= u_ScreenWidth || j >= u_ScreenHeight)
				//continue;
			texColor += texture(u_Bloomtex,fs_Texcoord + vec2(texelSize.x * i * blurScale , texelSize.y * j* blurScale))
				* Gaussian2D(float(i)*0.4,float(j)*0.4,dev);
			
		}
	}
	out_Color = texColor;
    return;
}

