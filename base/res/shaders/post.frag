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
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_Bloomtex;
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform int u_UseBloom;
uniform int u_UseToon;
uniform float u_Far;
uniform float u_Near;


in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

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

//Returns gradient squared
vec3 sobel(sampler2D tex, vec2 texcoords) {
	float xScale = 1.0/u_ScreenWidth;
	float yScale = 1.0/u_ScreenHeight;
	
	vec2 off = vec2(xScale,yScale);
	
	vec3 ul = texture(tex, texcoords-vec2(-1.0, 1.0)*off).xyz;
	vec3 u  = texture(tex, texcoords-vec2( 0.0, 1.0)*off).xyz*2.0;
	vec3 ur = texture(tex, texcoords-vec2( 1.0, 1.0)*off).xyz;
	vec3 r  = texture(tex, texcoords-vec2( 1.0, 0.0)*off).xyz*2.0;
	vec3 dr = texture(tex, texcoords-vec2( 1.0,-1.0)*off).xyz;
	vec3 d  = texture(tex, texcoords-vec2( 1.0,-1.0)*off).xyz*2.0;
	vec3 dl = texture(tex, texcoords-vec2( 0.0,-1.0)*off).xyz;
	vec3 l  = texture(tex, texcoords-vec2(-1.0, 0.0)*off).xyz*2.0;
	
	vec3 horizontal = (ul+l+dl)-(ur+r+dr);
	vec3 vertical   = (ul+u+ur)-(dl+d+dr);
	
	vec3 gradient_sq = horizontal*horizontal+vertical*vertical;
	
	return gradient_sq;
}

vec4 quantize(vec4 color_in, int numBins)
{
	//Assume intial distribution between 0 and 1
	color_in.rgb *= numBins;
    color_in.rgb += vec3(.5,.5,.5);
    ivec3 intrgb = ivec3(color_in.rgb);
    color_in.rgb = vec3(intrgb)/numBins;
	return color_in;
}



const int KERNEL_SIZE = 20;

float boxFilter(float x, float y)
{
	//TODO: Find better weight function
	int width = KERNEL_SIZE*2+1;
    return 1.0/float(width*width);
}


///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
    vec4 texel = texture(u_Posttex,fs_Texcoord);
	vec3 color = texel.rgb;

	//Base color
	out_Color = vec4(color, 1.0);
	
	
	if(u_UseBloom > 0.0){
		//Bloom color
		float xScale = 1.0/u_ScreenWidth;
		float yScale = 1.0/u_ScreenHeight;
		
		vec3 bloomColor = vec3(0.0);
		for(int x = -KERNEL_SIZE; x <= KERNEL_SIZE; x++)
		{
			for(int y = -KERNEL_SIZE; y <= KERNEL_SIZE; y++)
			{
				vec2 texCoord = fs_Texcoord + vec2(x*xScale,y*yScale);
				bloomColor +=  2.0*boxFilter(x,y)*texture(u_Bloomtex,texCoord).rgb;
			}
		}
		
		out_Color += vec4(bloomColor,0.0);
	}
	
	if(u_UseToon > 0.0)
	{
		//Toon Shading
		float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
		float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);	
		
		//Silouette
		vec3 gradient_sq = sobel(u_Normaltex, fs_Texcoord);
		if(length(gradient_sq) > 7.0){
			out_Color = vec4(0.0);
		}
		//quantize colors
		out_Color = quantize(out_Color,4);
	}
    return;
}

