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
    out_Color = vec4(0,0,0,1.0);
    if( u_DisplayType == DISPLAY_TOON )
	{
		out_Color = vec4(1,1,1,1);
        vec3 normals[8];
        normals[0] = sampleNrm(fs_Texcoord + vec2(-2.0/u_ScreenWidth, -2.0/u_ScreenHeight));
        normals[1] = sampleNrm(fs_Texcoord + vec2(2.0/u_ScreenWidth, -2.0/u_ScreenHeight));
        normals[2] = sampleNrm(fs_Texcoord + vec2(-2.0/u_ScreenWidth, 2.0/u_ScreenHeight));
        normals[3] = sampleNrm(fs_Texcoord + vec2(2.0/u_ScreenWidth, 2.0/u_ScreenHeight));
		normals[4] = sampleNrm(fs_Texcoord + vec2(-2.0/u_ScreenWidth, 0.0));
        normals[5] = sampleNrm(fs_Texcoord + vec2(2.0/u_ScreenWidth, 0.0));
        normals[6] = sampleNrm(fs_Texcoord + vec2(0.0, -2.0/u_ScreenHeight));
        normals[7] = sampleNrm(fs_Texcoord + vec2(0.0, 2.0/u_ScreenHeight));

		vec3 pos[8];
        pos[0] = samplePos(fs_Texcoord + vec2(-2.0/u_ScreenWidth, -2.0/u_ScreenHeight));
        pos[1] = samplePos(fs_Texcoord + vec2(2.0/u_ScreenWidth, -2.0/u_ScreenHeight));
        pos[2] = samplePos(fs_Texcoord + vec2(-2.0/u_ScreenWidth, 2.0/u_ScreenHeight));
        pos[3] = samplePos(fs_Texcoord + vec2(2.0/u_ScreenWidth, 2.0/u_ScreenHeight));
		pos[4] = samplePos(fs_Texcoord + vec2(-2.0/u_ScreenWidth, 0.0));
        pos[5] = samplePos(fs_Texcoord + vec2(2.0/u_ScreenWidth, 0.0));
        pos[6] = samplePos(fs_Texcoord + vec2(0.0, -2.0/u_ScreenHeight));
        pos[7] = samplePos(fs_Texcoord + vec2(0.0, 2.0/u_ScreenHeight));

        bool isEdge = false;
        for (int i=0; i<8; i++) {
            if (dot(normal, normals[i]) < 0.8) {
                isEdge = true;
                break;
            }
			if (length(position-pos[i]) > 1.0){
				isEdge = true;
				break;
			}
        }
        if (isEdge) {
            out_Color = vec4(vec3(0.0), 1.0);
        }
        else {
            //compute light contribution
            float amb = u_LightIl;
            float diffuse = max(0.0, dot(normalize(light),normal));
            out_Color = vec4(color*(lightRadius*diffuse + amb),1.0f);
				
            out_Color.r = round(out_Color.r/0.5) * 0.5;
            out_Color.g = round(out_Color.g/0.5) * 0.5;
            out_Color.b = round(out_Color.b/0.5) * 0.5;
            out_Color.a = 1.0;
        }
    }
    return;
}

