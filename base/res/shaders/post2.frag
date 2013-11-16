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
#define	DISPLAY_SSAO 9

#define ONE_OVER_2PI 0.15915494309
#define kernelWidth 50  // has to be an odd number

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_BlurPasstex;
uniform sampler2D u_Depthtex;

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

vec3 sampleCol(vec2 texcoords) {
    if(texcoords.x < 0.0 || texcoords.x > 1.0 || texcoords.y < 0.0 || texcoords.y > 1.0) return vec3(0.0);
	return texture(u_BlurPasstex,texcoords).xyz;
}

float convolution1D(float[kernelWidth] kernel, float[kernelWidth] input){
	float convol = 0.0;
	int step = 1;
	for(int i = 0; i < kernelWidth; i += step)	
		convol += input[i]*kernel[kernelWidth-i-1];
	return convol;
}

///////////////////////////////////
// MAIN
//////////////////////////////////

void main() {

	if(u_DisplayType != DISPLAY_BLOOM){
		vec3 color = sampleCol(fs_Texcoord);
		out_Color = vec4(color, 1.0);
	}
	else {

		float texDx = 1.0/u_ScreenWidth;
		float texDy = 1.0/u_ScreenHeight;

	    // modify background color to white for blending
	    vec3 ambientColor = texture(u_Posttex,fs_Texcoord).xyz;
	    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
		float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);
		if(lin_depth > 0.999999) ambientColor = vec3(1.0);

		float[kernelWidth] gaussianKernel1D;
		float[kernelWidth] input1D;
		float verticalOffset = 0.5*(kernelWidth-1);
		float deviation = 6.0;
		for(int i = 0; i < kernelWidth; ++i)
		{
			gaussianKernel1D[i] = sqrt(ONE_OVER_2PI) / deviation * exp(-(verticalOffset*verticalOffset) / (2.0*deviation*deviation));
			input1D[i] = sampleCol(vec2(fs_Texcoord.x, fs_Texcoord.y + verticalOffset*texDy)).r;
			verticalOffset -= 1.0;
		}

		//float blurred = convolutionGaussian(gaussianKernel, input);
		float blurred = convolution1D(gaussianKernel1D, input1D);

		//color = input[(kernelWidth-1)/2][(kernelWidth-1)/2]*vec3(1.0);
		ambientColor = (1-blurred)*ambientColor + blurred*vec3(1.0);

		out_Color = vec4(ambientColor, 1.0);
	}
    return;
}
