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
#define DISPLAY_GLOW 9


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Glowtex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

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
    vec3 color = sampleCol(fs_Texcoord);
    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
	vec3 glow = sampleGlow(fs_Texcoord);
    
	if (u_DisplayType == DISPLAY_GLOW)
	{
		vec2 glowWidth = textureSize(u_Glowtex, 0);

		int sz = 49;
		float gaussKernel[49] = float[](
			0.010463550522761987, 
			0.01131615298268607, 
			0.012197501949023151, 
			0.013103742011676042, 
			0.014030466903471261, 
			0.014972739372363077, 
			0.01592512163091689, 
			0.016881716501857894, 
			0.01783621912965036, 
			0.018781978866145247, 
			0.019712070670514083, 
			0.020619375097847215, 
			0.021496665695212178, 
			0.022336702387036944, 
			0.02313232922160704, 
			0.023876574674941628, 
			0.024562752574221497, 
			0.02518456161604963, 
			0.02573618141953292, 
			0.026212363073296496, 
			0.02660851221015391, 
			0.026920762772496484, 
			0.027146039812917307, 
			0.02728210990372968, 
			0.02732761799978166, 
			0.02728210990372968, 
			0.027146039812917307, 
			0.026920762772496484, 
			0.02660851221015391, 
			0.026212363073296496, 
			0.02573618141953292, 
			0.02518456161604963, 
			0.024562752574221497, 
			0.023876574674941628, 
			0.02313232922160704, 
			0.022336702387036944, 
			0.021496665695212178, 
			0.020619375097847215, 
			0.019712070670514083, 
			0.018781978866145247, 
			0.01783621912965036, 
			0.016881716501857894, 
			0.01592512163091689, 
			0.014972739372363077, 
			0.014030466903471261, 
			0.013103742011676042, 
			0.012197501949023151, 
			0.01131615298268607, 
			0.010463550522761987
		);

		float accum = 0.0;
		for (float i = -sz/2; i <= sz/2; i += 1.0) {
				int index = int(i) + sz/2;
				accum += gaussKernel[index] 
							 * sampleGlow(vec2(fs_Texcoord.s + (i/glowWidth.x), fs_Texcoord.t))
							 * sampleCol(vec2(fs_Texcoord.s + (i/glowWidth.x), fs_Texcoord.t));
				accum += gaussKernel[index] 
							 * sampleGlow(vec2(fs_Texcoord.s, fs_Texcoord.t + (i/glowWidth.y)))
							 * sampleCol(vec2(fs_Texcoord.s, fs_Texcoord.t + (i/glowWidth.y)));
		}

		vec3 c = accum + color;
		out_Color = vec4(c, 1.0);
	}
	else
	{
		out_Color = vec4(color,1.0);//vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	}
    return;
}

