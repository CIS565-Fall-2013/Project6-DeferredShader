#version 330

uniform int SMOOTH_KERNEL_SIZE=4;

uniform sampler2D u_aoResulttex;
uniform sampler2D u_Posttex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

in vec2 fs_Texcoord;
out vec4 out_Color;

void main()
{
    float blendResult = 0;
    vec2 delta = vec2( 1.0/u_ScreenWidth, 1.0/u_ScreenHeight );
	vec2 offset;
	
	vec2 start = vec2( -SMOOTH_KERNEL_SIZE*0.5 + 0.5 );
	for( int i = 0; i < SMOOTH_KERNEL_SIZE; ++i )
	    for( int j = 0; j < SMOOTH_KERNEL_SIZE; ++j )
	    {
		    offset = ( start + vec2( j, i ) ) * delta;
			blendResult += texture( u_aoResulttex, fs_Texcoord + offset ).r;
		}

	blendResult /= float( SMOOTH_KERNEL_SIZE*SMOOTH_KERNEL_SIZE );
    out_Color = vec4( blendResult );
	out_Color *= texture( u_Posttex, fs_Texcoord );
	
}

