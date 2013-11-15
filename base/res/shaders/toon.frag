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
#define DISPLAY_SPECULAR 6
#define DISPLAY_TOON 7
#define KERNEL_SIZE 9

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
uniform sampler2D u_Shininesstex;

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

float sampleShininess(vec2 texcoords) {
    return texture(u_Shininesstex,texcoords).r;
}

//Get a random scalar given a screen-space texture coordinate
//Fetches from a random texture
float getRandomScalar(vec2 texcoords) {
    ivec2 sz = textureSize(u_RandomScalartex,0);
    return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
                texcoords.t*u_ScreenHeight/sz.y)).r;
}


vec4 getToonColor(float intensity) {
	vec4 color = vec4(vec3(0.0),1.0);
	if(intensity >0.9)
		color = vec4( vec3(1.0),1.0);
	else if(intensity >0.7)
		color = vec4( vec3(0.8),1.0);
	else if(intensity >0.5)
		color = vec4( vec3(0.6),1.0);
	else if(intensity >0.3)
		color = vec4( vec3(0.4),1.0);
	else if(intensity >0.0)
		color = vec4( vec3(0.2),1.0);

	return color;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;

vec3 kernelNormals[KERNEL_SIZE];
float kernelDepths[KERNEL_SIZE];

float depthsDiff[KERNEL_SIZE];
float normalsDiff[KERNEL_SIZE];

vec2 offset[KERNEL_SIZE];

float d_threshold;
float n_threshold;

void initOffset()
{
	float delX = 1.0/u_ScreenWidth;
	float delY = 1.0/u_ScreenHeight;

	//int kernelX = sqrt(KERNEL_SIZE)/2;
	int kernelX = 1;
	int idx =0;
	for(int j=kernelX; j>= -kernelX; --j)
		for(int i= -kernelX; i<=kernelX; ++i)
		{
			offset[idx++] = vec2(i*delX,j*delY);
		}

}

void fillKernelValues(vec2 texCoord)
{
	for(int i=0; i< KERNEL_SIZE;++i)
	{
		float exp_depth = texture(u_Depthtex,texCoord+offset[i]).r;
		kernelDepths[i] = linearizeDepth(exp_depth,u_Near,u_Far);
		kernelNormals[i] = sampleNrm(texCoord+offset[i]);
	}
}

void main() {

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
	vec3 posToLight = light-position;


	//Referred
	//http://www.cs.rutgers.edu/~decarlo/671/readings/decaudin_1996.pdf
	//for the edge detection filter

    if( u_DisplayType == DISPLAY_TOON )
    {

		initOffset();

		for(int i=0; i<KERNEL_SIZE;i++)
		{
			vec2 texCoord = fs_Texcoord+offset[i];
			fillKernelValues(texCoord);
			depthsDiff[i] = 1/8.0*( abs(kernelDepths[0]-kernelDepths[4]) + 
									2.0*abs(kernelDepths[1]-kernelDepths[4])+
									abs(kernelDepths[2]-kernelDepths[4]) + 
									2.0*abs(kernelDepths[3]-kernelDepths[4]) +  
									2.0*abs(kernelDepths[5]-kernelDepths[4]) +
									abs(kernelDepths[6]-kernelDepths[4]) + 
									2.0*abs(kernelDepths[7]-kernelDepths[4]) + 
									abs(kernelDepths[8]-kernelDepths[4]) );

			normalsDiff[i] = 1/8.0*( length(kernelNormals[0]-kernelNormals[4]) + 
									2.0*length(kernelNormals[1]-kernelNormals[4])+
									length(kernelNormals[2]-kernelNormals[4]) + 
									2.0*length(kernelNormals[3]-kernelNormals[4]) +  
									2.0*length(kernelNormals[5]-kernelNormals[4]) +
									length(kernelNormals[6]-kernelNormals[4]) + 
									2.0*length(kernelNormals[7]-kernelNormals[4]) + 
									length(kernelNormals[8]-kernelNormals[4]) );
		}

		float gdmin=1.0;
		float gdmax=0.0;
		float gnmin=1.0;
		float gnmax=0.0;

		d_threshold = 0.0001;
		n_threshold = 0.0001;

		for(int i=0; i<KERNEL_SIZE; i++)
		{
			if (depthsDiff[i]>gdmax)
				gdmax = depthsDiff[i];
			if (depthsDiff[i]<gdmin)
				gdmin = depthsDiff[i];
			if (normalsDiff[i]>gnmax)
				gnmax = normalsDiff[i];
			if (normalsDiff[i]<gnmin)
				gnmin = normalsDiff[i];
		}

		float d = (gdmax-gdmin)/d_threshold;
		float n = (gnmax-gnmin)/n_threshold;

		float dp = min( d*d,1.0);
		float dn = min( n*n,1.0);

		if (dp==1.0 || dn==1.0)
			out_Color = vec4( vec3(0.0),1.0);
		else
		{
			vec3 posToLightDir = normalize(posToLight);
			float diffuse = max(0.0, dot(posToLightDir,normal));
			vec4 toonColor = getToonColor(diffuse);
			out_Color = vec4(color,1.0f)*toonColor;
		}
    }
    return;
}

