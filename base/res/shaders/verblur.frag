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

uniform sampler2D u_bluredTex;
uniform sampler2D u_horBluredTex;

uniform int u_DisplayType;
uniform float u_texelSizeX;
uniform float u_texelSizeY;
uniform int u_blurAmount;
uniform float u_blurScale;
uniform float u_blurStrength;

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
    return texture(u_bluredTex,texcoords).xyz;
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

	////////////////////////separable convolution////////////////////////////////
	if(u_DisplayType == DISPLAY_BLOOM)
	{
		/*out_Color = otherColor;
		return;*/
		float dev = float(u_blurAmount) * 0.3;		
		//verticle
		for(int i = -u_blurAmount/2; i<=u_blurAmount/2; ++i)
		{
			glowColor += texture(u_horBluredTex,fs_Texcoord + vec2(0, u_texelSizeY * i * u_blurScale))
				* Gaussian1D(float(i)*u_blurStrength,dev);
		}
		out_Color = clamp((glowColor + otherColor)-(glowColor * otherColor),0.0,1.0);
		//out_Color = glowColor;
		return;	
	}
	out_Color = otherColor;
    return;
}
