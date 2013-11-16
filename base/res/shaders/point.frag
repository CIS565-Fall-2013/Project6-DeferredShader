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
#define DISPLAY_BLOOM 6
#define DISPLAY_TOON 7

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
uniform sampler2D u_Bloomtex;

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
    vec3 light = u_Light.xyz; // light position
    float lightRadius = u_Light.w;
    vec3 light_dir = normalize(position - light);
    float light_dist = distance(position, light);
    //out_Color = vec4(0.0, 0.0, 0.0, 1.0);

    if( u_DisplayType == DISPLAY_LIGHTS )
    {
        //Visualize the fragment associated with this point light
        float diffuse = max(dot(normal, -light_dir), 0.0);
        out_Color     = vec4(vec3(0.4, 0.2, 0.8) * diffuse, 1.0);
    }
    else
    {
        //Put some code here to actually compute the light from the point light, using light attenuation formula from http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
        // Attenuation
        float d     = max(light_dist - lightRadius, 0.0);
        float ratio = light_dist / lightRadius;
        float denom = ratio + 1.0;
        float attenuation = 1.7 / (denom * denom) * max(1.0 - d, 0.0);

        // Diffuse with attenuation
        float diffuse = max(dot(normal, -light_dir), 0.0);
        out_Color = vec4(color * diffuse * attenuation, 1.0);

        if (u_DisplayType == DISPLAY_TOON) {
            if (light_dist < lightRadius) {
                if (diffuse >= 0.95)
                    diffuse = 1.0;
                else if (diffuse >= 0.9)
                    diffuse = 0.8;
                else if (diffuse >= 0.75)
                    diffuse = 0.75;
                else if (diffuse >= 0.5)
                    diffuse = 0.5;
                else
                    diffuse = 0.3;
                out_Color = vec4(color * diffuse, 1.0);
            
            } else
                out_Color = vec4(0.0, 0.0, 0.0, 1.0);
            
            // Draw the edges by finding normals of neighbours
            vec3 normal_up    = sampleNrm(fs_Texcoord + vec2(0.0, - 3.0 / float(u_ScreenHeight)));
            vec3 normal_right = sampleNrm(fs_Texcoord + vec2(3.0 / float(u_ScreenWidth),0.0)); 
            vec3 normal_down  = sampleNrm(fs_Texcoord + vec2(0.0, 3.0 / float(u_ScreenHeight))); 
            vec3 normal_left  = sampleNrm(fs_Texcoord + vec2(- 3.0 / float(u_ScreenWidth), 0.0));   

            if (dot(normal_up, normal) < 0.8 || dot(normal_up, normal) < 0.8 || dot(normal_up, normal) < 0.8 || dot(normal_up, normal) < 0.8)
                out_Color = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    return;
}
