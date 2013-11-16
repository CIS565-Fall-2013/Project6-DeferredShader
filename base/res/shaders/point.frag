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
#define	DISPLAY_OUTLINE 6
#define	DISPLAY_TOON 7
#define	DISPLAY_BLOOM 8


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
uniform sampler2D u_SpecularColortex;

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
    // Original G-buffer.
	return texture(u_Normaltex,texcoords).xyz;

	// Compact G-buffer.
	//float Nx = texture(u_Colortex,texcoords).w;
	//float Ny = texture(u_SpecularColortex,texcoords).w;
	//return vec3(Nx, Ny, sqrt(1.0 - Nx*Nx - Ny*Ny));
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack colors
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Colortex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack specular colors
vec3 sampleSpecCol(vec2 texcoords) {
    return texture(u_SpecularColortex,texcoords).xyz;
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
    if( u_DisplayType == DISPLAY_LIGHTS )
    {
        //Put some code here to visualize the fragment associated with this point light
		out_Color = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        //Put some code here to actually compute the light from the point light
		
		vec3 N_unit = normalize(normal);					// Normal.
		vec3 L      = light - position;						// Fragment to light.
		vec3 L_unit = normalize(L);
		vec3 E_unit = normalize(-position);					// Fragment to eye.
		vec3 R_unit = normalize(-reflect(L_unit, N_unit));	// Fragment to reflected light direction.

		float lightIntensity = u_LightIl * max(lightRadius-length(L), 0.0) / lightRadius;
		
		// Diffuse term
		vec3 diffuseColor = color * max(dot(N_unit, L_unit), 0.0);
		diffuseColor = clamp(diffuseColor, 0.0, 1.0);

		// Specular term
		vec3 specularColor = sampleSpecCol(fs_Texcoord);
		{
			float specularExponent = 15.0;
			float magnitude = max(dot(R_unit, E_unit), 0.0);
			specularColor *= pow(magnitude, specularExponent);
		}
		specularColor = clamp(specularColor, 0.0, 1.0);

		out_Color = vec4(lightIntensity * (diffuseColor + specularColor), 1.0);
		//out_Color = vec4(clamp(lightIntensity*color*diffuseTerm, 0.0, 1.0), 1.0);
    }
    return;
}

