#version 330

////////////////////////////
// ENUMERATIONS
////////////////////////////

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5
#define	DISPLAY_TOON 6
#define	DISPLAY_BLOOM 7
#define	DISPLAY_AA 8
#define	DISPLAY_SPECULAR 9

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////

uniform sampler2D u_BloomMapTex;


uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////


uniform float zerothresh = 1.0;
uniform float falloff = 0.1;


// return the resulting color
vec3 applyHorzGaussianFilter(vec2 texcoords)
{
	float pi = 3.141592653589;
	float sigma = 50.0;
	
	float sum = 0;  // for normalization
	float s = 2.0 * sigma * sigma;
	vec3 result = vec3(0,0,0);
	
	int bound = 18;
	
	for (int x = -bound ; x <= bound ; ++x)
	{
		float r = x*x;
		float w = (exp(-r / s)) / (pi * s);
		sum += w;
			
		ivec2 texSize = textureSize(u_BloomMapTex, 0);
		float texOffsetS = float(x - 1) / float(texSize.x);
		float texOffsetT = 0;
		vec3 color = w * texture(u_BloomMapTex, texcoords + vec2(texOffsetS, texOffsetT)).rgb;			
		result += color;
	}
	
	return result * (1 / sum);	
}



///////////////////////////////////
// MAIN
//////////////////////////////////

void main() {
	
	out_Color = vec4(applyHorzGaussianFilter(fs_Texcoord), 1);
	return;
}
