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


/////////////////////////////////////
// Uniforms, Attributes, and Outputs
////////////////////////////////////
uniform sampler2D u_Posttex;
//uniform sampler2D u_Normaltex;
uniform sampler2D u_RandomNormaltex;
uniform sampler2D u_RandomScalartex;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

uniform int u_DisplayType;

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

// //Helper function to automatically sample and unpack normals
// vec3 sampleNrm(vec2 texcoords) {
//     return texture(u_Normaltex,texcoords).xyz;
// }

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

                //vec3 c = (sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t + my)));     
                float c = length(sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t + my)));     

                magX += GX[j][i] * c;
                magY += GY[j][i] * c;
            }
        }
        float mag = sqrt((magX)*(magX) + (magY)*(magY));
        mag = clamp(mag, 0.0, 1.0);
        out_Color = vec4(color * mag + color, 1.0);
    }
    else
    {
        vec3 c = vec3(0.0);
        for(int i = -10; i < 10; i++)
        {
            for(int j = -10; j < 10; j++)
            {
                float mx = float(i) / u_ScreenWidth;
                float my = float(j) / u_ScreenHeight;
                //if(i!=0 && j != 0)
                  //  c += sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t + my)) / (abs(i)*abs(j));
                //else
                    c += sampleCol(vec2(fs_Texcoord.s + mx, fs_Texcoord.t+my));
            }
        }

        // for(int i = -20; i < 20; i++)
        // {          
        //         float my = float(i) / u_ScreenHeight;
        //         c += sampleCol(vec2(fs_Texcoord.s, fs_Texcoord.t + my));           
        // }



        c /= 400.0;

        float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
        float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
        out_Color = vec4(c , 1.0);
    }
    //out_Color = vec4(color, 1.0);

    //vec3 normal = sampleNrm(fs_Texcoord);

    /*vec3 color2 = sampleCol(vec2(fs_Texcoord.s + 1.0/u_ScreenWidth, fs_Texcoord.t + 1.0/u_ScreenHeight));
    vec3 color3 = sampleCol(vec2(fs_Texcoord.s - 1.0/u_ScreenWidth, fs_Texcoord.t - 1.0/u_ScreenHeight));
    vec3 color4 = sampleCol(vec2(fs_Texcoord.s - 1.0/u_ScreenWidth, fs_Texcoord.t + 1.0/u_ScreenHeight));
    vec3 color5 = sampleCol(vec2(fs_Texcoord.s + 1.0/u_ScreenWidth, fs_Texcoord.t - 1.0/u_ScreenHeight));

    color = (color + color2 + color3 + color4 + color5) / 5.0;*/

    //out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
    //out_Color = vec4(vec3(color), 1.0);

    return;
}

