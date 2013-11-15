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
#define	DISPLAY_AO 6
#define	DISPLAY_TOON 7
#define DISPLAY_SPEC 8


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

	// ambient light data
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;

	// computer SSAO (screen space ambient occlusion)
	// float occl = sampleBlurredDepth(fs_Texcoord);

	vec2 sz = textureSize(u_Positiontex, 0);
	
	float hup    = 0.0;	float tup    = 0.0;
	float hdown  = 0.0;	float tdown  = 0.0;
	float hleft  = 0.0;	float tleft  = 0.0;
	float hright = 0.0;	float tright = 0.0;

	for (float i = 1.0; i <= 6.0; i += 1.0) {

		// neighbor positions
		vec3 posUp   = samplePos(vec2(fs_Texcoord.s, fs_Texcoord.t + (1.0 / sz.y)));
		vec3 posDown = samplePos(vec2(fs_Texcoord.s, fs_Texcoord.t - (1.0 / sz.y)));
		vec3 posRight = samplePos(vec2(fs_Texcoord.s + (1.0 / sz.y), fs_Texcoord.t));
		vec3 posLeft = samplePos(vec2(fs_Texcoord.s - (1.0 / sz.y), fs_Texcoord.t));
	
		// up occlusion
		vec3 Hup = posUp - position;
		vec3 Tup = cross(normal, vec3(0,1,0));
		if (Hup.z > 0.01) {
			hup += atan(Hup.z / length(Hup.xy)) / 6.0;
			tup += atan(Tup.z / length(Tup.xy)) / 6.0;
		}
		
		// down occlusion
		vec3 Hdown = posDown - position;
		vec3 Tdown = cross(normal, vec3(0,-1,0));
		if (Hdown.z > 0.01) {
			hdown += atan(Hdown.z / length(Hdown.xy)) / 6.0;
			tdown += atan(Tdown.z / length(Tdown.xy)) / 6.0;
		}
	
		// right occlusion
		vec3 Hright = posRight - position;
		vec3 Tright = cross(normal, vec3(1,0,0));
		if (Hright.z > 0.01) {
			hright += atan(Hright.z / length(Hright.xy)) / 6.0;
			tright += atan(Tright.z / length(Tright.xy)) / 6.0;
		}
		
		// left occlusion
		vec3 Hleft = posLeft - position;
		vec3 Tleft = cross(normal, vec3(-1,0,0));
		if (Hleft.z > 0.01) {
			hleft += atan(Hleft.z / length(Hleft.xy)) / 6.0;
			tleft += atan(Tleft.z / length(Tleft.xy)) / 6.0;
		}
	}

	// ambient occlusion term
	float occl = (sin(hup) - sin(tup)) / 4.0 + (sin(hdown) - sin(tdown)) / 4.0 + (sin(hright) - sin(tright)) / 4.0 + (sin(hleft) - sin(tleft)) / 4.0;
	
    if( u_DisplayType == DISPLAY_TOON ) {

		ivec2 sz = textureSize(u_Normaltex,0);
		vec3 normal_up    = sampleNrm(vec2(fs_Texcoord.x+(1.0/sz.x), fs_Texcoord.y));
		vec3 normal_down  = sampleNrm(vec2(fs_Texcoord.x-(1.0/sz.x), fs_Texcoord.y));
		vec3 normal_right = sampleNrm(vec2(fs_Texcoord.x, fs_Texcoord.y+(1.0/sz.y)));
		vec3 normal_left  = sampleNrm(vec2(fs_Texcoord.x, fs_Texcoord.y-(1.0/sz.y)));

		if ( dot(normal, normal_up)    <= 0.96 ||
			 dot(normal, normal_down)  <= 0.96 ||
			 dot(normal, normal_right) <= 0.96 ||
			 dot(normal, normal_left)  <= 0.96)
		{
			out_Color = vec4(vec3(0), 1.0f);
		}

		else
		{
			float ambient = u_LightIl;
			float diffuse = max(0.0, dot(normalize(light),normal));
			
			// toon shading of ambient
			if		(diffuse <= 0.1) { diffuse = 0.2; }
			else if (diffuse <= 0.3) { diffuse = 0.4; }
			else if (diffuse <= 0.5) { diffuse = 0.6; }
			else if (diffuse <= 0.7) { diffuse = 0.8; }
			else					 { diffuse = 1.0; }
			
			out_Color = vec4(color*(strength*diffuse + ambient*(1.0-occl)),1.0f);
		}
	}
	
    return;
}

