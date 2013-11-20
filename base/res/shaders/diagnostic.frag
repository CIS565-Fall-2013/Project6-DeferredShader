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
#define DISPLAY_TOON 6
#define DISPLAY_BLUR 7
#define DISPLAY_DOF 8
#define DISPLAY_GLOW 9


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
uniform int u_OcclusionType;
uniform int u_DisplayType;

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

vec3 retrieveNormal(vec2 n){
    vec3 normal = vec3(n.x, n.y, 0.0); 
	normal.z = sqrt(1.0 - dot(n.xy, n.xy));
    return normal;
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return retrieveNormal(texture(u_Normaltex,texcoords).xy);
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automatically sample and unpack color
vec3 sampleCol(vec2 texcoords) {
     vec3 u = texture(u_Colortex,texcoords).xyz;
	 return vec3(float(u.x) / 255.0, float(u.y) / 255.0, float(u.z) / 255.0);
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

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float lightRadius = u_Light.w;

    switch (u_DisplayType) {
        case(DISPLAY_DEPTH):
            out_Color = vec4(vec3(lin_depth),1.0f);
            break;
        case(DISPLAY_NORMAL):
            out_Color = vec4(abs(normal),1.0f);
            break;
        case(DISPLAY_POSITION):
            out_Color = vec4(abs(position) / u_Far,1.0f);
            break;
        case(DISPLAY_COLOR):
            out_Color = vec4(color, 1.0);
            break;
		case(DISPLAY_GLOW):
        case(DISPLAY_LIGHTS):
        case(DISPLAY_TOTAL):
		case(DISPLAY_TOON):
		case(DISPLAY_BLUR):
		case(DISPLAY_DOF):
            break;
    }	

    return;
}

