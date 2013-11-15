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
uniform sampler2D u_Glowtex;
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

//Helper function to automicatlly sample and unpack positions
vec3 sampleGlow(vec2 texcoords) {
    return texture(u_Glowtex,texcoords).xyz;
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
	float glow = sampleGlow(fs_Texcoord).r;

    vec3 light = u_Light.xyz;
    float lightRadius = u_Light.w;
	out_Color = vec4(0, 0, 0, 0);
    if( u_DisplayType == DISPLAY_LIGHTS )
    {
		out_Color += vec4(0, 0, 0.4, 0);
    }
	else if (u_DisplayType == DISPLAY_TOON )
	{
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
			vec3 dirToLight = vec3(light.xyz - position.xyz);
			float distFromLight = length(dirToLight);
			if (distFromLight < lightRadius)
			{
				float I = max(0.0, dot(normal, normalize(dirToLight)))*u_Light.w / (distFromLight / (lightRadius - distFromLight));
			
				// toon shading
				if		(I <= 0.05) { I = 0.1; }
				else if (I <= 0.10) { I = 0.3; }
				else if (I <= 0.15) { I = 0.5; }
				else if (I <= 0.20) { I = 0.7; }
				else			    { I = 0.9; }

				out_Color += vec4(I);
				out_Color *= vec4(color, 1.0);
			}
		}
	}
    else
    {
		vec3 dirToLight = vec3(light.xyz - position.xyz);
		float distFromLight = length(dirToLight);
		if (distFromLight < lightRadius) {
			float I = max(0.0, dot(normal, normalize(dirToLight)))*u_Light.w / (distFromLight / (lightRadius - distFromLight));
			
			float diffuse = max(0.0, dot(normalize(dirToLight),normal));

			out_Color += diffuse*vec4(vec3(I), 1.0)*vec4(color, 1.0);
			
			if (u_DisplayType == DISPLAY_SPEC) {
				vec3 specRefl = 2.0 * dot(normal, dirToLight) * normal - dirToLight;
				float specular = clamp(pow(dot(-position, specRefl), glow), 0.0, 1.0);
				out_Color += glow*glow*specular*vec4(vec3(I), 1.0)*vec4(color,1.0);
			}			
		} 
    }
	
    return;
}

