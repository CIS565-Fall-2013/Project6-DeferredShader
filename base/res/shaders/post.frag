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
uniform sampler2D u_Posttex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_Colortex;
uniform sampler2D u_Bloomtex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Normaltex;

uniform vec3 u_cameraPos; // eyePos

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform int u_DisplayType;

uniform float u_texelSizeX;
uniform float u_texelSizeY;
uniform int u_blurAmount;
uniform float u_blurScale;
uniform float u_blurStrength;

uniform	mat3 u_GX = mat3( -1.0, 0.0, 1.0,-2.0, 0.0, 2.0,-1.0, 0.0, 1.0 );
uniform	mat3 u_GY = mat3(  1.0,  2.0,  1.0,0.0,  0.0,  0.0,-1.0, -2.0,	-1.0);

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

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}
//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {

	vec4 otherColor = texture(u_Posttex,fs_Texcoord);

	if(u_DisplayType == DISPLAY_SIL)
	{
		// I'm impelmenteng a sebel edge detection method
		//reference http://rastergrid.com/blog/2011/01/frei-chen-edge-detector/#more-532	
		float sumX = 0.0; float sumY = 0.0;
		for(int i = -1; i<= 1; ++i)
		{
			for(int j = -1; j<=1; ++j)
			{
				vec3 tmpColor = texture(u_Colortex,fs_Texcoord + vec2(i*u_texelSizeX, j*u_texelSizeY)).rgb;
				sumX += length(tmpColor) * u_GX[i+1][j+1];
				sumY += length(tmpColor) * u_GY[i+1][j+1];
			}
		}
		out_Color = clamp(0.5*sqrt(sumX*sumX + sumY*sumY) + otherColor,0.0,1.0);
		return;
	}
	/*else if(u_DisplayType == DISPLAY_BLOOM)
	{
		vec4 glowColor = vec4(0.0);
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
	out_Color = otherColor;
    return;
}

