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
uniform int u_OcclusionType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform vec4 u_Light;
uniform float u_LightIl;

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

// from fall 2012 base code
const float SS_RADIUS = 0.01f;
const float constAttenuation  = 1.0;
const float linearAttenuation = 20.0;
const float quadAttenuation = 50.0;
const float bias = 0.05;
const float AOIntensity = 2.0;
float gatherOcclusion( vec3 pt_normal, vec3 pt_position, vec3 occluder_normal, vec3 occluder_position) {

    //no occluder
    if(abs(length(occluder_position)) < 1e-6) return 0.0;

	vec3 occluderDir = occluder_position - pt_position;
	float distance = length(occluderDir);
	float attenuation = 1.0 / (constAttenuation + linearAttenuation*distance + quadAttenuation*distance*distance);
	float ambientOcclusion = max(dot(pt_normal, occluderDir) - bias, 0.0) * attenuation;
    return ambientOcclusion;
}
const float SAMPLE_STEP = 0.01f;
vec2 sampleDirection[4] = 
	{vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(-1.0, 0.0), vec2(0.0, -1.0)};
mat2 rot45 = mat2(vec2(0.707, 0.707), vec2(-0.707, 0.707));

const int SSAA_ITER = 4;
float occlusionWithRegularSamples(vec2 texcoord, vec3 position, vec3 normal, float linearDepth) {
		
		float radius = SS_RADIUS*(1.0 - linearDepth);
		float step = SAMPLE_STEP*(1.0 - linearDepth);
		float accumOcclusion = 0.0;
		for(int i = 0; i < SSAA_ITER; ++i)
		{
			for(int j = 0; j < sampleDirection.length; ++j)
			{
				vec2 dir = radius * sampleDirection[j];
				vec3 ranNormal = getRandomNormal(texcoord+dir);
				dir = reflect(-dir, ranNormal.xy);
				vec3 occ_position = samplePos(texcoord+dir);
				vec3 occ_normal = sampleNrm(texcoord+dir);
				accumOcclusion += 1.0 / (SSAA_ITER * sampleDirection.length) * gatherOcclusion(normal, position, occ_normal, occ_position) * AOIntensity;
				sampleDirection[j] = rot45*sampleDirection[j];
			}
			radius += step;
		}
			
        return accumOcclusion; 
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
	float quantizer = 1.0;
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;
	float ambient = u_LightIl;
	float diffuse;
	float occlusion = 0.0;
    if (lin_depth > 0.99f) {
        color = vec3(0.0);
    } else {      
        diffuse = clamp(dot(normalize(normal), normalize(light)), 0.0, 1.0);
        color *= strength*diffuse + ambient;
    }
	if(u_DisplayType == DISPLAY_TOON || u_DisplayType == DISPLAY_TOONEDGE )
	{
		if(diffuse > 0.0 && diffuse < 0.2)	quantizer = 0.4;
		if(diffuse > 0.2 && diffuse < 0.4)	quantizer = 0.3;
		if(diffuse > 0.4 && diffuse < 0.6)	quantizer = 0.5;
		if(diffuse > 0.6 && diffuse < 0.8)	quantizer = 0.7;
		if(diffuse > 0.8 && diffuse < 1.0)	quantizer = 0.9;

		color *= quantizer;
	}
	else if(u_DisplayType == DISPLAY_SSAO)
	{
		occlusion = occlusionWithRegularSamples(fs_Texcoord, position, normal, lin_depth);
		color = clamp(color - vec3(1.0)*occlusion, vec3(0.0), vec3(1.0));
	} 
	out_Color = vec4(color, 1.0);
	
    return;
}

