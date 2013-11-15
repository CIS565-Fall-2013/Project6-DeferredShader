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
uniform sampler2D u_Colortex;
uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_Far;
uniform float u_Near;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_BloomEnable;

in vec2 fs_Texcoord;

out vec4 out_Color;
///////////////////////////////////////

uniform float zerothresh = 1.0f;
uniform float falloff = 0.1f;

/////////////////////////////////////
//				UTILITY FUNCTIONS
/////////////////////////////////////
//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
}

//Depth used in the Z buffer is not linearly related to distance from camera
//This restores linear depth
float linearizeDepth(float exp_depth, float near, float far) {
    return	(2 * near) / (far + near -  exp_depth * (far - near)); 
}

float sampleDepth(vec2 texcoords) {
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    return linearizeDepth( exp_depth, u_Near, u_Far );
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

    
    //depth = sqrt(sampleDepth( fs_Texcoord ));
    //out_Color = vec4(depth, depth, depth, 1.0 );
    //out_Color = vec4(sampleNrm( fs_Texcoord ), 1.0);

    //out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
    /* 
    // Bloom
    if ( u_BloomEnable == 1 ) {
	out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
	return;	
    }
      
    int blur_range = 10;
    vec3 blur_val = vec3( 0.0, 0.0, 0.0 );  
    // Super-duper Naive implementation of bloom
    for ( int i=0; i < blur_range; i++ ) {
      for ( int j=0; j < blur_range; j++ ) {
	float x = fs_Texcoord.x+(i-blur_range/2)/float(u_ScreenWidth);
	float y = fs_Texcoord.y+(j-blur_range/2)/float(u_ScreenHeight);
	vec3 sample_color = texture(u_Colortex,vec2(x,y)).xyz;
	if ( sample_color.g > 0.1 && sample_color.r < 0.1)
	  blur_val += sample_color/float(blur_range);
      }
    }
    out_Color = vec4(color+blur_val, 1.0);
    */
    // Cell shading
    int num_shades = 15;
    out_Color = vec4(round(color.x*num_shades)/num_shades, round(color.y*num_shades)/num_shades, round(color.z*num_shades)/num_shades, 1.0 );

    // Naive sobel on normals
    mat3 sobel_kernel = mat3( 1.0, 2.0, 1.0 ,
			      0.0, 0.0, 0.0 , 
			     -1.0,-2.0,-1.0 );
    float Gx = 0.0;
    float Gy = 0.0;
    float depth;
    vec3 N;
    // Compute Gx 
    for ( int i=0; i<3; i++ ) {
      for ( int j=0; j<3; j++ ) {
	depth = sqrt(sampleDepth( vec2( fs_Texcoord.x+(i-1)/float(u_ScreenWidth) , fs_Texcoord.y+(j-1)/float(u_ScreenHeight)) ));
	
	N = sampleNrm( vec2( fs_Texcoord.x+(i-1)/float(u_ScreenWidth) , fs_Texcoord.y+(j-1)/float(u_ScreenHeight)) );
	depth = dot( N, vec3(1.0, 1.0, 1.0) );
	Gx += sobel_kernel[i,j]*depth;
	Gy += sobel_kernel[j,i]*depth; 
      }
    } 
    float G = sqrt( Gx*Gx + Gy*Gy );
    //out_Color = vec4( Gx, Gx, Gx, 1.0 );
    //out_Color = vec4( Gy, Gy, Gy, 1.0 );
    if ( G > 0.05 ) 
      out_Color = vec4( 0.0, 0.0, 0.0, 1.0 );
    
      
     
    return;
}

