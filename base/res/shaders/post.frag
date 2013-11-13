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
uniform sampler2D u_mv; 

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

in vec2 fs_Texcoord;

out vec4 out_Color;


uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;

vec4 motionBlur( vec2 texcoord, int nSamples )
{
    vec2 velocity = texture( u_mv, texcoord ).xy;
	vec4 blurredColor = vec4(0,0,0,0);
    for( int i = 1; i < nSamples; ++i )
    {
	    vec2 offset = velocity * ( float(i) / float(nSamples-1) - 0.5 );
		blurredColor += texture( u_Posttex, texcoord+offset );
	}

	return blurredColor / float(nSamples);
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() 
{
	out_Color = motionBlur( fs_Texcoord, 15 );
}

