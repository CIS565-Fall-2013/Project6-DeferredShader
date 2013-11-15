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

#define SSAO_SAMPLE_SIZE 64
/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_mv; 
uniform sampler2D u_normaltex;
uniform sampler2D u_depthtex;
uniform sampler2D u_postex;
uniform sampler2D u_noisetex;
uniform sampler1D u_kerneltex;

uniform int u_ScreenWixth;
uniform int u_ScreenHeight;
uniform vec2 u_noiseSampleFactor;
uniform float u_zNear;
uniform float u_zFar;
//uniform vec3 u_kernels[SSAO_SAMPLE_SIZE];
uniform mat4 u_proj;


in vec2 fs_Texcoord;
out vec4 out_Color;


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

float linearizeDepth(float exp_depth, float near, float far)
{
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}


float ambientOcclusion( vec2 texcoord )
{
    vec3 pos = texture( u_postex, texcoord ).rgb;
    vec3 N = texture( u_normaltex, texcoord ).rgb;
    //float depth = texture( u_depthtex, texcoord ).r;
	//depth = linearizeDepth( depth, u_zNear, u_zFar);


	vec3 rVec = texture( u_noisetex, texcoord * vec2( u_noiseSampleFactor.x, u_noiseSampleFactor.y )  ).xyz;// * 2.0 - 1.0;
	//rVec.z = 0;
	vec3 T = normalize( rVec - N * dot(rVec, N) );
	vec3 B = cross( T, N );
	mat3 TBN = mat3( T, B, N );
	
	float occlusion = 0;
	//sample the depth of sampling points
    for( int i = 0; i < SSAO_SAMPLE_SIZE; ++i )
	{
	    vec3 s_pos = TBN * texture( u_kerneltex, i / float(SSAO_SAMPLE_SIZE) ).xyz;
		s_pos = s_pos +  pos;
	
		//project sample pos to 2D screen space
		vec4 offset = u_proj * vec4( s_pos, 1.0 );
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
	    
		//float s_depth = texture( u_depthtex, offset.xy ).r;
  //      s_depth = linearizeDepth( s_depth , u_zNear, u_zFar );
		//offset.z =  linearizeDepth( offset.z, u_zNear, u_zFar );

		//float cutoff = abs( depth - s_depth ) < 0.03 ? 1.0 : 0.0;
		//occlusion += ( s_depth <= offset.z? 1.0 : 0.0 ) * cutoff ;
		
		vec3 sampleP = texture( u_postex, offset.xy ).xyz;
		float cutoff = abs( pos.z - sampleP.z ) <1.4 ? 1.0 : 0.0;
		occlusion += ( s_pos.z <= sampleP.z ? 1.0 : 0.0 ) * cutoff ;
	}
	occlusion /= float(SSAO_SAMPLE_SIZE);
	
	return occlusion;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() 
{
	out_Color = motionBlur( fs_Texcoord, 15 );
	float occ = ambientOcclusion( fs_Texcoord );
	//out_Color =  texture( u_Posttex, fs_Texcoord );

	//vec3 rVec = texture( u_noisetex, fs_Texcoord  ).rgb;
	out_Color = (1-occ)*out_Color;
}

