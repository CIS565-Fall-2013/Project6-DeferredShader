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
#define DISPLAY_SSAO 8


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
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform float u_Distance;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////


uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//                                UTILITY FUNCTIONS
/////////////////////////////////////

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return        (2 * near) / (far + near -  exp_depth * (far - near)); 
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
float gatherOcclusion( vec3 pt_normal, vec3 pt_position, vec3 occluder_normal, vec3 occluder_position ) {
    vec3 diff = occluder_position - pt_position;
    float d = length(diff);
	float returnValue = 1.0;
    if (d > u_Distance)
        returnValue *= 0.0;
    if (distance(pt_normal, occluder_normal) < 0.1)
        returnValue *= 0.0;
    returnValue *= (u_Distance-d)/u_Distance * max(0.0, dot(normalize(diff), pt_normal));
    return returnValue;
}

const float REGULAR_SAMPLE_STEP = 0.012f;
float occlusionWithRegularSamples(vec2 texcoord, vec3 position, vec3 normal) {
	float occlusion = 0.0;
	for (int i = -5; i <= 5; i++) {
		for (int j = -5; j <= 5; j++) {
			vec2 texCoord = fs_Texcoord + i * vec2(0.0, 1.0/u_ScreenHeight) + j * vec2(1.0/u_ScreenWidth, 0.0);
			occlusion += gatherOcclusion(normal, position, normalize(sampleNrm(texCoord)), samplePos(texCoord));
		}
	}
	return occlusion;
}

//////////////////////////////////////
// END TODO
//////////////////////////////////////


///////////////////////////////////
// MAIN, Shouldn't really need to mess with this much
//////////////////////////////////
const float occlusion_strength = 0.1f;
void main() {  
	   
	   float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
		float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

        vec3 normal = normalize(sampleNrm(fs_Texcoord));
        vec3 position = samplePos(fs_Texcoord);

        float occlusion = occlusionWithRegularSamples(fs_Texcoord, position, normal);
                
        occlusion = clamp(occlusion*occlusion_strength,0.0f,1.0f);   

		if( u_DisplayType == DISPLAY_SSAO )
		{
			if (lin_depth > 0.99f) {
                    out_Color = vec4(0.0f);
            } else {
                    vec3 eyeToPosition = -normalize(position);
                    float diffuse = dot(eyeToPosition,normal);
                    float val = diffuse*0.7 + .3*(1.0f - occlusion); 
                    out_Color = vec4(vec3(val),1.0f);
            }
		}

        return;
}