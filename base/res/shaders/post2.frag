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
uniform sampler2D u_Posttex;
uniform sampler2D u_Speculartex;
uniform sampler2D u_Positiontex;
uniform sampler2D u_Normaltex;
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
//              UTILITY FUNCTIONS
/////////////////////////////////////

//Helper function to automicatlly sample and unpack positions
vec3 sampleCol(vec2 texcoords) {
    return texture(u_Posttex,texcoords).xyz;
}

//Helper function to automatically sample and unpack normals
vec3 sampleNrm(vec2 texcoords) {
    return texture(u_Normaltex,texcoords).xyz;
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

    vec3 normal = sampleNrm(fs_Texcoord);

    
    if(u_DisplayType == DISPLAY_BLOOM)
    {        
        if(specular.w == 101)
        {         
            vec3 c = vec3(0.0);
            int range = 20;
            int count = 0;
            for(int i = -range; i < range; i++)
            {            
                float sd = 6.0;
                float my = float(i) / u_ScreenHeight;
                vec4 nearspe = sampleSpec(vec2(fs_Texcoord.s, fs_Texcoord.t+my)); 
                //if(nearspe.w == 101){
                    c += sampleCol(vec2(fs_Texcoord.s, fs_Texcoord.t+my))  * (1.0 / (2.0 * 3.141516 * sd * sd) * exp(- (i*i)/(2.0*sd*sd))) * 1000.0;       
                //    count++;
                //}
                //else
                //    c += color  * (1.0 / (2.0 * 3.141516 * sd * sd) * exp(- (i*i)/(2.0*sd*sd))) * 1000.0;       
            }
            c /= 2.0 * range;
            out_Color = vec4(c , 1.0);   
        }         
        else
        {
            out_Color = vec4(color , 1.0);                
        }
    }
    else if(u_DisplayType == DISPLAY_OCCLUSION)
    {
         int range = 2;
         vec3 c = vec3(0.0);
        for(int i = -range; i < range; i++)
        {  
             for(int j = -range; j < range; j++)
            {  
                float mx = float(i) / u_ScreenWidth;
                float my = float(j) / u_ScreenHeight;
                c += sampleCol(vec2(fs_Texcoord.s+mx, fs_Texcoord.t+my));
            }
        }
        range *= 2;
        c /= range * range;
        out_Color = vec4(c , 1.0);  
    }
    else{
        float gray = dot(color, vec3(0.2125, 0.7154, 0.0721));
        float vin = min(2*distance(vec2(0.5), fs_Texcoord), 1.0);
        out_Color = vec4(color , 1.0);
    }
    
   
    //out_Color = vec4(mix(pow(color,vec3(1.0/1.8)),vec3(gray),vin), 1.0);
    //out_Color = vec4(vec3(color), 1.0);

    return;
}

