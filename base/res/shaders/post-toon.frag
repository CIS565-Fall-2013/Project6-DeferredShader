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
uniform sampler2D u_depthtex;
uniform sampler2D u_normaltex;

uniform int u_ScreenHeight;
uniform int u_ScreenWidth;
uniform float u_zNear;
uniform float u_zFar;

in vec2 fs_Texcoord;
out vec4 out_Color;

float linearizeDepth(float exp_depth, float near, float far)
{
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

///////////////////////////////////
// MAIN
//////////////////////////////////
void main() 
{
    vec2 offset = vec2( 1.0/u_ScreenWidth, 1.0/u_ScreenHeight );
	float gradientX = 0;
	float gradientY = 0;
	vec3 gradX;
	vec3 gradY;
	float weight = 1;
	//x direction (depth)
    gradientX += linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y - offset.y ) ).r, u_zNear, u_zFar );
	gradientX += 2*linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y ) ).r, u_zNear, u_zFar );
	gradientX += linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y + offset.y ) ).r, u_zNear, u_zFar );

	gradientX -= linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y - offset.y ) ).r, u_zNear, u_zFar );
	gradientX -= 2*linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y ) ).r, u_zNear, u_zFar );
	gradientX -= linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y + offset.y ) ).r, u_zNear, u_zFar );

	//y direction (depth)
    gradientY += linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y - offset.y ) ).r, u_zNear, u_zFar );
	gradientY += 2*linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x, fs_Texcoord.y - offset.y ) ).r, u_zNear, u_zFar );
	gradientY += linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y - offset.y ) ).r, u_zNear, u_zFar );

	gradientY-= linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y + offset.y ) ).r, u_zNear, u_zFar );
	gradientY -= 2*linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x, fs_Texcoord.y + offset.y ) ).r, u_zNear, u_zFar );
	gradientY -= linearizeDepth( texture( u_depthtex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y + offset.y ) ).r, u_zNear, u_zFar );

	//Xdirection
    gradX = texture( u_normaltex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y - offset.y ) ).xyz;
	gradX += 2*texture(u_normaltex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y ) ).xyz;
	gradX += texture( u_normaltex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y + offset.y ) ).xyz;

	gradX -= texture( u_normaltex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y - offset.y ) ).xyz;
	gradX -= 2*texture( u_normaltex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y ) ).xyz;
	gradX -= texture( u_normaltex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y + offset.y ) ).xyz;

	//y direction (depth)
    gradY = texture( u_normaltex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y - offset.y ) ).xyz;
	gradY += 2*texture( u_normaltex, vec2( fs_Texcoord.x, fs_Texcoord.y - offset.y ) ).xyz;
	gradY += texture( u_normaltex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y - offset.y ) ).xyz;

	gradY -= texture( u_normaltex, vec2( fs_Texcoord.x - offset.x, fs_Texcoord.y + offset.y ) ).xyz;
	gradY -= 2*texture( u_normaltex, vec2( fs_Texcoord.x, fs_Texcoord.y + offset.y ) ).xyz;
	gradY -= texture( u_normaltex, vec2( fs_Texcoord.x + offset.x, fs_Texcoord.y + offset.y ) ).xyz;

	out_Color = texture( u_Posttex, fs_Texcoord );

	if( gradientX > 0.1 || gradientY > 0.1 ) 
	    weight = 0;
	if( length( gradX ) > 1.1 || length(gradY) > 1.1)
	    weight = 0;
	out_Color *= weight;
}

