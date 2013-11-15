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
uniform mat4 u_Persp;

uniform sampler2D u_Depthtex;
uniform sampler2D u_Normaltex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Colortex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform float u_Far;
uniform float u_Near;
uniform int u_OcclusionType;
uniform int u_DisplayType;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_UseToon;

uniform vec4 u_Light;
uniform float u_LightIl;
uniform vec3 u_viewDir; //view direction for contours

uniform mat4x4 u_Model;
uniform mat4x4 u_View;

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
    return texture(u_Normaltex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Colortex,texcoords).xyz;
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
    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;
    if (lin_depth > 0.99f) {
        out_Color = vec4(vec3(0.0), 1.0);
    } else {
        float ambient = u_LightIl;
        float diffuse = max(0.0, dot(normalize(light),normal));
        //based on tutorial at http://www.lighthouse3d.com/tutorials/glsl-tutorial/toon-shading-version-iii/
        vec3 toonColor;
        if( diffuse > 0.95 ){
            toonColor = color;
        } else if( diffuse > 0.5){
            toonColor = 0.6 * color;
        } else if(diffuse > 0.25){
            toonColor = 0.4 * color;
        } else {
            toonColor = 0.2 * color;
        }
        vec4 final_color = vec4(0.0);
        if(u_UseToon == 1){
            final_color = vec4(toonColor*(strength + ambient),1.0f);
        } else { //use phong
            final_color = vec4(diffuse*color*(strength + ambient),1.0f);
        }
        /*vec4 cs_normal = u_View * u_Model * vec4(normal,1);//normal in camera space*/
        /*vec3 cam_view_dir = vec3(0, 0, -1);*/
        /*float norm_dot_dir = max(0.0, dot(normalize(cam_view_dir), -normalize(normal)));*/

        float ss_x = fs_Texcoord.x * u_ScreenWidth;
        float ss_y = fs_Texcoord.y * u_ScreenHeight;
        //apply Sobel filter to find edges
        mat3 xFilt = transpose(mat3(-1, 0, 1, 
                                    -2, 0, 2,
                                    -1, 0, 1));
        mat3 yFilt = transpose(mat3(-1,-2,-1, 
                                    0, 0, 0,
                                    1, 2, 1));

        vec3 GxRGB = convolveFlipped(xFilt, ss_x, ss_y);
        vec3 GyRGB = convolveFlipped(yFilt, ss_x, ss_y);
        //find average gradient in X and Y as a scalar, based on average of gradient in red, green, blue channels
        float GxAvg = (1.0/3.0) * (GxRGB.r + GxRGB.g + GxRGB.b);
        float GyAvg = (1.0/3.0) * (GyRGB.r + GyRGB.g + GyRGB.b);
        float G = sqrt( GxAvg*GxAvg + GyAvg*GyAvg ); //magnitude of gradient

        float threshold = 0.1;

        if( G > threshold ){ //we are on an edge
            out_Color = vec4(1, 1, 1, 1);
        } else { //not an edge
            out_Color = final_color;
        }
        /*out_Color = vec4(norm_dot_dir, norm_dot_dir, norm_dot_dir, 1.0f);*/
        /*if( norm_dot_dir > 0.3) {*/
            /*out_Color = vec4(toonColor*(strength + ambient),1.0f);*/
        /*} else {*/
            /*out_Color = vec4(1.0f, 1.0f, 1.0f, 1.0f); */
        /*}*/
        /*out_Color = vec4(color*(strength*diffuse + ambient),1.0f);*/
        /*out_Color = vec4(toonColor*(strength + ambient),1.0f);*/
    }	
    return;
}

