#version 330

////////////////////////////
//       ENUMERATIONS
////////////////////////////

#define DISPLAY_DEPTH 0
#define DISPLAY_NORMAL 1
#define DISPLAY_POSITION 2
#define DISPLAY_COLOR 3
#define DISPLAY_TOTAL 4
#define DISPLAY_LIGHTS 5
#define DISPLAY_TONE 6
#define DISPLAY_BLOOM 7
#define DISPLAY_OCCLUSION 8


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
uniform sampler2D u_Speculartex;

uniform float u_Far;
uniform float u_Near;
uniform int u_OcclusionType;
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

    float exp_depth = texture(u_Depthtex, fs_Texcoord).r;
    float lin_depth = linearizeDepth(exp_depth,u_Near,u_Far);

    vec3 normal = sampleNrm(fs_Texcoord);
    vec3 position = samplePos(fs_Texcoord);
    vec3 color = sampleCol(fs_Texcoord);
    vec3 light = u_Light.xyz;
    float strength = u_Light.w;
    if (lin_depth > 0.99f) {
        out_Color = vec4(vec3(0.0), 1.0);
    } else{
        float ambient = u_LightIl;
        float diffuse = max(0.0, dot(normalize(light),normal));
        out_Color = vec4(color*(strength*diffuse + ambient),1.0f);


        if(u_DisplayType == DISPLAY_TONE){
            //Sober filter
            mat3 GX, GY;
            GX[0] = vec3(1.0,2.0,1.0);
            GX[1] = vec3(0.0,0.0,0.0);
            GX[2] = vec3(-1.0,-2.0,-1.0);

            GY[0] = vec3(1.0,0.0,-1.0);
            GY[1] = vec3(2.0,0.0,-2.0);
            GY[2] = vec3(1.0,0.0,-1.0);

            float magX = 0.0;
            float magY = 0.0;

            for(int i = 0;i < 3; i++){
                for(int j = 0; j < 3; j++){
                    float mx = (j-1.0) / u_ScreenWidth;
                    float my = (i-1.0) / u_ScreenHeight;
                          
                    float c = length(sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t + my)));     

                    magX += GX[j][i] * c;
                    magY += GY[j][i] * c;
                }
            }
            float mag = sqrt((magX)*(magX) + (magY)*(magY));
            mag = clamp(mag, 0.0, 1.0);
            out_Color = vec4(vec3(out_Color * (mag) * 5.0 + out_Color), 1.0);
            //out_Color = vec4(color, 1.0);
        }



    }	
    return;
}

