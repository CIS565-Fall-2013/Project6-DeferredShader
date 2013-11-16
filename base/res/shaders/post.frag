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
#define	DISPLAY_OUTLINE 6
#define DISPLAY_TOON 7
#define DISPLAY_BLOOM 8


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform int u_DisplayType;

uniform sampler2D u_Colortex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Posttex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

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

vec3 multiplyNormal(int x, int y, float n)
{
	vec2 offset = vec2(float(x)/u_ScreenWidth, float(y)/u_ScreenHeight);
	return texture(u_Normaltex, fs_Texcoord + offset).xyz * n;
}

vec3 convolve3x3(float i11, float i12, float i13, float i21, float i22, float i23,
				 float i31, float i32, float i33)
{
	vec3 v11 = multiplyNormal(-1, 1, i11);
	vec3 v12 = multiplyNormal(0, 1, i12);
	vec3 v13 = multiplyNormal(1, 1, i13);
	vec3 v21 = multiplyNormal(-1, 0, i21);
	vec3 v22 = multiplyNormal(0, 0, i22);
	vec3 v23 = multiplyNormal(1, 0, i23);
	vec3 v31 = multiplyNormal(-1, -1, i31);
	vec3 v32 = multiplyNormal(0, -1, i32);
	vec3 v33 = multiplyNormal(1, -1, i33);
	return v11+v12+v13+v21+v22+v23+v31+v32+v33;
}

float sobel()
{
	vec3 gx = convolve3x3(1.0, 0.0, -1.0, 2.0, 0.0, -2.0, 1.0, 0.0, -1.0);
	vec3 gy = convolve3x3(1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0);
	float gxGray = (gx.x + gx.y + gx.z) / 3.0;
	float gyGray = (gy.x + gy.y + gy.z) / 3.0;
	return sqrt(gxGray*gxGray + gyGray*gyGray);
}

vec3 discretize(vec3 value, int numBins)
{
	float t = numBins;
	return floor(value * t) / t;
}

vec3 multiplyBrightColor(int x, int y, float n)
{
	vec2 offset = vec2(float(x)/u_ScreenWidth, float(y)/u_ScreenHeight);
	vec3 value = texture(u_Colortex, fs_Texcoord + offset).xyz;
	return value * n * float(value.x + value.y + value.z > 2.6);
}

// Gaussian blur is too slow.
/*
uniform float kernel[25] = float[](
	1, 4, 7, 4, 1, 
	4, 16, 26, 16, 4, 
	7, 26, 41, 26, 7, 
	4, 16, 26, 16, 4, 
	1, 4, 7, 4, 1);
*/

vec3 getBlurColor()
{
	vec3 color = vec3(0.0);
	// Box blur 13x13
	for (int i = -6; i < 7; ++i)
	{
		for (int j = -6; j < 7; ++j)
		{
			vec2 offset = vec2(float(i)/u_ScreenWidth, float(j)/u_ScreenHeight);
			vec3 value = texture(u_Colortex, fs_Texcoord + offset).xyz;
			if (value.x + value.y + value.z > 2.6) color += value;
		}
	}
	color /= 169.0;
	/*
	// Gaussian blur 5x5
	for (int i = -2; i < 3; ++i)
	{
		for (int j = -2; j < 3; ++j)
		{
			int idx = (i+2)*5 + (j+2);
			color += multiplyBrightColor(i, j, kernel[idx] / 273.0);
		}
	}
	*/
	return color;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
    vec3 color = sampleCol(fs_Texcoord);
	vec3 outlineColor = vec3(sobel() < 0.1);
	vec3 discretizedColor = discretize(color, 10);

	if (u_DisplayType == DISPLAY_OUTLINE)
	{
		out_Color = vec4(outlineColor, 1.0);
		return;
	}

	if (u_DisplayType == DISPLAY_TOON)
	{
		out_Color = vec4(outlineColor * discretizedColor, 1.0);
		return;
	}

	if (u_DisplayType == DISPLAY_BLOOM)
	{
		out_Color = vec4(color + getBlurColor() * 0.7, 1.0);
		return;
	}

	float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	
    return;
}

