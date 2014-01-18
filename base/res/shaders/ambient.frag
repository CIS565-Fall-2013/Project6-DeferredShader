#version 330

////////////////////////////
// ENUMERATIONS
////////////////////////////

#define	DISPLAY_DEPTH 0
#define	DISPLAY_NORMAL 1
#define	DISPLAY_POSITION 2
#define	DISPLAY_COLOR 3
#define	DISPLAY_TOTAL 4
#define	DISPLAY_LIGHTS 5
#define	DISPLAY_TOON 6
#define	DISPLAY_BLOOM 7
#define	DISPLAY_AA 8
#define	DISPLAY_SPECULAR 9

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

out vec4 out_Color; // diffuse only
out vec4 out_Spec;
out vec4 out_BloomMap;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//	UTILITY FUNCTIONS
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

// Get toon color, given the current color and intensity of the light on the surface
vec3 getToonColor(float intensity, vec3 color)
{
	vec3 toonColor = vec3(0,0,0);
	
	if (intensity > 0.95)
		toonColor = color;
	else if (intensity > 0.5)
		toonColor = 0.6 * color;
	else if (intensity > 0.25)
		toonColor = 0.4 * color;
	else
		toonColor = 0.2 * color;
		
	return toonColor;
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
    float strength = u_Light.w;
	
	if (u_DisplayType == DISPLAY_LIGHTS)
	{
		out_Color = vec4(0,0,0,1);
	}
	else if (u_DisplayType != DISPLAY_TOON)
	{
		if (lin_depth > 0.99f) {
			out_Color = vec4(vec3(0.0), 1.0);
		} else {
			float ambient = u_LightIl;
			float diffuse = max(0.0, dot(normalize(light),normal));
			out_Color = vec4(color*(strength*diffuse + ambient),1.0f);
		}	
	}
	else // Ambient Toon Shading
	{
		float intensity = max(0.0, dot(normalize(light-position),normal));
		vec3 toonAmbColor = vec3(0.1,0.1,0.1);
		vec3 toonColor = getToonColor(intensity, toonAmbColor);
		out_Color = vec4(toonColor, 1.0f);
	}
	
	if (u_DisplayType == DISPLAY_BLOOM) // Look here: Alpha value is being used to mark if an object will be bloomed or not
	{
		vec4 origColor = texture(u_Colortex, fs_Texcoord);
		float shiny = origColor.a;
		if (shiny >= 1 || shiny == 0)
			out_BloomMap = vec4(0,0,0,0);
		else
			out_BloomMap = origColor;
	}
	else
	{
		out_BloomMap = vec4(0,0,0,0);
	}
	
	
	// No specular contribution for ambient
	out_Spec = vec4(0,0,0,0); 	
    return;
}

