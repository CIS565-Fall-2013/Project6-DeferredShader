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
#define DISPLAY_SIL 7

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;
uniform sampler2D u_Colortex;
uniform sampler2D u_Bloomtex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Normaltex;

uniform vec3 u_cameraPos; // eyePos

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform int u_DisplayType;

uniform float u_texelSizeX;
uniform float u_texelSizeY;
uniform int u_blurAmount;
uniform float u_blurScale;
uniform float u_blurStrength;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;


/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
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

float Gaussian1D(float x, float y, float dev)
{
	return (1.0/sqrt(2.0*3.14159 * dev)) * exp(-(x*x)/(2.0*dev));			
}
float Gaussian2D(float x, float y, float dev)
{
	return ( 1.0 / (2.0*3.14159 * dev * dev)) * exp(-((x*x+y*y)/(2.0*dev*dev)));
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}
//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
	vec3 normal = sampleNrm(fs_Texcoord);
	vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    vec4 otherColor = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	vec4 glowColor = vec4(0.0);
	if(u_DisplayType == DISPLAY_BLOOM)
	{

		float dev = float(u_blurAmount) * 0.3;
		for(int i = -u_blurAmount/2; i<=u_blurAmount/2; ++i)
		{
			for(int j = -u_blurAmount/2; j<=u_blurAmount/2; ++j)
			{
				glowColor += texture(u_Bloomtex,fs_Texcoord + vec2(u_texelSizeX * i * u_blurScale , u_texelSizeY * j* u_blurScale))
					* Gaussian2D(float(i)*u_blurStrength,float(j)*u_blurStrength,dev);
			
			}
		}
		out_Color = clamp((glowColor + otherColor)-(glowColor * otherColor),0.0,1.0);
		return;		
	}
	else if(u_DisplayType == DISPLAY_SIL)
	{
		vec3 color1 =vec3(0.0,0.0,0.0);
		vec3 eyeVector = vec3(0.0,0.0,5.0) - position;
		eyeVector = normalize(eyeVector);
		float sil = max(dot(normal,eyeVector), 0.0);
		if (sil < 0.1)
			out_Color = vec4(color1,1.0);
		else
			out_Color = otherColor;
		return;
		
	}
	out_Color = otherColor;
    return;
}

