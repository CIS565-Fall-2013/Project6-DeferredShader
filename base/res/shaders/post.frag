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
#define	DISPLAY_TOON 6
#define	DISPLAY_BLOOM 7

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////


uniform float zerothresh = 1.0;
uniform float falloff = 0.1;


/////////////////////////////////////
//	UTILITY FUNCTIONS
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

// helper method for applySobelOperator: Computes gradient in x and y direction 
// by applying a convolution of [ -1 0 1   & [-1 -2 -1   
//											  -2 0 2       0  0  0
//											  -1 0 1 ]    1  2  1 ] with the current image
vec2 computeGrad(vec2 texcoords, ivec2 texSize)
{
	mat3 mX = mat3 (
		-1, 0, 1, // first column 
		-2, 0, 2, // second column
		-1, 0,1   // third column
	);
	
	mat3 mY = mat3 (
		-1, -2, -1, // first column 
		0, 0, 0,   // second column
		1, 2,1     // third column
	);
	
	float gradX = 0.0;
	float gradY = 0.0;
	
	for (int i = 0 ; i < 3 ; ++i)
	{
		for (int j = 0 ; j < 3 ; ++j)
		{
			// get texture coordinates
			float texOffsetS = float(i - 1) / float(texSize.x);
			float texOffsetT = float(j - 1) / float(texSize.y);
			vec3 color = texture(u_Normaltex, vec2(texOffsetS, texOffsetT) + texcoords).rgb;
			//float colorFactor = length(abs(color));
			// convert to grayscale since I am using the normal map
			float colorFactor = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; 
			
			
			gradX += mX[i][j] * colorFactor;
			gradY += mY[i][j] * colorFactor;
		}
	}
	
	return vec2(gradX, gradY);
}

// Reference: http://image-processing-is-fun.blogspot.com/2011/11/one-of-first-edge-detection-filters-one.html
// applies the sobel operator to find edges. Returns the magnitude of the 
// sum of gradient in x and y direction
float applySobelOperator(vec2 texcoords) 
{
	vec2 gradient = computeGrad(texcoords, textureSize(u_Normaltex,0));
	return abs(gradient.x) + abs(gradient.y); // approx for greater efficiency
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {

	vec3 color = sampleCol(fs_Texcoord);
	if (u_DisplayType == DISPLAY_TOTAL)
	{
		float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
		float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
		out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	}
	else if (u_DisplayType == DISPLAY_TOON)
	{
		// apply sobel operator to get outlines
		float gradMag = applySobelOperator(fs_Texcoord);
		gradMag = clamp(gradMag, 0, 1.0);
		float multiplier = 1.0 - gradMag;
		//out_Color = vec4(abs(texture(u_Normaltex, fs_Texcoord).rgb), 1.0);
		//out_Color = vec4(multiplier,multiplier,multiplier,1.0);
		out_Color = vec4(multiplier,multiplier,multiplier,1.0) *  vec4(color, 1.0);
		
	}
	else if (u_DisplayType == DISPLAY_BLOOM)
	{
	
	}
	else 
	{
		out_Color = vec4(color, 1.0);
	}
	
	return;
}
