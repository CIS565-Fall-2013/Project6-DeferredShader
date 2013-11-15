#version 330

////////////////////////////
//       ENUMERATIONS
////////////////////////////
#define OCCLUSION_NONE 0
#define OCCLUSION_REGULAR_SAMPLES 1
#define OCCLUSION_POISSON_SS_SAMPLES 2
#define OCCLUSION_WORLD_SPACE_SAMPLES 3


#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_OCCLUSION 3
#define	DISPLAY_TOTAL 4

#define M_PI 3.14159265358

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform mat4 u_Persp;

uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_Far;
uniform float u_Near;
uniform int u_OcclusionType;
uniform int u_DisplayType;

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

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
	return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack normals
vec3 samplePos(vec2 texcoords) {
	return texture(u_Positiontex,texcoords).xyz;
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



////////////////////////////////////////////////
/// BEGIN TODO: START HERE
//////////////////////////////////////////////

//Estimate occlusion based on a point and a sampled occluder
//Design this function based on specified constraints
float gatherOcclusion( vec3 pt_normal, vec3 pt_position, vec3 occluder_normal, vec3 occluder_position) {

	//distance constraint
	float dist = 1.0 / (1.0 + distance(pt_position, occluder_position));

	//planarrity test, clamp between 0 and 1
	float planar = min(1.0 - dot(pt_normal, occluder_normal), 1.0);

	//overhead 
	float overhead = dot(normalize(occluder_position - pt_position), pt_normal);
	overhead = max(0.0, overhead);	//clamp between 0 and 1

	return dist * planar * overhead;///IMPLEMENT THIS
}

const float REGULAR_SAMPLE_STEP = 0.008f;
float occlusionWithRegularSamples(vec2 texcoord, vec3 position, vec3 normal) {

	float occlusion = 0.0;

	//sample on 4*4 grid
	for (int i = -1; i < 3; ++i) {
		for (int j = -1; j < 3; ++j) {
			vec2 offset = 0.5 * REGULAR_SAMPLE_STEP * vec2(i, j) + texcoord;
			//get the occlusion on fragment from this sample
			occlusion += gatherOcclusion(normal, position, sampleNrm(offset), samplePos(offset));
		}
	}

	return occlusion / 16.0f; //IMPLEMENT THIS
}

//Disk of samples for the SS sampling
#define NUM_SS_SAMPLES 16
vec2 poissonDisk[NUM_SS_SAMPLES] = vec2[]( 
    vec2( -0.94201624, -0.39906216 ), 
	vec2( 0.94558609, -0.76890725 ), 
	vec2( -0.094184101, -0.92938870 ), 
	vec2( 0.34495938, 0.29387760 ), 
	vec2( -0.91588581, 0.45771432 ), 
	vec2( -0.81544232, -0.87912464 ), 
	vec2( -0.38277543, 0.27676845 ), 
	vec2( 0.97484398, 0.75648379 ), 
	vec2( 0.44323325, -0.97511554 ), 
	vec2( 0.53742981, -0.47373420 ), 
	vec2( -0.26496911, -0.41893023 ), 
	vec2( 0.79197514, 0.19090188 ), 
	vec2( -0.24188840, 0.99706507 ), 
	vec2( -0.81409955, 0.91437590 ), 
	vec2( 0.19984126, 0.78641367 ), 
	vec2( 0.14383161, -0.14100790)
);

const float SS_RADIUS = 0.01f;
float occlusionWithPoissonSSSamples(vec2 texcoord, vec3 position, vec3 normal) {

	float occlusion = 0.0;
	
	for (int i = 0; i < NUM_SS_SAMPLES; ++i) {
		//random number between 0 and 2PI
		float rand = 2.0 * M_PI * getRandomScalar(texcoord);
		vec2 point = SS_RADIUS * poissonDisk[i];

		//roate point around origin with rand radians
		vec2 offset;
		offset.s = cos(rand) * point.s - sin(rand) * point.t;
		offset.t = sin(rand) * point.s + cos(rand) * point.t; 
		offset += texcoord;
		occlusion += gatherOcclusion(normal, position, sampleNrm(offset), samplePos(offset));
	}

	return occlusion/float(NUM_SS_SAMPLES); //IMPLEMENT THIS
}


//Sphere of samples for the World Space sampling
#define NUM_WS_SAMPLES 16
vec3 poissonSphere[NUM_WS_SAMPLES] = vec3[](
	vec3(0.53812504, 0.18565957, -0.43192),
	vec3(0.13790712, 0.24864247, 0.44301823),
	vec3(0.33715037, 0.56794053, -0.005789503),
	vec3(-0.6999805, -0.04511441, -0.0019965635),
	vec3(0.06896307, -0.15983082, -0.85477847),
	vec3(0.056099437, 0.006954967, -0.1843352),
	vec3(-0.014653638, 0.14027752, 0.0762037),
	vec3(0.010019933, -0.1924225, -0.034443386),
	vec3(-0.35775623, -0.5301969, -0.43581226),
	vec3(-0.3169221, 0.106360726, 0.015860917),
	vec3(0.010350345, -0.58698344, 0.0046293875),
	vec3(-0.08972908, -0.49408212, 0.3287904),
	vec3(0.7119986, -0.0154690035, -0.09183723),
	vec3(-0.053382345, 0.059675813, -0.5411899),
	vec3(0.035267662, -0.063188605, 0.54602677),
	vec3(-0.47761092, 0.2847911, -0.0271716));



const float SPHERE_RADIUS = 0.2f;
float occlusionWithWorldSpaceSamples(vec2 texcoord, vec3 position, vec3 normal) {

	float occlusion = 0.0;

	//get random normal 
	vec3 randNorm = getRandomNormal(texcoord);

	for (int i = 0; i < NUM_WS_SAMPLES; ++i) {
		vec3 point = SPHERE_RADIUS * poissonSphere[i];
		//reflect point across plane defined by normal

		//reflect point across plane of normal
		point = reflect(point, randNorm);
		
		//constrain points in hemisphere
		if (dot(normal, normalize(point)) < 0)
			point *= -1.0;

		point += position;
		//transform point to clipping space
		vec4 screenCoord = u_Persp * vec4(point, 1.0);
		screenCoord.xyz /= screenCoord.w;

		//transform to screen space
		screenCoord.x = screenCoord.x*0.5 + (0.5);
		screenCoord.y = screenCoord.y*0.5 + (0.5);
		//screenCoord.x = (screenCoord.x+1.0) * (u_ScreenWidth/2.0);
		//screenCoord.y = (-screenCoord.y+1.0) * (u_ScreenHeight/2.0);

		//find occlusion
		vec2 offset = screenCoord.xy;
		occlusion += gatherOcclusion( normal, position, sampleNrm(offset), samplePos(offset));

	}

	return occlusion / float(NUM_WS_SAMPLES); //IMPLEMENT THIS
}

//////////////////////////////////////
// END TODO
//////////////////////////////////////


///////////////////////////////////
// MAIN, Shouldn't really need to mess with this much
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {

	float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

	float occlusion = 0.0f;

	vec3 normal = sampleNrm(fs_Texcoord);
	vec3 position = samplePos(fs_Texcoord);

	
	switch (u_OcclusionType) {
		case(OCCLUSION_NONE):
			break;
		case(OCCLUSION_REGULAR_SAMPLES):
			occlusion = occlusionWithRegularSamples(fs_Texcoord, position, normal);
			break;
		case(OCCLUSION_POISSON_SS_SAMPLES):
			occlusion = occlusionWithPoissonSSSamples(fs_Texcoord, position, normal);
			break;
		case(OCCLUSION_WORLD_SPACE_SAMPLES):
			occlusion = occlusionWithWorldSpaceSamples(fs_Texcoord, position, normal);
			break;
	}
		
	occlusion = clamp(occlusion*occlusion_strength,0.0f,1.0f);

	switch (u_DisplayType) {
		case(DISPLAY_DEPTH):
			//out_Color = vec4(exp_depth,exp_depth,exp_depth,1.0f);
			out_Color = vec4(lin_depth,lin_depth,lin_depth,1.0f);
			break;
		case(DISPLAY_NORMAL):
			out_Color = vec4(abs(normal),1.0f);
			break;
		case(DISPLAY_POSITION):
			out_Color = vec4(abs(position) / u_Far,1.0f);
			break;
		case(DISPLAY_OCCLUSION):
			out_Color = vec4(vec3(1.0f) - occlusion, 1.0f);
			break;
		case(DISPLAY_TOTAL):

		if (lin_depth > 0.99f) {
			out_Color = vec4(0.0f);
		} 
		else {
			vec3 to_eye = -normalize(position);
			float diffuse = dot(to_eye,normal);
			float ambient = 0.5;
			float val = diffuse*0.5 + ambient*(1.0f - occlusion); 
			out_Color = vec4(vec3(val),1.0f);
		}   
		break;
  	}	

	return;
}
