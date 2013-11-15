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
#define DISPLAY_TOON 7

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

uniform float u_Far;
uniform float u_Near;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform vec4 u_Light;
uniform vec3 u_CamPosition;

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


///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {

	vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);

	//sobel filtering
	float stepX = 1.0/u_ScreenWidth;
	float stepY = 1.0/u_ScreenHeight;

	vec3 samples[9]; 
	int count = 0;

	//sample the pixel and its 8 neighbors
	for (int j = -1; j < 2; ++j){
		for (int i = -1; i < 2; ++i){
			vec2 offset = vec2 (i * stepX, j * stepY);
			samples[count] = sampleCol(fs_Texcoord.st + offset);
			count ++;		
		}
	}
	
	//horizontal gradient
	// -1  0  1
	// -2  0  2
	// -1  0  1
	vec3 Dx = -samples[0] + samples[2] - 2.0*samples[3] + 2.0*samples[5] - samples[6] + samples[8];
	
	//vertical gradient
	//  1  2  1
	//  0  0  0
	// -1 -2 -1
	vec3 Dy = samples[0] + 2.0*samples[1] + samples[2] - samples[6] - 2.0*samples[7] - samples[8];

	//calculate magnitude of gradient, if s is high, there is an edge, else, no edge
	float s = sqrt(dot(Dx, Dx) + dot(Dy, Dy));

	//vec3 eyeDir = normalize(u_CamPosition - position);
	
	//if (dot(eyeDir, normal) < 0.0)
	//	s += 0.0;

	//toon shading based on normals
	vec3 lightDir = normalize(u_Light.xyz - position);
	float shade = max(dot(lightDir, normal), 0.0);

	if (shade > 0.9) {
		out_Color.rgb = 1.3 * color; 
	}
	else if (shade > 0.7) {
		out_Color.rgb = color;
	}
	else if (shade > 0.3) {
		out_Color.rgb = 0.3 * color;
	} 
	else{
		out_Color.rgb = 0.2 * color;
	}
	
	out_Color.rgb *= (1.0 - s);
    return;
}

