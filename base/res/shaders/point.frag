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
uniform sampler2D u_Bloomtex;

uniform float u_Far;
uniform float u_Near;
uniform int u_DisplayType;
uniform vec3 u_lightColor;

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
	vec3 lightVector = light - position;
	float dist = length(lightVector);
	//vec3 lightColor = vec3(1.0,0.0,1.0);
	float Intensity = 2.0;
    if( u_DisplayType == DISPLAY_LIGHTS )
    {
		//Put some code here to visualize the fragment associated with this point light
		if(dist <= lightRadius)
		{
			vec3 markColor = vec3(1.0,1.0,1.0)*0.4 + color * 0.6;
			out_Color = vec4(markColor,1.0);
		}	
    }
    else
    {	
		float diffuse = max(0.0,dot(normalize(lightVector),normal));
		float isInradius = step(0.0,lightRadius - length(lightVector));
		float attenuation = dist/(1-(dist/lightRadius)*(dist/lightRadius));
		attenuation = attenuation / lightRadius + 1;
		attenuation = 1.0/(attenuation * attenuation);
		vec3 diffuseColor = Intensity * color * diffuse * u_lightColor * attenuation * isInradius;
		out_Color = vec4(diffuseColor , 1.0);	
    }
    return;
}

//light attenuation equation got from here
//http://imdoingitwrong.wordpress.com/2011/02/10/improved-light-attenuation/
// 1/(d/r + 1)^2
// d is the position distance from light
// r = 1 - (d/lightRadius);

