#version 330

////////////////////////////
//  ENUMERATIONS
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

uniform float u_ks = 0.2;
uniform float u_specPower = 1.0;
uniform vec3 u_specColor = vec3(1,1,1);

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
    return normalize(texture(u_Normaltex,texcoords).xyz);
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
    float lightRadius = u_Light.w;
    vec3 toLight = light - position;
	float distToLight = length(toLight);
	float attenCoefficient = 1.0 / (20.0 * distToLight * distToLight);


	out_Color = vec4(0,0,0,1.0);
	
    if( u_DisplayType == DISPLAY_LIGHTS )
    {
		if (distToLight < lightRadius)
		{
			out_Color = attenCoefficient * vec4(1, 1, 1, 1);
			//out_Color = (1.0 / (5.0 * lightRadius)) * vec4(1, 1, 1, 1);
			out_Spec = vec4(0, 0, 0, 1);
		}
		
    }
	else if (u_DisplayType == DISPLAY_TOON)
	{
		float intensity = max(0.0, dot(normalize(toLight),normal));
		vec3 toonColor = getToonColor(intensity, color);
		out_Color = vec4(toonColor, 1.0);
		out_Spec = vec4(0,0,0,1);
	}
    else 
    {
		if (distToLight < lightRadius)
		{
			//out_Color = attenCoefficient * vec4(1);
			float atten = (1.0 / (2.0 * distToLight));

			// compute diffuse color

			if (distToLight < 0.002)
			{
				out_Spec = vec4(0, 0, 0, 1);
				out_Color = vec4(1);
			}
			else
			{
				float diffuse = min(max(0.0, dot(normalize(toLight), normal)), 1.0);
				diffuse = clamp(dot(normal, normalize(toLight)), 0.0, 1.0);


				out_Color = vec4(0.8*diffuse*color, 1.0f);

				// compute specular color
				vec3 reflectDirection = vec3(0, 0, 0);

				// no specular reflection since light source is on the wrong side
				if (dot(normal, toLight) < 0.0)
				{
					out_Spec = vec4(0, 0, 0, 1);
				}
				else
				{
					reflectDirection = reflect(toLight, normal);
					vec3 viewDirection = normalize(position - vec3(0.0));
					vec3 specularHighlight = clamp(u_specColor * pow(max(0.0, dot(reflectDirection, viewDirection)), u_specPower), 0, 1);
					out_Spec = vec4(u_ks*specularHighlight, 1.0);
				}
			}
		}
		else
		{
			out_Spec = vec4(0, 0, 0, 1);
			out_Color = vec4(0, 0, 0, 1);
		}		
    }

	out_BloomMap = vec4(0,0,0,0); // no contribution
	
    return;
}
