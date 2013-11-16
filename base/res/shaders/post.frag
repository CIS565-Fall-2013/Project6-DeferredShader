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
#define DISPLAY_TONE 6
#define DISPLAY_BLOOM 7


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
uniform sampler2D u_Speculartex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;


uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

in vec2 fs_Texcoord;

out vec4 out_Color;

uniform vec4 u_Light;
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

//Helper function to automicatlly sample and unpack positions
vec3 samplePos(vec2 texcoords) {
    return texture(u_Positiontex,texcoords).xyz;
}

//Helper function to automicatlly sample and unpack specular
vec4 sampleSpec(vec2 texcoords) {
    return texture(u_Speculartex,texcoords).xyzw;
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
    vec4 specular = sampleSpec(fs_Texcoord); 
    vec3 position = samplePos(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float lightRadius = u_Light.w; 

    if(u_DisplayType == DISPLAY_BLOOM){
        //if(length(position - light) <= lightRadius){
        if(specular.w == 101){
            vec3 c = vec3(0.0);
            int range = 20;
            int count = 0;
            for(int i = -range; i < range; i++)
            {
                float sd = 6.0;
                float mx = float(i) / u_ScreenWidth;
                vec4 nearspe = sampleSpec(vec2(fs_Texcoord.s + mx, fs_Texcoord.t)); 
                //if(nearspe.w == 101){
                    c += sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t)) * ((1.0 / (2.0 * 3.141516 * sd * sd) * exp(-(i*i)/(2.0*sd*sd)))) * 1000.0;
                    
               // }
               // else
                ///    c += color * ((1.0 / (2.0 * 3.141516 * sd * sd) * exp(-(i*i)/(2.0*sd*sd)))) * 1000.0;
            }
            c /= 2.0 * range;
            out_Color = vec4(c , 1.0);
        }
        else
            out_Color = vec4(color , 1.0); 
    }
    else{
        out_Color = vec4(color , 1.0);        
    }
    return;
}


    