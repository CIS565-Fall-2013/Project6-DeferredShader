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
uniform sampler2D u_aoResulttex;
uniform sampler2D u_normaltex;
uniform sampler2D u_colortex; 

in vec2 fs_Texcoord;
out vec4 out_Color;


vec4 motionBlur( vec2 texcoord, int nSamples )
{
    vec2 velocity = vec2( texture( u_normaltex, texcoord ).w, texture( u_colortex, texcoord ).w) ;
	vec4 blurredColor = vec4(0,0,0,0);
    for( int i = 1; i < nSamples; ++i )
    {
	    vec2 offset = velocity * ( float(i) / float(nSamples-1) - 0.5 );
		blurredColor += texture( u_aoResulttex, texcoord+offset );
	}

	return blurredColor / float(nSamples);
}


///////////////////////////////////
// MAIN
//////////////////////////////////
void main() 
{
	out_Color = motionBlur( fs_Texcoord, 15 );
}

