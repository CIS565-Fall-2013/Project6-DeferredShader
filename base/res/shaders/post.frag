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
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

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

//convert from pixel coordinates to NDC (between 0 and 1) 
vec2 pixToNDC(float pix_x, float pix_y){ 
    return vec2( pix_x / u_ScreenWidth, pix_y / u_ScreenHeight);    
}

//assume the kernel is already FLIPPED
vec3 convolveFlipped(mat3 kernel, float pix_x, float pix_y) {
    //color 00 is the color at (0,0) of the kernel centered at (pix_x, pix_y)
    //color xy is the color at (x, y) of the kernel centered at (pix_x, pix_y)
    vec3 color_00 =  sampleCol(pixToNDC(pix_x-1,pix_y-1));
    vec3 color_10 =  sampleCol(pixToNDC(pix_x,pix_y-1));
    vec3 color_20 =  sampleCol(pixToNDC(pix_x+1,pix_y-1));

    vec3 color_01 =  sampleCol(pixToNDC(pix_x-1,pix_y));
    vec3 color_11 =  sampleCol(pixToNDC(pix_x,pix_y));
    vec3 color_21 =  sampleCol(pixToNDC(pix_x+1,pix_y));

    vec3 color_02 =  sampleCol(pixToNDC(pix_x-1,pix_y+1));
    vec3 color_12 =  sampleCol(pixToNDC(pix_x,pix_y+1));
    vec3 color_22 =  sampleCol(pixToNDC(pix_x+1,pix_y+1));

    //ASSUMING THE KERNEL IS ALREAYD FLIPPED! 
    vec3 finalColor = kernel[0][0]*color_00 + kernel[1][0]*color_10 + kernel[2][0]*color_20 + kernel[0][1]*color_01 + kernel[1][1]*color_11 + kernel[2][1]*color_21 + kernel[0][2]*color_02 + kernel[1][2]*color_12 + kernel[2][2]*color_22; 

    return finalColor;
}

///////////////////////////////////
// MAIN
//////////////////////////////////
const float occlusion_strength = 1.5f;
void main() {
    vec3 color = sampleCol(fs_Texcoord);
    float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
    float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
    /*out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);*/
    /*out_Color = vec4(color,1);*/

    float ss_x = fs_Texcoord.x * u_ScreenWidth;
    float ss_y = u_ScreenHeight - fs_Texcoord.y * u_ScreenHeight;

    //matrices are stored COLUMN MAJOR
    mat3 basicBlur = mat3(1.0/9.0, 1.0/9.0, 1.0/9.0,
                          1.0/9.0, 1.0/9.0, 1.0/9.0,
                          1.0/9.0, 1.0/9.0, 1.0/9.0);

    vec3 convResult = convolveFlipped(basicBlur, ss_x, ss_y);
    out_Color = vec4(convResult, 1);
    /*out_Color = vec4(color,1);*/
    
    /*if(ss_x > u_ScreenWidth/2 && ss_y > u_ScreenHeight/2){*/
        /*out_Color = vec4(fs_Texcoord.x, fs_Texcoord.y, 1, 0);*/
    /*} else {*/
        /*out_Color = vec4(1, 0, 0, 0);*/
    /*}*/

    return;
}

