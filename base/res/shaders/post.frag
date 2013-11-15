#version 440

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
#define	DISPLAY_TOONEDGE 7
#define	DISPLAY_BLOOM 8

#define ONE_OVER_2PI 0.15915494309
#define kernelWidth 50   // has to be an odd number

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Depthtex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

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

//Helper function to automicatlly sample and unpack positions
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

// glow map is vec3(1.0) where linear depth is 1 (background), black everywhere else
vec3 sampleGlow(vec2 texcoords){
	if(texcoords.x < 0.0 || texcoords.x > 1.0 || texcoords.y < 0.0 || texcoords.y > 1.0) return vec3(0.0);
	float exp_depth = texture(u_Depthtex, texcoords).r;
	float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);
	if(lin_depth > 0.999999) return vec3(1.0);
	else return vec3(0.0);
}

vec3 sampleCol(vec2 texcoords) {
    if(texcoords.x < 0.0 || texcoords.x > 1.0 || texcoords.y < 0.0 || texcoords.y > 1.0) return vec3(0.0);
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

float convolutionGaussian(float[kernelWidth][kernelWidth] kernel, float[kernelWidth][kernelWidth] input){
	float convol = 0.0;
	int step = 1;
	for(int i = 0; i < kernelWidth; i += step)
		for(int j = 0; j < kernelWidth; j += step)
		{
			convol += input[i][j]*kernel[kernelWidth-i-1][kernelWidth-j-1];

		}
	return convol;
}

float convolution1D(float[kernelWidth] kernel, float[kernelWidth] input){
	float convol = 0.0;
	int step = 1;
	for(int i = 0; i < kernelWidth; i += step)	
		convol += input[i]*kernel[kernelWidth-i-1];
	return convol;
}


float convolution2D(float[3][3] kernel, float[3][3] input){
	float convol =
		input[0][0]*kernel[2][2] + input[0][1]*kernel[2][1] + input[0][2]*kernel[2][0]
	   +input[1][0]*kernel[1][2] + input[1][1]*kernel[1][1] + input[1][2]*kernel[1][0]
	   +input[2][0]*kernel[0][2] + input[2][1]*kernel[0][1] + input[2][2]*kernel[0][0];

	return convol;
}
///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;

void main() {
	
	vec3 color = sampleCol(fs_Texcoord);
	float texDx = 1.0/u_ScreenWidth;
	float texDy = 1.0/u_ScreenHeight;

	if(u_DisplayType == DISPLAY_TOONEDGE){

		float[3][3] horizontalKernel;
		float[3][3] verticalKernel;
		float[3][3] input;

		horizontalKernel[0][0] = 1.0; horizontalKernel[0][1] = 0.0; horizontalKernel[0][2] = -1.0; 
		horizontalKernel[1][0] = 2.0; horizontalKernel[1][1] = 0.0; horizontalKernel[1][2] = -2.0; 
		horizontalKernel[2][0] = 1.0; horizontalKernel[2][1] = 0.0; horizontalKernel[2][2] = -1.0; 

		verticalKernel[0][0] =  1.0; verticalKernel[0][1] =  2.0; verticalKernel[0][2] =  1.0; 
		verticalKernel[1][0] =  0.0; verticalKernel[1][1] =  0.0; verticalKernel[1][2] =  0.0; 
		verticalKernel[2][0] = -1.0; verticalKernel[2][1] = -2.0; verticalKernel[2][2] = -1.0; 
			
		input[0][0] = length(sampleCol(vec2(fs_Texcoord.x - texDx, fs_Texcoord.y + texDy))); input[0][1] = length(sampleCol(vec2(fs_Texcoord.x, fs_Texcoord.y + texDy))); input[0][2] = length(sampleCol(vec2(fs_Texcoord.x + texDx, fs_Texcoord.y + texDy)));
		input[1][0] = length(sampleCol(vec2(fs_Texcoord.x - texDx, fs_Texcoord.y        ))); input[1][1] = length(sampleCol(vec2(fs_Texcoord.x, fs_Texcoord.y        ))); input[1][2] = length(sampleCol(vec2(fs_Texcoord.x + texDx, fs_Texcoord.y        )));
		input[2][0] = length(sampleCol(vec2(fs_Texcoord.x - texDx, fs_Texcoord.y - texDy))); input[2][1] = length(sampleCol(vec2(fs_Texcoord.x, fs_Texcoord.y - texDy))); input[2][2] = length(sampleCol(vec2(fs_Texcoord.x + texDx, fs_Texcoord.y - texDy)));
 
		float horizontalDir = convolution2D(horizontalKernel, input);
		float verticalDir   = convolution2D(verticalKernel  , input);
		float gradient = sqrt(horizontalDir*horizontalDir + verticalDir*verticalDir);
		float gradientDir = atan(verticalDir, horizontalDir);
		out_Color = vec4(gradient*vec3(1.0) + color, 1.0);
	}
	else if(u_DisplayType == DISPLAY_BLOOM){

	    // modify background color
		float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
		float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);
		if(lin_depth > 0.999999) color = vec3(1.0);
		
		float horizontalOffset = -0.5*(kernelWidth-1);
		float verticalOffset   = -horizontalOffset;
		float deviation = 2;
		float[kernelWidth][kernelWidth] gaussianKernel;
		float[kernelWidth][kernelWidth] input;
		
		for(int j = 0; j < kernelWidth; ++j)
		{
			horizontalOffset = -0.5*(kernelWidth-1);
			for(int i = 0; i < kernelWidth; ++i)
			{
				gaussianKernel[j][i] = ONE_OVER_2PI / (deviation*deviation) * exp(-(horizontalOffset*horizontalOffset + verticalOffset*verticalOffset) / (2.0*deviation*deviation));
				input[j][i] = sampleGlow(vec2(fs_Texcoord.x + texDx*horizontalOffset, fs_Texcoord.y + texDy*verticalOffset));
				horizontalOffset += 1.0;
			}
			verticalOffset -= 1.0;
		}

		float[kernelWidth] gaussianKernel1D;
		float[kernelWidth] input1D;
		horizontalOffset = -0.5*(kernelWidth-1);
		for(int j = 0; j < kernelWidth; ++j)
		{
			gaussianKernel1D[j] = sqrt(ONE_OVER_2PI) / deviation * exp(-(horizontalOffset*horizontalOffset) / (2.0*deviation*deviation));
			input1D[j] = sampleGlow(vec2(fs_Texcoord.x + texDx*horizontalOffset, fs_Texcoord.y));
			horizontalOffset += 1.0;
		}

		//float blurred = convolutionGaussian(gaussianKernel, input);
		float blurred = convolution1D(gaussianKernel1D, input1D);

		//color = input[(kernelWidth-1)/2][(kernelWidth-1)/2]*vec3(1.0);
		color = /*(1-blurred)*color + */blurred*vec3(1.0);

		out_Color = vec4(color, 1.0);
	}
	else{
		float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
		float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
		//out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
		out_Color = vec4(color, 1.0);
	}
    return;
}

