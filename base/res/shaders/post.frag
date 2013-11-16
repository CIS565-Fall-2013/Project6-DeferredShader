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

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

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


const int KERNEL_SIZE = 5;

float blurWeight(float x, float y)
{
	//TODO: Find better weight function
    return ( exp(-(x*x+y*y) / (2.0)));
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
	
	//Bloom color
    ivec2 sz = textureSize(u_Bloomtex,0);
	float xScale = sz.x/u_ScreenWidth;
	float yScale = sz.y/u_ScreenHeight;
	
	vec3 bloomColor = vec3(0.0);
	for(int x = -KERNEL_SIZE; x <= KERNEL_SIZE; x++)
	{
		for(int y = -KERNEL_SIZE; y <= KERNEL_SIZE; y++)
		{
			vec2 texCoord = fs_Texcoord + vec2(x*xScale,y*yScale);
			bloomColor +=  blurWeight(x,y)*texture(u_Bloomtex,texCoord).rgb;
		}
	}
	
	out_Color += vec4(bloomColor,0.0);
	
    return;
}

