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
    float lightRadius = u_Light.w;
    //out_Color = vec4(0,0,0,1.0);

    //vec4 posWorld =  u_Persp * vec4(position, 1.0);
    //vec4 lightWorld =  u_Persp * vec4(light, 1.0);

    if( u_DisplayType == DISPLAY_LIGHTS )
    {
        //out_Color = vec4(0,0,0,1.0);
        float diffuse = max(dot(normal, normalize(light - position)),0.0);
        out_Color = vec4(diffuse * vec3(0.2,0.2,0.8), 0.2);
        //Put some code here to visualize the fragment associated with this point light
    }
    else
    {
        if(length(position - light) <= lightRadius){
            float diffuse = max(dot(normal, normalize(light - position)),0.0);

            //For sepcular color
            vec4 specular = sampleSpec(fs_Texcoord);
            vec3 viewDirection = normalize(position - vec3(0.0));
            vec3 lightDirecion = normalize(position - light);
            vec3 specularColor;
             if (dot(normal, lightDirecion) < 0.0)
            {
                //angle betweeen light dir and normal is too large
                specularColor = vec3(0.0);
            }
            else{
                specularColor = vec3(specular.xyz) * pow(max(0.0, dot(reflect(lightDirecion, normal), viewDirection)), specular.w);
            }

            specularColor = vec3(0.0);
            out_Color = vec4(color * diffuse + specularColor, 1.0);

            //Tone shading
            if(u_DisplayType == DISPLAY_TONE){
                if(diffuse > 0.85)
                    out_Color = vec4(1.0 * color, 1.0);
                else if(diffuse > 0.65)
                    out_Color = vec4(0.7 * color, 1.0);
                else if(diffuse > 0.45)
                    out_Color = vec4(0.5 * color, 1.0);
                else if(diffuse > 0.0)
                    out_Color = vec4(0.2 * color, 1.0);
            }
        }
        //Put some code here to actually compute the light from the point light
    }
    return;
}

