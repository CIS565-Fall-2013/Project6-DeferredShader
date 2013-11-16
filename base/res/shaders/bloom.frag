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
uniform sampler2D u_Posttex;
uniform sampler2D u_Shininess;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;


uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////




uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;

uniform int kernelX = 100;

/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
}

float sampleBloomAlpha(vec2 texcoords) {
    return texture(u_Shininess,texcoords).r;
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
void main() {
    vec3 color = sampleCol(fs_Texcoord);

	int kxHalf = kernelX/2;
	int count = 0;
	float delX = 1.0/u_ScreenWidth;

	vec3 sumColor = vec3(0.0);

	for(int i=-kxHalf; i<=kxHalf; ++i)
	{
		vec2 texCoord = vec2(fs_Texcoord.s+i*delX, fs_Texcoord.t);

		float alpha = sampleBloomAlpha(texCoord);
		vec3 color = sampleCol(texCoord);
		
		sumColor += alpha*color;
		count++;
	}

	sumColor = 1.0/count * sumColor;

	out_Color = vec4(sumColor,1.0);

	//out_Color = vec4(sampleCol(fs_Texcoord),1.0);
    return;
}

