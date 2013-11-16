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
#define DISPLAY_TOON 6

/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform mat4 u_Persp;
uniform mat4 u_invPersp;

uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
//uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

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
 //   vec3 G = texture(u_Normaltex,texcoords).xyz;
	//vec3 N;
	//N.z = dot(G.xy,G.xy)*2-1;
	//N.xy = normalize( G.xy ) * sqrt( 1-N.z*N.z );
	vec3 N = texture( u_Normaltex, texcoords ).xyz;
	return N;
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    float depth = texture( u_Depthtex, texcoords).r * 2.0 - 1.0f;
	//depth = u_Persp[3][2] / ( depth -u_Persp[2][2] );

	vec2 vxy = texcoords * 2.0 - 1;
    vec4 P = vec4(vxy.x,vxy.y,depth,1);
	P = u_invPersp * P;
    return vec3(P.xyz/P.w );
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Colortex,texcoords).xyz;
}

//Get a random normal vector  given a screen-space texture coordinate
//Actually accesses a texture of random vectors
//vec3 getRandomNormal(vec2 texcoords) {
//    ivec2 sz = textureSize(u_RandomNormaltex,0);
//    return texture(u_RandomNormaltex,vec2(texcoords.s* (u_ScreenWidth)/sz.x,
//                (texcoords.t)*(u_ScreenHeight)/sz.y)).rgb;
//}


////Get a random scalar given a screen-space texture coordinate
////Fetches from a random texture
//float getRandomScalar(vec2 texcoords) {
//    ivec2 sz = textureSize(u_RandomScalartex,0);
//    return texture(u_RandomScalartex,vec2(texcoords.s*u_ScreenWidth/sz.x,
//                texcoords.t*u_ScreenHeight/sz.y)).r;
//}

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

	float dst_factor = 5/distance( light, position );
    if( u_DisplayType == DISPLAY_LIGHTS )
    {
        //Put some code here to visualize the fragment associated with this point light
		out_Color = ( dst_factor*dst_factor ) * vec4( 1.0, 1.0, 1.0, 1 ); 
		
    }
    else if( u_DisplayType == DISPLAY_TOON )
    {
        //Put some code here to actually compute the light from the point light
		vec3 lightDir = normalize( light - position );
		
		float NdotL = max( 0, dot( normal, lightDir ));
		if( NdotL > 0.95 )
		    NdotL = 1;
		else if( NdotL > 0.7 )
		    NdotL = 0.7;
		else if( NdotL > 0.5 )
		    NdotL = 0.5;
		else if( NdotL > 0.25 )
		    NdotL = 0.25;
	    else
		    NdotL = 0.1;

        out_Color = vec4( color * NdotL, 1 );
    }
	else
	{
	    vec3 lightDir = normalize( light - position );
		out_Color = dst_factor*dst_factor*vec4( color * max( 0, dot( normal, lightDir )), 1 );
	}
    
}

